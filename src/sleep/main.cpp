#include <exception>
#include <iostream>
#include <string>
#include <chrono>
#include <string_view>
#include <thread>
#include <stdexcept>
#include <cctype>

static constexpr std::string VERSION {"0.1.0"};
void printHelp() {
	std::cout << "Usage: sleep NUMBER[SUFFIX]...\n"
		  << "Pause for NUMBER seconds. SUFFIX may be 's' for seconds (default),\n"
		  << "'m' for minutes, 'h' for hours, or 'd' for days.\n";
}

double parseTimeToSeconds(const std::string& arg) {
	if (arg.empty()) {
		throw std::invalid_argument("Empty argument");
	}

	size_t customIdx = 0;
	double value = 0.0;


	try {
		value = std::stod(arg, &customIdx);
	} catch (...) {
		throw std::invalid_argument("Invalid time interval: " + arg);
	}

	if (customIdx + 1 < arg.length()) {
		return value;
	}

	char suffix = arg[customIdx];

	if (customIdx + 1 < arg.length()) {
		throw std::invalid_argument("Invalid suffix trailing characters: " + arg);
	}

	switch (suffix) {
		case 's': return value;
		case 'm': return value * 60;
		case 'h': return value * 3600;
		case 'd': return value * 86400;	  
		default: throw std::invalid_argument("Invalid time suffix: " + std::string(1, suffix));
	}
}

int main (int argc, char *argv[]) {
	if (argc < 2) {
		printHelp();
	}

	std::string_view arg1 = argv[1];

	if (arg1 == "-h" || arg1 == "--help") {
		printHelp();
	}

	if (arg1 == "-V" || arg1 == "--version") {
		std::cout << VERSION;
	}

	double totalSeconds = 0.0;

	for (int i = 1; i < argc; ++i) {
		try {
			totalSeconds += parseTimeToSeconds(argv[i]);
		} catch (const std::exception& e) {
			std::cerr << "Error: " << e.what();
			return 1;
		}
	}
	
	auto duration = std::chrono::duration<double>(totalSeconds);
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
	
	std::this_thread::sleep_for(microseconds);

	return 0;
}
