@echo off
REM Launch script for Fleet Commander GUI
REM This script runs from the project root so assets can be found

cd /d "%~dp0"

if not exist "build\fleet_commander_gui.exe" (
    echo ERROR: fleet_commander_gui.exe not found!
    echo Please build the project first using build_msys2.bat
    pause
    exit /b 1
)

REM Check if DLLs are present in build directory
if not exist "build\libsfml-system-2.dll" (
    echo Copying required DLLs...
    copy /Y "C:\msys64\mingw64\bin\libsfml-*.dll" build\ >nul 2>&1
    copy /Y "C:\msys64\mingw64\bin\libgcc_s_seh-1.dll" build\ >nul 2>&1
    copy /Y "C:\msys64\mingw64\bin\libstdc++-6.dll" build\ >nul 2>&1
    copy /Y "C:\msys64\mingw64\bin\libwinpthread-1.dll" build\ >nul 2>&1
)

echo Starting Fleet Commander GUI with custom sprites...
start build\fleet_commander_gui.exe
