# Phantom Writer

A high-performance, distraction-free text editor built with Vulkan rendering and designed for focused writing.

## Features

- **Vulkan-based Rendering**: Hardware-accelerated text rendering for smooth performance
- **Cross-Platform**: Supports Windows, Linux, and Android
- **Minimal Interface**: Focus on your writing without distractions
- **Revision Mode**: Track and manage document revisions
- **Auto-save**: Never lose your work with automatic saving and swap files
- **X11 Exclusive Fullscreen**: Maximum GPU performance on Linux

## Platform Support

### Windows

Windows 10/11 with Vulkan support

**Two Window Backends Available:**
- **Sokol + Vulkan** (Default, Recommended) - Modern cross-platform abstraction
- **Pure Win32 + Vulkan** (Legacy) - Direct Win32 API usage

Quick start:
```batch
# Build with Sokol+Vulkan (default)
build_windows.bat

# Build with pure Win32
build_windows.bat --no-sokol

# Release build
build_windows.bat Release
```

See [WINDOWS_SOKOL_INTEGRATION.md](WINDOWS_SOKOL_INTEGRATION.md) for detailed Windows build instructions and Sokol integration documentation.

### Linux

X11-based Linux distributions with Vulkan support

See [README_LINUX.md](README_LINUX.md) for detailed Linux build instructions.

Quick start:
```bash
./build_linux.sh
```

### Android

Android 8.0+ with Vulkan support

See [README_ANDROID.md](README_ANDROID.md) for detailed Android build instructions.

Quick start:
1. Open `android/` directory in Android Studio
2. Sync Gradle
3. Run on device/emulator

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
REM Automated build (recommended)
build_windows.bat

REM Or manual build
compile_shaders_windows.bat
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DUSE_SOKOL_VULKAN=ON ..
cmake --build . --config Release
cd ..
```

**Build Options:**
- `USE_SOKOL_VULKAN=ON` - Use Sokol+Vulkan backend (default)
- `USE_SOKOL_VULKAN=OFF` - Use pure Win32+Vulkan backend

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
│   │   └── windows/          # Windows (Sokol+Win32+Vulkan)
│   ├── rendering/            # Rendering subsystem
│   │   ├── core/            # Platform-agnostic rendering
│   │   └── vulkan/          # Vulkan renderer
│   ├── ui/                   # UI components
│   └── utils/                # Utilities and logging
├── third_party/
│   └── sokol/                # Sokol headers (Windows)
├── build_linux.sh                    # Linux automated build script
├── build_windows.bat                 # Windows automated build script
├── compile_shaders_linux.sh          # Linux shader compiler
├── compile_shaders_windows.bat       # Windows shader compiler
├── copy_assets_linux.sh              # Linux asset copier
├── WINDOWS_SOKOL_INTEGRATION.md      # Windows Sokol documentation
└── TESTING.md                        # Testing documentation
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
- stb_truetype for font loading
- Sokol (Windows - default) for window/input abstraction
- X11 (Linux) / Win32 (Windows - legacy) for windowing
