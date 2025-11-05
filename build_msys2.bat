@echo off
REM Build script for Fleet Commander using MSYS2

echo ========================================
echo Fleet Commander - MSYS2 Build Script
echo ========================================
echo.

REM Set MSYS2 paths
set MSYS2_ROOT=C:\msys64
set PATH=%MSYS2_ROOT%\mingw64\bin;%PATH%

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM Create build directory
mkdir build
cd build

echo.
echo Configuring with CMake...
echo.

cmake .. -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building project...
echo.

cmake --build . -j 4

if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo Copying SFML DLLs and dependencies...
copy /Y "%MSYS2_ROOT%\mingw64\bin\libsfml-*.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libgcc_s_seh-1.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libstdc++-6.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libwinpthread-1.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libfreetype-6.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libbz2-1.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libpng16-16.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\zlib1.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libbrotlidec.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libbrotlicommon.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libharfbuzz-0.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libglib-2.0-0.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libgraphite2.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libintl-8.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libiconv-2.dll" . >nul 2>&1
copy /Y "%MSYS2_ROOT%\mingw64\bin\libpcre2-8-0.dll" . >nul 2>&1

echo.
echo ========================================
echo Build successful!
echo ========================================
echo.
echo Executables:
dir /b *.exe 2>nul
echo.
echo To run: .\fleet_commander_gui.exe
echo.

pause
