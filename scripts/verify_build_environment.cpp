#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace fs = std::filesystem;

class BuildEnvironmentVerifier {
public:
    BuildEnvironmentVerifier() = default;

    bool verify_cmake_version() {
        std::cout << "Verifying CMake version..." << std::endl;
        
        std::string output = execute_command("cmake --version");
        if (output.find("cmake version") != std::string::npos) {
            std::cout << "  ✓ CMake found: " << output.substr(0, output.find("\n")) << std::endl;
            return true;
        } else {
            std::cout << "  ✗ CMake not found or version check failed" << std::endl;
            return false;
        }
    }

    bool verify_juce_integration() {
        std::cout << "Verifying JUCE integration..." << std::endl;
        
        bool has_juce = false;
        bool has_modules = false;
        
        // Check for JUCE in CMakeLists.txt
        std::ifstream cmake_file("CMakeLists.txt");
        if (!cmake_file.is_open()) {
            std::cout << "  ✗ CMakeLists.txt not found" << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(cmake_file, line)) {
            if (line.find("find_package(JUCE") != std::string::npos ||
                line.find("FetchContent_Declare(JUCE") != std::string::npos) {
                has_juce = true;
            }
        }
        
        if (has_juce) {
            std::cout << "  ✓ JUCE integration found in CMakeLists.txt" << std::endl;
        } else {
            std::cout << "  ✗ JUCE integration not found in CMakeLists.txt" << std::endl;
        }
        
        // Check for JUCE modules directory
        if (fs::exists("modules") && fs::is_directory("modules")) {
            std::cout << "  ✓ JUCE modules directory exists" << std::endl;
            has_modules = true;
        } else {
            std::cout << "  ✗ JUCE modules directory not found" << std::endl;
        }
        
        return has_juce && has_modules;
    }

    bool verify_source_files() {
        std::cout << "Verifying source files structure..." << std::endl;
        
        std::vector<std::string> required_directories = {
            "Source",
            "Source/Effects",
            "Source/GUI",
            "Source/parameters",
            "Source/dsp"
        };
        
        bool all_dirs_exist = true;
        for (const auto& dir : required_directories) {
            if (fs::exists(dir) && fs::is_directory(dir)) {
                std::cout << "  ✓ Directory exists: " << dir << std::endl;
            } else {
                std::cout << "  ✗ Directory missing: " << dir << std::endl;
                all_dirs_exist = false;
            }
        }
        
        if (!all_dirs_exist) {
            std::cout << "  ✗ Some required directories are missing" << std::endl;
            return false;
        }
        
        // Check for key source files
        std::vector<std::string> required_files = {
            "Source/PluginProcessor.cpp",
            "Source/PluginEditor.cpp",
            "Source/PluginProcessor.h",
            "Source/PluginEditor.h",
            "Source/parameters/PluginParameters.cpp",
            "Source/parameters/PluginParameters.h",
            "Source/effects/EffectChain.cpp",
            "Source/effects/EffectChain.h"
        };
        
        bool all_files_exist = true;
        for (const auto& file : required_files) {
            if (fs::exists(file)) {
                std::cout << "  ✓ File exists: " << file << std::endl;
            } else {
                std::cout << "  ✗ File missing: " << file << std::endl;
                all_files_exist = false;
            }
        }
        
        return all_files_exist;
    }

