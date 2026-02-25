#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

class BuildSetupAnalyzer {
public:
    BuildSetupAnalyzer() = default;

    bool analyze_cmake_lists() {
        std::cout << "Analyzing CMakeLists.txt..." << std::endl;
        
        std::ifstream cmake_file("CMakeLists.txt");
        if (!cmake_file.is_open()) {
            std::cerr << "  ✗ CMakeLists.txt not found" << std::endl;
            return false;
        }

        bool has_cpp17 = false;
        bool has_juce = false;
        bool has_plugin_target = false;
        std::string line;

        while (std::getline(cmake_file, line)) {
            // Check for C++17 standard
            if (line.find("set(CMAKE_CXX_STANDARD 17)") != std::string::npos ||
                line.find("CMAKE_CXX_STANDARD 17") != std::string::npos) {
                has_cpp17 = true;
                std::cout << "  ✓ C++17 standard configured" << std::endl;
            }
            
            // Check for JUCE integration
            if (line.find("FetchContent_Declare(JUCE") != std::string::npos ||
                line.find("find_package(JUCE") != std::string::npos) {
                has_juce = true;
                std::cout << "  ✓ JUCE framework integration found" << std::endl;
            }
            
            // Check for plugin target
            if (line.find("juce_add_plugin") != std::string::npos ||
                line.find("add_plugin") != std::string::npos) {
                has_plugin_target = true;
                std::cout << "  ✓ Plugin target configuration found" << std::endl;
            }
        }

        cmake_file.close();

        if (!has_cpp17) {
            std::cerr << "  ✗ C++17 standard not configured" << std::endl;
        }
        if (!has_juce) {
            std::cerr << "  ✗ JUCE integration not found" << std::endl;
        }
        if (!has_plugin_target) {
            std::cerr << "  ✗ Plugin target not configured" << std::endl;
        }

        return has_cpp17 && has_juce && has_plugin_target;
    }

    bool analyze_package_json() {
        std::cout << "Analyzing package.json..." << std::endl;
        
        if (!fs::exists("package.json")) {
            std::cerr << "  ✗ package.json not found" << std::endl;
            return false;
        }

        std::ifstream package_file("package.json");
        if (!package_file.is_open()) {
            std::cerr << "  ✗ Could not open package.json" << std::endl;
            return false;
        }

        bool has_build_scripts = false;
        bool has_cmake_scripts = false;
        std::string line;

        while (std::getline(package_file, line)) {
            if (line.find("\"build\"") != std::string::npos) {
                has_build_scripts = true;
                std::cout << "  ✓ Build scripts found" << std::endl;
            }
            if (line.find("cmake") != std::string::npos) {
                has_cmake_scripts = true;
                std::cout << "  ✓ CMake build scripts found" << std::endl;
            }
        }

        package_file.close();

        if (!has_build_scripts) {
            std::cerr << "  ✗ Build scripts not configured" << std::endl;
        }
        if (!has_cmake_scripts) {
            std::cerr << "  ✗ CMake build scripts not found" << std::endl;
        }

        return has_build_scripts && has_cmake_scripts;
    }

    bool analyze_makefile() {
        std::cout << "Analyzing Makefile..." << std::endl;
        
        if (!fs::exists("Makefile")) {
            std::cerr << "  ✗ Makefile not found" << std::endl;
            return false;
        }

        std::ifstream makefile("Makefile");
        if (!makefile.is_open()) {
            std::cerr << "  ✗ Could not open Makefile" << std::endl;
            return false;
        }

        bool has_build_target = false;
        bool has_clean_target = false;
        bool has_cmake_targets = false;
        std::string line;

        while (std::getline(makefile, line)) {
            if (line.find("build:") != std::string::npos) {
                has_build_target = true;
                std::cout << "  ✓ Build target found" << std::endl;
            }
            if (line.find("clean:") != std::string::npos) {
                has_clean_target = true;
                std::cout << "  ✓ Clean target found" << std::endl;
            }
            if (line.find("cmake") != std::string::npos) {
                has_cmake_targets = true;
                std::cout << "  ✓ CMake commands found" << std::endl;
            }
        }

        makefile.close();

        if (!has_build_target) {
            std::cerr << "  ✗ Build target not found" << std::endl;
        }
        if (!has_clean_target) {
            std::cerr << "  ✗ Clean target not found" << std::endl;
        }
        if (!has_cmake_targets) {
            std::cerr << "  ✗ CMake commands not found" << std::endl;
        }

        return has_build_target && has_clean_target && has_cmake_targets;
    }

    bool analyze_source_structure() {
        std::cout << "Analyzing source structure..." << std::endl;
        
        std::vector<std::string> required_dirs = {
            "Source",
            "Source/Effects",
            "Source/GUI",
            "qodeyard"
        };

        bool all_dirs_exist = true;
        for (const auto& dir : required_dirs) {
            if (fs::exists(dir) && fs::is_directory(dir)) {
                std::cout << "  ✓ Directory exists: " << dir << std::endl;
            } else {
                std::cerr << "  ✗ Directory missing: " << dir << std::endl;
                all_dirs_exist = false;
            }
        }

        std::vector<std::string> required_files = {
            "CMakeLists.txt",
            "package.json",
            "Makefile",
            "README.md"
        };

        bool all_files_exist = true;
        for (const auto& file : required_files) {
            if (fs::exists(file)) {
                std::cout << "  ✓ File exists: " << file << std::endl;
            } else {
                std::cerr << "  ✗ File missing: " << file << std::endl;
                all_files_exist = false;
            }
        }

        return all_dirs_exist && all_files_exist;
    }

