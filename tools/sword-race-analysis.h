#include "rtl/sword_common.h"
#include <boost/range/iterator_range.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/filesystem.hpp>

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

#include <algorithm>
#include <map>
#include <vector>

#define SWORD_REPORT		"sword_report"
#define MB					1048576.00

boost::filesystem::path traces_data;
boost::filesystem::path report_data;

boost::lockfree::queue<unsigned> thread_queue(1024);
std::vector<std::thread *> lm_thread;
std::vector<std::vector<TraceItem>> file_buffers;

struct TraceInfo {
	uint64_t trace_size;
	std::vector<unsigned> thread_id;

	TraceInfo() {
		trace_size = 0;
	}
};
