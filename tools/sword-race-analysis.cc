#include "sword_intervaltree.h"
#include "sword-race-analysis.h"
#include <boost/algorithm/string.hpp>

#ifdef SNAPPY
#include "../rtl/snappy.h"
#endif

#ifdef LZ4
#include "../rtl/lz4.h"
#endif

#include <sched.h>
#include <stdio.h>
#include <unistd.h>

#include <atomic>
#include <algorithm>
#include <map>
#include <thread>

#define PRINT 0

void SaveReport(std::string filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  size_t size = races.size();
  file.write((char*) &size, sizeof(size));
  file.write(reinterpret_cast<char*>(races.data()), races.size() * sizeof(RaceInfo));
  file.close();

  //	std::string race1 = "";
  //	std::string race2 = "";
  //
  //	{
  //		std::string command = shell_path + " -c '" + symbolizer_path + " -pretty-print" + " < <(echo \"" + executable + " " + std::to_string(race.pc1) + "\")'";
  //		execute_command(command.c_str(), &race1, 2);
  //	}
  //
  //	{
  //		std::string command = shell_path + " -c '" + symbolizer_path + " -pretty-print" + " < <(echo \"" + executable + " " + std::to_string(race.pc2) + "\")'";
  //		execute_command(command.c_str(), &race2, 2);
  //	}
  //
  //	INFO(std::cerr, "--------------------------------------------------");
  //	INFO(std::cerr, "WARNING: SWORD: data race (program=" << executable << ")");
  //	INFO(std::cerr, "Two different threads made the following accesses:");
  //	INFO(std::cerr, AccessTypeStrings[race.rw1] << " of size " << std::dec << (1 << race.size1) << " at 0x" << std::hex << race.address << " in " << race1);
  //	INFO(std::cerr, AccessTypeStrings[race.rw2] << " of size " << std::dec << (1 << race.size2) << " at 0x" << std::hex << race.address << " in " << race2);
  //	INFO(std::cerr, "--------------------------------------------------");
  //	INFO(std::cerr, "");
}

void ReportRace(uint64_t address, uint8_t rw1, uint8_t rw2, uint8_t size1, uint8_t size2, uint64_t pc1, uint64_t pc2) {
  std::size_t hash = 0;
  boost::hash_combine(hash, pc1);
  boost::hash_combine(hash, pc2);
  rmtx.lock();
  const bool reported = hash_races.find(hash) != hash_races.end();
  rmtx.unlock();
  if(!reported) {
    rmtx.lock();
    hash_races.insert(hash);
    rmtx.unlock();
    races.push_back(RaceInfo(address, rw1, size1, pc1, rw2, size2, pc2));

#if PRINT
    std::string race1 = "";
    std::string race2 = "";

    {
      std::string command = shell_path + " -c '" + symbolizer_path + " -pretty-print" + " < <(echo \"" + executable + " " + std::to_string(pc1) + "\")'";
      execute_command(command.c_str(), &race1, 2);
    }

    {
      std::string command = shell_path + " -c '" + symbolizer_path + " -pretty-print" + " < <(echo \"" + executable + " " + std::to_string(pc2) + "\")'";
      execute_command(command.c_str(), &race2, 2);
    }

    INFO(std::cerr, "--------------------------------------------------");
    INFO(std::cerr, "WARNING: SWORD: data race (program=" << executable << ")");
    INFO(std::cerr, AccessTypeStrings[rw1] << " of size " << std::dec << (1 << size1) << " at 0x" << std::hex << address << " in " << race1);
    INFO(std::cerr, AccessTypeStrings[rw2] << " of size " << std::dec << (1 << size2) << " at 0x" << std::hex << address << " in " << race2);
    INFO(std::cerr, "--------------------------------------------------");
    INFO(std::cerr, "");
#endif
  }
}

