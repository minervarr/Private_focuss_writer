#!/bin/bash
# Complete build script for Phantom Writer on Linux
# This script will compile shaders, build the project, and copy assets

set -e  # Exit on error

echo "============================================"
echo "Phantom Writer - Linux Build Script"
echo "============================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Check dependencies
echo "Step 1: Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: cmake not found${NC}"
    echo "Install with: sudo apt install cmake (Ubuntu/Debian)"
    exit 1
fi
echo -e "${GREEN}✓ CMake found${NC}"

# Check for Vulkan SDK
if ! command -v glslc &> /dev/null && ! command -v glslangValidator &> /dev/null; then
    echo -e "${RED}ERROR: No Vulkan shader compiler found${NC}"
    echo "Install Vulkan SDK:"
    echo "  - Ubuntu/Debian: sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers"
    echo "  - Or download from: https://vulkan.lunarg.com/sdk/home"
    exit 1
fi
echo -e "${GREEN}✓ Vulkan shader compiler found${NC}"

# Check for X11 development libraries
if ! pkg-config --exists x11 2>/dev/null; then
    echo -e "${YELLOW}WARNING: X11 development libraries not found${NC}"
    echo "Install with: sudo apt install libx11-dev (Ubuntu/Debian)"
    echo "Continuing anyway, build may fail..."
fi

# Check for FreeType (for font rendering)
if ! pkg-config --exists freetype2 2>/dev/null; then
    echo -e "${YELLOW}WARNING: FreeType development libraries not found${NC}"
    echo "Install with: sudo apt install libfreetype-dev (Ubuntu/Debian)"
    echo "Continuing anyway, build may fail..."
fi

echo ""

# Step 2: Compile shaders
echo "Step 2: Compiling shaders..."
if [ -f "compile_shaders_linux.sh" ]; then
    ./compile_shaders_linux.sh
else
    echo -e "${YELLOW}WARNING: compile_shaders_linux.sh not found${NC}"
    if [ -f "shaders/compile_shaders.sh" ]; then
        cd shaders && ./compile_shaders.sh && cd ..
    else
        echo -e "${RED}ERROR: No shader compilation script found${NC}"
        exit 1
    fi
fi
echo ""

# Step 3: Create build directory and configure
echo "Step 3: Configuring CMake..."
mkdir -p build
cd build

BUILD_TYPE="${1:-Debug}"
echo "Build type: $BUILD_TYPE"

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
echo ""

# Step 4: Build
echo "Step 4: Building project..."
make -j$(nproc)
echo ""

# Step 5: Copy assets
cd ..
echo "Step 5: Copying assets..."
if [ -f "copy_assets_linux.sh" ]; then
    ./copy_assets_linux.sh
else
    echo -e "${YELLOW}WARNING: copy_assets_linux.sh not found, copying manually...${NC}"
    mkdir -p build/bin/assets/fonts
    mkdir -p build/bin/shaders
    cp -v assets/fonts/* build/bin/assets/fonts/ 2>/dev/null || true
    cp -v shaders/*.spv build/bin/shaders/ 2>/dev/null || true
fi

echo ""
echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""
echo "To run the application:"
echo "  ./build/bin/phantom-writer"
echo ""
echo "To rebuild:"
echo "  cd build && make && cd .."
echo ""
