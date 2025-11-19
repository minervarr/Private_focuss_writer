#!/bin/bash
# Copy assets to build directory for Linux

set -e  # Exit on error

echo "Copying assets to build directory..."

# Function to copy assets to a specific directory
copy_assets_to_dir() {
    local target_dir="$1"

    if [ -d "$target_dir" ]; then
        echo "Creating assets directory in $target_dir..."
        mkdir -p "$target_dir/assets/fonts"
        mkdir -p "$target_dir/shaders"

        # Copy fonts
        if [ -d "assets/fonts" ]; then
            cp -v assets/fonts/* "$target_dir/assets/fonts/" 2>/dev/null || true
        fi

        # Copy compiled shaders
        if [ -d "shaders" ]; then
            cp -v shaders/*.spv "$target_dir/shaders/" 2>/dev/null || true
        fi

        echo "Assets copied to $target_dir"
        return 0
    else
        return 1
    fi
}

# Track if any copy was successful
copied_any=false

# Copy to Debug build
if copy_assets_to_dir "build/bin/Debug"; then
    copied_any=true
fi

# Copy to Release build
if copy_assets_to_dir "build/bin/Release"; then
    copied_any=true
fi

# Also copy to build/bin for running from there
if copy_assets_to_dir "build/bin"; then
    copied_any=true
fi

if [ "$copied_any" = false ]; then
    echo ""
    echo "WARNING: No build directories found!"
    echo "Please run cmake and build the project first:"
    echo "  mkdir -p build"
    echo "  cd build"
    echo "  cmake .."
    echo "  make"
    echo "  cd .."
    echo "  ./copy_assets_linux.sh"
    exit 1
fi

echo ""
echo "Done! Assets copied to build directories."
echo "You can now run the executable from build/bin/Debug/ or build/bin/Release/"
