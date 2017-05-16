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

#ifdef SNAPPY
#include "snappy/snappy.h"
#endif

#ifdef LZ4
#include "lz4/lz4.h"
#define ACCELERATION 5
#endif

#include <boost/filesystem.hpp>

#include <assert.h>
#include <stdlib.h>
#include <zlib.h>

#include <cmath>
#include <fstream>
#include <sstream>

thread_local uint64_t nsync = 0;

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

bool dump_to_file(std::vector<TraceItem> *accesses, size_t size, size_t nmemb,
		FILE *file, unsigned char *buffer, size_t *offset) {

#ifdef LZO
	// LZO
    lzo_uint *out_len = (lzo_uint *) buffer;
    lzo1x_1_compress((unsigned char *) accesses->data(), size * nmemb, buffer + sizeof(lzo_uint), out_len, wrkmem);

    size_t tsize = *out_len + sizeof(lzo_uint);
    fwrite((char *) buffer, tsize, 1, file);
    file_offset += tsize;
	// LZO
#elif defined(SNAPPY)
        size_t *out_len = (size_t *) buffer;
        snappy::RawCompress((char *) accesses, size * nmemb, (char*) buffer + sizeof(size_t), out_len);
        fwrite((char *) buffer, *out_len + sizeof(size_t), 1, file);
#elif defined(LZ4)
        const int64_t max_dst_size = LZ4_compressBound(size * nmemb);
        int64_t *out_len = (int64_t *) buffer;
        *out_len = LZ4_compress_fast((char *) accesses, (char*) buffer + sizeof(int64_t), size * nmemb, max_dst_size, ACCELERATION);

        // if (out_len < 0) {
        //   printf("A negative result from LZ4_compress_default indicates a failure trying to compress the data.");
        // } else if (out_len == 0) {
        //   printf("A result of 0 means compression worked, but was stopped because the destination buffer couldn't hold all the information.");
        // }

        fwrite((char *) buffer, *out_len + sizeof(int64_t), 1, file);
#elif defined(HUFFMAN)
#elif defined(ARITHMETIC)
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
			set.clear();													\
			SWAP_BUFFER														\
		}

#define DUMPNOCHECK_TO_FILE													\
		if(idx > 0) {														\
			fut.wait();														\
			fut = std::async(dump_to_file, accesses,						\
					sizeof(TraceItem), idx, datafile,						\
					out, &offset);											\
					idx = 0;												\
					set.clear();											\
					SWAP_BUFFER 											\
		}

#define SAVE_ACCESS(asize, atype)											\
	TraceItem item = TraceItem(data_access, Access(asize,					\
							   atype, (size_t) addr, CALLERPC));			\
	if(set.check_insert(hash_value(item))) {		 						\
		(*accesses)[idx++] = item;											\
	} 																		\
	if(idx == NUM_OF_ACCESSES)	{											\
		set.clear(); 														\
		fut.wait();															\
		fut = std::async(dump_to_file, accesses,							\
						 sizeof(TraceItem), NUM_OF_ACCESSES, datafile, 		\
						 out, &offset);	  									\
		idx = 0;															\
		SWAP_BUFFER															\
	}

extern "C" {

#include "sword_interface.inl"

static void on_ompt_callback_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_data_t *thread_data) {
	tid = my_next_id();

	accesses1 = new std::vector<TraceItem>(NUM_OF_ACCESSES);
	accesses2 = new std::vector<TraceItem>(NUM_OF_ACCESSES);
	set.reserve(87382);
	accesses = accesses1;
    out = (unsigned char *) malloc(OUT_LEN);
    pdata = new ParallelData();

    // Create datafile
    std::string filename = sword_flags->traces_path + "/datafile_" + std::to_string(tid);
	datafile = fopen(filename.c_str(), "ab");
	if (!datafile) {
		INFO(std::cerr, "SWORD: Error opening datafile: " << filename << " - " << strerror(errno) << ".");
		exit(-1);
	}

	// Create metafile
	filename = sword_flags->traces_path + "/metafile_" + std::to_string(tid);
	metafile = fopen(filename.c_str(), "a");
	if (!metafile) {
		INFO(std::cerr, "SWORD: Error opening metafile: " << filename << " - " << strerror(errno) << ".");
		exit(-1);
	}

	fut = std::async(dummy);
}

