//===-- swordrt_rtl.cc -------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Sword/SwordRT, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#include "swordrt_rtl.h"
#include <zlib.h>
#include <assert.h>
#include <fstream>

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

static ompt_get_task_data_t ompt_get_task_data;
static ompt_get_task_frame_t ompt_get_task_frame;
static ompt_get_thread_data_t ompt_get_thread_data;
static ompt_get_parallel_data_t ompt_get_parallel_data;
static ompt_get_unique_id_t ompt_get_unique_id;

#include <sstream>
#include <cmath>
#include "swordrt_tsan_interface.h"

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

	// deflateInit(&strm, Z_BEST_COMPRESSION);
	// deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	deflateInit(&strm, Z_BEST_SPEED);
	// deflateInit(&strm, Z_NO_COMPRESSION);
	// deflateInit2(&strm, Z_BEST_SPEED, Z_DEFLATED, 10, 8, Z_HUFFMAN_ONLY);

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

#define SWAP_BUFFER															\
		if(accesses_heap == accesses_heap1) {								\
			accesses_heap == accesses_heap2;								\
		} else {															\
			accesses_heap == accesses_heap1;								\
		}

bool dump_to_file(AccessInfo *accesses, size_t size, size_t nmemb,
		FILE *file, unsigned char *buffer, size_t *offset) {

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

	/*
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
	*/

	//	size_t ret = fwrite((char *) accesses, size * nmemb, 1, file); // Write plain

	return true;
}

//#define SAVE_ACCESS(dsize, type)
#define SAVE_ACCESS(dsize, type)											\
			accesses_heap[idxs_heap].setData(data, (size_t) addr,			\
				dsize, type, CALLERPC); 									\
			idxs_heap++;													\
			if(idxs_heap == NUM_OF_ACCESSES)	{							\
				bool ret = fut.get();										\
				fut = std::async(dump_to_file, accesses_heap,				\
						sizeof(AccessInfo), NUM_OF_ACCESSES, datafile, 		\
						out, &offset);	  									\
				idxs_heap = 0;												\
				SWAP_BUFFER													\
			}

extern "C" {

#include "swordrt_interface.inl"

static void on_ompt_callback_thread_begin(ompt_thread_type_t thread_type,
		ompt_thread_data_t *thread_data) {
	tid = omp_get_thread_num();

	datafile = fopen(std::string(std::string(SWORD_DATA) + "/threadtrace_" + std::to_string(tid)).c_str(), "ab");
	if (!datafile) {
		INFO(std::cerr, "SWORD: Error opening file.");
		exit(-1);
	}
	accesses_heap1 = (AccessInfo *) malloc(BLOCK_SIZE);
	accesses_heap2 = (AccessInfo *) malloc(BLOCK_SIZE);
	accesses_heap = accesses_heap1;
	out = (unsigned char *) malloc(OUT_LEN);

	//buffer = (char *) malloc(MB_LIMIT);
	fut = std::async(dump_to_file, accesses_heap, 0, 0, datafile, out, &offset);
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
	//if(__swordomp_status__ == 0) {
	//INFO(std::cout, "INFO: " << parallel_data->value);
	//}
}

static void on_ompt_callback_parallel_end(ompt_data_t *parallel_data,
		ompt_task_data_t *task_data,
		ompt_invoker_t invoker,
		const void *codeptr_ra) {

}

static void on_ompt_event_runtime_shutdown(void)
{
  printf("%d: ompt_event_runtime_shutdown\n", omp_get_thread_num());
}

#define register_callback(name) {                                           \
	if (ompt_set_callback(name, (ompt_callback_t)&on_##name) == 			\
			ompt_has_event_no_callback)                             		\
			printf("0: Could not register callback '" #name "'\n");   		\
}

int ompt_initialize(ompt_function_lookup_t lookup,
		ompt_fns_t* fns) {
	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	ompt_get_task_data = (ompt_get_task_data_t) lookup("ompt_get_task_data");
	ompt_get_task_frame = (ompt_get_task_frame_t) lookup("ompt_get_task_frame");
	ompt_get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
	ompt_get_parallel_data = (ompt_get_parallel_data_t) lookup("ompt_get_parallel_data");
	ompt_get_unique_id = (ompt_get_unique_id_t) lookup("ompt_get_unique_id");

    register_callback(ompt_callback_thread_begin);
    register_callback(ompt_callback_parallel_begin);

	std::string str = "rm -rf " + std::string(SWORD_DATA);
	system(str.c_str());
	str = "mkdir " + std::string(SWORD_DATA);
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
