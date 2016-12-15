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

namespace __ompsan {
	extern void FlushShadowMemory();
}

#define DEF_ACCESS												\
		size_t access = (size_t) addr;							\
		size_t pc = CALLERPC;
#define TSAN_CHECK(name)
/*
#define TSAN_CHECK(name)										\
		if(access_tsan_enabled) {								\
			std::unordered_set<uint64_t>::iterator it =	\
				access_tsan_checks.find(hash);					\
			if(it != access_tsan_checks.end())					\
				__tsan_ ## name(addr);							\
			return;  											\
  	    }
  	    */
#define SAVE_ACCESS(name, size, type)							\
		bool exists = (accesses.end() != std::find_if(accesses.begin(), accesses.end(), ByHash(hash))); \
		if(exists) return;										\
		bool conflict = false;									\
		uint64_t p = 0;											\
		__ompsan_ ## name(addr, pc, &conflict, &p);				\
		if(conflict)											\
			accesses.push_back(AccessInfo(hash, size, 			\
							   type, pc, p));

extern "C" {

#include "swordrt_interface.inl"

static void on_swordrt_ompt_event_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_id_t thread_id) {
	tid = omp_get_thread_num();

	accesses.reserve(NUM_OF_CONFLICTS);
}

static void on_swordrt_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_id_t parallel_id,
		uint32_t requested_team_size,
		void *parallel_function) {

	__ompsan::FlushShadowMemory();

	if(__swordomp_status__ == 0) {
		// Open file
		/* SAVE TO FILE
		datafile.open(std::string(ARCHER_DATA) + "/parallelregion_" + std::to_string(parallel_id));
		DATA(datafile, "PARALLEL_START[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "]\n");
		accesses.clear();
		*/
	} else {
		/* SAVE TO FILE
		DATA(datafile, "PARALLEL_START[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << __swordrt_prev_offset__ + omp_get_thread_num() << ":" << omp_get_num_threads() << "]\n");
		*/
		// __swordrt_prev_offset__ = omp_get_thread_num() + omp_get_num_threads();
	}
}

static void on_swordrt_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id,
		ompt_invoker_t invoker) {

	/* SIMONE
	if(access_tsan_checks.size() > 0) {
		std::ostringstream oss;
		for(std::unordered_set<uint64_t>::iterator it = access_tsan_checks.begin(); it != access_tsan_checks.end(); ++it)
			oss << *it << "\n";
		DATA(accessdatafile, oss.str());
	}
	if(entry_tsan_checks.size() > 0) {
		std::ostringstream oss;
		for(std::unordered_set<uint64_t>::iterator it = entry_tsan_checks.begin(); it != entry_tsan_checks.end(); ++it)
			oss << *it << "\n";
		DATA(entrydatafile, oss.str());
	}
	access_tsan_checks.clear();
	entry_tsan_checks.clear();
	*/

	if(__swordomp_status__ == 0) {
		// DATA(datafile, "PARALLEL_END[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << omp_get_thread_num() + omp_get_num_threads() << ":" << omp_get_num_threads() << "]\n");
		/* SAVE TO FILE
		DATA(datafile, "PARALLEL_BREAK\n");
		*/
		datafile.close();
	} else {
		// DATA(datafile, "PARALLEL_END[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << __swordrt_prev_offset__ + omp_get_thread_num() + omp_get_num_threads() << ":" << omp_get_num_threads() << "]\n");
		__swordrt_prev_offset__ += omp_get_thread_num() + omp_get_num_threads();
	}
}

static void on_swordrt_ompt_event_acquired_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__ = true;
}

static void on_swordrt_ompt_event_release_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__ = false;
}

static void on_swordrt_ompt_event_barrier_begin(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id) {
	std::ostringstream oss;

	__swordrt_barrier__++;

	// INFO(std::cout, tid << ": Size: " << accesses.size());

	/* SAVE TO FILE
	for(std::unordered_set<uint64_t>::iterator it = conflicts.begin(); it != conflicts.end(); ++it)
		accesses.erase(*it);

	oss << "DATA_BEGIN[" << std::dec << parallel_id << "," << omp_get_thread_num() << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	for (std::unordered_map<uint64_t, AccessInfo>::iterator it = accesses.begin(); it != accesses.end(); ++it) {
		oss << "DATA[" << std::dec << it->first << "," << std::hex << "0x" << it->second.address << it->second.size << "," << it->second.type << "," << "0x" << std::hex << it->second.pc << "]\n";
	}
	oss << "DATA_END[" << std::dec << parallel_id << "," << omp_get_thread_num() << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	DATA(datafile, oss.str());
	accesses.clear();
	*/

	/* SIMONE
	smtx.lock();
	if(conflicts.size() > 0) {
		for(std::unordered_set<uint64_t>::iterator it = conflicts.begin(); it != conflicts.end(); ++it)
			access_tsan_checks.insert(*it);
	}
	if(entries.size() > 0) {
		for(std::unordered_set<uint64_t>::iterator it = entries.begin(); it != entries.end(); ++it)
			entry_tsan_checks.insert(*it);
	}
	smtx.unlock();
	conflicts.clear();
	entries.clear();
	*/
}