static void on_ompt_callback_thread_end(ompt_data_t *thread_data)
{
	fclose(datafile);
	fclose(metafile);
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
		parallel_data->ptr = new ParallelData(pid, __sword_status__, 0, 1);
	} else {
		ompt_id_t pid = ompt_get_unique_id();
		ParallelData *par_data;
		if(pdata->getState()) {
			par_data = new ParallelData(pid, __sword_status__, pdata->getOffset(), pdata->getSpan());
		} else {
			par_data = new ParallelData(pid, __sword_status__, omp_get_thread_num(), omp_get_num_threads());
		}
		parallel_data->ptr = par_data;
		pdata->setData(par_data);
	}
}

static void on_ompt_callback_parallel_end(ompt_data_t *parallel_data,
		ompt_data_t *task_data,
		ompt_invoker_t invoker,
		const void *codeptr_ra) {
	if(__sword_status__ >= 1) {
		pdata->setData(pdata->getParallelID(), __sword_status__, pdata->getOffset() + pdata->getSpan(), pdata->getSpan());
		pdata->setState(1);
	}
}

static void on_ompt_callback_implicit_task(ompt_scope_endpoint_t endpoint,
	    ompt_data_t *parallel_data,
	    ompt_data_t *task_data,
	    unsigned int team_size,
	    unsigned int thread_num) {

	if(endpoint == ompt_scope_begin) {
		ParallelData *par_data = (ParallelData *) parallel_data->ptr;
		pdata->setData((ParallelData *) parallel_data->ptr);
		task_data->ptr = par_data;

		__sword_status__ = par_data->getParallelLevel();

		if(__sword_status__ == 1) {
			bid = 0;
		}

		fprintf(metafile, "%u,%lu,%lu\n", par_data->getParallelLevel(), bid, file_offset);
	}
//	} else if(endpoint == ompt_scope_end) {
//		__sword_status__--;
//		ParallelData *par_data = (ParallelData *) task_data->ptr;
//
//		if(par_data) {
//			if(pdata->getParallelID() == par_data->getParallelID()) {
//				DUMPNOCHECK_TO_FILE
//				fut.wait();
//			}
//		}
//	}
}

static void on_ompt_callback_sync_region(ompt_sync_region_kind_t kind,
		ompt_scope_endpoint_t endpoint,
		ompt_data_t *parallel_data,
		ompt_data_t *task_data,
		const void *codeptr_ra) {
	ParallelData *par_data = (ParallelData *) task_data->ptr;

	if(endpoint == ompt_scope_end) {
		DUMPNOCHECK_TO_FILE
		bid++;
		fprintf(metafile, "%u,%lu,%lu\n", par_data->getParallelLevel(), bid, file_offset);
		nsync++;
	}
}

static void on_ompt_callback_mutex_acquired(ompt_mutex_kind_t kind,
		ompt_wait_id_t wait_id,
		const void *codeptr_ra) {
	(*accesses)[idx] = TraceItem(mutex_acquired, MutexRegion(kind, wait_id));
	DUMP_TO_FILE
}

static void on_ompt_callback_mutex_released(ompt_mutex_kind_t kind,
		ompt_wait_id_t wait_id,
		const void *codeptr_ra) {
	(*accesses)[idx] = TraceItem(mutex_released, MutexRegion(kind, wait_id));
	DUMP_TO_FILE
}

#define register_callback(name) {                                           \
	if (ompt_set_callback(name, (ompt_callback_t)&on_##name) == 			\
		ompt_has_event_no_callback)                             			\
		printf("0: Could not register callback '" #name "'\n");   			\
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
	register_callback(ompt_callback_thread_end);
	register_callback(ompt_callback_parallel_begin);
	register_callback(ompt_callback_parallel_end);
	register_callback(ompt_callback_implicit_task);
	register_callback(ompt_callback_sync_region);
	register_callback(ompt_callback_mutex_acquired);
	register_callback(ompt_callback_mutex_released);

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

#if defined(LZO)
	if(lzo_init() != LZO_E_OK) {
		printf("internal error - lzo_init() failed !!!\n");
		printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)\n");
		exit(-1);
	}
#endif

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

void ompt_finalize(ompt_fns_t* fns) { INFO(std::cout, nsync); }

ompt_fns_t* ompt_start_tool(unsigned int omp_version,
		const char *runtime_version) {
	static ompt_fns_t ompt_fns = { &ompt_initialize, &ompt_finalize };
	return &ompt_fns;
}

}
