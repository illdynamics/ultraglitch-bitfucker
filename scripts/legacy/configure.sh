#!/bin/bash

set -e

echo "=== CyQle1 Configuration Script ==="

# Configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
JUCE_ROOT="${JUCE_ROOT:-}"

# Check for JUCE_ROOT
if [ -z "$JUCE_ROOT" ]; then
    echo "JUCE_ROOT environment variable is not set."
    echo ""
    echo "Please set JUCE_ROOT to point to your JUCE framework directory."
    echo "You can do this in your shell profile or temporarily for this session:"
    echo ""
    echo "  export JUCE_ROOT=\"/path/to/JUCE\""
    echo ""
    
    # Try to find JUCE in common locations
    echo "Searching for JUCE in common locations..."
    
    POSSIBLE_PATHS=(
        "$HOME/JUCE"
        "$HOME/juce"
        "/usr/local/JUCE"
        "/opt/JUCE"
        "$PROJECT_DIR/../JUCE"
        "$PROJECT_DIR/external/JUCE"
    )
    
    for path in "${POSSIBLE_PATHS[@]}"; do
        if [ -d "$path" ] && [ -f "$path/CMakeLists.txt" ]; then
            echo "Found JUCE at: $path"
            read -p "Use this path? [Y/n]: " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
                JUCE_ROOT="$path"
                break
            fi
        fi
    done
    
    if [ -z "$JUCE_ROOT" ]; then
        echo ""
        echo "JUCE not found automatically."
        echo "Please download JUCE from: https://juce.com/get-juce"
        echo "Then set the JUCE_ROOT environment variable."
        exit 1
    fi
fi

# Verify JUCE directory
if [ ! -d "$JUCE_ROOT" ]; then
    echo "ERROR: JUCE directory not found at: $JUCE_ROOT"
    exit 1
fi

if [ ! -f "$JUCE_ROOT/CMakeLists.txt" ]; then
    echo "ERROR: Invalid JUCE directory (CMakeLists.txt not found)"
    exit 1
fi

echo "Using JUCE from: $JUCE_ROOT"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake 3.15 or higher."
    echo "Visit: https://cmake.org/download/"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
echo "CMake version: $CMAKE_VERSION"

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: No C++ compiler found. Please install g++ or clang++."
    exit 1
fi

if command -v g++ &> /dev/null; then
    COMPILER_INFO=$(g++ --version | head -n1)
    echo "Compiler: $COMPILER_INFO"
elif command -v clang++ &> /dev/null; then
    COMPILER_INFO=$(clang++ --version | head -n1)
    echo "Compiler: $COMPILER_INFO"
fi

# Create build directory
echo ""
echo "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Run CMake configuration
echo ""
echo "=== Running CMake Configuration ==="
cd "$BUILD_DIR"

cmake .. -DJUCE_ROOT="$JUCE_ROOT" \
         -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
         -DCMAKE_VERBOSE_MAKEFILE=ON

# Generate compile_commands.json for tooling
if [ -f "compile_commands.json" ]; then
    echo ""
    echo "Generated compile_commands.json for IDE integration"
    
    # Symlink to project root for easier access
    ln -sf "$BUILD_DIR/compile_commands.json" "$PROJECT_DIR/compile_commands.json" 2>/dev/null || true
fi

# Display configuration summary
echo ""
echo "=== Configuration Summary ==="
echo "Project: CyQle1"
echo "Build type: Debug"
echo "C++ standard: 17"
echo "Build directory: $BUILD_DIR"
echo "JUCE framework: $JUCE_ROOT"
echo ""
echo "Next steps:"
echo "1. Run ./build.sh to build the project"
echo "2. Run ./test.sh to run basic tests"
echo "3. Load the plugin in your DAW for testing"
echo ""
echo "✅ Configuration complete!"