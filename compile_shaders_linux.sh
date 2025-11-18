#!/bin/bash
# Compile Vulkan shaders on Linux
# Requires Vulkan SDK to be installed

set -e  # Exit on error

echo "Compiling Vulkan shaders..."

# Try to find glslc
GLSLC="glslc"

# Check if glslc is in PATH
if ! command -v $GLSLC &> /dev/null; then
    echo "ERROR: glslc not found in PATH"
    echo "Please make sure the Vulkan SDK is installed and in your PATH"
    echo "You can install it via:"
    echo "  - Ubuntu/Debian: sudo apt install vulkan-tools vulkan-sdk"
    echo "  - Arch: sudo pacman -S vulkan-tools shaderc"
    echo "  - Or download from: https://vulkan.lunarg.com/sdk/home"
    echo ""
    echo "After installing, you may need to source the setup script:"
    echo "  source /path/to/vulkan/sdk/setup-env.sh"
    exit 1
fi

# Compile vertex shader
echo "Compiling text.vert..."
$GLSLC shaders/text.vert -o shaders/text_vert.spv
echo "  text_vert.spv created"

# Compile fragment shader
echo "Compiling text.frag..."
$GLSLC shaders/text.frag -o shaders/text_frag.spv
echo "  text_frag.spv created"

echo ""
echo "Shaders compiled successfully!"
echo "Now run ./copy_assets_linux.sh to copy them to the build directory."
