#include "rtl/sword_common.h"
#include "sword-tool-common.h"
#include <boost/functional/hash.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>

void PrintReport() {
	size_t current_size = 0;
	for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(report_data), {})) {
		size_t filesize = boost::filesystem::file_size(entry.path());
		if(filesize > 0) {
			std::ifstream file(entry.path().string(), std::ios::in | std::ios::binary);
			size_t size;
			file.read((char*) &size, sizeof(size));
			races.resize(current_size + size);
			file.read(reinterpret_cast<char*>(races.data() + current_size), size * sizeof(RaceInfo));
			current_size += size;
			file.close();
		}
	}

	for(std::vector<RaceInfo>::const_iterator race = races.cbegin() ; race != races.cend(); ++race) {
		std::size_t hash = 0;
		boost::hash_combine(hash, race->pc1);
		boost::hash_combine(hash, race->pc2);
		const bool reported = hash_races.find(hash) != hash_races.end();
		if(!reported) {
			hash_races.insert(hash);
			std::string race1 = "";
			std::string race2 = "";

			{
				std::string command = shell_path + " -c '" + symbolizer_path + " -pretty-print" + " < <(echo \"" + executable + " " + std::to_string(race->pc1) + "\")'";
				execute_command(command.c_str(), &race1, 2);
			}

			{
				std::string command = shell_path + " -c '" + symbolizer_path + " -pretty-print" + " < <(echo \"" + executable + " " + std::to_string(race->pc2) + "\")'";
				execute_command(command.c_str(), &race2, 2);
			}

			INFO(std::cerr, "--------------------------------------------------");
			INFO(std::cerr, "WARNING: SWORD: array data race (program=" << executable << ")");
			INFO(std::cerr, "Two different threads made the following accesses:");
			INFO(std::cerr, AccessTypeStrings[race->rw1] << " of size " << std::dec << (1 << race->size1) << " in " << race1);
			INFO(std::cerr, AccessTypeStrings[race->rw2] << " of size " << std::dec << (1 << race->size2) << " in " << race2);
			INFO(std::cerr, "--------------------------------------------------");
			INFO(std::cerr, "");
		}
	}
}

int main(int argc, char **argv) {
	std::string unknown_option = "";

	if(argc < 5)
		INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--executable <path-to-executable-name> --report-path <path-to-report-folder>\n\n");

	for(int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--help") {
			INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--executable <path-to-executable-name> --report-path <path-to-report-folder>\n\n");
			return 0;
		} else if (std::string(argv[i]) == "--report-path") {
			if (i + 1 < argc) {
				report_data += argv[++i];
			} else {
				INFO(std::cerr, "--report-path option requires one argument.");
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

	try {
		// Output folder
		if(boost::algorithm::ends_with(report_data.string(), SWORD_REPORT)) {
			report_data.append("/");
		} else if(boost::algorithm::ends_with(report_data.string(), std::string(SWORD_REPORT) + std::string("/"))) {

		} else if(boost::algorithm::ends_with(report_data.string(), "/")) {
			report_data.append(SWORD_REPORT);
		} else {
			report_data.append("/").append(SWORD_REPORT);
		}

		if(!boost::filesystem::is_directory(report_data)) {
			INFO(std::cout, "Report folder not found, please specify a correct path to the report folder.");
			return -1;
		}

		// Executable check
		if (!boost::filesystem::exists(executable)) {
			INFO(std::cerr, "The executable '" << executable << "' does not exists.\nPlease specify the correct path and name for the executable.");
			return -1;
		}
	} catch(boost::filesystem::filesystem_error const & e) {
        INFO(std::cerr, e.what());
        return false;
    }

	// Look for shell
	execute_command(GET_SHELL, &shell_path);
    // Look for shell

	// Look for llvm-symbolizer
	execute_command(GET_SYMBOLIZER, &symbolizer_path);
    // Look for llvm-symbolizer

	boost::filesystem::recursive_directory_iterator dir(report_data), end;
	if(dir == end) {
		INFO(std::cerr, "SWORD did not find any race on '" << executable << "'.");
		exit(0);
	}

	PrintReport();
}
