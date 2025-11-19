@echo off
REM Standalone shader compilation script for Windows
REM Compiles GLSL shaders to SPIR-V bytecode
REM
REM Prerequisites:
REM   - Vulkan SDK with glslc or glslangValidator

setlocal enabledelayedexpansion

echo ============================================
echo Phantom Writer - Shader Compilation
echo ============================================
echo.

REM Colors for output
set "GREEN=[92m"
set "RED=[91m"
set "YELLOW=[93m"
set "NC=[0m"

REM Check for Vulkan SDK
if not defined VULKAN_SDK (
    echo %RED%ERROR: VULKAN_SDK environment variable not set%NC%
    echo Please install the Vulkan SDK from: https://vulkan.lunarg.com/sdk/home#windows
    exit /b 1
)

REM Find shader compiler
set "COMPILER="
set "COMPILER_ARGS="

REM Try glslc first (preferred)
where glslc >nul 2>nul
if %errorlevel% equ 0 (
    set "COMPILER=glslc"
    echo %GREEN%[OK] Using glslc for shader compilation%NC%
) else (
    REM Fallback to glslangValidator
    where glslangValidator >nul 2>nul
    if %errorlevel% equ 0 (
        set "COMPILER=glslangValidator"
        set "COMPILER_ARGS=-V"
        echo %YELLOW%[WARN] glslc not found, using glslangValidator%NC%
    ) else (
        echo %RED%ERROR: No Vulkan shader compiler found%NC%
        echo Expected glslc or glslangValidator in PATH
        echo Check your Vulkan SDK installation: %VULKAN_SDK%
        exit /b 1
    )
)
echo.

REM Shader directories
set "SHADER_SOURCE_DIR=shaders"
set "SHADER_OUTPUT_DIR=build\shaders"

REM Check if shader source directory exists
if not exist "%SHADER_SOURCE_DIR%" (
    echo %RED%ERROR: Shader source directory not found: %SHADER_SOURCE_DIR%%NC%
    exit /b 1
)

REM Create output directory if it doesn't exist
if not exist "%SHADER_OUTPUT_DIR%" (
    echo Creating shader output directory: %SHADER_OUTPUT_DIR%
    mkdir "%SHADER_OUTPUT_DIR%"
)

echo Compiling shaders from: %SHADER_SOURCE_DIR%
echo Output directory: %SHADER_OUTPUT_DIR%
echo.

REM Compile vertex shader
set "VERTEX_SHADER=%SHADER_SOURCE_DIR%\text.vert"
set "VERTEX_OUTPUT=%SHADER_OUTPUT_DIR%\text_vert.spv"

if not exist "%VERTEX_SHADER%" (
    echo %RED%ERROR: Vertex shader not found: %VERTEX_SHADER%%NC%
    exit /b 1
)

echo Compiling: text.vert -^> text_vert.spv
%COMPILER% %COMPILER_ARGS% "%VERTEX_SHADER%" -o "%VERTEX_OUTPUT%"
if %errorlevel% neq 0 (
    echo %RED%ERROR: Failed to compile vertex shader%NC%
    exit /b 1
)
echo %GREEN%[OK] Vertex shader compiled%NC%
echo.

REM Compile fragment shader
set "FRAGMENT_SHADER=%SHADER_SOURCE_DIR%\text.frag"
set "FRAGMENT_OUTPUT=%SHADER_OUTPUT_DIR%\text_frag.spv"

if not exist "%FRAGMENT_SHADER%" (
    echo %RED%ERROR: Fragment shader not found: %FRAGMENT_SHADER%%NC%
    exit /b 1
)

echo Compiling: text.frag -^> text_frag.spv
%COMPILER% %COMPILER_ARGS% "%FRAGMENT_SHADER%" -o "%FRAGMENT_OUTPUT%"
if %errorlevel% neq 0 (
    echo %RED%ERROR: Failed to compile fragment shader%NC%
    exit /b 1
)
echo %GREEN%[OK] Fragment shader compiled%NC%
echo.

REM Copy to bin directory if it exists
if exist "build\bin\shaders" (
    echo Copying shaders to build\bin\shaders...
    copy /Y "%VERTEX_OUTPUT%" "build\bin\shaders\text_vert.spv" >nul
    copy /Y "%FRAGMENT_OUTPUT%" "build\bin\shaders\text_frag.spv" >nul
    echo %GREEN%[OK] Shaders copied to binary directory%NC%
    echo.
)

echo %GREEN%============================================%NC%
echo %GREEN%Shader compilation completed successfully!%NC%
echo %GREEN%============================================%NC%
echo.
echo Output files:
echo   %VERTEX_OUTPUT%
echo   %FRAGMENT_OUTPUT%
echo.

endlocal
