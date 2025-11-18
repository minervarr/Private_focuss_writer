#!/bin/bash
# Compile Vulkan shaders on Linux
# Requires Vulkan SDK to be installed

set -e  # Exit on error

echo "Compiling Vulkan shaders..."

# Try to find shader compiler (prefer glslc, fallback to glslangValidator)
COMPILER=""
COMPILER_ARGS=""

if command -v glslc &> /dev/null; then
    COMPILER="glslc"
    COMPILER_ARGS=""
    echo "Using glslc compiler"
elif command -v glslangValidator &> /dev/null; then
    COMPILER="glslangValidator"
    COMPILER_ARGS="-V"
    echo "Using glslangValidator compiler"
else
    echo "ERROR: No Vulkan shader compiler found"
    echo "Please install one of the following:"
    echo "  - Ubuntu/Debian: sudo apt install glslang-tools"
    echo "  - Arch: sudo pacman -S shaderc"
    echo "  - Or download Vulkan SDK from: https://vulkan.lunarg.com/sdk/home"
    exit 1
fi

# Compile vertex shader
echo "Compiling text.vert..."
$COMPILER $COMPILER_ARGS shaders/text.vert -o shaders/text_vert.spv
echo "  text_vert.spv created"

# Compile fragment shader
echo "Compiling text.frag..."
$COMPILER $COMPILER_ARGS shaders/text.frag -o shaders/text_frag.spv
echo "  text_frag.spv created"

echo ""
echo "Shaders compiled successfully!"
echo "Now run ./copy_assets_linux.sh to copy them to the build directory."
