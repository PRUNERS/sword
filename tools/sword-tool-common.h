#ifndef TOOLS_SWORD_TOOL_COMMON_H_
#define TOOLS_SWORD_TOOL_COMMON_H_

#include <boost/functional/hash.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <vector>

#define SWORD_REPORT		"sword_report"
#define SHELL				"bash"
#define GET_SHELL			"which bash"
#define SYMBOLIZER			"llvm-symbolizer"
#define GET_SYMBOLIZER		"which llvm-symbolizer"
#define MB					1048576.00

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

	RaceInfo() {
		this->address = 0;
		this->rw1 = 0;
		this->size1 = 0;
		this->pc1 = 0;
		this->rw2 = 0;
		this->size2 = 0;
		this->pc2 = 0;
	}
};

std::string executable;
std::string shell_path;
std::string symbolizer_path;
std::vector<RaceInfo> races;

boost::filesystem::path report_data;

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

#endif /* TOOLS_SWORD_TOOL_COMMON_H_ */
