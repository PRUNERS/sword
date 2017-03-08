#include "sword-race-analysis.h"

#define PRINT 0

void execute_command(const char *cmd, std::string *buf, unsigned carrier = 1) {
    std::array<char, 128> buffer;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
        	*buf += buffer.data();
    }
    buf->erase(buf->size() - carrier);
}

void ReportRace(unsigned t1, unsigned t2, uint64_t address, uint8_t rw1, uint8_t rw2, uint8_t size1, uint8_t size2, uint64_t pc1, uint64_t pc2) {
	    std::size_t hash = 0;
	    boost::hash_combine(hash, pc1);
	    boost::hash_combine(hash, pc2);
	    rmtx.lock();
	    std::vector<size_t>::iterator it = std::find(hash_races.begin(), hash_races.end(), hash);
	    rmtx.unlock();
	    if(it == hash_races.end()) {
	    	hash_races.push_back(hash);
	    	races.push_back(RaceInfo(rw1, size1, pc1, rw2, size2, pc2));

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
	    	INFO(std::cerr, "WARNING: Archer: array data race (program=" << executable << ")");
	    	INFO(std::cerr, AccessTypeStrings[rw1] << " of size " << std::dec << (1 << size1) << " at 0x" << std::hex << address << " by thread T" << std::dec << t1 << " in " << race1);
	    	INFO(std::cerr, AccessTypeStrings[rw2] << " of size " << std::dec << (1 << size2) << " at 0x" << std::hex << address << " by thread T" << std::dec << t2 << " in " << race2);
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

#define RACE_CHECK(t1, t2) \
	(t1->data.access.getAddress() == t2->data.access.getAddress()) &&		\
	((t1->data.access.getAccessType() == unsafe_write) ||					\
	 (t2->data.access.getAccessType() == unsafe_write) ||					\
	 ((t1->data.access.getAccessType() == atomic_write) &&					\
	  (t2->data.access.getAccessType() == unsafe_read)) ||					\
	 ((t2->data.access.getAccessType() == atomic_write) &&					\
	  (t1->data.access.getAccessType() == unsafe_read)))

void analyze_traces(unsigned bid, unsigned t1, unsigned t2, std::vector<std::vector<TraceItem>> &file_buffers, std::atomic<int> &available_threads) {
//	INFO(std::cout, "Analyzing pair (" << t1 << "," << t2 << ").");

	if(file_buffers.size() > 0) {
		for(std::vector<TraceItem>::iterator i = file_buffers[t1].begin() ; i != file_buffers[t1].end(); ++i) {
			switch(i->getType()) {
			case data_access:
				for(std::vector<TraceItem>::iterator j = file_buffers[t2].begin(); j != file_buffers[t2].end(); ++j) {
					switch(j->getType()) {
					case data_access:
						if(RACE_CHECK(i,j)) {
							ReportRace(t1, t2, i->data.access.getAddress(),
									i->data.access.getAccessType(), j->data.access.getAccessType(),
									i->data.access.getAccessSize(),
									j->data.access.getAccessSize(),
									i->data.access.getPC() - 1, j->data.access.getPC() - 1);
						}
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
		}
	}

	available_threads++;
}

void load_file(boost::filesystem::path path, unsigned bid, unsigned t, std::vector<TraceItem> &file_buffers) {
	std::string filename(path.string() + "/threadtrace_" + std::to_string(t) + "_" + std::to_string(bid));
	uint64_t filesize = boost::filesystem::file_size(filename);
    lzo_uint out_len;
    lzo_uint new_len;

    FILE *datafile = fopen(filename.c_str(), "r");
	if (!datafile) {
		INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
		exit(-1);
	}

	unsigned char compressed_buffer[OUT_LEN];
	TraceItem *uncompressed_buffer = (TraceItem *) malloc(BLOCK_SIZE);

	lzo_uint block_size;
	size_t size = fread(&block_size, sizeof(lzo_uint), 1, datafile);
	unsigned neof = 0;

	if(block_size + sizeof(lzo_uint) < filesize) {
		block_size += sizeof(lzo_uint);
		neof = 1;
	}

	size_t total_size = sizeof(lzo_uint);

	while(total_size < filesize) {
		size_t ret = fread(compressed_buffer, 1, block_size, datafile);
		total_size += block_size;
		if(ret != block_size) {
	        printf("Error reading data from the file\n");
	        exit(-1);
		}

	    lzo_uint r = lzo1x_decompress(compressed_buffer, block_size - (sizeof(lzo_uint) * neof), (unsigned char *) uncompressed_buffer, &new_len, NULL);
	    if (r != LZO_E_OK) {
	        /* this should NEVER happen */
	        printf("internal error - decompression failed: %lu\n", r);
	        exit(-1);
	    }

		file_buffers.insert(file_buffers.end(), uncompressed_buffer, uncompressed_buffer + (new_len / sizeof(TraceItem)));

		memcpy(&block_size, compressed_buffer + block_size - sizeof(lzo_uint), sizeof(lzo_uint));
		if(total_size + block_size + sizeof(lzo_uint) < filesize) {
			block_size += sizeof(lzo_uint);
			neof = 1;
		} else {
			neof = 0;
		}
	}

//	for(int i = 0; i < file_buffers[t].size(); i++) {
//		if(file_buffers[t][i].getType() == data_access)
//			INFO(std::cout, t << ": 0x" << std::hex << file_buffers[t][i].data.access.getAddress() << ":" << file_buffers[t][i].data.access.getPC());
//	}

	// INFO(std::cout, "End of file: " << t);
	free(uncompressed_buffer);
    fclose(datafile);
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
		} else {
			unknown_option = argv[i++];
		}
	}

	if(!unknown_option.empty()) {
		INFO(std::cerr, "Sword Error: " << unknown_option << " is an unknown option.\nSpecify --help for usage.");
		return -1;
	}

	/* Checks already done in Python script
	try {
		// Output folder
		if(report_data.string().empty()) {
			INFO(std::cout, "Using default report folder: ./" << SWORD_REPORT);
		} else if(boost::filesystem::is_directory(report_data)) {
			INFO(std::cerr, "The path '" << report_data.string() << "' is not valid.");
			return -1;
		}

		report_data.append(SWORD_REPORT);
		if(boost::filesystem::is_directory(report_data)) {
			INFO(std::cout, "Found existing report folder, please delete or rename it before proceeding with analysis.");
			return -1;
		}
		if(!boost::filesystem::create_directory(report_data)) {
			INFO(std::cerr, "Unable to create destination directory " << report_data.string());
			return -1;
		}

		// Input folder
		traces_data.append(SWORD_DATA);
		if(!boost::filesystem::is_directory(traces_data)) {
			INFO(std::cerr, "The traces folder '" << traces_data.string() << "' does not exists.\nPlease specify the correct path with the option '--traces-path <path-to-traces-folder>'.");
			return -1;
		}

		// Executable check
		if (!boost::filesystem::exists(executable)) {
			INFO(std::cerr, "The executable '" << executable << "' does not exists.\nPlease specify the correct path and name for the executable.");
			return -1;
		}
	} catch( boost::filesystem::filesystem_error const & e) {
        INFO(std::cerr, e.what());
        return false;
    }
    */

	// Look for shell
	execute_command(GET_SHELL, &shell_path);
    // Look for shell

	// Look for llvm-symbolizer
	execute_command(GET_SYMBOLIZER, &symbolizer_path);
    // Look for llvm-symbolizer

	// Get cores info
	unsigned num_threads = sysconf(_SC_NPROCESSORS_ONLN);
	// Get cores info

	// Initialize decompressor
	if(lzo_init() != LZO_E_OK) {
		INFO(std::cerr, "Internal error - lzo_init() failed!");
		INFO(std::cerr, "This usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics.");
		exit(-1);
	}
	// Initialize decompressor

	// Get list of folders (parallel region) inside traces folder
//	std::vector<boost::filesystem::path> dir_list;
//	copy(boost::filesystem::directory_iterator(traces_data), boost::filesystem::directory_iterator(), std::back_inserter(dir_list));
	// Sort folder
//	std::sort(dir_list.begin(), dir_list.end());
	// Reverse order
	// std::sort(dir_list.rbegin(), dir_list.rend());
	// Sort folder
	// Get list of folders (parallel region) inside traces folder

	// Ready to iterate folders (outer parallel regions) in traces folder
//    for(auto& dir : dir_list) {
	std::string dir = traces_data.string();
	std::map<unsigned, TraceInfo> traces;

	hash_races.clear();
	races.clear();

	// Iterate files within folder and create map of barriers intervals and list of threads within the barrier interval
  	boost::filesystem::directory_iterator end_it;
	boost::filesystem::directory_iterator begin_it(dir);
	if(boost::filesystem::is_directory(dir) && (begin_it != end_it)) {
		for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dir), {})) {
			if (entry.path().filename().string().find("threadtrace_") != std::string::npos) {
				unsigned bid;
				unsigned tid;
				sscanf(entry.path().filename().string().c_str(), "threadtrace_%d_%d", &tid, &bid);
				traces[bid].trace_size += boost::filesystem::file_size(entry.path());
				traces[bid].thread_id.push_back(tid);
			}
		}
		// Iterate files within folder and create maps of barriers intervals and list of threads within th barrier interval

		// Iterate barrier intervals
		// it->first: bid
		// it->second.trace_size: total size of thread traces
		// it->second.thread_id: array of thread ids
		for(std::map<unsigned, TraceInfo>::const_iterator it = traces.begin(); it != traces.end(); ++it) {
#if PRINT
			INFO(std::cout, "Parallel region: " << dir << " - Barrier: " << it->first);
#endif

			// Sort list of thread id
			std::sort(traces[it->first].thread_id.begin(), traces[it->first].thread_id.end());

			// Create thread pairs for comparisons
			std::vector<std::pair<unsigned,unsigned>> thread_pairs;
			unsigned length = it->second.thread_id.size();
			unsigned len = length / 2;
			if(length % 2 == 1) {
				for(int i = 0; i < length; i++) {
					for(int k = i + 1; k < i + len + 1; k++) {
						thread_pairs.push_back(std::make_pair(it->second.thread_id[i], it->second.thread_id[i]));
					}
				}
			} else {
				for(int i = 0; i < length; i++) {
					unsigned ub = 0;
					if(i < len)
						ub = i + len + 1;
					else
						ub = i + len;
					for(int k = i + 1; k < ub; k++) {
						if(it->second.thread_id[i] < it->second.thread_id[k % length])
							thread_pairs.push_back(std::make_pair(it->second.thread_id[i], it->second.thread_id[k % length]));
						else
							thread_pairs.push_back(std::make_pair(it->second.thread_id[k % length], it->second.thread_id[i]));
						std::sort(thread_pairs.begin(), thread_pairs.end());
					}
				}
			}
			// Create thread pairs for comparisons

			// Load data into memory of all the threads in given barrier interval, if it fits in memory,
			// data are compressed so not sure how to check if everything will fit in memory
			if(it->second.trace_size >= getTotalSystemMemory()) {
				std::cerr << "Can't fit all files in memory!\n";
				exit(-1);
			}

			// Struct to load uncompressed data from file
			std::vector<std::thread> lm_thread;
			lm_thread.reserve(it->second.thread_id.size());
			std::vector<std::vector<TraceItem>> file_buffers;
			file_buffers.resize(it->second.thread_id.size());
			for(std::vector<unsigned>::const_iterator th_id = it->second.thread_id.begin(); th_id != it->second.thread_id.end(); ++th_id) {
				lm_thread.push_back(std::thread(load_file, dir, it->first, *th_id, std::ref(file_buffers[*th_id])));
			}
			for(int k = 0; k < lm_thread.size(); k++) {
				lm_thread[k].join();
			}
			lm_thread.clear();
			// Load data into memory of all the threads in given barrier interval, if it fits in memory,
			// data are compressed so not sure how to check if everything will fit in memory

			// Now we can start analyzing the pairs
			std::atomic<int> available_threads;
			std::vector<std::thread> thread_list;
			available_threads = num_threads;
			for(std::vector<std::pair<unsigned,unsigned>>::const_iterator p = thread_pairs.begin(); p != thread_pairs.end(); ++p) {
				while(!available_threads) { /* usleep(1000); */ }
				available_threads--;

				// Create thread
				thread_list.push_back(std::thread(analyze_traces, it->first, p->first, p->second, std::ref(file_buffers), std::ref(available_threads)));
			}
			for(std::vector<std::thread>::iterator th = thread_list.begin(); th != thread_list.end(); th++) {
				th->join();
			}
			thread_list.clear();
		}
	} else {
		INFO(std::cout, "Folder '" << dir << "' does not exists or it's empty. Exiting...");
	}
//    }

	return 0;
}
