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

#define SET_SIZE 87382

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
                  FILE *file, unsigned char *buffer, size_t *file_offset_end) {

#ifdef LZO
  // LZO
  lzo_uint *out_len = (lzo_uint *) buffer;
  lzo1x_1_compress((unsigned char *) accesses->data(), size * nmemb, buffer + sizeof(lzo_uint), out_len, wrkmem);

  size_t tsize = *out_len + sizeof(lzo_uint);
  fwrite((char *) buffer, tsize, 1, file);
  *file_offset_end += tsize;
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

#define SWAP_BUFFER                                     \
  if(__sword_accesses__ == __sword_accesses1__) {       \
    __sword_accesses__ == __sword_accesses2__;          \
  } else {                                              \
    __sword_accesses__ == __sword_accesses1__;          \
  }

#define DUMP_TO_FILE                                                    \
  __sword_idx__++;                                                      \
  if(__sword_idx__ == NUM_OF_ACCESSES)	{                               \
    fut.wait();                                                         \
    fut = std::async(dump_to_file, __sword_accesses__,                  \
                     sizeof(TraceItem), NUM_OF_ACCESSES, __sword_datafile__, \
                     out, &__sword_file_offset_end__); \
    __sword_idx__ = 0;                                                  \
    set.clear();                                                        \
    SWAP_BUFFER                                                         \
      }

#define DUMPNOCHECK_TO_FILE                                             \
  if(__sword_idx__ > 0) {                                               \
    fut.wait();                                                         \
    fut = std::async(dump_to_file, __sword_accesses__,                  \
                     sizeof(TraceItem), __sword_idx__, __sword_datafile__, \
                     out, &__sword_file_offset_end__); \
    __sword_idx__ = 0;                                                            \
    set.clear();                                                        \
    SWAP_BUFFER                                                         \
      }

#define SAVE_ACCESS(asize, atype)                                       \
  TraceItem item = TraceItem(data_access, Access(asize,                 \
                                                 atype, (size_t) addr, CALLERPC)); \
  if(set.check_insert(hash_value(item))) {                              \
    (*__sword_accesses__)[__sword_idx__] = item;                        \
    DUMP_TO_FILE                                                        \
      }

extern "C" {

#include "sword_interface.inl"

  static inline ParallelData *ToParallelData(ompt_data_t *parallel_data) {
    return reinterpret_cast<ParallelData*>(parallel_data->ptr);
  }

  static void on_ompt_callback_thread_begin(ompt_thread_type_t thread_type,
                                            ompt_data_t *thread_data) {
    __sword_tid__ = my_next_id();

    __sword_accesses1__ = new std::vector<TraceItem>(NUM_OF_ACCESSES);
    __sword_accesses2__ = new std::vector<TraceItem>(NUM_OF_ACCESSES);
    set.reserve(SET_SIZE);
    __sword_accesses__ = __sword_accesses1__;
    out = (unsigned char *) malloc(OUT_LEN);

    // Create datafile
    std::string filename = sword_flags->traces_path + "/datafile_" + std::to_string(__sword_tid__);
    __sword_datafile__ = fopen(filename.c_str(), "ab");
    if (!__sword_datafile__) {
      INFO(std::cerr, "SWORD: Error opening datafile: " << filename << " - " << strerror(errno) << ".");
      exit(-1);
    }
    __sword_file_offset_begin__ = 0;
    __sword_file_offset_end__ = 0;

    // Create metafile
    filename = sword_flags->traces_path + "/metafile_" + std::to_string(__sword_tid__);
    __sword_metafile__ = fopen(filename.c_str(), "a");
    if (!__sword_metafile__) {
      INFO(std::cerr, "SWORD: Error opening metafile: " << filename << " - " << strerror(errno) << ".");
      exit(-1);
    }
    // fprintf(metafile, "#parallel_id,parent_parallel_id,bid,offset,span,level,file_offset_begin,file_offset_end\n");
    __sword_offset__ = 0;
    __sword_span__ = 0;

    fut = std::async(dummy);
  }

  static void on_ompt_callback_thread_end(ompt_data_t *thread_data)
  {
    fclose(__sword_datafile__);
    fclose(__sword_metafile__);
  }

  static void on_ompt_callback_parallel_begin(ompt_data_t *parent_task_data,
                                              const ompt_frame_t *parent_task_frame,
                                              ompt_data_t* parallel_data,
                                              uint32_t requested_team_size,
                                              ompt_invoker_t invoker,
                                              const void *codeptr_ra) {
    __sword_status__++;

    if(__sword_status__ == 1) {
      ompt_id_t pid = ompt_get_unique_id();
      parallel_data->ptr = new ParallelData(pid, 0, __sword_status__, omp_get_thread_num(), requested_team_size);
    } else {
      ompt_id_t pid = ompt_get_unique_id();
      ParallelData *task_data = ToParallelData(parent_task_data);
      ParallelData *par_data = new ParallelData(pid, task_data->parallel_id, __sword_status__, omp_get_thread_num(), requested_team_size);
      if(__sword_span__ != 0) {
        __sword_offset__ += __sword_span__;
        __sword_span__ = requested_team_size;
      } else {
        __sword_offset__ = omp_get_thread_num() + requested_team_size;
        __sword_span__ = requested_team_size;
      }
      parallel_data->ptr = par_data;
    }
  }

  static void on_ompt_callback_parallel_end(ompt_data_t *parallel_data,
                                            ompt_data_t *task_data,
                                            ompt_invoker_t invoker,
                                            const void *codeptr_ra) {
    ParallelData *par_data = ToParallelData(parallel_data);
    delete par_data;
  }

  static void on_ompt_callback_implicit_task(ompt_scope_endpoint_t endpoint,
                                             ompt_data_t *parallel_data,
                                             ompt_data_t *task_data,
                                             unsigned int team_size,
                                             unsigned int thread_num) {

    if(endpoint == ompt_scope_begin) {
      task_data->ptr = new ParallelData(ToParallelData(parallel_data));
      ParallelData *par_data = ToParallelData(task_data);
      __sword_status__ = par_data->level;

      if(__sword_status__ == 1) {
        __sword_bid__ = 0;

        DUMPNOCHECK_TO_FILE
          fut.wait();
        fprintf(__sword_metafile__, "%lu,%lu,%lu,%u,%u,%d,%lu,%lu\n", par_data->parallel_id, par_data->parent_parallel_id, __sword_bid__, omp_get_thread_num(), team_size, par_data->level, __sword_file_offset_begin__, __sword_file_offset_end__);
        if(__sword_span__ == 0) {
          __sword_offset__ = omp_get_thread_num();
          __sword_span__ = team_size;
        }

        __sword_file_offset_begin__ = __sword_file_offset_end__;
      }
    } else { // ompt_scope_end
      ParallelData *tsk_data = ToParallelData(task_data);
      assert(tsk_data->freed == 0 && "Implicit task end should only be called once!");
      tsk_data->freed = 1;
      delete tsk_data;
      __sword_status__--;
    }
  }

  static void on_ompt_callback_sync_region(ompt_sync_region_kind_t kind,
                                           ompt_scope_endpoint_t endpoint,
                                           ompt_data_t *parallel_data,
                                           ompt_data_t *task_data,
                                           const void *codeptr_ra) {
    if(endpoint == ompt_scope_begin) {
      ParallelData *par_data = (ParallelData *) task_data->ptr;
      DUMPNOCHECK_TO_FILE
        fut.wait();
      fprintf(__sword_metafile__, "%lu,%lu,%lu,%u,%u,%d,%lu,%lu\n", par_data->parallel_id, par_data->parent_parallel_id, __sword_bid__, __sword_offset__, __sword_span__, par_data->level, __sword_file_offset_begin__, __sword_file_offset_end__);
      __sword_file_offset_begin__ = __sword_file_offset_end__;
      __sword_bid__++;
    }
  }

  static void on_ompt_callback_mutex_acquired(ompt_mutex_kind_t kind,
                                              ompt_wait_id_t wait_id,
                                              const void *codeptr_ra) {
    (*__sword_accesses__)[__sword_idx__] = TraceItem(mutex_acquired, MutexRegion(kind, wait_id));
    DUMP_TO_FILE
      }

  static void on_ompt_callback_mutex_released(ompt_mutex_kind_t kind,
                                              ompt_wait_id_t wait_id,
                                              const void *codeptr_ra) {
    (*__sword_accesses__)[__sword_idx__] = TraceItem(mutex_released, MutexRegion(kind, wait_id));
    DUMP_TO_FILE
      }

#define register_callback_t(name, type)                       \
do {                                                          \
  type f_##name = &on_##name;                                 \
  if (ompt_set_callback(name, (ompt_callback_t) f_##name) ==  \
      ompt_set_never)                                         \
    printf("0: Could not register callback '" #name "'\n");   \
} while(0)

#define register_callback(name) register_callback_t(name, name##_t)

  int ompt_initialize(ompt_function_lookup_t lookup,
                      ompt_data_t* tool_data) {
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
    register_callback_t(ompt_callback_mutex_acquired, ompt_callback_mutex_t);
    register_callback_t(ompt_callback_mutex_released, ompt_callback_mutex_t);

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

    return 1;
  }

  void ompt_finalize(ompt_data_t *tool_data) {
    fflush(NULL);
  }

ompt_start_tool_result_t* ompt_start_tool(
  unsigned int omp_version,
  const char *runtime_version) {
    static ompt_start_tool_result_t ompt_start_tool_result = {&ompt_initialize, &ompt_finalize, {0}};
    return &ompt_start_tool_result;
  }

}
