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

#define GET_STACK	 										\
		pthread_t self = pthread_self(); 					\
		pthread_attr_t attr;								\
		pthread_getattr_np(self, &attr);					\
		pthread_attr_getstack(&attr, &stack, &stacksize);	\
		pthread_attr_destroy(&attr);

SwordRT::SwordRT() {}

SwordRT::~SwordRT() {}

bool ALWAYS_INLINE SwordRT::Contains(size_t access, uint64_t filter) {
	filterMtx.lock();
	if(!filters[filter])
		filters[filter] = new boost::bloom_filters::counting_bloom_filter<size_t, NUM_OF_ITEMS, TID_NUM_OF_BITS, hash_function>;
	bool res = filters[filter]->contains(access, tid);
	filterMtx.unlock();
	return (res);
}

void ALWAYS_INLINE SwordRT::Insert(size_t access, uint64_t tid, uint64_t filter) {
	filterMtx.lock();
	if(!filters[filter])
		filters[filter] = new boost::bloom_filters::counting_bloom_filter<size_t, NUM_OF_ITEMS, TID_NUM_OF_BITS, hash_function>;
	filters[filter]->insert(access, tid);
	filterMtx.unlock();
}

inline void SwordRT::ReportRace(size_t access, size_t pc, uint64_t tid, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	// Will call a class that executes llvm-symbolizer at the end of each parallel region,
	// here we just keep filling up the file that holds all the executable/addresses
	// We also put the address of a parallel region so we know the parallel region where the
	// access belongs to
	raceMtx.lock();
	reported_races.insert(pc);
	raceMtx.unlock();
	DEBUG(std::cerr, "There was a race for thread " << std::dec << tid << " at [" << std::hex << access << "][" << (void *) pc << "]");
}

void SwordRT::clear(uint64_t parallel_id) {
	for(unsigned i = unsafe_read; i <= mutex_write; i++)
		filters.erase(parallel_id + i);
}

void ALWAYS_INLINE SwordRT::AddMemoryAccess(uint64_t tid, void *stack, uint64_t stacksize, size_t access, size_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	Access new_access;
	new_access.valid = true;
	new_access.tid = tid;
	new_access.parallel_id = outer_parallel_id;
	new_access.stack = stack;
	new_access.stacksize = stacksize;
	new_access.access = access;
	new_access.pc = pc;
	new_access.access_size = access_size;
	new_access.access_type = access_type;
	new_access.nutex_name = nutex_name;
	access_queues[tid - 1]->unsynchronized_push(new_access);
}