unsigned long long getTotalSystemMemory()
{
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

void analyze_traces(unsigned t1, unsigned t2, std::vector<IntervalTree*> &interval_buffers, std::atomic<int> &available_threads) {
  std::vector<std::pair<Interval,Interval>> res;
  //	INFO(std::cout, "Analyzing pair (" << t1 << "," << t2 << ").");

  if(t1 != t2) {
    if(interval_buffers.size() > 0) {
      IntervalTree::intersectIntervals(interval_buffers[t1]->root, interval_buffers[t2]->root, res);
    }

    for(std::vector<std::pair<Interval, Interval>>::iterator it = res.begin(); it != res.end(); ++it) {
      Interval i = std::get<0>(*it);
      Interval j = std::get<1>(*it);
      ReportRace(i.address,
                 i.getAccessType(), j.getAccessType(),
                 i.getAccessSize(),
                 j.getAccessSize(),
                 i.pc.num - 1, j.pc.num - 1);
    }
  }

  available_threads++;
}

void load_and_convert_file(boost::filesystem::path path, unsigned t, uint64_t fob, uint64_t foe, IntervalTree *interval_buffer) {
  std::string filename(path.string() + "/datafile_" + std::to_string(t));
  uint64_t filesize = foe - fob;
  uint64_t out_len;
  uint64_t new_len;

  if(filesize > 0) {
    FILE *datafile = fopen(filename.c_str(), "r");
    if (!datafile) {
      INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
      exit(-1);
    }

    unsigned char compressed_buffer[OUT_LEN];
    // TraceItem *uncompressed_buffer = (TraceItem *) malloc(BLOCK_SIZE);
    TraceItem uncompressed_buffer[NUM_OF_ACCESSES];

    uint64_t block_size;
    fseek(datafile, fob, SEEK_SET);
    size_t size = fread(&block_size, sizeof(uint64_t), 1, datafile);
    unsigned neof = 0;

    if(block_size + sizeof(uint64_t) < filesize) {
      block_size += sizeof(uint64_t);
      neof = 1;
    }

    size_t total_size = sizeof(uint64_t);
    std::vector<TraceItem> file_buffer;
    IntervalTree intervals;
    // size_t uncompressed_size = 0;
    while(total_size < filesize) {
      size_t ret = fread(compressed_buffer, 1, block_size, datafile);
      total_size += block_size;
      if(ret != block_size) {
        printf("Error reading data from the file\n");
        exit(-1);
      }

#if defined(LZO)
      lzo_uint r = lzo1x_decompress(compressed_buffer, block_size - (sizeof(uint64_t) * neof), (unsigned char *) uncompressed_buffer, &new_len, NULL);
      if (r != LZO_E_OK) {
        /* this should NEVER happen */
        printf("internal error - decompression failed: %lu\n", r);
        exit(-1);
      }
#elif defined(SNAPPY)
      snappy::GetUncompressedLength((char *) compressed_buffer, block_size - (sizeof(uint64_t) * neof), &new_len);
      bool r = snappy::RawUncompress((char *) compressed_buffer, block_size - (sizeof(uint64_t) * neof), (char *) uncompressed_buffer);
      if (!r) {
        /* this should NEVER happen */
        printf("internal error - decompression failed\n");
        exit(-1);
      }
#elif defined(LZ4)
      new_len = LZ4_decompress_safe((char *) compressed_buffer, (char *) uncompressed_buffer, block_size - (sizeof(uint64_t) * neof), BLOCK_SIZE);
#else
#endif

      // uncompressed_size += new_len;

      file_buffer.insert(file_buffer.end(), uncompressed_buffer, uncompressed_buffer + (new_len / sizeof(TraceItem)));

      std::set<size_t> mutex;
      for(std::vector<TraceItem>::const_iterator it = file_buffer.begin(); it != file_buffer.end(); ++it) {
        switch(it->getType()) {
        case data_access:
          interval_buffer->root = interval_buffer->insertNode(interval_buffer->root, it->data.access, mutex);
          break;
        case mutex_acquired:
          mutex.insert(it->data.mutex_region.getWaitId());
          break;
        case mutex_released:
          mutex.erase(it->data.mutex_region.getWaitId());
          break;
        default:
          break;
        }
      }

      file_buffer.clear();

      memcpy(&block_size, compressed_buffer + block_size - sizeof(uint64_t), sizeof(uint64_t));
      if(total_size + block_size + sizeof(uint64_t) < filesize) {
        block_size += sizeof(uint64_t);
        neof = 1;
      } else {
        neof = 0;
      }
    }

    // INFO(std::cout, "Total Size: " << uncompressed_size);

    // free(uncompressed_buffer);
    fclose(datafile);
  }
}

int main(int argc, char **argv) {
  std::string unknown_option = "";

  //	if(argc < 7)
  //		INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--executable <path-to-executable-name> --traces-path <path-to-traces-folder> --report-path <path-to-report-folder>\n\n");
  //		INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--executable <path-to-executable-name> --report-path <path-to-report-folder> --traces-path <path-to-traces-folder>\n\n");

  for(int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--help") {
      INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--executable <path-to-executable-name> --traces-path <path-to-traces-folder> --report-path <path-to-report-folder>\n\n");
      return 0;
    } else if (std::string(argv[i]) == "--report-path") {
      if (i + 1 < argc) {
        report_data += argv[++i];
      } else {
        INFO(std::cerr, "--report-path option requires one argument.");
        return -1;
      }
    } else if (std::string(argv[i]) == "--traces-path") {
      if (i + 1 < argc) {
        traces_data += argv[++i];
      } else {
        INFO(std::cerr, "--traces-path option requires one argument.");
        return -1;
      }
    } else if (std::string(argv[i]) == "--executable") {
      if (i + 1 < argc) {
        executable += argv[++i];
      } else {
        INFO(std::cerr, "--executable option requires one argument.");
        return -1;
      }
    } else if (std::string(argv[i]) == "--pregion") {
      if (i + 1 < argc) {
        pregion = std::strtoull(argv[++i],NULL,0);
      } else {
        INFO(std::cerr, "--pregion option requires one argument.");
        return -1;
      }
    } else if (std::string(argv[i]) == "--bid") {
      if (i + 1 < argc) {
        barrier_id = std::strtoull(argv[++i],NULL,0);
      } else {
        INFO(std::cerr, "--bid option requires one argument.");
        return -1;
      }
    } else if (std::string(argv[i]) == "--nested") {
      if (i + 1 < argc) {
        nested += argv[++i];
      } else {
        INFO(std::cerr, "--nested option requires one argument.");
        return -1;
      }
    } else {
      unknown_option = argv[i++];
    }
  }

  if(!unknown_option.empty()) {
    INFO(std::cerr, "Sword Error: " << unknown_option << " is an unknown option.\nSpecify --help for usage.");
    return -1;
  }

#if PRINT
  // Look for shell
  execute_command(GET_SHELL, &shell_path);
  // Look for shell

  // Look for llvm-symbolizer
  execute_command(GET_SYMBOLIZER, &symbolizer_path);
  // Look for llvm-symbolizer
#endif

  // Get cores info
  // unsigned num_threads = sysconf(_SC_NPROCESSORS_ONLN);
  unsigned num_threads = std::thread::hardware_concurrency();
  // Get cores info

#ifdef LZO
  // Initialize decompressor
  if(lzo_init() != LZO_E_OK) {
    INFO(std::cerr, "Internal error - lzo_init() failed!");
    INFO(std::cerr, "This usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics.");
    exit(-1);
  }
  // Initialize decompressor
#endif

  std::string dir = traces_data.string();
  std::map<unsigned, TraceInfo> traces;

  std::unordered_map<unsigned, std::set<unsigned>> creators_threads;
  std::unordered_map<unsigned, std::set<unsigned>> schedule_threads;

  hash_races.clear();
  races.clear();

  // Iterate files within folder and create map of barriers intervals and list of threads within the barrier interval
  boost::filesystem::directory_iterator end_it;
  boost::filesystem::directory_iterator begin_it(dir);
  if(boost::filesystem::is_directory(dir) && (begin_it != end_it)) {
    if ("." != boost::filesystem::path(dir).filename())
      dir += boost::filesystem::path::preferred_separator;
    for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dir), {})) {
      if (entry.path().filename().string().find("metafile_") != std::string::npos) {
        unsigned tid;
        sscanf(entry.path().filename().string().c_str(), "metafile_%d", &tid);
        // INFO(std::cout, entry.path().filename().string());
        std::string filename(dir + entry.path().filename().string());
        std::ifstream file(filename);
        std::string str;
        while (std::getline(file, str))	{
          uint64_t pid, ppid, bid, obegin, oend;
          int level;
          unsigned span, offset;
          sscanf(str.c_str(), "%lu,%lu,%lu,%u,%u,%d,%lu,%lu\n", &pid, &ppid, &bid, &offset, &span, &level, &obegin, &oend);
          if((pregion == pid) && (barrier_id == bid)) {
            traces[tid] = TraceInfo(obegin, oend);
          }
        }
      }
    }

    // Create thread pairs for comparisons
    std::set<std::pair<unsigned,unsigned>> thread_pairs;
    for (std::map<unsigned, TraceInfo>::iterator it = traces.begin(); it != traces.end(); ++it) {
      for (std::map<unsigned, TraceInfo>::iterator it2 = std::next(it, 1); it2 != traces.end(); ++it2) {
        thread_pairs.insert(std::make_pair(it->first, it2->first));
      }
    }
    // Create thread pairs for comparisons

    // Struct to load uncompressed data from file
    std::vector<std::thread> lm_thread;
    lm_thread.reserve(traces.size());
    std::vector<IntervalTree*> interval_buffers;
    interval_buffers.resize(traces.size());
    for (std::map<unsigned, TraceInfo>::iterator th = traces.begin(); th != traces.end(); ++th) {
      interval_buffers[th->first] = new IntervalTree();
      lm_thread.push_back(std::thread(load_and_convert_file, dir, th->first, th->second.file_offset_begin, th->second.file_offset_end, interval_buffers[th->first]));
    }
    for(int k = 0; k < lm_thread.size(); k++) {
      lm_thread[k].join();
    }
    lm_thread.clear();

    // Load data into memory of all the threads in given barrier interval, if it fits in memory,
    // data are compressed so not sure how to check if everything will fit in memory

    // INFO(std::cout, "T0");
    // interval_buffers[0]->printTree(interval_buffers[0]->root);
    // interval_buffers[0]->bst_print_dot(interval_buffers[0]->root);
    // INFO(std::cout, "T1");
    // interval_buffers[1]->printTree(interval_buffers[1]->root);
    // interval_buffers[1]->bst_print_dot(interval_buffers[1]->root);

    // Now we can start analyzing the pairs
    std::atomic<int> available_threads;
    std::vector<std::thread> thread_list;
    available_threads = num_threads;
    for(std::set<std::pair<unsigned,unsigned>>::const_iterator p = thread_pairs.begin(); p != thread_pairs.end(); ++p) {
      while(!available_threads) { } // { usleep(1000); }
      available_threads--;

      // Create thread
      // INFO(std::cout, "Analyzing pair: " << p->first << "," << p->second);
      thread_list.push_back(std::thread(analyze_traces, p->first, p->second, std::ref(interval_buffers), std::ref(available_threads)));
    }
    for(std::vector<std::thread>::iterator th = thread_list.begin(); th != thread_list.end(); th++) {
      th->join();
    }
    thread_list.clear();
    if(races.size() > 0) {
      std::string filename = report_data.string() + "/" + "race_report_" + std::to_string(pregion);
      SaveReport(filename);
    }
  } else {
    INFO(std::cout, "Folder '" << dir << "' does not exists or it's empty. Exiting...");
  }
  //    }

  return 0;
}
