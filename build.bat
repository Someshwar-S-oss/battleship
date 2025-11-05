@echo off
REM Build script for Fleet Commander (Windows)

echo ========================================
echo Fleet Commander - Build Script
echo ========================================
echo.

REM Check if build directory exists
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Check for vcpkg toolchain file
set VCPKG_ROOT=C:\vcpkg
set TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

if exist "%TOOLCHAIN_FILE%" (
    echo Using vcpkg toolchain: %TOOLCHAIN_FILE%
    cmake .. -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE%
) else (
    echo vcpkg not found at %VCPKG_ROOT%
    echo Trying system-wide SFML installation...
    cmake ..
)

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: CMake configuration failed!
    echo ========================================
    echo.
    echo Possible solutions:
    echo 1. Install SFML: vcpkg install sfml:x64-windows
    echo 2. Set SFML_DIR environment variable
    echo 3. Check SETUP.md for detailed instructions
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Building project...
echo ========================================
echo.

cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Build failed!
    echo ========================================
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo ========================================
echo.

REM Check what was built
if exist "Release\fleet_commander.exe" (
    echo Built: fleet_commander.exe (Terminal version)
)

if exist "Release\fleet_commander_gui.exe" (
    echo Built: fleet_commander_gui.exe (GUI version)
) else (
    echo Warning: GUI version not built (SFML not found)
)

echo.
echo To run the game:
echo   Terminal: .\Release\fleet_commander.exe
echo   GUI:      .\Release\fleet_commander_gui.exe
echo.

pause
