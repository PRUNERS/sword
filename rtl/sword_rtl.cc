
//===-- sword_rtl.cc -------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Sword/Sword, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#include "sword_rtl.h"
#include "sword_flags.h"

#include <boost/filesystem.hpp>


#include <assert.h>
#include <stdlib.h>
#include <zlib.h>

#include <cmath>
#include <fstream>
#include <sstream>

#define LZO 1

static const char* ompt_thread_type_t_values[] = {
		NULL,
		"ompt_thread_initial",
		"ompt_thread_worker",
		"ompt_thread_other"
};

static const char* ompt_task_type_t_values[] = {
		NULL,
		"ompt_task_initial",
		"ompt_task_implicit",
		"ompt_task_explicit",
		"ompt_task_target"
};

static ompt_get_state_t ompt_get_state;
static ompt_get_task_info_t ompt_get_task_info;
static ompt_get_thread_data_t ompt_get_thread_data;
static ompt_get_parallel_info_t ompt_get_parallel_info;
static ompt_get_unique_id_t ompt_get_unique_id;

static uint64_t my_next_id()
{
	static uint64_t ID = 0;
	uint64_t ret = __sync_fetch_and_add(&ID,1);
	// assert(ret < omp_get_max_threads() && "Maximum number of allowed threads is limited by MAX_THREADS");
	return ret;
}

SwordFlags *sword_flags;

bool dummy() {
	return true;
}

bool dump_to_file(TraceItem *accesses, size_t size, size_t nmemb,
		FILE *file, unsigned char *buffer, size_t *offset) {

#if LZO
	// LZO
	lzo_uint out_len;
	int r = lzo1x_1_compress((unsigned char *) accesses, size * nmemb, buffer + sizeof(out_len), &out_len, wrkmem);
	if (r != LZO_E_OK) {
		printf("internal error - compression failed: %d\n", r);
		return -1;
	}
	// check for an incompressible block
	if (out_len >= BLOCK_SIZE) {
		printf("This block contains incompressible data.\n");
		return -1;
	}

	memcpy(buffer, &out_len, sizeof(out_len));
	size_t ret = fwrite((char *) buffer, out_len + sizeof(out_len), 1, file);
	// LZO
#elif HUFFMAN
#elif ARITHMETIC
#elif TCGEN
#else
	size_t ret = fwrite((char *) accesses, size * nmemb, 1, file); // Write plain
#endif

	return true;
}

#define SWAP_BUFFER															\
		if(accesses == accesses1) {											\
			accesses == accesses2;											\
		} else {															\
			accesses == accesses1;											\
		}

#define DUMP_TO_FILE														\
		idx++;																\
		if(idx == NUM_OF_ACCESSES)	{										\
			fut.wait();														\
			fut = std::async(dump_to_file, accesses,						\
					sizeof(TraceItem), NUM_OF_ACCESSES, datafile,			\
					out, &offset);											\
			idx = 0;														\
			SWAP_BUFFER														\
		}

//		INFO(std::cout, tid << ": " << std::hex << addr << ":" << CALLERPC);	\
// #define SAVE_ACCESS(asize, atype)

// It adds a lot of runtime overhead because of the TLS access and seems it's not needed, will add later if we find any false positives
// if(__sword_ignore_access) return; 									\

#define SAVE_ACCESS(asize, atype)											\
		accesses[idx].setType(data_access);									\
		accesses[idx].data.access = Access(asize, atype,					\
				(size_t) addr, CALLERPC); 									\
				idx++;														\
				if(idx == NUM_OF_ACCESSES)	{								\
					fut.wait();												\
					fut = std::async(dump_to_file, accesses,				\
							sizeof(TraceItem), NUM_OF_ACCESSES, datafile, 	\
							out, &offset);	  								\
							idx = 0;										\
							SWAP_BUFFER										\
				}

extern "C" {

#include "sword_interface.inl"

static void on_ompt_callback_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_data_t *thread_data) {
	tid = my_next_id();

	accesses1 = (TraceItem *) malloc(BLOCK_SIZE);
	accesses2 = (TraceItem *) malloc(BLOCK_SIZE);
	accesses = accesses1;
	out = (unsigned char *) malloc(OUT_LEN);

	fut = std::async(dummy);
}