static void on_swordrt_ompt_event_runtime_shutdown() {
	accessdatafile.close();
	entrydatafile.close();
}

void swordrt_load_access_tsan_file() {
	FILE *fp;
	std::string filename = std::string(ARCHER_DATA) + "/access_tsan_checks";
	if((fp = fopen(filename.c_str(), "r")) != NULL) {
		access_tsan_checks.clear();
		uint64_t hash;
		while (true) {
			int ret = fscanf(fp, "%lu", &hash);
			if(ret == 1) {
				access_tsan_checks.insert(hash);
			} else if(errno != 0) {
				perror("scanf");
				exit(-1);
			} else if(ret == EOF) {
				break;
			} else {
				perror("No match.");
				exit(-1);
			}
		}
	}
}

void swordrt_load_entry_tsan_file() {
	FILE *fp;
	std::string filename = std::string(ARCHER_DATA) + "/entry_tsan_checks";
	if((fp = fopen(filename.c_str(), "r")) != NULL) {
		access_tsan_checks.clear();
		uint64_t hash;
		while (true) {
			int ret = fscanf(fp, "%lu", &hash);
			if(ret == 1) {
				entries.insert(hash);
			} else if(errno != 0) {
				perror("scanf");
				exit(-1);
			} else if(ret == EOF) {
				break;
			} else {
				perror("No match.");
				exit(-1);
			}
		}
	}
}

static void ompt_initialize_fn(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {

	if(!access_tsan_enabled && !entry_tsan_enabled) {
		DEBUG(std::cout, "OMPT Initizialization: Runtime Version: " << std::dec << runtime_version << ", OMPT Version: " << std::dec << ompt_version);

		ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
		ompt_get_thread_id = (ompt_get_thread_id_t) lookup("ompt_get_thread_id");
		ompt_get_parallel_id = (ompt_get_parallel_id_t) lookup("ompt_get_parallel_id");

		ompt_set_callback(ompt_event_thread_begin,
				(ompt_callback_t) &on_swordrt_ompt_event_thread_begin);
		ompt_set_callback(ompt_event_parallel_begin,
				(ompt_callback_t) &on_swordrt_ompt_event_parallel_begin);
		ompt_set_callback(ompt_event_parallel_end,
				(ompt_callback_t) &on_swordrt_ompt_event_parallel_end);
		ompt_set_callback(ompt_event_acquired_critical,
				(ompt_callback_t) &on_swordrt_ompt_event_acquired_critical);
		ompt_set_callback(ompt_event_release_critical,
				(ompt_callback_t) &on_swordrt_ompt_event_release_critical);
		ompt_set_callback(ompt_event_barrier_begin,
				(ompt_callback_t) &on_swordrt_ompt_event_barrier_begin);
		ompt_set_callback(ompt_event_runtime_shutdown,
				(ompt_callback_t) &on_swordrt_ompt_event_runtime_shutdown);

		std::string str = "rm -rf " + std::string(ARCHER_DATA);
		system(str.c_str());
		str = "mkdir " + std::string(ARCHER_DATA);
		system(str.c_str());

		// Opening file to save possible tsan checks
		accessdatafile.open(std::string(ARCHER_DATA) + "/access_tsan_checks", std::ofstream::app);
		entrydatafile.open(std::string(ARCHER_DATA) + "/entry_tsan_checks", std::ofstream::app);
	} else {
		std::thread access_tsan_thread(swordrt_load_access_tsan_file);
		std::thread entry_tsan_thread(swordrt_load_entry_tsan_file);

		access_tsan_thread.join();
		entry_tsan_thread.join();
	}
}

ompt_initialize_t ompt_tool(void) {
	return ompt_initialize_fn;
}

}
