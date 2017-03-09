#include "rtl/sword_common.h"
#include <boost/functional/hash.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <vector>

#define SWORD_REPORT		"sword_report"
#define SHELL				"bash"
#define GET_SHELL			"which bash"
#define SYMBOLIZER			"llvm-symbolizer"
#define GET_SYMBOLIZER		"which llvm-symbolizer"
#define MB					1048576.00

struct TraceInfo {
	uint64_t trace_size;
	std::vector<unsigned> thread_id;

	TraceInfo() {
		trace_size = 0;
	}
};

struct RaceInfo {
	uint64_t address;
	uint8_t rw1;
	uint8_t size1;
	uint64_t pc1;
	uint8_t rw2;
	uint8_t size2;
	uint64_t pc2;

	RaceInfo(uint64_t address, uint8_t rw1, uint8_t size1, uint64_t pc1,
			uint8_t rw2, uint8_t size2, uint64_t pc2) {
		this->address = address;
		this->rw1 = rw1;
		this->size1 = size1;
		this->pc1 = pc1;
		this->rw2 = rw2;
		this->size2 = size2;
		this->pc2 = pc2;
	}
};

boost::filesystem::path traces_data;
boost::filesystem::path report_data;

std::string executable;
std::string shell_path;
std::string symbolizer_path;
std::vector<size_t> hash_races;
// Read element two by two
std::vector<RaceInfo> races;
std::mutex rmtx;
