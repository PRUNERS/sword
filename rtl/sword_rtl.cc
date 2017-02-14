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
#include <zlib.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <cmath>

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

static ompt_get_task_info_t ompt_get_task_info;
static ompt_get_thread_data_t ompt_get_thread_data;
static ompt_get_parallel_info_t ompt_get_parallel_info;
static ompt_get_unique_id_t ompt_get_unique_id;

SwordFlags *sword_flags;

void compress_memory(void *in_data, size_t in_data_size, std::vector<uint8_t> &out_data)
{
	std::vector<uint8_t> buffer;

	const size_t BUFSIZE = 128 * 1024;
	uint8_t temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<uint8_t *>(in_data);
	strm.avail_in = in_data_size;
	strm.next_out = temp_buffer;
	strm.avail_out = BUFSIZE;

	// Options: Z_BEST_SPEED, Z_BEST_COMPRESSION, Z_DEFAULT_COMPRESSION, Z_DEFAULT_COMPRESSION
	deflateInit(&strm, Z_BEST_SPEED);

	while (strm.avail_in != 0)
	{
		int res = deflate(&strm, Z_NO_FLUSH);
		assert(res == Z_OK);
		if (strm.avail_out == 0)
		{
			buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
	}

	int deflate_res = Z_OK;
	while (deflate_res == Z_OK)
	{
		if (strm.avail_out == 0)
		{
			buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
		deflate_res = deflate(&strm, Z_FINISH);
	}

	assert(deflate_res == Z_STREAM_END);
	buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
	deflateEnd(&strm);

	out_data.swap(buffer);
}

bool dummy() {
	return true;
}

bool dump_to_file(TraceItem *accesses, size_t size, size_t nmemb,
		FILE *file, unsigned char *buffer, size_t *offset) {

#ifdef ZLIB
	// ZLIB
	std::vector<uint8_t> out_data;
	compress_memory((void *) accesses, size * nmemb, out_data);

	if(*offset + out_data.size() < MB_LIMIT) {
		memcpy(buffer + *offset, (void *) out_data.data(), out_data.size());
		*offset += out_data.size();
	} else {
		size_t ret = fwrite(buffer + 8, *offset, 1, file);
		*offset = 0;
	}

	size_t ret = fwrite(out_data.data(), out_data.size(), 1, file);
	// ZLIB
#elif LZO
	// LZO
	lzo_uint out_len;
	int r = lzo1x_1_compress((unsigned char *) accesses, BLOCK_SIZE, buffer, &out_len, wrkmem);
	if (r != LZO_E_OK) {
		printf("internal error - compression failed: %d\n", r);
		return 2;
	}
	// check for an incompressible block
	if (out_len >= BLOCK_SIZE) {
		printf("This block contains incompressible data.\n");
		return 0;
	}

	size_t ret = fwrite((char *) buffer, out_len, 1, file);
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

// #define SAVE_ACCESS(asize, atype)
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
	tid = omp_get_thread_num();

	accesses1 = (TraceItem *) malloc(BLOCK_SIZE);
	accesses2 = (TraceItem *) malloc(BLOCK_SIZE);
	accesses = accesses1;
	out = (unsigned char *) malloc(OUT_LEN);

	fut = std::async(dummy);
}

static void on_ompt_callback_thread_end(ompt_data_t *thread_data)
{

}

static void on_ompt_callback_parallel_begin(ompt_task_data_t parent_task_data,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_data_t *parallel_data,
		uint32_t requested_team_size,
		void *parallel_function,
		ompt_invoker_t invoker) {
	parallel_data->value = ompt_get_unique_id();

	current_parallel_idx.store(parallel_data->value, std::memory_order_relaxed);
	if(__sword_status__ == 0) {
		std::string str = "mkdir " + sword_flags->trace_path + std::string(SWORD_DATA) + "/" + std::to_string(parallel_data->value);
		system(str.c_str());
	}
}

static void on_ompt_callback_implicit_task(ompt_scope_endpoint_t endpoint,
	    ompt_data_t *parallel_data,
	    ompt_data_t *task_data,
	    unsigned int team_size,
	    unsigned int thread_num) {
	if(endpoint == ompt_scope_begin) {
		if(__sword_status__ == 0) {
			parallel_idx = parallel_data->value;
			task_data->value = parallel_data->value;
		} else {
			parallel_idx = current_parallel_idx.load(std::memory_order_relaxed);
			task_data->value = parallel_data->value;
		}
		__sword_status__++;
		accesses[idx].setType(parallel_begin);
		accesses[idx].data.parallel = Parallel(parallel_idx);
		idx++;
		// Task starting from outer Parallel Region
		std::string filename = std::string(sword_flags->trace_path + std::string(SWORD_DATA) + "/" + std::to_string(parallel_idx) + "/threadtrace_" + std::to_string(tid));
		datafile = fopen(filename.c_str(), "ab");
		if (!datafile) {
			INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
			exit(-1);
		}
	} else if(endpoint == ompt_scope_end) {
		__sword_status__--;
		if(parallel_idx == task_data->value) {
			accesses[idx].setType(parallel_end);
			accesses[idx].data.parallel = Parallel(parallel_idx);
			idx++;
			fut.wait();
			fut = std::async(dump_to_file, accesses, sizeof(TraceItem), idx, datafile, out, &offset);
			idx = 0;
			SWAP_BUFFER
			fut.wait();
			if(datafile)
				fclose(datafile);
			datafile = NULL;
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
	accesses[idx].setType(sync_region);
	// Find a way to set the barrier id, also figure out if we really need it
	ompt_id_t bid = 0;
	accesses[idx].data.sync_region = SyncRegion(bid, kind, endpoint);
	DUMP_TO_FILE
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


static void on_ompt_event_runtime_shutdown(void)
{
	printf("%d: ompt_event_runtime_shutdown\n", omp_get_thread_num());
}

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
	ompt_get_task_info = (ompt_get_task_info_t) lookup("ompt_get_task_info");
	ompt_get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
	ompt_get_parallel_info = (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");
	ompt_get_unique_id = (ompt_get_unique_id_t) lookup("ompt_get_unique_id");

	register_callback(ompt_callback_thread_begin);
	register_callback(ompt_callback_parallel_begin);
	register_callback(ompt_callback_implicit_task);
	register_callback(ompt_callback_work);
	register_callback(ompt_callback_sync_region);
	register_callback(ompt_callback_master);
	register_callback(ompt_callback_mutex_acquired);
	register_callback(ompt_callback_mutex_released);

	std::string str = "rm -rf " + sword_flags->trace_path + std::string(SWORD_DATA);
	system(str.c_str());
	str = "mkdir " + sword_flags->trace_path + std::string(SWORD_DATA);
	system(str.c_str());

	if(lzo_init() != LZO_E_OK) {
		printf("internal error - lzo_init() failed !!!\n");
		printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)\n");
		exit(-1);
	}

	return 0;
}

void ompt_finalize(ompt_fns_t* fns) {
	on_ompt_event_runtime_shutdown();
}

ompt_fns_t* ompt_start_tool(unsigned int omp_version,
		const char *runtime_version) {
	static ompt_fns_t ompt_fns = { &ompt_initialize, &ompt_finalize };
	return &ompt_fns;
}

}
