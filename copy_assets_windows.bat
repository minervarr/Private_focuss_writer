@echo off
REM Copy assets to build directory for Windows

echo Copying assets to build directory...

REM Create assets directory in Debug build
if exist "build\bin\Debug" (
    echo Creating Debug assets directory...
    if not exist "build\bin\Debug\assets" mkdir "build\bin\Debug\assets"
    if not exist "build\bin\Debug\assets\fonts" mkdir "build\bin\Debug\assets\fonts"
    xcopy /Y /Q "assets\fonts\*" "build\bin\Debug\assets\fonts\"
    echo Debug assets copied.
)

REM Create assets directory in Release build
if exist "build\bin\Release" (
    echo Creating Release assets directory...
    if not exist "build\bin\Release\assets" mkdir "build\bin\Release\assets"
    if not exist "build\bin\Release\assets\fonts" mkdir "build\bin\Release\assets\fonts"
    xcopy /Y /Q "assets\fonts\*" "build\bin\Release\assets\fonts\"
    echo Release assets copied.
)

REM Also copy to build/bin for running from there
if exist "build\bin" (
    echo Creating bin assets directory...
    if not exist "build\bin\assets" mkdir "build\bin\assets"
    if not exist "build\bin\assets\fonts" mkdir "build\bin\assets\fonts"
    xcopy /Y /Q "assets\fonts\*" "build\bin\assets\fonts\"
    echo Bin assets copied.
)

echo.
echo Done! Assets copied to build directories.
echo You can now run the executable from build\bin\Debug\ or build\bin\Release\
pause
