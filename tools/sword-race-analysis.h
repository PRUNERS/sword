#include "rtl/sword_common.h"
#include "sword-tool-common.h"

#include <set>
#include <unordered_map>

struct TraceInfo {
public:
	uint64_t file_offset_begin;
	uint64_t file_offset_end;

	TraceInfo() = default;

	TraceInfo(uint64_t fob, uint64_t foe) {
		file_offset_begin = fob;
		file_offset_end = foe;
	}
};

boost::filesystem::path traces_data;
std::mutex rmtx;

