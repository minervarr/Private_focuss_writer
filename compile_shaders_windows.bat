@echo off
REM Compile Vulkan shaders on Windows
REM Requires Vulkan SDK to be installed

echo Compiling Vulkan shaders...

REM Try to find glslc in common Vulkan SDK locations
set GLSLC=glslc.exe

REM Check if glslc is in PATH
where %GLSLC% >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: glslc.exe not found in PATH
    echo Please make sure the Vulkan SDK is installed and in your PATH
    echo Or run: set PATH=%%PATH%%;C:\VulkanSDK\{version}\Bin
    pause
    exit /b 1
)

REM Compile vertex shader
echo Compiling text.vert...
%GLSLC% shaders/text.vert -o shaders/text_vert.spv
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile text.vert
    pause
    exit /b 1
)
echo   text_vert.spv created

REM Compile fragment shader
echo Compiling text.frag...
%GLSLC% shaders/text.frag -o shaders/text_frag.spv
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile text.frag
    pause
    exit /b 1
)
echo   text_frag.spv created

echo.
echo Shaders compiled successfully!
echo Now run copy_assets_windows.bat to copy them to the build directory.
pause
