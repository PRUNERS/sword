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
#define TSAN_CHECK(name)										\
		if(access_tsan_enabled) {								\
			std::unordered_set<uint64_t/* , Hasher */>::iterator it =	\
				access_tsan_checks.find(hash);					\
			if(it != access_tsan_checks.end())					\
				__tsan_ ## name(addr);							\
			return;  											\
  	    }
#define SAVE_ACCESS(size, type)															\
		if(tsan_checks.find(hash) != tsan_checks.end())									\
			return;																		\
		uint64_t diff;																	\
		std::unordered_map<uint64_t, AccessInfo/* , Hasher, HasherEqualFn */>::iterator item = accesses.find(hash); 	\
		if(item == accesses.end()) {													\
			accesses.insert(std::make_pair(hash, AccessInfo(access, access, UINT_MAX,	\
							0, ULLONG_MAX, size, type, pc)));							\
		} else {																		\
			switch(item->second.count) {												\
			case 0:																		\
				break;																	\
			/* Second access */															\
			case ULLONG_MAX:															\
				if(access < item->second.address) {										\
					diff = item->second.prev_address - access;							\
					item->second.address = access;										\
					item->second.count = 1;												\
					item->second.diff = diff;											\
				    item->second.stride = 1;											\
				    item->second.prev_address = access;									\
				} else if(access > item->second.address) {								\
					diff = access - item->second.prev_address;							\
					item->second.count = 1;												\
					item->second.diff = diff;											\
				    item->second.stride = 1;											\
				    item->second.prev_address = access;									\
				} else {																\
					item->second.count = 0;												\
				}																		\
				break;																	\
			default:																	\
				if(access < item->second.prev_address) {								\
					diff = item->second.prev_address - access;							\
					if(diff == item->second.diff) {										\
						item->second.address = access;									\
						item->second.count++;											\
					    item->second.stride = 1;										\
					    item->second.prev_address = access;								\
					} else {															\
						/* Check with Tsan */											\
						tsan_checks.insert(hash);										\
					}																	\
				} else if(access > item->second.prev_address) {							\
					diff = access - item->second.prev_address;							\
					if(diff == item->second.diff) {										\
						item->second.count++;											\
					    item->second.stride = 1;										\
					    item->second.prev_address = access;								\
					} else {															\
						/* Check with Tsan */											\
						/* DEBUG(std::cout, "Check tsan" << pc); */						\
						tsan_checks.insert(hash);										\
					}																	\
				} else {																\
					item->second.count = 0;												\
				}																		\
				break;																	\
			}																			\
		}

//INFO(std::cout, "PATTERN:" << tid << "," << hash << ","<< access << "," << item->second.prev_address << "," << item->second.stride <<	"," << (1 << size) << "," << pc);

extern "C" {

#include "swordrt_interface.inl"

static void on_swordrt_ompt_event_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_id_t thread_id) {

	// Set thread id
	tid = thread_id;
	// Get stack pointer and stack size
	GET_STACK
}

static void on_swordrt_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_id_t parallel_id,
		uint32_t requested_team_size,
		void *parallel_function) {

	if(__swordomp_status__ == 0) {
		// Open file
		datafile.open(std::string(ARCHER_DATA) + "/parallelregion_" + std::to_string(parallel_id));
		DATA(datafile, "PARALLEL_START[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "]\n");
		accesses.clear();
	} else {
		DATA(datafile, "PARALLEL_START[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << __swordrt_prev_offset__ + omp_get_thread_num() << ":" << omp_get_num_threads() << "]\n");
		// __swordrt_prev_offset__ = omp_get_thread_num() + omp_get_num_threads();
	}

	if(access_tsan_checks.size() > 0) {
		std::ostringstream oss;
		for(std::unordered_set<uint64_t/* , Hasher */>::iterator it = access_tsan_checks.begin(); it != access_tsan_checks.end(); ++it)
			oss << *it << "\n";
		DATA(accessdatafile, oss.str());
	}
	if(entry_tsan_checks.size() > 0) {
		std::ostringstream oss;
		for(std::unordered_set<uint64_t/* , Hasher */>::iterator it = entry_tsan_checks.begin(); it != entry_tsan_checks.end(); ++it)
			oss << *it << "\n";
		DATA(entrydatafile, oss.str());
	}
}

static void on_swordrt_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id,
		ompt_invoker_t invoker) {
	if(__swordomp_status__ == 0) {
		// DATA(datafile, "PARALLEL_END[" << std::dec << parallel_id << "," << ompt_get_parallel_id(0) << "," << omp_get_thread_num() + omp_get_num_threads() << ":" << omp_get_num_threads() << "]\n");
		DATA(datafile, "PARALLEL_BREAK\n");
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

	for(std::unordered_set<uint64_t/* , Hasher */>::iterator it = tsan_checks.begin(); it != tsan_checks.end(); ++it)
		accesses.erase(*it);

	oss << "DATA_BEGIN[" << std::dec << parallel_id << "," << tid << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	for (std::unordered_map<uint64_t, AccessInfo/* , Hasher, HasherEqualFn */>::iterator it = accesses.begin(); it != accesses.end(); ++it) {
		if(it->second.count == ULLONG_MAX)
			it->second.count = 0;
		oss << "DATA[" << std::dec << it->first << "," << std::hex << "0x" << it->second.address << "," << std::dec << it->second.count << "," << it->second.size << "," << it->second.type << "," << "0x" << std::hex << it->second.pc << "," << std::dec << it->second.stride << "]\n";
	}
	oss << "DATA_END[" << std::dec << parallel_id << "," << tid << "," << omp_get_thread_num() << ":" << omp_get_num_threads() << "," << __swordrt_barrier__ << "]\n";
	DATA(datafile, oss.str());
	accesses.clear();

	smtx.lock();
	if(tsan_checks.size() > 0) {
		for(std::unordered_set<uint64_t/* , Hasher */>::iterator it = tsan_checks.begin(); it != tsan_checks.end(); ++it)
			access_tsan_checks.insert(*it);
		entry_tsan_checks.insert(__swordrt_hash__);
	}
	smtx.unlock();
	tsan_checks.clear();
//	if(tsan_checks.size() > 0) {
//		std::ostringstream access_str;
//		for(std::unordered_set<uint64_t/* , Hasher */>::iterator it = tsan_checks.begin(); it != tsan_checks.end(); ++it)
//			access_str << *it << "\n";
//		tsan_checks.clear();
//		std::ostringstream entry_str;
//		entry_str << __swordrt_hash__ << "\n";
//		smtx.lock();
//		// accessdatafile.open(std::string(ARCHER_DATA) + "/access_tsan_checks", std::ofstream::app);
//		DATA(accessdatafile, access_str.str());
//		DATA(entrydatafile, entry_str.str());
//		// accessdatafile.close();
//		smtx.unlock();
//	}
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
				entry_tsan_checks.insert(hash);
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
