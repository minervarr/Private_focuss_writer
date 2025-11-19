# Phantom Writer - Linux Build Instructions

This document provides detailed instructions for building and running Phantom Writer on Linux.

## Prerequisites

### Required Dependencies

1. **C++ Compiler** (GCC 7+ or Clang 5+)
   ```bash
   sudo apt install build-essential  # Ubuntu/Debian
   sudo pacman -S base-devel         # Arch Linux
   sudo dnf install gcc-c++ make     # Fedora
   ```

2. **CMake** (3.20 or higher)
   ```bash
   sudo apt install cmake            # Ubuntu/Debian
   sudo pacman -S cmake              # Arch Linux
   sudo dnf install cmake            # Fedora
   ```

3. **Vulkan SDK**
   ```bash
   # Ubuntu/Debian
   sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools

   # Arch Linux
   sudo pacman -S vulkan-devel vulkan-tools shaderc

   # Fedora
   sudo dnf install vulkan-tools vulkan-loader-devel vulkan-validation-layers-devel

   # Or download the full SDK from: https://vulkan.lunarg.com/sdk/home
   ```

4. **X11 Development Libraries**
   ```bash
   sudo apt install libx11-dev       # Ubuntu/Debian
   sudo pacman -S libx11             # Arch Linux
   sudo dnf install libX11-devel     # Fedora
   ```

5. **FreeType** (for font rendering)
   ```bash
   sudo apt install libfreetype-dev  # Ubuntu/Debian
   sudo pacman -S freetype2          # Arch Linux
   sudo dnf install freetype-devel   # Fedora
   ```

## Quick Start

### Option 1: Automated Build (Recommended)

Run the all-in-one build script:

```bash
./build_linux.sh
```

For a release build:
```bash
./build_linux.sh Release
```

### Option 2: Manual Build

1. **Compile Shaders**
   ```bash
   ./compile_shaders_linux.sh
   ```

2. **Configure and Build**
   ```bash
   mkdir -p build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make -j$(nproc)
   cd ..
   ```

3. **Copy Assets**
   ```bash
   ./copy_assets_linux.sh
   ```

4. **Run**
   ```bash
   ./build/bin/phantom-writer
   ```

## Build Types

- **Debug** (default): Includes debug symbols, no optimization
  ```bash
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  ```

- **Release**: Optimized for performance
  ```bash
  cmake -DCMAKE_BUILD_TYPE=Release ..
  ```

## Troubleshooting

### Vulkan SDK Not Found

If CMake cannot find Vulkan, you may need to set the `VULKAN_SDK` environment variable:

```bash
export VULKAN_SDK=/path/to/vulkan/sdk
source $VULKAN_SDK/setup-env.sh
```

### Shader Compilation Fails

Make sure you have `glslc` or `glslangValidator` installed:

```bash
# Check if glslc is available
which glslc

# Check if glslangValidator is available
which glslangValidator
```

If neither is found, install the Vulkan SDK tools:

```bash
sudo apt install shaderc  # Ubuntu/Debian
sudo pacman -S shaderc    # Arch Linux
```

### X11 Libraries Not Found

Ensure X11 development libraries are installed:

```bash
sudo apt install libx11-dev xorg-dev  # Ubuntu/Debian
sudo pacman -S libx11                 # Arch Linux
```

### Missing FreeType

Install FreeType development libraries:

```bash
sudo apt install libfreetype-dev      # Ubuntu/Debian
sudo pacman -S freetype2              # Arch Linux
```

## Project Structure

```
Private_focuss_writer/
├── assets/
│   └── fonts/              # Font files
├── shaders/                # GLSL shader source files
│   ├── text.vert          # Vertex shader
│   └── text.frag          # Fragment shader
├── src/
│   ├── platform/linux/    # Linux-specific implementations
│   ├── rendering/vulkan/  # Vulkan renderer
│   └── ...
├── build/                 # Build output (created during build)
│   └── bin/              # Executable location
├── compile_shaders_linux.sh   # Shader compilation script
├── copy_assets_linux.sh       # Asset copying script
└── build_linux.sh            # All-in-one build script
```

## Running the Application

After a successful build:

```bash
./build/bin/phantom-writer
```

## Development

### Rebuilding After Changes

If you only modified C++ source files:

```bash
cd build
make -j$(nproc)
cd ..
```

If you modified shaders:

```bash
./compile_shaders_linux.sh
./copy_assets_linux.sh
```

### Cleaning Build

To clean and rebuild from scratch:

```bash
rm -rf build
./build_linux.sh
```

## Platform-Specific Notes

- The application uses X11 for window management
- Wayland users may need XWayland for compatibility
- The Vulkan renderer requires a Vulkan-capable GPU and drivers

## Getting Help

If you encounter issues:

1. Check that all dependencies are installed
2. Verify your Vulkan installation with `vulkaninfo`
3. Check the build logs for specific error messages
4. Ensure your GPU drivers support Vulkan

## License

See LICENSE file for details.
