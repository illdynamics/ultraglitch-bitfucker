#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Cyqle1 Build and Test Script ===${NC}"
echo ""

# Create build directory
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Configure with CMake
echo "Configuring project with CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

echo -e "${GREEN}CMake configuration successful.${NC}"
echo ""

# Build the project
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build successful.${NC}"
echo ""

# Check for warnings
echo "Checking for compiler warnings..."
WARNINGS=$(make 2>&1 | grep -i warning || true)

if [ -n "$WARNINGS" ]; then
    echo -e "${YELLOW}Found warnings:${NC}"
    echo "$WARNINGS"
else
    echo -e "${GREEN}No warnings found.${NC}"
fi

echo ""

# Verify plugin files exist
echo "Verifying plugin files..."
PLUGIN_FILES=(
    "Cyqle1_artefacts/Release/Standalone/Cyqle1"
    "Cyqle1_artefacts/Release/VST3/Cyqle1.vst3"
    "Cyqle1_artefacts/Release/AU/Cyqle1.component"
)

ALL_FILES_EXIST=true
for file in "${PLUGIN_FILES[@]}"; do
    if [ -f "$file" ] || [ -d "$file" ]; then
        echo -e "${GREEN}✓ $file exists${NC}"
    else
        echo -e "${RED}✗ $file missing${NC}"
        ALL_FILES_EXIST=false
    fi
done

echo ""

if [ "$ALL_FILES_EXIST" = true ]; then
    echo -e "${GREEN}=== All build checks passed! ===${NC}"
    echo "Plugin successfully compiled and ready for testing."
else
    echo -e "${RED}=== Build verification failed! ===${NC}"
    exit 1
fi

# Return to original directory
cd ..