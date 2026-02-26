#!/bin/bash

# Script to fetch JUCE framework if not already present

set -e

echo "=== Fetching JUCE Framework ==="

# Check if we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Must be run from project root directory"
    exit 1
fi

# Check if JUCE is already fetched
if [ -d "build/_deps/juce-src" ]; then
    echo "JUCE already fetched in build/_deps/juce-src"
    echo "To refetch, delete build/_deps/juce-src and run again"
    exit 0
fi

# Create build directory if it doesn't exist
mkdir -p build

# Configure CMake to fetch JUCE
echo "Configuring CMake to fetch JUCE..."
cmake -B build -DCMAKE_BUILD_TYPE=Release

if [ $? -eq 0 ]; then
    echo "✓ CMake configuration successful"
    
    # Verify JUCE was fetched
    if [ -d "build/_deps/juce-src" ]; then
        echo "✓ JUCE framework successfully fetched"
        echo "Location: build/_deps/juce-src"
        
        # Show JUCE version
        if [ -f "build/_deps/juce-src/CHANGELOG.md" ]; then
            echo "JUCE version:"
            head -5 build/_deps/juce-src/CHANGELOG.md | grep -i version || true
        fi
    else
        echo "✗ JUCE framework not found after CMake configuration"
        exit 1
    fi
else
    echo "✗ CMake configuration failed"
    exit 1
fi

echo ""
echo "=== JUCE Fetch Complete ==="
echo "You can now build the project with:"
echo "  cmake --build build"
echo "Or use the provided scripts:"
echo "  npm run build"
echo "  make build"