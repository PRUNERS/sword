#ifndef SWORD_FLAGS_H
#define SWORD_FLAGS_H

#include <string>

class SwordFlags {
public:
	std::string traces_path;

	SwordFlags(const char *env) : traces_path("") {
		if(env) {
			char tmp_string[255];
			int ret = sscanf(env, "traces_path=%s", tmp_string);
			if(!ret) {
				std::cerr << "Illegal values for SWORD_OPTIONS variable, no option has been set." << std::endl;

			} else {
				traces_path = tmp_string;
//				if(traces_path.back() != '/')
//					traces_path += "/";
			}
		}
	}
};

#endif  // SWORD_FLAGS_H
