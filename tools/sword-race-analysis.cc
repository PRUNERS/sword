#include "sword-race-analysis.h"

int main(int argc, char **argv) {
	std::string unknown_option = "";

	for(int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--help") {
			INFO(std::cout, "Usage:\n\n  " << argv[0] << " " << "--report-path <path-to-report-folder> --traces-path <path-to-traces-folder>\n\n");
			return 0;
		} else if (std::string(argv[i]) == "--report-path") {
			if (i + 1 < argc) {
				report_data += argv[++i];
			} else {
				INFO(std::cerr, "--report-path option requires one argument.");
				return -1;
			}
		} else if (std::string(argv[i]) == "--traces-path") {
			if (i + 1 < argc) {
				traces_data += argv[++i];
			} else {
				INFO(std::cerr, "--traces-path option requires one argument.");
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
		if(report_data.string().empty()) {
			INFO(std::cout, "Using default report folder: ./" << SWORD_REPORT);
		} else if(boost::filesystem::is_directory(report_data)) {
			INFO(std::cerr, "The path '" << report_data.string() << "' is not valid.");
			return -1;
		}

		report_data.append(SWORD_REPORT);
		if(boost::filesystem::is_directory(report_data)) {
			INFO(std::cout, "Found existing report folder, please delete or rename it before proceeding with analysis.");
			return -1;
		}
		if(!boost::filesystem::create_directory(report_data)) {
			INFO(std::cerr, "Unable to create destination directory " << report_data.string());
			return -1;
		}

		// Input folder
		traces_data.append(SWORD_DATA);
		if(!boost::filesystem::is_directory(traces_data)) {
			INFO(std::cerr, "The traces folder '" << traces_data.string() << "' does not exists.\nPlease specify the correct path with the option '--traces-path <path-to-traces-folder>'.");
			return -1;
		}
	} catch( boost::filesystem::filesystem_error const & e) {
        INFO(std::cerr, e.what());
        return false;
    }

	// Ready to iterate folders (parallel regions) in traces folder
	boost::filesystem::directory_iterator end_iter;
	boost::filesystem::directory_iterator file_itr;
	for (boost::filesystem::directory_iterator dir_itr(traces_data); dir_itr != end_iter; ++dir_itr) {
		INFO(std::cout, dir_itr->path());
	}

	return 0;
}
