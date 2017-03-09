#include "rtl/sword_common.h"
#include "sword-tool-common.h"

struct TraceInfo {
	uint64_t trace_size;
	std::vector<unsigned> thread_id;

	TraceInfo() {
		trace_size = 0;
	}
};

boost::filesystem::path traces_data;
std::vector<size_t> hash_races;
std::mutex rmtx;
