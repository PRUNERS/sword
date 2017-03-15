#include "rtl/sword_common.h"
#include "sword-tool-common.h"

#include <set>

struct TraceInfo {
	uint64_t trace_size;
	std::vector<unsigned> thread_id;

	TraceInfo() {
		trace_size = 0;
	}
};

boost::filesystem::path traces_data;
std::mutex rmtx;

bool overlap(const std::set<size_t>& s1, const std::set<size_t>& s2);
