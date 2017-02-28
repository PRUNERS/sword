#include "rtl/sword_common.h"
#include <boost/range/iterator_range.hpp>
#include <boost/lockfree/queue.hpp>
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

boost::filesystem::path traces_data;
boost::filesystem::path report_data;

std::string executable;
std::string shell_path;
std::string symbolizer_path;
std::vector<size_t> hash_races;
std::mutex rmtx;

struct TraceInfo {
	uint64_t trace_size;
	std::vector<unsigned> thread_id;

	TraceInfo() {
		trace_size = 0;
	}
};
