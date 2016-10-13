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
//===----------------------------------------------------------------------===//

#include "swordrt_rtl.h"

SwordRT *swordRT;

// Class SwordRT

#if defined(TLS) || defined(NOTLS)
#define GET_STACK	 										\
		pthread_t self = pthread_self(); 					\
		pthread_attr_t attr;								\
		pthread_getattr_np(self, &attr);					\
		pthread_attr_getstack(&attr, (void **) &stack, &stacksize);	\
		pthread_attr_destroy(&attr);
#else
#define GET_STACK	 										\
		pthread_t self = pthread_self(); 					\
		pthread_attr_t attr;								\
		pthread_getattr_np(self, &attr);					\
		pthread_attr_getstack(&attr, (void **) &threadInfo[tid - 1].stack, &threadInfo[tid - 1].stacksize);	\
		pthread_attr_destroy(&attr);
#endif

SwordRT::SwordRT() {}

SwordRT::~SwordRT() {}

inline void SwordRT::ReportRace(size_t access, size_t pc, uint64_t tid, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	// Will call a class that executes llvm-symbolizer at the end of each parallel region,
	// here we just keep filling up the file that holds all the executable/addresses
	// We also put the address of a parallel region so we know the parallel region where the
	// access belongs to
	reported_races.insert(pc);
	DEBUG(std::cout, "RACE[" << std::hex << access << "] - [" << (void *) pc << "]");
	DEBUG(std::cerr, "RACE[" << (void *) pc << "]");
}

void SwordRT::clear() {
	// DEBUG(std::cout, "Reset filters!");
	reported_races.clear();
	CLEAR_FILTER(unsafe_read);
	CLEAR_FILTER(unsafe_write);
	CLEAR_FILTER(mutex_read);
	CLEAR_FILTER(mutex_write);
	CLEAR_FILTER(atomic_read);
	CLEAR_FILTER(atomic_write);
}

void ALWAYS_INLINE SwordRT::CheckMemoryAccess(size_t access, size_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	bool conflict = false;
	AccessType conflict_type = none;

	if(reported_races.probably_contains(pc, hash_value_pc))
		return;

	// Return if variable belong to thread stack (is local)
#if defined(TLS) || defined(NOTLS)
	if((access >= (size_t) stack) &&
			(access < (size_t) stack + stacksize))
		return;
#else
	if((access >= (size_t) threadInfo[tid - 1].stack) &&
			(access < (size_t) threadInfo[tid - 1].stack + threadInfo[tid - 1].stacksize))
		return;
#endif

	switch(access_type) {
	case unsafe_read:
		if(CONTAINS(access, unsafe_write, hash_values)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(CONTAINS_HASH(hash_values, access, atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(CONTAINS_HASH(hash_values, access, mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		if(!conflict)
			INSERT_HASH(hash_values, access, unsafe_read);
		break;
	case unsafe_write:
		if(CONTAINS(access, unsafe_read, hash_values)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(CONTAINS_HASH(hash_values, access, unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(CONTAINS_HASH(hash_values, access, atomic_read)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(CONTAINS_HASH(hash_values, access, atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(CONTAINS_HASH(hash_values, access, mutex_read)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(CONTAINS_HASH(hash_values, access, mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		if(!conflict)
			INSERT_HASH(hash_values, access, unsafe_write);
		break;
	case mutex_read:
		if(CONTAINS(access, unsafe_write, hash_values)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(CONTAINS_HASH(hash_values, access, atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		if(!conflict)
			INSERT_HASH(hash_values, access, mutex_read);
		break;
	case mutex_write:
		if(CONTAINS(access, unsafe_read, hash_values)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(CONTAINS_HASH(hash_values, access, unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(CONTAINS_HASH(hash_values, access, atomic_read)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(CONTAINS_HASH(hash_values, access, atomic_write)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		if(!conflict)
			INSERT_HASH(hash_values, access, mutex_write);
		break;
	case atomic_read:
		if(CONTAINS(access, unsafe_write, hash_values)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(CONTAINS_HASH(hash_values, access, mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		if(!conflict)
			INSERT_HASH(hash_values, access, atomic_read);
		break;
	case atomic_write:
		if(CONTAINS(access, unsafe_read, hash_values)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(CONTAINS_HASH(hash_values, access, unsafe_write)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(CONTAINS_HASH(hash_values, access, mutex_read)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(CONTAINS_HASH(hash_values, access, mutex_write)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		if(!conflict)
			INSERT_HASH(hash_values, access, atomic_write);
		break;
	case nutex_read:
		break;
	case nutex_write:
		break;
	default:
		return;
	}

	if(conflict) {
		// ReportRace(access, pc, tid, access_size, conflict_type, nutex_name);
		reported_races.insert(pc, hash_value_pc);
		// DEBUG(std::cout, "RACE[" << std::hex << access << "] - [" << hash_value_pc[0] << "] - [" << tid << "] - [" << (void *) pc << "]");
		DEBUG(std::cerr, "RACE[" << (void *) pc << "]");
	}
}

// Class SwordRT

extern "C" {

#if defined(TLS) || defined(NOTLS)
#include "swordrt_interface.inl"
#else
#include "swordrt_interface.inl"
#endif

static void on_ompt_event_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_id_t thread_id) {
	// Set thread id
	tid = thread_id;
	// Get stack pointer and stack size
	GET_STACK
}

static void on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_id_t parallel_id,
		uint32_t requested_team_size,
		void *parallel_function) {

#if defined(TLS) || defined(NOTLS)
	if(__swordomp_status__ == 0) {
		swordRT->clear();
		// clear(binAccesses);
		// binAccesses = NULL;
		binAccesses.clear();
	}
#else
	if(threadInfo[tid - 1].__swordomp_status__ == 0) {
		swordRT->clear();
	}
#endif
}

//static void on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
//		ompt_task_id_t task_id,
//		ompt_invoker_t invoker) {}

static void on_acquired_critical(ompt_wait_id_t wait_id) {
#if defined(TLS) || defined(NOTLS)
	__swordomp_is_critical__ = true;
#else
	threadInfo[tid - 1].__swordomp_is_critical__ = true;
#endif
}

static void on_release_critical(ompt_wait_id_t wait_id) {
#if defined(TLS) || defined(NOTLS)
	__swordomp_is_critical__ = false;
#else
	threadInfo[tid - 1].__swordomp_is_critical__ = false;
#endif
}

static void on_ompt_event_barrier_begin(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id) {
	// in_order_traversal(binAccesses);
	//	 for (std::unordered_map<uint64_t, node>::iterator it = binAccesses.begin(); it != binAccesses.end(); ++it)
	//		 printf("%lu-%lu-0x%lx-%lu-%u\n", tid, it->first, it->second.address, it->second.count, it->second.type);
}

static void on_ompt_event_barrier_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id) {

}

static void ompt_initialize_fn(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {

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
	ompt_set_callback(ompt_event_barrier_begin,
			(ompt_callback_t) &on_ompt_event_barrier_begin);
	ompt_set_callback(ompt_event_barrier_end,
			(ompt_callback_t) &on_ompt_event_barrier_end);
}

ompt_initialize_t ompt_tool(void) {
	return ompt_initialize_fn;
}

}
