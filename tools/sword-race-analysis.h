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

// Task Data Structure (only Tied Tasks)
// - Task Creator
// 	 - Accesses (no race with tasks that come after)
//   - Task creation event
// 	 - Accesses (may with tasks that come after)

// - Task Workers
//   - Schedule start
//   - Accesses
//   - Schedule ends
// - Task Workers
// - ...
// - Task Workers

//std::vector<std::vector<TraceItem>> task_creators;
//std::unordered_map<unsigned, std::vector<TraceItem>> task;
//std::set<unsigned> analyzed_threads;