static void on_ompt_callback_parallel_begin(ompt_task_data_t parent_task_data,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_data_t *parallel_data,
		uint32_t requested_team_size,
		void *parallel_function,
		ompt_invoker_t invoker) {

	__sword_status__++;

	if(__sword_status__ == 1) {
		ompt_id_t pid = ompt_get_unique_id();
		std::string str = sword_flags->traces_path + "/" + std::to_string(pid);
		try {
			boost::filesystem::create_directory(str);
		} catch( boost::filesystem::filesystem_error const & e) {
			INFO(std::cerr, e.what());
			exit(-1);
		}
		parallel_data->ptr = new ParallelData(pid, __sword_status__, str, 0, 1);
	} else {
		ompt_id_t pid = ompt_get_unique_id();
		char buff[9];
		if(pdata.getState()) {
			snprintf(buff, sizeof(buff), OFFSET_SPAN_FORMAT, pdata.getOffset(), pdata.getSpan());
		} else {
			snprintf(buff, sizeof(buff), OFFSET_SPAN_FORMAT, omp_get_thread_num(), omp_get_num_threads());
		}
		std::string str = pdata.getPath() + "/" + (const char *) buff;
		try {
			boost::filesystem::create_directory(str);
		} catch( boost::filesystem::filesystem_error const & e) {
			INFO(std::cerr, e.what());
			exit(-1);
		}
		ParallelData *par_data;
		if(pdata.getState()) {
			par_data = new ParallelData(pid, __sword_status__, str, pdata.getOffset(), pdata.getSpan());
		} else {
			par_data = new ParallelData(pid, __sword_status__, str, omp_get_thread_num(), omp_get_num_threads());
		}
		parallel_data->ptr = par_data;
		pdata.setData(par_data);
	}
}

static void on_ompt_callback_parallel_end(ompt_data_t *parallel_data,
		ompt_data_t *task_data,
		ompt_invoker_t invoker,
		const void *codeptr_ra) {
	if(__sword_status__ >= 1) {
		std::string filename = pdata.getPath(-1) + "/threadtrace_" + std::to_string(tid);
		datafile = fopen(filename.c_str(), "ab");
		if (!datafile) {
			INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
			exit(-1);
		}
		pdata.setData(pdata.getParallelID(), __sword_status__, pdata.getPath(-1), pdata.getOffset() + pdata.getSpan(), pdata.getSpan());
		pdata.setState(1);
	}
}

static void on_ompt_callback_implicit_task(ompt_scope_endpoint_t endpoint,
	    ompt_data_t *parallel_data,
	    ompt_data_t *task_data,
	    unsigned int team_size,
	    unsigned int thread_num) {

	if(endpoint == ompt_scope_begin) {
		ParallelData *par_data = (ParallelData *) parallel_data->ptr;
		pdata.setData((ParallelData *) parallel_data->ptr);
		task_data->ptr = par_data;

		__sword_status__ = par_data->getParallelLevel();

		if(__sword_status__ == 1) {
			bid = 0;
			std::string filename = par_data->getPath() + "/threadtrace_" + std::to_string(tid) + "_" + std::to_string(bid);
			datafile = fopen(filename.c_str(), "ab");
			if (!datafile) {
				INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
				exit(-1);
			}
//			ParallelData *par_data = (ParallelData *) parallel_data->ptr;
//			pdata.setData(par_data->getParallelID(), __sword_status__, filename, omp_get_thread_num(), team_size);
		} else {
			if(datafile) {
				fclose(datafile);
			}
			std::string filename = par_data->getPath() + "/threadtrace_" + std::to_string(tid) + "_" + std::to_string(bid);
			datafile = fopen(filename.c_str(), "ab");
			if (!datafile) {
				INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
				exit(-1);
			}
		}

		accesses[idx].setType(parallel_begin);
		accesses[idx].data.parallel = Parallel(par_data->getParallelID(), team_size);
		DUMP_TO_FILE

	} else if(endpoint == ompt_scope_end) {
		__sword_status__--;
		ParallelData *par_data = (ParallelData *) task_data->ptr;

		if(par_data) {
			if(pdata.getParallelID() == par_data->getParallelID()) {
				if(ftell(datafile) > 0) {
					accesses[idx].setType(parallel_end);
					accesses[idx].data.parallel = Parallel(pdata.getParallelID(), team_size);
					idx++;
					fut.wait();
					fut = std::async(dump_to_file, accesses, sizeof(TraceItem), idx, datafile, out, &offset);
					idx = 0;
					SWAP_BUFFER
					fut.wait();
					if(datafile) {
						fclose(datafile);
						datafile = NULL;
					}
				} else {
					if(datafile) {
						fclose(datafile);
						datafile = NULL;
					}
					std::string filename = par_data->getPath() + "/threadtrace_" + std::to_string(tid) + "_" + std::to_string(bid);
					if(boost::filesystem::exists(filename))
						boost::filesystem::remove(filename);
				}
			}
		}
	}
}

