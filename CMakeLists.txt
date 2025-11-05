cmake_minimum_required(VERSION 3.16)
project(FleetCommander LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure proper MinGW paths are set
if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Terminal version (original)
add_executable(fleet_commander
    src/main.cpp
)

target_compile_options(fleet_commander PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# GUI version with SFML
# Find SFML
find_package(SFML 2.5 COMPONENTS system window graphics QUIET)

if(SFML_FOUND)
    # Game logic library (shared between terminal and GUI versions)
    add_library(game_logic STATIC
        src/GameLogic.cpp
        src/GameLogic.h
    )
    
    target_compile_options(game_logic PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # GUI executable
    add_executable(fleet_commander_gui
        src/main_gui.cpp
        src/GameGUI.cpp
        src/GameGUI.h
    )

    target_link_libraries(fleet_commander_gui PRIVATE
        game_logic
        sfml-system
        sfml-window
        sfml-graphics
    )

    target_compile_options(fleet_commander_gui PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    message(STATUS "SFML found - GUI version will be built")
    message(STATUS "Build both: fleet_commander (terminal) and fleet_commander_gui (GUI)")
else()
    message(WARNING "SFML not found - only terminal version will be built")
    message(STATUS "To install SFML on Windows with vcpkg: vcpkg install sfml")
    message(STATUS "To install SFML on Linux: sudo apt-get install libsfml-dev")
    message(STATUS "To install SFML on macOS: brew install sfml")
endif()