void ALWAYS_INLINE SwordRT::CheckMemoryAccess(uint64_t tid, uint64_t parallel_id, void *stack, uint64_t stacksize, size_t access, size_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	bool conflict = false;
	AccessType conflict_type = none;
	std::string nutex;

	raceMtx.lock();
	if(reported_races.probably_contains(pc)) {
		raceMtx.unlock();
		return;
	}
	raceMtx.unlock();

	// Return if variable belong to thread stack (is local)
	if((access >= (size_t) stack) &&
			(access < (size_t) stack + stacksize))
		return;

	switch(access_type) {
	case unsafe_read:
		if(swordRT->Contains(access, parallel_id + unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, parallel_id + atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, parallel_id + mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case unsafe_write:
		if(swordRT->Contains(access, parallel_id + unsafe_read)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, parallel_id + unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, parallel_id + atomic_read)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, parallel_id + atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, parallel_id + mutex_read)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, parallel_id + mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_read:
		if(swordRT->Contains(access, parallel_id + unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, parallel_id + mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_write:
		if(swordRT->Contains(access, parallel_id + unsafe_read)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, parallel_id + unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, parallel_id + mutex_read)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, parallel_id + mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case mutex_read:
		if(swordRT->Contains(access, parallel_id + unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, parallel_id + atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		break;
	case mutex_write:
		if(swordRT->Contains(access, parallel_id + unsafe_read)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, parallel_id + unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, parallel_id + atomic_read)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, parallel_id + atomic_write)) {
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

	swordRT->Insert(access, tid, parallel_id + access_type);

	if(conflict)
		ReportRace(access, pc, tid, access_size, conflict_type, nutex_name);
}

// Class SwordRT

extern "C" {

#include "sword_interface1.inl"

void analyze_thread(uint64_t thread_id) {
	uint64_t tid;
	tid = thread_id;
	while(1) {
		if(!access_queues[tid - 1]->empty()) {
			Access access;
			access_queues[tid - 1]->unsynchronized_pop(access);
			if(!access.valid) {
				if(access.parallel_id == UINT64_MAX)
					break;
				else {
					std::atomic_fetch_sub(&num_of_checker_threads, (unsigned) 1);
					if(num_of_checker_threads == 0)
						swordRT->clear(access.parallel_id);
				}
			}
			ASSERT(tid == access.tid);
			swordRT->CheckMemoryAccess(access.tid, access.parallel_id, access.stack, access.stacksize, access.access, access.pc, access.access_size, access.access_type, access.nutex_name);
			usleep(100000);
		}
	}
}

static void on_ompt_event_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_id_t thread_id) {
	// Set thread id
	tid = thread_id;
	// Get stack pointer and stack size
	GET_STACK

	if(access_queues.size() < tid) {
		queueMtx.lock();
		if(access_queues.size() < tid)
			access_queues.resize(access_queues.size() * (numThreads / 2), NULL);
		queueMtx.lock();
	}
	if(threads.size() < tid) {
		threadMtx.lock();
		if(threads.size() < tid)
			threads.resize(access_queues.size());
		threadMtx.unlock();
	}
	access_queues[tid - 1] = new boost::lockfree::queue<Access>;
	threads[tid - 1] = std::thread(analyze_thread, tid);
	// threads[tid - 1].detach();
}

void __swordomp_internal_end_checker_threads( void )
{
	for(size_t i = 0; i < access_queues.size(); i++) {
		Access new_access;
		new_access.valid = false;
		new_access.parallel_id = UINT64_MAX;
		if(access_queues[i])
			access_queues[i]->push(new_access);
	}
}

__attribute__(( destructor )) void __swordomp_internal_end_dtor( void )
{
	for(size_t i = 0; i < threads.size(); i++) {
		if(threads[i].joinable())
			threads[i].join();
	}
}

static void on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_id_t parallel_id,
		uint32_t requested_team_size,
		void *parallel_function) {

	if(__swordomp_status__ == 0) {
		outer_parallel_id = parallel_id;
		swordRT->clear(parallel_id);
	}
}

//static void on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
//		ompt_task_id_t task_id,
//		ompt_invoker_t invoker) {}

static void on_ompt_event_idle_begin(ompt_thread_id_t thread_id) {
	Access new_access;
	new_access.valid = false;
	new_access.tid = tid;
	new_access.parallel_id = outer_parallel_id;
	access_queues[tid - 1]->unsynchronized_push(new_access);
	std::atomic_fetch_add(&num_of_checker_threads, (unsigned) 1);
}

static void on_ompt_event_idle_end(ompt_thread_id_t thread_id) {

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

	numThreads = std::thread::hardware_concurrency();

	DEBUG(std::cout, "Detected " << numThreads << " number of threads (std::thread::hardware_concurrency())");
	DEBUG(std::cout, "Detected " << sysconf(_SC_NPROCESSORS_ONLN) << " number of threads (sysconf(_SC_NPROCESSORS_ONLN))");

	access_queues.resize(numThreads / 2);
	threads.resize(numThreads / 2);
	num_of_checker_threads = 0;

	swordRT = new SwordRT();

	if(!swordRT) {
		std::cerr << "Error initializing the runtime!" << std::endl;
		exit(-1);
	}

	DEBUG(std::cout, "OMPT Initizialization: Runtime Version: " << runtime_version << ", OMPT Version: " << ompt_version);

	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	ompt_get_thread_id = (ompt_get_thread_id_t) lookup("ompt_get_thread_id");

	ompt_set_callback(ompt_event_thread_begin,
			(ompt_callback_t) &on_ompt_event_thread_begin);
	ompt_set_callback(ompt_event_parallel_begin,
			(ompt_callback_t) &on_ompt_event_parallel_begin);
	//	ompt_set_callback(ompt_event_parallel_end,
	//			(ompt_callback_t) &on_ompt_event_parallel_end);
	ompt_set_callback(ompt_event_acquired_critical,
			(ompt_callback_t) &on_acquired_critical);
	ompt_set_callback(ompt_event_release_critical,
			(ompt_callback_t) &on_release_critical);
	ompt_set_callback(ompt_event_idle_begin,
			(ompt_callback_t) &on_ompt_event_idle_begin);
	ompt_set_callback(ompt_event_idle_end,
			(ompt_callback_t) &on_ompt_event_idle_end);
}

ompt_initialize_t ompt_tool(void) {
	return ompt_initialize_fn;
}

}