    bool analyze_build_scripts() {
        std::cout << "Analyzing build scripts..." << std::endl;
        
        std::vector<std::string> build_scripts = {
            "scripts/verify_juce.py",
            "scripts/fetch_juce.sh"
        };

        bool all_scripts_exist = true;
        for (const auto& script : build_scripts) {
            if (fs::exists(script)) {
                std::cout << "  ✓ Script exists: " << script << std::endl;
                
                // Check if script is executable (Unix-like systems)
                #ifndef _WIN32
                auto p = fs::status(script).permissions();
                if ((p & fs::perms::owner_exec) != fs::perms::none) {
                    std::cout << "  ✓ Script is executable: " << script << std::endl;
                } else {
                    std::cerr << "  ⚠ Script not executable: " << script << std::endl;
                }
                #endif
            } else {
                std::cerr << "  ✗ Script missing: " << script << std::endl;
                all_scripts_exist = false;
            }
        }

        return all_scripts_exist;
    }

    bool verify_cmake_version() {
        std::cout << "Verifying CMake version..." << std::endl;
        
        #ifdef _WIN32
        std::string command = "cmake --version";
        #else
        std::string command = "cmake --version 2>/dev/null || echo \"cmake not found\"";
        #endif
        
        std::string output = execute_command(command);
        
        if (output.find("cmake version") != std::string::npos) {
            std::cout << "  ✓ CMake found: " << output.substr(0, output.find('\n')) << std::endl;
            
            // Extract version number
            size_t version_pos = output.find("cmake version");
            if (version_pos != std::string::npos) {
                std::string version_str = output.substr(version_pos + 13);
                int major = 0, minor = 0, patch = 0;
                sscanf(version_str.c_str(), "%d.%d.%d", &major, &minor, &patch);
                
                if (major > 3 || (major == 3 && minor >= 15)) {
                    std::cout << "  ✓ CMake version >= 3.15" << std::endl;
                    return true;
                } else {
                    std::cerr << "  ✗ CMake version too old: " << major << "." << minor << "." << patch << std::endl;
                    return false;
                }
            }
        } else {
            std::cerr << "  ✗ CMake not found or version check failed" << std::endl;
            return false;
        }
        
        return false;
    }

    bool verify_compiler_cpp17() {
        std::cout << "Verifying C++17 compiler support..." << std::endl;
        
        #if __cplusplus >= 201703L
        std::cout << "  ✓ Compiler supports C++17 (__cplusplus = " << __cplusplus << ")" << std::endl;
        return true;
        #else
        std::cerr << "  ✗ Compiler does not support C++17 (__cplusplus = " << __cplusplus << ")" << std::endl;
        return false;
        #endif
    }

    std::string execute_command(const std::string& command) {
        #ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
        #else
        FILE* pipe = popen(command.c_str(), "r");
        #endif
        
        if (!pipe) {
            return "ERROR";
        }
        
        std::array<char, 128> buffer;
        std::string result;
        
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        
        #ifdef _WIN32
        _pclose(pipe);
        #else
        pclose(pipe);
        #endif
        
        return result;
    }

    bool run_full_analysis() {
        std::cout << "\n=== Running Full Build Setup Analysis ===\n" << std::endl;
        
        bool cmake_ok = analyze_cmake_lists();
        std::cout << "CMakeLists.txt: " << (cmake_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool package_ok = analyze_package_json();
        std::cout << "package.json: " << (package_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool makefile_ok = analyze_makefile();
        std::cout << "Makefile: " << (makefile_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool structure_ok = analyze_source_structure();
        std::cout << "Source structure: " << (structure_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool scripts_ok = analyze_build_scripts();
        std::cout << "Build scripts: " << (scripts_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool cmake_version_ok = verify_cmake_version();
        std::cout << "CMake version: " << (cmake_version_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool compiler_ok = verify_compiler_cpp17();
        std::cout << "Compiler C++17: " << (compiler_ok ? "✓ PASS" : "✗ FAIL") << std::endl;
        
        bool all_passed = cmake_ok && package_ok && makefile_ok && 
                         structure_ok && scripts_ok && 
                         cmake_version_ok && compiler_ok;
        
        std::cout << "\n=== Analysis Complete ===" << std::endl;
        if (all_passed) {
            std::cout << "✓ All checks passed. Build setup is valid." << std::endl;
        } else {
            std::cout << "✗ Some checks failed. Please fix the issues above." << std::endl;
        }
        
        return all_passed;
    }
};

int main() {
    BuildSetupAnalyzer analyzer;
    
    if (analyzer.run_full_analysis()) {
        return 0;
    } else {
        return 1;
    }
}