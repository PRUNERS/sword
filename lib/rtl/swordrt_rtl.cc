//===-- swordrt_rtl.cc -------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Archer/SwordRT, an OpenMP race detector.
//
// Main file (entry points) for the TSan run-time.
//===----------------------------------------------------------------------===//

#include "swordrt_rtl.h"

SwordRT *swordRT;

// Class SwordRT

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>

int is_stack(void *ptr)
{
	pthread_t self = pthread_self();
	pthread_attr_t attr;
	void *stack;
	size_t stacksize;
	pthread_getattr_np(self, &attr);
	pthread_attr_getstack(&attr, &stack, &stacksize);
	return ((uintptr_t) ptr >= (uintptr_t) stack
			&& (uintptr_t) ptr < (uintptr_t) stack + stacksize);
}

SwordRT::SwordRT() {
//	filters.insert({ UNSAFE_READ , new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3> });
//	filters.insert({ UNSAFE_WRITE, new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3> });
//	filters.insert({ ATOMIC_READ , new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3> });
//	filters.insert({ ATOMIC_WRITE, new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3> });
//	filters.insert({ MUTEX_READ  , new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3> });
//	filters.insert({ MUTEX_WRITE , new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3> });
	filters.insert({ UNSAFE_READ , NULL });
	filters.insert({ UNSAFE_WRITE, NULL });
	filters.insert({ ATOMIC_READ , NULL });
	filters.insert({ ATOMIC_WRITE, NULL });
	filters.insert({ MUTEX_READ  , NULL });
	filters.insert({ MUTEX_WRITE , NULL });
}

SwordRT::~SwordRT() {
	// filters.clear();
}

bool ALWAYS_INLINE SwordRT::Contains(size_t access, AccessType access_type, int tid) {
	if(!filters[FilterType[access_type]])
		filters[FilterType[access_type]] = new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3>;
	//mtx.lock();
	bool res = filters[FilterType[access_type]]->contains(access, tid); // ((filters[filter_type]->lookup(access) != 0) && (filters[filter_type]->lookup(access) != tid))
	//mtx.unlock();
	return (res);
}

void ALWAYS_INLINE SwordRT::Insert(size_t access, AccessType access_type, int tid) {
	//mtx.lock();
	if(!filters[FilterType[access_type]])
		filters[FilterType[access_type]] = new boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3>;
	filters[FilterType[access_type]]->insert(access, tid);
	//mtx.unlock();
}

inline void SwordRT::ReportRace(size_t access, size_t pc, int tid, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	// Will call a class that executes llvm-symbolizer at the end of each parallel region,
	// here we just keep filling up the file that holds all the executable/addresses
	// We also put the address of a parallel region so we know the parallel region where the
	// access belongs to
	// mtx.lock();
	reported_races.insert(pc);
	std::cerr << "There was a race for thread " << std::dec << tid << " at [" << std::hex << access << "][" << (void *) pc << "][" << FilterType[access_type] << "]" << std::endl;
	// mtx.unlock();
}

void SwordRT::clear() {
//	for (auto& f: filters) {
//		f.second->clear();
//	}
	filters.clear();
	filters.insert({ UNSAFE_READ , NULL });
	filters.insert({ UNSAFE_WRITE, NULL });
	filters.insert({ ATOMIC_READ , NULL });
	filters.insert({ ATOMIC_WRITE, NULL });
	filters.insert({ MUTEX_READ  , NULL });
	filters.insert({ MUTEX_WRITE , NULL });
}

void ALWAYS_INLINE SwordRT::CheckMemoryAccess(size_t access, size_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	bool conflict = false;
	AccessType conflict_type = none;
	std::string nutex;

	//mtx.lock();
	if(reported_races.probably_contains(pc)) {
		//mtx.unlock();
		return;
	}
	//mtx.unlock();

	if(is_stack((void *) access))
		return;

	int tid = ompt_get_thread_id();

	// printf("AccessType: %s[0x%lx]\n", FilterType[access_type], access);
	// printf("[0x%lx]%d\n", access, omp_get_thread_num());
	// std::cout << "[" << std::hex << access << "]" << omp_get_thread_num() << std::endl;

	switch(access_type) {
	case unsafe_read:
		if(swordRT->Contains(access, unsafe_write, tid)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, atomic_write, tid)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, mutex_write, tid)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case unsafe_write:
		if(swordRT->Contains(access, unsafe_read, tid)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, unsafe_write, tid)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, atomic_read, tid)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, atomic_write, tid)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, mutex_read, tid)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, mutex_write, tid)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_read:
		if(swordRT->Contains(access, unsafe_write, tid)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, mutex_write, tid)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_write:
		if(swordRT->Contains(access, unsafe_read, tid)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, unsafe_write, tid)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, mutex_read, tid)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, mutex_write, tid)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case mutex_read:
		if(swordRT->Contains(access, unsafe_write, tid)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, atomic_write, tid)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		break;
	case mutex_write:
		if(swordRT->Contains(access, unsafe_read, tid)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, unsafe_write, tid)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, atomic_read, tid)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, atomic_write, tid)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		break;
	case nutex_read:
		break;
	case nutex_write:
		break;
	default:
		return;
	}

	swordRT->Insert(access, access_type, tid);

	if(conflict)
		ReportRace(access, pc, tid, access_size, conflict_type, nutex_name);
	//	else {
	//		mtx.lock();
	//		std::cerr << "No race" << ompt_get_thread_id() << " at [" << std::hex << access << "][" << (void *) pc << "][" << nutex_name << "][" << FilterType[access_type] << "]" << std::endl;
	//		mtx.unlock();
	//	}
}

// Class SwordRT

extern "C" {

#include "sword_interface.inl"

static void on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_id_t parallel_id,
		uint32_t requested_team_size,
		void *parallel_function,
		ompt_invoker_t invoker) {
}

static void on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id,
		ompt_invoker_t invoker) {

	if(__swordomp_status__ == 0) {
		swordRT->clear();
	}
}

static void on_acquired_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__ = true;
}

static void on_release_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__ = false;
}

static void ompt_initialize_fn(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {
	swordRT = new SwordRT();

	if(!swordRT) {
		std::cerr << "Error initializing the runtime!" << std::endl;
		exit(1);
	}

	printf("OMPT Initialization...\n");
	printf("runtime_version: %s, ompt_version: %i\n", runtime_version, ompt_version);

	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	ompt_get_thread_id = (ompt_get_thread_id_t) lookup("ompt_get_thread_id");

	ompt_set_callback(ompt_event_parallel_begin,
			(ompt_callback_t) &on_ompt_event_parallel_begin);
	ompt_set_callback(ompt_event_parallel_end,
			(ompt_callback_t) &on_ompt_event_parallel_end);
	// ompt_set_callback(ompt_event_barrier_begin,
	//                   (ompt_callback_t) &on_ompt_event_barrier_begin);
	// ompt_set_callback(ompt_event_barrier_end,
	//                   (ompt_callback_t) &on_ompt_event_barrier_end);
	ompt_set_callback(ompt_event_acquired_critical,
			(ompt_callback_t) &on_acquired_critical);
	ompt_set_callback(ompt_event_release_critical,
			(ompt_callback_t) &on_release_critical);
}

ompt_initialize_t ompt_tool(void) { return ompt_initialize_fn; }

}
