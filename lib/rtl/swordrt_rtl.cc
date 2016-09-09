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
	//InitializeBloomFilterParameters();

//	filters.insert({ UNSAFE_READ , new bloom_filter(parameters) });
//	filters.insert({ UNSAFE_WRITE, new bloom_filter(parameters) });
//	filters.insert({ ATOMIC_READ , new bloom_filter(parameters) });
//	filters.insert({ ATOMIC_WRITE, new bloom_filter(parameters) });
//	filters.insert({ MUTEX_READ  , new bloom_filter(parameters) });
//	filters.insert({ MUTEX_WRITE , new bloom_filter(parameters) });

	filters.insert({ UNSAFE_READ , new CountMinSketch(0.01, 0.1) });
	filters.insert({ UNSAFE_WRITE, new CountMinSketch(0.01, 0.1) });
	filters.insert({ ATOMIC_READ , new CountMinSketch(0.01, 0.1) });
	filters.insert({ ATOMIC_WRITE, new CountMinSketch(0.01, 0.1) });
	filters.insert({ MUTEX_READ  , new CountMinSketch(0.01, 0.1) });
	filters.insert({ MUTEX_WRITE , new CountMinSketch(0.01, 0.1) });

	// c = new CountMinSketch(0.01, 0.01);
}

SwordRT::~SwordRT() {
	// filters.clear();
}

void SwordRT::InitializeBloomFilterParameters() {
	// Initialize Bloom Filters
	// This number should be roughly the number of different addresses
	// that a parallel region access during its execution, how do we
	// find it out?
	parameters.projected_element_count = 100000;

	// What maximum tolerable false positive probability? (0,1)
	parameters.false_positive_probability = 0.001; // 1 in 1000

	// Simple randomizer (optional) - Try different seeds?
	parameters.random_seed = 0xA5A5A5A5;

	if (!parameters) {
		std::cerr << "Error - Invalid set of bloom filter parameters!" << std::endl;
		exit(1);
	}

	parameters.compute_optimal_parameters();
}

bool ALWAYS_INLINE SwordRT::Contains(uint64_t access, std::string filter_type) {
	// return filters[filter_type]->contains(access);
	int tid = ompt_get_thread_id();
	char str[16];
	sprintf(str, "%lx", access);
	mtx.lock();
	bool res = ((filters[filter_type]->estimate(str) != -1) && (filters[filter_type]->estimate(str) != tid)); // && !is_stack((void *) access));
    mtx.unlock();
	return (res);
}

void ALWAYS_INLINE SwordRT::Insert(uint64_t access, std::string filter_type) {
	// filters[filter_type]->insert(access);
	int tid = ompt_get_thread_id();
	char str[16];
	sprintf(str, "%lx", access);
	mtx.lock();
	filters[filter_type]->update(str, tid);
	mtx.unlock();
}

bool ALWAYS_INLINE SwordRT::ContainsAndInsert(uint64_t access, std::string filter_type) {
//	bool res = filters[filter_type]->contains(access);
//	filters[filter_type]->insert(access);
//	return res;
}

inline void SwordRT::ReportRace(uint64_t access, uint64_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	// Will call a class that executes llvm-symbolizer at the end of each parallel region,
	// here we just keep filling up the file that holds all the executable/addresses
	// We also put the address of a parallel region so we know the parallel region where the
	// access belongs to
	mtx.lock();
	std::cerr << "There was a race for thread " << std::dec << ompt_get_thread_id() << " at [" << std::hex << access << "][" << (void *) pc << "][" << nutex_name << "][" << FilterType[access_type] << "]" << std::endl;
	mtx.unlock();
}

bloom_parameters SwordRT::getParameters() {
	return parameters;
}

void SwordRT::clear() {
	for (auto& f: filters) {
		f.second->clear();
	}
}

void ALWAYS_INLINE SwordRT::CheckMemoryAccess(uint64_t access, uint64_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	bool conflict = false;
	AccessType conflict_type = none;
	std::string nutex;

	if(is_stack((void *) access))
		return;

	// printf("AccessType: %s[0x%lx]\n", FilterType[access_type], access);
	// printf("[0x%lx]%d\n", access, omp_get_thread_num());
	// std::cout << "[" << std::hex << access << "]" << omp_get_thread_num() << std::endl;

	switch(access_type) {
	case unsafe_read:
		if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case unsafe_write:
		if(swordRT->Contains(access, UNSAFE_READ)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_READ)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, MUTEX_READ)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_read:
		if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_write:
		if(swordRT->Contains(access, UNSAFE_READ)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, MUTEX_READ)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case mutex_read:
		if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		break;
	case mutex_write:
		if(swordRT->Contains(access, UNSAFE_READ)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_READ)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
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

	// Trying to manage the thread races with itself
	//	if(!tls_filter)
	//		tls_filter = new bloom_filter(swordRT->parameters);
	//	if(tls_filter->contains(access)) {
	//		conflict = false;
	//	}
	//
	//	tls_filter->insert(access);

//	int tid = ompt_get_thread_id();
//	char str[8];
//	sprintf(str, "%lx", access);
//	if(conflict && (c->estimate(str) == tid))
//		conflict = false;

	swordRT->Insert(access, FilterType[access_type]);
	// c->update(str, tid);

	if(conflict)
		ReportRace(access, pc, access_size, conflict_type, nutex_name);
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
