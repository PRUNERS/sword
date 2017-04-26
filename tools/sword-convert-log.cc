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

bool overlap(const std::set<size_t>& s1, const std::set<size_t>& s2) {
	for(const auto& i : s1) {
		if(std::binary_search(s2.begin(), s2.end(), i))
			return true;
	}
	return false;
}

#define RACE_CHECK(t1, t2) \
		(t1->data.access.getAddress() == t2->data.access.getAddress()) &&		\
		((t1->data.access.getAccessType() == unsafe_write) ||					\
				(t2->data.access.getAccessType() == unsafe_write) ||					\
				((t1->data.access.getAccessType() == atomic_write) &&					\
						(t2->data.access.getAccessType() == unsafe_read)) ||					\
						((t2->data.access.getAccessType() == atomic_write) &&					\
								(t1->data.access.getAccessType() == unsafe_read)))

#define UNSAFE() !overlap(mt1, mt2)

void analyze_traces(unsigned bid, unsigned t1, unsigned t2, std::vector<std::vector<TraceItem>> &file_buffers, std::atomic<int> &available_threads) {
	//	INFO(std::cout, "Analyzing pair (" << t1 << "," << t2 << ").");

	if(file_buffers.size() > 0) {
		std::set<size_t> mt1;
		for(std::vector<TraceItem>::iterator i = file_buffers[t1].begin() ; i != file_buffers[t1].end(); ++i) {
			std::set<size_t> mt2;
			switch(i->getType()) {
			case data_access:
				for(std::vector<TraceItem>::iterator j = file_buffers[t2].begin(); j != file_buffers[t2].end(); ++j) {
					switch(j->getType()) {
					case data_access:
						if(RACE_CHECK(i,j) &&
								UNSAFE()) {
							ReportRace(i->data.access.getAddress(),
									i->data.access.getAccessType(), j->data.access.getAccessType(),
									i->data.access.getAccessSize(),
									j->data.access.getAccessSize(),
									i->data.access.getPC() - 1, j->data.access.getPC() - 1);
						}
						break;
					case mutex_acquired:
						mt2.insert(j->data.mutex_region.getWaitId());
						break;
					case mutex_released:
						mt2.erase(j->data.mutex_region.getWaitId());
						break;
					default:
						break;
					}
				}
				break;
			case mutex_acquired:
				//				std::size_t hash = 0;
				//				boost::hash_combine(hash, i->data.mutex_region.kind);
				//				boost::hash_combine(hash, i->data.mutex_region.wait_id);
				//				mt1.insert(hash);
				mt1.insert(i->data.mutex_region.getWaitId());
				break;
			case mutex_released:
				//				std::size_t hash = 0;
				//				boost::hash_combine(hash, i->data.mutex_region.kind);
				//				boost::hash_combine(hash, i->data.mutex_region.wait_id);
				//				mt1.insert(hash);
				mt1.erase(i->data.mutex_region.getWaitId());
				break;
			default:
				break;
			}
		}
	}

	available_threads++;
}

void load_file(std::string filename, std::vector<TraceItem> &file_buffer) {
	uint64_t filesize = boost::filesystem::file_size(filename);
	uint64_t out_len;
	uint64_t new_len;

	FILE *datafile = fopen(filename.c_str(), "r");
	if (!datafile) {
		INFO(std::cerr, "SWORD: Error opening file: " << filename << " - " << strerror(errno) << ".");
		exit(-1);
	}

	unsigned char compressed_buffer[OUT_LEN];
	TraceItem *uncompressed_buffer = (TraceItem *) malloc(BLOCK_SIZE);

	unsigned int block_size;
	size_t size = fread(&block_size, sizeof(uint64_t), 1, datafile);
	unsigned neof = 0;

	if(block_size + sizeof(uint64_t) < filesize) {
          block_size += sizeof(uint64_t);
          neof = 1;
	}

	size_t total_size = sizeof(uint64_t);

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

        file_buffer.insert(file_buffer.end(), uncompressed_buffer, uncompressed_buffer + (new_len / sizeof(TraceItem)));

		memcpy(&block_size, compressed_buffer + block_size - sizeof(uint64_t), sizeof(uint64_t));
		if(total_size + block_size + sizeof(uint64_t) < filesize) {
			block_size += sizeof(uint64_t);
			neof = 1;
		} else {
			neof = 0;
		}
	}

	free(uncompressed_buffer);
	fclose(datafile);
}

int main(int argc, char **argv) {

#ifdef LZO
	// Initialize decompressor
	if(lzo_init() != LZO_E_OK) {
		INFO(std::cerr, "Internal error - lzo_init() failed!");
		INFO(std::cerr, "This usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics.");
		exit(-1);
	}
	// Initialize decompressor
#endif

	if(argc < 2) {
		INFO(std::cerr, "Please specify file to analyze.");
	}

	std::string filename(argv[1]);
	INFO(std::cout, "Analyzing file: " << filename << std::endl);

	std::vector<TraceItem> buffer;
	load_file(filename, buffer);

	INFO(std::cout, "Buffer size: " << buffer.size() << std::endl);

	return 0;
}
