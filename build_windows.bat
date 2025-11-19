@echo off
REM Complete build script for Phantom Writer on Windows
REM This script will compile shaders, build the project, and copy assets
REM
REM Prerequisites:
REM   - CMake (https://cmake.org/download/)
REM   - Visual Studio 2019 or newer (or MinGW)
REM   - Vulkan SDK (https://vulkan.lunarg.com/sdk/home#windows)

setlocal enabledelayedexpansion

echo ============================================
echo Phantom Writer - Windows Build Script
echo ============================================
echo.

REM Colors for output (Windows 10+ only)
set "GREEN=[92m"
set "RED=[91m"
set "YELLOW=[93m"
set "NC=[0m"

REM Step 1: Check dependencies
echo Step 1: Checking dependencies...
echo.

REM Check for CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo %RED%ERROR: cmake not found%NC%
    echo Download from: https://cmake.org/download/
    exit /b 1
)
echo %GREEN%[OK] CMake found%NC%

REM Check for Vulkan SDK
if not defined VULKAN_SDK (
    echo %RED%ERROR: VULKAN_SDK environment variable not set%NC%
    echo Please install the Vulkan SDK from: https://vulkan.lunarg.com/sdk/home#windows
    echo After installation, restart your terminal to load environment variables
    exit /b 1
)
echo %GREEN%[OK] Vulkan SDK found: %VULKAN_SDK%%NC%

REM Check for Visual Studio (or try MinGW as fallback)
set "GENERATOR="
set "USE_MINGW=0"

where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo %GREEN%[OK] Visual Studio compiler found%NC%
    set "GENERATOR=Visual Studio 17 2022"
) else (
    where g++ >nul 2>nul
    if %errorlevel% equ 0 (
        echo %YELLOW%[WARN] Visual Studio not found, using MinGW%NC%
        set "GENERATOR=MinGW Makefiles"
        set "USE_MINGW=1"
    ) else (
        echo %RED%ERROR: No compiler found (Visual Studio or MinGW)%NC%
        echo Install Visual Studio 2019+ with C++ support, or MinGW-w64
        exit /b 1
    )
)

echo.

REM Step 2: Configure build options
echo Step 2: Configuring build options...
echo.

REM Allow user to choose Sokol+Vulkan or pure Win32+Vulkan
set "USE_SOKOL=ON"
if "%1"=="--no-sokol" (
    set "USE_SOKOL=OFF"
    echo Building with pure Win32+Vulkan backend
) else (
    echo Building with Sokol+Vulkan backend (use --no-sokol for Win32)
)

REM Build type
set "BUILD_TYPE=Debug"
if "%2"=="Release" (
    set "BUILD_TYPE=Release"
) else if "%1"=="Release" (
    set "BUILD_TYPE=Release"
)
echo Build type: %BUILD_TYPE%
echo.

REM Step 3: Create build directory and configure
echo Step 3: Configuring CMake...
if not exist build mkdir build
cd build

if "%GENERATOR%"=="Visual Studio 17 2022" (
    cmake -G "Visual Studio 17 2022" -A x64 ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DUSE_SOKOL_VULKAN=%USE_SOKOL% ^
        ..
) else if "%GENERATOR%"=="Visual Studio 16 2019" (
    cmake -G "Visual Studio 16 2019" -A x64 ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DUSE_SOKOL_VULKAN=%USE_SOKOL% ^
        ..
) else (
    cmake -G "MinGW Makefiles" ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DUSE_SOKOL_VULKAN=%USE_SOKOL% ^
        ..
)

if %errorlevel% neq 0 (
    echo %RED%ERROR: CMake configuration failed%NC%
    cd ..
    exit /b 1
)
echo.

REM Step 4: Build
echo Step 4: Building project...

if "%USE_MINGW%"=="1" (
    mingw32-make -j%NUMBER_OF_PROCESSORS%
) else (
    cmake --build . --config %BUILD_TYPE% --parallel %NUMBER_OF_PROCESSORS%
)

if %errorlevel% neq 0 (
    echo %RED%ERROR: Build failed%NC%
    cd ..
    exit /b 1
)

cd ..
echo.

REM Step 5: Success message
echo %GREEN%============================================%NC%
echo %GREEN%Build completed successfully!%NC%
echo %GREEN%============================================%NC%
echo.
echo Executable location:
if "%USE_MINGW%"=="1" (
    echo   build\bin\phantom-writer.exe
) else (
    echo   build\bin\%BUILD_TYPE%\phantom-writer.exe
)
echo.
echo Compiled shaders:
echo   build\bin\shaders\text_vert.spv
echo   build\bin\shaders\text_frag.spv
echo.
echo Assets:
echo   build\bin\assets\fonts\
echo.
echo To run the application:
if "%USE_MINGW%"=="1" (
    echo   cd build\bin ^&^& phantom-writer.exe
) else (
    echo   cd build\bin\%BUILD_TYPE% ^&^& phantom-writer.exe
)
echo.
echo To rebuild:
if "%USE_MINGW%"=="1" (
    echo   cd build ^&^& mingw32-make ^&^& cd ..
) else (
    echo   cd build ^&^& cmake --build . --config %BUILD_TYPE% ^&^& cd ..
)
echo.
echo Build options:
echo   --no-sokol     : Use pure Win32+Vulkan instead of Sokol+Vulkan
echo   Debug/Release  : Set build configuration (default: Debug)
echo.

endlocal
