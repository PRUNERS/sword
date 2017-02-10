#ifndef SWORD_FLAGS_H
#define SWORD_FLAGS_H

#include <string>

class SwordFlags {
public:
	std::string trace_path;

	SwordFlags(const char *env) : trace_path("") {
		if(env) {
			char tmp_string[255];
			int ret = sscanf(env, "trace_path=%s", tmp_string);
			if(!ret) {
				std::cerr << "Illegal values for SWORD_OPTIONS variable, no option has been set." << std::endl;

			} else {
				trace_path = tmp_string;
				if(trace_path.back() != '/')
					trace_path += "/";
			}
		}
	}
};

#endif  // SWORD_FLAGS_H