static void on_ompt_callback_work(ompt_work_type_t wstype,
		ompt_scope_endpoint_t endpoint,
		ompt_data_t *parallel_data,
		ompt_data_t *task_data,
		uint64_t count,
		const void *codeptr_ra) {
	accesses[idx].setType(work);
	accesses[idx].data.work = Work(wstype, endpoint);
	DUMP_TO_FILE
}

static void on_ompt_callback_sync_region(ompt_sync_region_kind_t kind,
		ompt_scope_endpoint_t endpoint,
		ompt_data_t *parallel_data,
		ompt_data_t *task_data,
		const void *codeptr_ra) {
	ParallelData *par_data = (ParallelData *) task_data->ptr;

	// It adds a lot of runtime overhead because of the TLS access and seems it's not needed, will add later if we find any false positives
//	if(endpoint == ompt_scope_begin) {
//		__sword_ignore_access = 1;
//	} else {

	if(endpoint == ompt_scope_end) {
		bid++;
		fut.wait();
		fut = std::async(dump_to_file, accesses, sizeof(TraceItem), idx, datafile, out, &offset);
		idx = 0;
		SWAP_BUFFER
		fut.wait();
		if(datafile) {
			fclose(datafile);
			datafile = NULL;
		}
		std::string filename = pdata.getPath() + "/threadtrace_" + std::to_string(tid) + "_" + std::to_string(bid);
		datafile = fopen(filename.c_str(), "ab");
		if (!datafile) {
			INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
			exit(-1);
		}
		// It adds a lot of runtime overhead because of the TLS access and seems it's not needed, will add later if we find any false positives
		// __sword_ignore_access = 0;
	}
}

static void on_ompt_callback_master(ompt_scope_endpoint_t endpoint,
		ompt_data_t *parallel_data,
		ompt_data_t *task_data,
		const void *codeptr_ra) {
	accesses[idx].setType(master);
	accesses[idx].data.master = Master(endpoint);
	DUMP_TO_FILE
}

static void on_ompt_callback_mutex_acquired(ompt_mutex_kind_t kind,
		ompt_wait_id_t wait_id,
		const void *codeptr_ra) {
	accesses[idx].setType(mutex_acquired);
	accesses[idx].data.mutex_region = MutexRegion(kind, wait_id);
	DUMP_TO_FILE
}

static void on_ompt_callback_mutex_released(ompt_mutex_kind_t kind,
		ompt_wait_id_t wait_id,
		const void *codeptr_ra) {
	accesses[idx].setType(mutex_released);
	accesses[idx].data.mutex_region = MutexRegion(kind, wait_id);
	DUMP_TO_FILE
}

#ifdef TASK_SUPPORT
static void on_ompt_callback_task_create(ompt_data_t *parent_task_data,
		const ompt_frame_t *parent_frame,
		ompt_data_t* new_task_data,
		ompt_task_type_t type,
		int has_dependences,
		const void *codeptr_ra) {

	ompt_id_t task_id = ompt_get_unique_id();
	new_task_data->ptr = new TaskData(task_id);

   if(type == ompt_task_explicit) {
		accesses[idx].setType(task_create);
		accesses[idx].data.task_create = TaskCreate(task_id, type, has_dependences);
		DUMP_TO_FILE
		// Create empty file to flag that this parallel region has explicit
		// tasks and needs to be threated differently during the data race
		// detection analysis
//		std::string filename = pdata.getPath() + "/create_tasks_" + std::to_string(tid) + "_" + std::to_string(bid);
//		if(!boost::filesystem::exists(filename))
//			system(std::string("touch " + filename).c_str());
	}
}

static void on_ompt_callback_task_schedule(ompt_data_t *prior_task_data,
		ompt_task_status_t prior_task_status,
		ompt_data_t *next_task_data) {
	TaskData *tdata1 = (TaskData *) prior_task_data->ptr;
	TaskData *tdata2 = (TaskData *) next_task_data->ptr;
	accesses[idx].setType(task_schedule);
	if(prior_task_status == ompt_task_complete) {
//		std::string filename = pdata.getPath() + "/schedule_tasks_" + std::to_string(tid) + "_" + std::to_string(bid);
//		if(!boost::filesystem::exists(filename))
//			system(std::string("touch " + filename).c_str());
		accesses[idx].data.task_schedule = TaskSchedule(tdata1->getTaskID(), ompt_task_complete);
	} else {
		accesses[idx].data.task_schedule = TaskSchedule(tdata2->getTaskID(), prior_task_status);
	}
	DUMP_TO_FILE
}

