#include "rtl/sword_common.h"
#include "interval_tree.h"
#include "sword-race-analysis.h"
#include <boost/algorithm/string.hpp>

#define PRINT_RACE 0

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
#include <list>
#include <map>
#include <thread>

#include <boost/lockfree/queue.hpp>

struct TreeRoot {
  unsigned tid;
  rb_root *root;

  TreeRoot(int id, rb_root *r) {
    tid = id;
    root = r;
  }
};

void SaveReport(std::string filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  size_t size = races.size();
  file.write((char*) &size, sizeof(size));
  file.write(reinterpret_cast<char*>(races.data()), races.size() * sizeof(RaceInfo));
  file.close();
}

void ReportRace(uint64_t address, uint8_t rw1, uint8_t rw2, uint8_t size1, uint8_t size2, uint64_t pc1, uint64_t pc2) {
  std::size_t hash1 = 0;
  boost::hash_combine(hash1, pc1);
  boost::hash_combine(hash1, pc2);
  std::size_t hash2 = 0;
  boost::hash_combine(hash2, pc2);
  boost::hash_combine(hash2, pc1);
  rmtx.lock();
  const bool reported = (hash_races.find(hash1) != hash_races.end()) || (hash_races.find(hash2) != hash_races.end());
  rmtx.unlock();
  if(!reported) {
    rmtx.lock();
    hash_races.insert(hash1);
    hash_races.insert(hash2);
    races.push_back(RaceInfo(address, rw1, size1, pc1, rw2, size2, pc2));
    rmtx.unlock();

#if PRINT_RACE
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

void analyze_trees(bool last, unsigned t1, rb_root *tree1, unsigned t2, rb_root *tree2,
     std::vector<std::pair<interval_tree_node,interval_tree_node>> &races) {
  if(tree1 && tree2) {
    interval_tree_overlap(rmtx, t1, tree1, t2, tree2, races);
    if(!last)
      interval_tree_merge(tree1, tree2);
  }
}

void load_and_convert_file(boost::filesystem::path path, unsigned t, uint64_t fob, uint64_t foe, rb_root *interval_tree_root) {
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
          interval_tree_insert_data(interval_tree_node(it->data.access.address, it->data.access.address, it->data.access.size_type, (size_t) it->data.access.pc.num, mutex), interval_tree_root, t);
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
  bool print = false;

  if(argc < 7)
    INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--executable <path-to-executable-name> --traces-path <path-to-traces-folder> --report-path <path-to-report-folder>\n\n");

  if (!glp_config("TLS")) {
    printf("The loaded GLPK library does not support thread local memory.\n"
           "You need a version of the library configured with "
           "--enable-reentrant=yes to run this program.\n");
    exit(EXIT_FAILURE);
  }

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
    } else if (std::string(argv[i]) == "--print") {
      print = true;
    } else {
      unknown_option = argv[i++];
    }
  }

  if(!unknown_option.empty()) {
    INFO(std::cerr, "Sword Error: " << unknown_option << " is an unknown option.\nSpecify --help for usage.");
    return -1;
  }

#if PRINT_RACE
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

    // Struct to load uncompressed data from file
    std::vector<std::thread> lm_thread;
    lm_thread.reserve(traces.size());
    std::list<TreeRoot> interval_trees;
    for(std::map<unsigned, TraceInfo>::iterator th = traces.begin(); th != traces.end(); ++th) {
      rb_root *root = new rb_root();
      interval_trees.push_back(TreeRoot(th->first, root));
      lm_thread.push_back(std::thread(load_and_convert_file, dir, th->first, th->second.file_offset_begin, th->second.file_offset_end, root));
    }
    for(int k = 0; k < lm_thread.size(); k++) {
      lm_thread[k].join();
    }
    lm_thread.clear();

    if(print) {
      for(std::list<TreeRoot>::iterator it = interval_trees.begin();
          it != interval_trees.end(); it++) {
        std::ofstream out0("thread" + std::to_string(it->tid) + ".dot");
        std::streambuf *coutbuf0 = std::cout.rdbuf();
        std::cout.rdbuf(out0.rdbuf());
        interval_tree_print(it->root);
        std::cout.rdbuf(coutbuf0);
      }
    }

    std::vector<std::pair<interval_tree_node,interval_tree_node>> rep_races;
    std::list<TreeRoot>::iterator it;
    std::list<TreeRoot>::iterator del;
    bool last;
    while(interval_trees.size() > 1) {
      int lst_size = interval_trees.size();
      it = interval_trees.begin();
      while(lst_size != 1 && it != interval_trees.end()) {
        last = (interval_trees.size() == 2);
        TreeRoot tree1 = *it;
        std::advance(it, 1);
        TreeRoot tree2 = *it;
        del = it;
        std::advance(it, 1);
        interval_trees.erase(del);
        lst_size -= 2;
        lm_thread.push_back(std::thread(analyze_trees, last, tree1.tid, tree1.root, tree2.tid, tree2.root, std::ref(rep_races)));
      }
      for(int k = 0; k < lm_thread.size(); k++) {
        lm_thread[k].join();
      }
      lm_thread.clear();
    }

    if(print) {
      std::ofstream out("thread.dot");
      std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
      std::cout.rdbuf(out.rdbuf());
      interval_tree_print(interval_trees.front().root);
      std::cout.rdbuf(coutbuf);
    }

    for(std::vector<std::pair<interval_tree_node,interval_tree_node>>::iterator it = rep_races.begin(); it != rep_races.end(); ++it) {
      interval_tree_node i = std::get<0>(*it);
      interval_tree_node j = std::get<1>(*it);
      ReportRace(i.start,
                 ((AccessType) (i.size_type & 0x0F)), ((AccessType) (j.size_type & 0x0F)),
                 i.size_type >> 4,
                 j.size_type >> 4,
                 i.pc - 1, j.pc - 1);
    }

    if(races.size() > 0) {
      std::string filename = report_data.string() + "/" + "race_report_" + std::to_string(pregion);
      SaveReport(filename);
    }
  } else {
    INFO(std::cout, "Folder '" << dir << "' does not exists or it's empty. Exiting...");
  }

  exit(0);
}
