# Building Phantom Writer on Windows 11

## Prerequisites

1. **Visual Studio 2019 or later** with C++ development tools
   - Install "Desktop development with C++" workload
   - Make sure CMake is included
   - **OR MinGW-w64** (GCC for Windows)

2. **Vulkan SDK**
   - Download from: https://vulkan.lunarg.com/
   - Install with default options
   - Set environment variable: `VULKAN_SDK`

3. **CMake 3.20+**
   - Included with Visual Studio or download from cmake.org

## Quick Start (Easiest Method)

1. Open terminal in project root
2. **FIRST: Compile the shaders** (requires Vulkan SDK in PATH):
```cmd
compile_shaders_windows.bat
```
3. Then build and run:
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Debug
cd ..
copy_assets_windows.bat
cd build\bin\Debug
phantom-writer.exe
```

**Note**: If `glslc.exe` is not found, make sure Vulkan SDK's `Bin` folder is in your PATH:
```cmd
set PATH=%PATH%;C:\VulkanSDK\1.3.xxx.x\Bin
```

## Build Steps

### Option 1: Using CMake GUI

1. Open CMake GUI
2. Set source directory: `<path-to-repo>`
3. Set build directory: `<path-to-repo>/build`
4. Click "Configure" and select your Visual Studio version
5. Click "Generate"
6. Click "Open Project" to open in Visual Studio
7. Build the solution (F7)

### Option 2: Using Command Line

Open "Developer Command Prompt for VS":

```cmd
cd <path-to-repo>
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Debug
```

For Release build:
```cmd
cmake --build . --config Release
```

### Option 3: Using Visual Studio directly

1. Open Visual Studio 2019/2022
2. File → Open → CMake...
3. Select the root `CMakeLists.txt`
4. Visual Studio will automatically configure
5. Build → Build All (Ctrl+Shift+B)

## IMPORTANT: Copy Assets After Building

After building, you MUST copy the assets and shaders to the executable directory:

```cmd
copy_assets_windows.bat
```

This script copies:
- `assets/fonts/` → `build/bin/Debug/assets/fonts/`
- `shaders/*.spv` → `build/bin/Debug/shaders/`
- (Same for Release and bin directories)

**Without this step, the program will crash immediately** because it can't find the font file or shader files.

## Running

The executable will be in:
- Debug: `build/bin/Debug/phantom-writer.exe`
- Release: `build/bin/Release/phantom-writer.exe`

Run from the executable's directory:
```cmd
cd build\bin\Debug
phantom-writer.exe
```

## Font Setup

The default monospace font should be in:
```
assets/fonts/default_mono.ttf
```

If the font is missing, you can use any monospace TTF font (like Consolas, Courier New, etc.)

## Troubleshooting

### "Failed to open shader file: shaders/text_vert.spv"

**Cause**: Shaders were not compiled or not copied.

**Solution**:
1. **FIRST**: Compile shaders by running `compile_shaders_windows.bat`
   - This creates `text_vert.spv` and `text_frag.spv` from the GLSL source files
   - Requires Vulkan SDK's `glslc.exe` to be in PATH
2. **THEN**: Run `copy_assets_windows.bat` to copy the compiled shaders to the build directory
3. Verify these files exist in your shader directory:
   - `shaders/text_vert.spv` (compiled vertex shader, ~1.6KB)
   - `shaders/text_frag.spv` (compiled fragment shader, ~1.5KB)
   - NOT `text.vert` or `text.frag` (those are source files)

### Program Closes Immediately

**Cause**: Font file or shader files not found.

**Solution**:
1. Make sure you compiled shaders first: `compile_shaders_windows.bat`
2. Run `copy_assets_windows.bat` from project root
3. Or manually copy both:
   - `assets/` folder to the same directory as `phantom-writer.exe`
   - `shaders/` folder to the same directory as `phantom-writer.exe`
4. Verify these files exist in `build\bin\Debug\`:
   - `assets/fonts/default_mono.ttf`
   - `shaders/text_vert.spv` (NOT text.vert)
   - `shaders/text_frag.spv` (NOT text.frag)

The program will show a MessageBox with error details if something fails.

### Vulkan SDK Not Found
```
CMake Error: Could not find Vulkan
```
**Solution**: Install Vulkan SDK and restart terminal/Visual Studio

### Missing Windows SDK
```
Error: Windows SDK not found
```
**Solution**: Install Windows 10/11 SDK through Visual Studio Installer

### Font Loading Error
```
Failed to load font
```
**Solution**:
- Verify `assets/fonts/default_mono.ttf` exists
- Or copy a .ttf font to that location
- Run from project root directory

## Platform-Specific Features

Windows implementation includes:
- Native Win32 window management
- DirectInput for keyboard/mouse
- Windows file system paths (backslash separators)
- AppData directory for config: `%APPDATA%\PhantomWriter`
- My Documents for default save location

## Performance

For best performance, build in Release mode:
```cmd
cmake --build build --config Release
```

## Development

When developing on Windows:
- Use Visual Studio debugger (F5)
- Logs are written to `phantom_writer.log` in the executable directory
- Swap files (autosave) go to temp directory
- Config files go to `%APPDATA%\PhantomWriter`
