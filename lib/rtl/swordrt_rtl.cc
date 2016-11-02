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
#include <sstream>
#include <cmath>

#if defined(TLS) || defined(NOTLS)
#define GET_STACK	 												\
		pthread_t self = pthread_self(); 							\
		pthread_attr_t attr;										\
		pthread_getattr_np(self, &attr);							\
		pthread_attr_getstack(&attr, (void **) &stack, &stacksize);	\
		pthread_attr_destroy(&attr);
#define DEF_ACCESS												\
		size_t access = (size_t) addr;							\
		size_t pc = CALLERPC;
#define CHECK_STACK												\
		if((access >= (size_t) stack) &&						\
				(access < (size_t) stack + stacksize))			\
				return;
#define SAVE_ACCESS(size, type)																	\
		std::unordered_map<uint64_t, AccessInfo>::iterator item = accesses.find(hash); 			\
		if(item == accesses.end()) {															\
			accesses.insert(std::make_pair(hash, AccessInfo(access, 0, size, type, pc)));		\
		} else {																				\
			if(access < item->second.address) {													\
				item->second.address = access;													\
				item->second.count++;															\
			} else if(((access - item->second.address) / (1 << size)) >= item->second.count) {	\
				item->second.count++;															\
			}																					\
		}
#else
#define GET_STACK	 										\
		pthread_t self = pthread_self(); 					\
		pthread_attr_t attr;								\
		pthread_getattr_np(self, &attr);					\
		pthread_attr_getstack(&attr, (void **) &threadInfo[tid - 1].stack, &threadInfo[tid - 1].stacksize);	\
		pthread_attr_destroy(&attr);
#define DEF_ACCESS												\
		size_t access = (size_t) addr;							\
		size_t pc = CALLERPC;
#define CHECK_STACK																				\
		if((access >= (size_t) threadInfo[tid - 1].stack) &&									\
				(access < (size_t) threadInfo[tid - 1].stack + threadInfo[tid - 1].stacksize))	\
				return;
#define SAVE_ACCESS(size, type)																	\
		std::unordered_map<uint64_t, AccessInfo>::iterator item = accesses.find(hash); 			\
		if(item == accesses.end()) {															\
			accesses.insert(std::make_pair(hash, AccessInfo(access, 0, size, type, pc)));		\
		} else {																				\
			if(access < item->second.address) {													\
				item->second.address = access;													\
				item->second.count++;															\
			} else if(((access - item->second.address) / (1 << size)) >= item->second.count) {	\
				item->second.count++;															\
			}																					\
		}
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
		// Open file
		datafile.open(std::string(ARCHER_DATA) + "/parallelregion_" + std::to_string(parallel_id));
		DATA(datafile, "PARALLEL_START[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "]\n");
		accesses.clear();
	} else {
		DATA(datafile, "PARALLEL_START[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << __swordrt_prev_offset__ + omp_get_thread_num() << ":" << omp_get_num_threads() << "]\n");
		// __swordrt_prev_offset__ = omp_get_thread_num() + omp_get_num_threads();
	}
#else
	if(__swordomp_status__ == 0) {
		// Open file
		datafile.open(std::string(ARCHER_DATA) + "/parallelregion_" + std::to_string(parallel_id));
		threadInfo[tid - 1].accesses.clear();
	}
#endif
}

static void on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id,
		ompt_invoker_t invoker) {
#if defined(TLS) || defined(NOTLS)
	if(__swordomp_status__ == 0) {
		// DATA(datafile, "PARALLEL_END[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << omp_get_thread_num() + omp_get_num_threads() << ":" << omp_get_num_threads() << "]\n");
		DATA(datafile, "PARALLEL_BREAK\n");
		datafile.close();
	} else {
		// DATA(datafile, "PARALLEL_END[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << __swordrt_prev_offset__ + omp_get_thread_num() + omp_get_num_threads() << ":" << omp_get_num_threads() << "]\n");
		__swordrt_prev_offset__ += omp_get_thread_num() + omp_get_num_threads();
	}
#else
	if(__swordomp_status__ == 0) {
		DATA(datafile, "PARALLEL_BREAK\n");
		datafile.close();
	}
#endif
}

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
	std::ostringstream oss;

	__swordrt_barrier__++;

#if defined(TLS) || defined(NOTLS)
	oss << "DATA_BEGIN[" << std::dec << parallel_id << "," << tid << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	for (std::unordered_map<uint64_t, AccessInfo>::iterator it = accesses.begin(); it != accesses.end(); ++it) {
		oss << "DATA[" << std::dec << it->first << "," << std::hex << "0x" << it->second.address << "," << std::dec << it->second.count << "," << it->second.size << "," << it->second.type << "," << "0x" << std::hex << it->second.pc << "]\n";
	}
	oss << "DATA_END[" << std::dec << parallel_id << "," << tid << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	DATA(datafile, oss.str());
	accesses.clear();
#else
	oss << "DATA_BEGIN[" << std::dec << parallel_id << "," << tid << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	for (std::unordered_map<uint64_t, AccessInfo>::iterator it = threadInfo[tid - 1].accesses.begin(); it != threadInfo[tid - 1].accesses.end(); ++it) {
		oss << "DATA[" << std::dec << it->first << "," << std::hex << "0x" << it->second.address << "," << std::dec << it->second.count << "," << it->second.size << "," << it->second.type << "," << "0x" << std::hex << it->second.pc << "]\n";
	}
	oss << "DATA_END[" << std::dec << parallel_id << "," << tid << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	DATA(datafile, oss.str());
	threadInfo[tid - 1].accesses.clear();
#endif

}

static void ompt_initialize_fn(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {

	DEBUG(std::cout, "OMPT Initizialization: Runtime Version: " << std::dec << runtime_version << ", OMPT Version: " << std::dec << ompt_version);

	std::string str = "rm -rf " + std::string(ARCHER_DATA);
	system(str.c_str());
	str = "mkdir " + std::string(ARCHER_DATA);
	system(str.c_str());

	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	ompt_get_thread_id = (ompt_get_thread_id_t) lookup("ompt_get_thread_id");
	ompt_get_parallel_id = (ompt_get_parallel_id_t) lookup("ompt_get_parallel_id");

	ompt_set_callback(ompt_event_thread_begin,
			(ompt_callback_t) &on_ompt_event_thread_begin);
	ompt_set_callback(ompt_event_parallel_begin,
			(ompt_callback_t) &on_ompt_event_parallel_begin);
	ompt_set_callback(ompt_event_parallel_end,
			(ompt_callback_t) &on_ompt_event_parallel_end);
	ompt_set_callback(ompt_event_acquired_critical,
			(ompt_callback_t) &on_acquired_critical);
	ompt_set_callback(ompt_event_release_critical,
			(ompt_callback_t) &on_release_critical);
	ompt_set_callback(ompt_event_barrier_begin,
			(ompt_callback_t) &on_ompt_event_barrier_begin);
}

ompt_initialize_t ompt_tool(void) {
	return ompt_initialize_fn;
}

}
