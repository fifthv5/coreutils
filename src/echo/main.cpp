#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <unistd.h>

#define ISATTY isatty 
#define FILENO fileno 

static constexpr std::string_view VERSION{"0.0.1"};

int main (int argc, char *argv[]) {

	bool has_piped_input = !ISATTY(FILENO(stdin));

	if (argc < 2 && !has_piped_input) {
		fprintf(stderr, "Usage: %s [text]", argv[0]);
		return 1;
	}
	if (argc >= 2) {
		std::string_view arg1{argv[1]};

		if (arg1 == "-h" || arg1 == "--help") {
			fprintf(stdout, "Usage: %s [text]", argv[0]);
			return 0;
		}	
		if (arg1 == "-V" || arg1 == "--version") {
			std::cout << VERSION;
			return 0;
		}

		for (int i = 1; i < argc; i++) {
			std::cout << argv[i];
			if (i < argc - 1) {
				std::cout << ' ';
			}
		}
		std::cout << '\n';
		return 0;
	}

	if (has_piped_input) {
		std::string line;
		while (std::getline(std::cin, line)) {
			std::cout << line << '\n';
		}
	}

	return 0;
}
