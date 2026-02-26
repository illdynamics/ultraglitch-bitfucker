#!/bin/bash

set -e

echo "=== CyQle1 Build Script ==="

# Configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
JUCE_ROOT="${JUCE_ROOT:-}"

if [ -z "$JUCE_ROOT" ]; then
    echo "ERROR: JUCE_ROOT environment variable not set"
    echo "Please set JUCE_ROOT to point to your JUCE framework directory"
    echo "Example: export JUCE_ROOT=\"/path/to/JUCE\""
    exit 1
fi

if [ ! -d "$JUCE_ROOT" ]; then
    echo "ERROR: JUCE directory not found at: $JUCE_ROOT"
    exit 1
fi

echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"
echo "JUCE directory: $JUCE_ROOT"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo ""
echo "=== Configuring with CMake ==="
cmake .. -DJUCE_ROOT="$JUCE_ROOT" \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
echo ""
echo "=== Building Project ==="
cmake --build . --config Release --parallel $(nproc)

# Verify build artifacts
echo ""
echo "=== Verifying Build Artifacts ==="

check_artifact() {
    local artifact="$1"
    if [ -f "$artifact" ]; then
        echo "✓ Found: $(basename "$artifact")"
        return 0
    else
        echo "✗ Missing: $(basename "$artifact")"
        return 1
    fi
}

# Check for common build outputs
ARTIFACTS_FOUND=0
ARTIFACTS_TOTAL=0

if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS artifacts
    VST3_PLUGIN="${BUILD_DIR}/CyQle1_artefacts/VST3/CyQle1.vst3"
    STANDALONE_APP="${BUILD_DIR}/CyQle1_artefacts/Standalone/CyQle1.app"
    
    check_artifact "$VST3_PLUGIN" && ((ARTIFACTS_FOUND++))
    ((ARTIFACTS_TOTAL++))
    
    check_artifact "$STANDALONE_APP" && ((ARTIFACTS_FOUND++))
    ((ARTIFACTS_TOTAL++))
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux artifacts
    VST3_PLUGIN="${BUILD_DIR}/CyQle1_artefacts/VST3/CyQle1.so"
    STANDALONE_APP="${BUILD_DIR}/CyQle1_artefacts/Standalone/CyQle1"
    
    check_artifact "$VST3_PLUGIN" && ((ARTIFACTS_FOUND++))
    ((ARTIFACTS_TOTAL++))
    
    check_artifact "$STANDALONE_APP" && ((ARTIFACTS_FOUND++))
    ((ARTIFACTS_TOTAL++))
    
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows artifacts
    VST3_PLUGIN="${BUILD_DIR}/CyQle1_artefacts/VST3/CyQle1.vst3"
    STANDALONE_APP="${BUILD_DIR}/CyQle1_artefacts/Standalone/CyQle1.exe"
    
    check_artifact "$VST3_PLUGIN" && ((ARTIFACTS_FOUND++))
    ((ARTIFACTS_TOTAL++))
    
    check_artifact "$STANDALONE_APP" && ((ARTIFACTS_FOUND++))
    ((ARTIFACTS_TOTAL++))
fi

echo ""
echo "=== Build Summary ==="
echo "Artifacts found: $ARTIFACTS_FOUND/$ARTIFACTS_TOTAL"

if [ $ARTIFACTS_FOUND -eq $ARTIFACTS_TOTAL ] && [ $ARTIFACTS_TOTAL -gt 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "To test the plugin:"
    echo "1. VST3: Copy to your DAW's VST3 folder"
    echo "2. Standalone: Run the standalone application"
    
    # Copy plugin to common locations (optional)
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo ""
        echo "Optional: Copy VST3 to common location:"
        echo "  cp -r \"$VST3_PLUGIN\" \"$HOME/Library/Audio/Plug-Ins/VST3/\""
    fi
    
    exit 0
else
    echo "❌ Build incomplete or failed"
    exit 1
fi