static void on_ompt_callback_task_dependences(ompt_data_t* task_data,
		const ompt_task_dependence_t *deps,
		int ndeps) {
	TaskData *tdata = (TaskData *) task_data->ptr;
	for(int i = 0; i < ndeps; i++) {
		accesses[idx].setType(task_dependence);
		accesses[idx].data.task_dependence = TaskDependence(tdata->getTaskID(), deps[i].variable_addr, deps[i].dependence_flags);
		DUMP_TO_FILE
	}
}
#endif

//static void on_ompt_event_runtime_shutdown(void) {
//	printf("%d: ompt_event_runtime_shutdown\n", omp_get_thread_num());
//}

#define register_callback(name) {                                           \
		if (ompt_set_callback(name, (ompt_callback_t)&on_##name) == 		\
				ompt_has_event_no_callback)                             	\
				printf("0: Could not register callback '" #name "'\n");   	\
}

int ompt_initialize(ompt_function_lookup_t lookup,
		ompt_fns_t* fns) {
	const char *options = getenv("SWORD_OPTIONS");
	sword_flags = new SwordFlags(options);

	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	ompt_get_state = (ompt_get_state_t) lookup("ompt_get_state");
	ompt_get_task_info = (ompt_get_task_info_t) lookup("ompt_get_task_info");
	ompt_get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
	ompt_get_parallel_info = (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");
	ompt_get_unique_id = (ompt_get_unique_id_t) lookup("ompt_get_unique_id");

	register_callback(ompt_callback_thread_begin);
	register_callback(ompt_callback_parallel_begin);
	register_callback(ompt_callback_parallel_end);
	register_callback(ompt_callback_implicit_task);
	// register_callback(ompt_callback_work);
	register_callback(ompt_callback_sync_region);
	// register_callback(ompt_callback_master);
	register_callback(ompt_callback_mutex_acquired);
	register_callback(ompt_callback_mutex_released);

#ifdef TASK_SUPPORT
	register_callback(ompt_callback_task_create);
	register_callback(ompt_callback_task_schedule);
	register_callback(ompt_callback_task_dependences);
#endif

	std::string str = sword_flags->traces_path;
	if(sword_flags->traces_path.empty()) {
		str = std::string("./") + std::string(SWORD_DATA);
		sword_flags->traces_path = str;
	}
	if(boost::filesystem::is_directory(str)) {
		try {
			boost::filesystem::rename(str, str + ".old");
		} catch( boost::filesystem::filesystem_error const & e) {
			boost::filesystem::remove_all(str + ".old");
			boost::filesystem::rename(str.c_str(), str + ".old");
			// INFO(std::cerr, e.what());
		}
		// INFO(std::cout, "WARNING: Please remove or rename '" << str "' directory before running the program.");
	}
	str = sword_flags->traces_path;
	if(sword_flags->traces_path.empty()) {
		str = std::string("./") + std::string(SWORD_DATA);
		sword_flags->traces_path = str;
	}
	boost::filesystem::create_directory(str);

	if(lzo_init() != LZO_E_OK) {
		printf("internal error - lzo_init() failed !!!\n");
		printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)\n");
		exit(-1);
	}

        // INFO(std::cout, "SIZE:" << sizeof(TraceItem));
        // INFO(std::cout, "SIZE ACCESS:" << sizeof(Access));
        // INFO(std::cout, "SIZE PARALLEL:" << sizeof(Parallel));
        // INFO(std::cout, "SIZE WORK:" << sizeof(Work));
        // INFO(std::cout, "SIZE MASTER:" << sizeof(Master));
        // INFO(std::cout, "SIZE SYNC REGION:" << sizeof(SyncRegion));
        // INFO(std::cout, "SIZE MUTEX REGION:" << sizeof(MutexRegion));
        // INFO(std::cout, "SIZE TASK CREATE:" << sizeof(TaskCreate));
        // INFO(std::cout, "SIZE TASK SCHEDULE:" << sizeof(TaskSchedule));
        // INFO(std::cout, "SIZE TASK DEPENDENCE:" << sizeof(TaskDependence));
        // INFO(std::cout, "SIZE OFFSET SPAN:" << sizeof(OffsetSpan));

	return 0;
}

void ompt_finalize(ompt_fns_t* fns) {
	// on_ompt_event_runtime_shutdown();
}

ompt_fns_t* ompt_start_tool(unsigned int omp_version,
		const char *runtime_version) {
	static ompt_fns_t ompt_fns = { &ompt_initialize, &ompt_finalize };
	return &ompt_fns;
}

}
