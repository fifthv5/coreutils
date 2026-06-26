#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error> // Required for std::error_code
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

static constexpr std::string_view VERSION{"0.1.3"};

bool is_executable(const fs::directory_entry& entry) {
    std::error_code ec;
    
    // Use the no-throw overload of is_regular_file
    if (!entry.is_regular_file(ec)) return false;

    #if defined(_WIN32)
        return entry.path().extension() == ".exe";
    #else
        // Use the no-throw overload of status()
        fs::file_status status = entry.status(ec);
        if (ec) return false; // Skip if we can't read the file status

        fs::perms p = status.permissions();
        return (p & (fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec)) != fs::perms::none;
    #endif
}

int main (int argc, char *argv[]) {
	std::string_view arg1 = (argc > 1) ? argv[1] : "";

	if (arg1 == "--help" || arg1 == "-h") {
		std::cout << "Usage: ls [OPTION]... [FILE]...\n";
		std::cout << "List information about the FILEs (the current directory by default).\n";
		std::cout << "Sort entries alphabetically if none of -cftuvSUX nor --sort is specified.\n\n";
		std::cout << "Mandatory arguments to tong options are mandatory for short options too.\n\n";
		std::cout << "  -a, --all";
		std::cout << "          do not ingore entries starting with .";
		std::cout << "      --zero\n";
		std::cout << "          end each output line with NUL, not newline";
		std::cout << "  -1\n";
		std::cout << "          list one file per line\n";
		std::cout << "       --help\n";
		std::cout << "          display this help and exit\n";
		std::cout << "       --version\n";
		std::cout << "          output version information and exit";

		
		return 0;

	}
	if (arg1 == "--version" || arg1 == "-V") {
		std::cout << VERSION << "\n";
		return 0;
	}
	fs::path current_path(".");
	std::vector<fs::directory_entry> entries;

	try {
		for (const auto& entry : fs::directory_iterator(current_path)) {
			entries.push_back(entry);
		} 
	} catch (const fs::filesystem_error& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	std::ranges::sort(entries, {}, [](const fs::directory_entry& entry) {
		return entry.path().filename().string();
	});


	if (arg1 == "-a" || arg1 == "--all") {
		for (const auto& entry : entries) {
			std::string filename = entry.path().filename().string();

			if (entry.is_directory()) {
				filename += "/";
			}
			else if (is_executable(entry)) {
				filename += "*";
			} 

			std::cout << filename << " ";
		}
		return 0;
	}
	if (arg1 == "--zero") {
		for (const auto& entry : entries) {
			std::string filename = entry.path().filename().string();						
			if (filename.starts_with(".")) continue;

			if (entry.is_directory()) {
				filename += "/";
			}
			else if (is_executable(entry)) {
				filename += "*";
			} 

			std::cout << filename << '\0';
		}
		return 0;
	}
	if (arg1 == "-1") {
		for (const auto& entry : entries) {
			std::string filename = entry.path().filename().string();
			if (filename.starts_with(".")) continue;
			if (entry.is_directory()) {
				filename += "/";
			}
			else if (is_executable(entry)) {
				filename += "*";
			} 

			std::cout << filename << "\n";
		}
		return 0;
	}
	
	for (const auto& entry : entries) {
		std::string filename = entry.path().filename().string();
		if (filename.starts_with(".")) continue;
		if (entry.is_directory()) {
			filename += "/";
		}
		else if (is_executable(entry)) {
			filename += "*";
		}

		std::cout << filename << " ";
	}

	return 0;
}
