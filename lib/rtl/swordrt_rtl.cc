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
		binAccesses.clear();
	}
#else
	if(threadInfo[tid - 1].__swordomp_status__ == 0) {
		binAccesses.clear();
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
	//	 for (std::unordered_map<uint64_t, AccessInfo>::iterator it = binAccesses.begin(); it != binAccesses.end(); ++it)
	//		 printf("%lu-%lu-0x%lx-%lu-%u\n", tid, it->first, it->second.address, it->second.count, it->second.type);
}

static void on_ompt_event_barrier_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id) {

}

static void ompt_initialize_fn(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {

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
