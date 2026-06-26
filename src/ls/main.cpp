// Copyright (c) 2026 fifthv5. All Rights Reserved.

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error> // Required for std::error_code
#include <vector>
#include <algorithm>
#include <chrono>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <iomanip>

namespace fs = std::filesystem;

static constexpr std::string_view VERSION{"0.1.3"};

std::string get_perms_string(fs::perms p) {
	std::string s = "---------";
	if ((p & fs::perms::owner_read)   != fs::perms::none) s[0] = 'r';
    	if ((p & fs::perms::owner_write)  != fs::perms::none) s[1] = 'w';
    	if ((p & fs::perms::owner_exec)   != fs::perms::none) s[2] = 'x';
    	if ((p & fs::perms::group_read)   != fs::perms::none) s[3] = 'r';
    	if ((p & fs::perms::group_write)  != fs::perms::none) s[4] = 'w';
    	if ((p & fs::perms::group_exec)   != fs::perms::none) s[5] = 'x';
    	if ((p & fs::perms::others_read)  != fs::perms::none) s[6] = 'r';
    	if ((p & fs::perms::others_write) != fs::perms::none) s[7] = 'w';
    	if ((p & fs::perms::others_exec)  != fs::perms::none) s[8] = 'x';
    	return s;
}

void print_long_listing(const fs::directory_entry& entry) {
    try {
        const auto& path = entry.path();
        struct stat file_stat;
        
        // Use stat system call for Unix-specific details (owner, links)
        if (stat(path.c_str(), &file_stat) != 0) return;

        // 1. File Type Indicator
        char type = '-';
        if (entry.is_directory())  type = 'd';
        else if (entry.is_symlink())    type = 'l';

        // 2. Permissions
        std::string perms = get_perms_string(entry.status().permissions());

        // 3. Number of Hard Links
        nlink_t links = file_stat.st_nlink;

        // 4. Owner Name
        struct passwd* pw = getpwuid(file_stat.st_uid);
        std::string owner = pw ? pw->pw_name : std::to_string(file_stat.st_uid);

        // 5. Group Name
        struct group* gr = getgrgid(file_stat.st_gid);
        std::string group = gr ? gr->gr_name : std::to_string(file_stat.st_gid);

        // 6. File Size
        uintmax_t size = entry.is_directory() ? 0 : entry.file_size();

        // 7. Modification Time
        auto ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
        
        // Format and Print Output
        std::cout << type << perms << " "
                  << std::setw(3) << links << " "
                  << std::setw(8) << owner << " "
                  << std::setw(8) << group << " "
                  << std::setw(8) << size << " "
                  << std::put_time(std::localtime(&ctime), "%b %d %H:%M") << " "
                  << path.filename().string() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error reading " << entry.path().filename() << ": " << e.what() << "\n";
    }
}

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
		std::cout << "  -a, --all\n";
		std::cout << "          do not ingore entries starting with .\n";
		std::cout << "      --zero\n";
		std::cout << "          end each output line with NUL, not newline\n";
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

	if (arg1 == "-l"){
		fs::path target_path(".");
		for (const auto& entry : fs::directory_iterator(target_path)) {
			if (entry.path().filename().string().starts_with(".")) continue;

			print_long_listing(entry);
		}
		return 0;

	}
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
