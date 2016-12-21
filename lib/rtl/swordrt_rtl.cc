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
#include <dlfcn.h>

#include <sstream>
#include <cmath>
#include "swordrt_tsan_interface.h"
#include "swordrt_ompsan_interface.h"

#define DEF_ACCESS												\
		size_t pc = CALLERPC;
#define SAVE_ACCESS(name, size, type)							\
		bool exists = (accesses[tid].end() !=					\
			std::find_if(accesses[tid].begin(), 				\
					accesses[tid].end(), ByHash(hash))); 		\
		if(exists) return;										\
		u64 cell1 = 0;											\
		u64 cell2 = 0;											\
		__ompsan_ ## name(addr, pc, &cell1, &cell2);			\
		if(cell1)												\
		accesses[tid].push_back(AccessInfo(hash, size, 			\
				type, pc, cell1, cell2));

/// Required OMPT inquiry functions.
ompt_get_parallel_data_t ompt_get_parallel_data;

extern "C" {

#include "swordrt_interface.inl"

static void on_swordrt_ompt_event_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_data_t *thread_data) {
	tid = omp_get_thread_num();

	accesses[tid].reserve(NUM_OF_CONFLICTS);

	datafile[tid].open(std::string(ARCHER_DATA) + "/threadtrace_" + std::to_string(tid));
}

//static void on_swordrt_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
//		ompt_frame_t *parent_task_frame,
//		ompt_parallel_id_t parallel_id,
//		uint32_t requested_team_size,
//		void *parallel_function) {
//}

static void on_swordrt_ompt_event_parallel_end(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data,
		ompt_invoker_t invoker) {
	if(__swordomp_status__ == 0) {
		__ompsan_flush_shadow();
		__swordrt_prev_offset__[tid] = 0;
	} else {
		INFO(std::cout, "Tid:" << tid);
		__swordrt_prev_offset__[tid] += omp_get_thread_num() + omp_get_num_threads();
	}
}

static void on_swordrt_ompt_event_acquired_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__[tid] = true;
}

static void on_swordrt_ompt_event_release_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__[tid] = false;
}

static void on_swordrt_ompt_event_barrier_begin(ompt_parallel_data_t parallel_data,
	    ompt_task_data_t task_data) {
	std::ostringstream oss;

	int parallel_id = 0;

	__swordrt_barrier__[tid]++;

	if(accesses[tid].size() > 0) {
		oss << "DATA_BEGIN[" << std::dec << parallel_id << "," << omp_get_thread_num() << "," << omp_get_thread_num() << ":" << __swordrt_prev_offset__[tid] + omp_get_thread_num() << "," << __swordrt_barrier__[tid]<< "]\n";
		for (std::vector<AccessInfo>::iterator it = accesses[tid].begin(); it != accesses[tid].end(); ++it) {
			oss << "DATA[" << std::hex << "0x" << it->hash << "," << it->size << "," << it->type << "," << "0x" << std::hex << it->pc1 << "," << "0x" << std::hex << it->cell1  << "," << "0x" << it->cell2 << "]\n";
		}
		oss << "DATA_END[" << std::dec << parallel_id << "," << omp_get_thread_num() << "," << omp_get_thread_num() << ":" << __swordrt_prev_offset__[tid] + omp_get_thread_num() << "," << __swordrt_barrier__[tid]<< "]\n";
	}
	DATA(datafile[tid], oss.str());
	accesses[tid].clear();
}

static void on_swordrt_ompt_event_runtime_shutdown() {}

static void ompt_tsan_initialize(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {

	DEBUG(std::cout, "OMPT Initizialization: Runtime Version: " << std::dec << runtime_version << ", OMPT Version: " << std::dec << ompt_version);

	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	if (ompt_set_callback == NULL) {
		std::cerr << "Could not set callback, exiting..." << std::endl;
		std::exit(1);
	}
	ompt_get_parallel_data = (ompt_get_parallel_data_t) lookup("ompt_get_parallel_data");
	if (ompt_get_parallel_data == NULL) {
		fprintf(stderr, "Could not get inquiry function 'ompt_get_parallel_data', exiting...\n");
		exit(1);
	}

	ompt_set_callback(ompt_event_thread_begin,
			(ompt_callback_t) &on_swordrt_ompt_event_thread_begin);
//	ompt_set_callback(ompt_event_parallel_begin,
//			(ompt_callback_t) &on_swordrt_ompt_event_parallel_begin);
	ompt_set_callback(ompt_event_parallel_end,
			(ompt_callback_t) &on_swordrt_ompt_event_parallel_end);
	ompt_set_callback(ompt_event_acquired_critical,
			(ompt_callback_t) &on_swordrt_ompt_event_acquired_critical);
	ompt_set_callback(ompt_event_release_critical,
			(ompt_callback_t) &on_swordrt_ompt_event_release_critical);
	ompt_set_callback(ompt_event_barrier_begin,
			(ompt_callback_t) &on_swordrt_ompt_event_barrier_begin);
//	ompt_set_callback(ompt_event_runtime_shutdown,
//			(ompt_callback_t) &on_swordrt_ompt_event_runtime_shutdown);

	std::string str = "rm -rf " + std::string(ARCHER_DATA);
	system(str.c_str());
	str = "mkdir " + std::string(ARCHER_DATA);
	system(str.c_str());
}

ompt_initialize_t ompt_tool() {
  return &ompt_tsan_initialize;
}


}