    bool verify_build_configuration() {
        std::cout << "Verifying build configuration..." << std::endl;
        
        bool package_json_ok = false;
        bool makefile_ok = false;
        
        // Check package.json
        if (fs::exists("package.json")) {
            std::ifstream package_file("package.json");
            std::string content((std::istreambuf_iterator<char>(package_file)),
                               std::istreambuf_iterator<char>());
            
            if (content.find("\"configure\"") != std::string::npos &&
                content.find("\"build\"") != std::string::npos) {
                std::cout << "  ✓ package.json has build scripts" << std::endl;
                package_json_ok = true;
            } else {
                std::cout << "  ✗ package.json missing build scripts" << std::endl;
            }
        } else {
            std::cout << "  ✗ package.json not found" << std::endl;
        }
        
        // Check Makefile
        if (fs::exists("Makefile")) {
            std::ifstream makefile("Makefile");
            std::string line;
            bool has_build_target = false;
            bool has_clean_target = false;
            
            while (std::getline(makefile, line)) {
                if (line.find("build:") != std::string::npos) {
                    has_build_target = true;
                }
                if (line.find("clean:") != std::string::npos) {
                    has_clean_target = true;
                }
            }
            
            if (has_build_target && has_clean_target) {
                std::cout << "  ✓ Makefile has build and clean targets" << std::endl;
                makefile_ok = true;
            } else {
                std::cout << "  ✗ Makefile missing build or clean targets" << std::endl;
            }
        } else {
            std::cout << "  ✗ Makefile not found" << std::endl;
        }
        
        return package_json_ok && makefile_ok;
    }

    bool verify_environment_variables() {
        std::cout << "Verifying environment variables..." << std::endl;
        
        const char* env_vars[] = {
            "PATH",
            "HOME",
            "USER"
        };
        
        bool all_vars_ok = true;
        for (const auto& var : env_vars) {
            const char* value = std::getenv(var);
            if (value != nullptr && std::string(value).length() > 0) {
                std::cout << "  ✓ Environment variable set: " << var << std::endl;
            } else {
                std::cout << "  ✗ Environment variable not set: " << var << std::endl;
                all_vars_ok = false;
            }
        }
        
        return all_vars_ok;
    }

    bool verify_compiler() {
        std::cout << "Verifying compiler..." << std::endl;
        
        #ifdef __clang__
            std::cout << "  ✓ Compiler: Clang" << std::endl;
            return true;
        #elif __GNUC__
            std::cout << "  ✓ Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << std::endl;
            return true;
        #elif _MSC_VER
            std::cout << "  ✓ Compiler: MSVC " << _MSC_VER << std::endl;
            return true;
        #else
            std::cout << "  ✗ Unknown compiler" << std::endl;
            return false;
        #endif
    }

    bool verify_cpp_standard() {
        std::cout << "Verifying C++ standard..." << std::endl;
        
        #if __cplusplus >= 201703L
            std::cout << "  ✓ C++17 or later detected (__cplusplus = " << __cplusplus << ")" << std::endl;
            return true;
        #else
            std::cout << "  ✗ C++17 not detected (__cplusplus = " << __cplusplus << ")" << std::endl;
            return false;
        #endif
    }

    bool run_full_verification() {
        std::cout << "=========================================" << std::endl;
        std::cout << "Build Environment Verification" << std::endl;
        std::cout << "=========================================" << std::endl;
        
        bool cmake_ok = verify_cmake_version();
        bool juce_ok = verify_juce_integration();
        bool sources_ok = verify_source_files();
        bool build_ok = verify_build_configuration();
        bool env_ok = verify_environment_variables();
        bool compiler_ok = verify_compiler();
        bool cpp_std_ok = verify_cpp_standard();
        
        std::cout << "=========================================" << std::endl;
        
        bool all_passed = cmake_ok && juce_ok && sources_ok && 
                         build_ok && env_ok && compiler_ok && cpp_std_ok;
        
        if (all_passed) {
            std::cout << "✓ All verifications passed!" << std::endl;
            std::cout << "Build environment is ready for compilation." << std::endl;
        } else {
            std::cout << "✗ Some verifications failed!" << std::endl;
            std::cout << "Please fix the issues above before building." << std::endl;
        }
        
        std::cout << "=========================================" << std::endl;
        
        return all_passed;
    }

private:
    std::string execute_command(const std::string& command) {
        #ifdef _WIN32
            FILE* pipe = _popen(command.c_str(), "r");
        #else
            FILE* pipe = popen(command.c_str(), "r");
        #endif
        
        if (!pipe) {
            return "";
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
};

int main() {
    BuildEnvironmentVerifier verifier;
    
    if (verifier.run_full_verification()) {
        return 0;
    } else {
        return 1;
    }
}