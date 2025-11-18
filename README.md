# Phantom Writer

A high-performance, distraction-free text editor built with Vulkan rendering and designed for focused writing.

## Features

- **Vulkan-based Rendering**: Hardware-accelerated text rendering for smooth performance
- **Cross-Platform**: Supports both Windows and Linux
- **Minimal Interface**: Focus on your writing without distractions
- **Revision Mode**: Track and manage document revisions
- **Auto-save**: Never lose your work with automatic saving and swap files

## Platform Support

### Windows

Windows 10/11 with Vulkan support

See [Windows Build Instructions](README_WINDOWS.md) (if available) or use:
- `compile_shaders_windows.bat` - Compile shaders
- `copy_assets_windows.bat` - Copy assets to build directory

### Linux

X11-based Linux distributions with Vulkan support

See [README_LINUX.md](README_LINUX.md) for detailed Linux build instructions.

Quick start:
```bash
./build_linux.sh
```

## Quick Start

### Prerequisites

Both platforms require:
- **CMake** 3.20+
- **C++17** compatible compiler
- **Vulkan SDK** (1.2+)

Platform-specific requirements are detailed in the respective README files.

### Building

#### Linux
```bash
# Automated build
./build_linux.sh

# Or manual
./compile_shaders_linux.sh
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ..
./copy_assets_linux.sh
```

#### Windows
```batch
REM Compile shaders
compile_shaders_windows.bat

REM Build with Visual Studio
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..

REM Copy assets
copy_assets_windows.bat
```

### Running

After building:

**Linux:**
```bash
./build/bin/phantom-writer
```

**Windows:**
```batch
build\bin\Release\phantom-writer.exe
```

## Project Structure

```
Private_focuss_writer/
├── assets/                     # Assets (fonts, etc.)
│   └── fonts/
├── shaders/                    # GLSL shader sources
│   ├── text.vert
│   └── text.frag
├── src/
│   ├── core/                  # Core editor functionality
│   ├── persistence/           # Auto-save and swap files
│   ├── platform/              # Platform-specific code
│   │   ├── linux/            # Linux (X11) implementation
│   │   └── windows/          # Windows (Win32) implementation
│   ├── rendering/            # Rendering subsystem
│   │   ├── core/            # Platform-agnostic rendering
│   │   └── vulkan/          # Vulkan renderer
│   ├── ui/                   # UI components
│   └── utils/                # Utilities and logging
├── compile_shaders_linux.sh   # Linux shader compiler
├── compile_shaders_windows.bat # Windows shader compiler
├── copy_assets_linux.sh       # Linux asset copier
├── copy_assets_windows.bat    # Windows asset copier
└── build_linux.sh            # Linux automated build script
```

## Development

### Architecture

- **Platform Layer**: Abstracts OS-specific functionality (window management, input, file system)
- **Rendering Layer**: Vulkan-based text rendering with glyph caching and batching
- **Core Layer**: Editor state, buffer management, cursor operations
- **Persistence Layer**: Auto-save, swap files, and crash recovery

### Building for Development

Use Debug builds during development:

**Linux:**
```bash
./build_linux.sh Debug
```

**Windows:**
```batch
cmake --build build --config Debug
```

## Troubleshooting

### Vulkan Not Found

Ensure the Vulkan SDK is installed and in your PATH:
- **Linux**: `sudo apt install vulkan-tools libvulkan-dev`
- **Windows**: Download from https://vulkan.lunarg.com/

### Shader Compilation Fails

Make sure `glslc` or `glslangValidator` is available:
```bash
# Linux
which glslc

# Windows
where glslc.exe
```

### Application Won't Start

1. Verify your GPU supports Vulkan: `vulkaninfo`
2. Check that assets are copied to the build directory
3. Ensure compiled shaders (.spv files) exist

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on your platform
5. Submit a pull request

## License

See LICENSE file for details.

## Credits

Built with:
- Vulkan for rendering
- FreeType for font loading
- X11 (Linux) / Win32 (Windows) for windowing
