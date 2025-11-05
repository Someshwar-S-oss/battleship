#!/bin/bash
# Build script for Fleet Commander (Linux/macOS)

set -e  # Exit on error

echo "========================================"
echo "Fleet Commander - Build Script"
echo "========================================"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for dependencies
echo "Checking dependencies..."

# Check for cmake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: cmake not found${NC}"
    echo "Install: sudo apt-get install cmake (Ubuntu/Debian)"
    echo "        sudo dnf install cmake (Fedora)"
    echo "        brew install cmake (macOS)"
    exit 1
fi

# Check for g++ or clang++
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}ERROR: No C++ compiler found${NC}"
    echo "Install: sudo apt-get install build-essential (Ubuntu/Debian)"
    echo "        sudo dnf install gcc-c++ (Fedora)"
    echo "        xcode-select --install (macOS)"
    exit 1
fi

# Check for SFML (optional but recommended)
SFML_WARNING=0
if pkg-config --exists sfml-graphics 2>/dev/null; then
    echo -e "${GREEN}✓ SFML found${NC}"
elif [ -d "/usr/local/lib/libsfml-graphics.dylib" ] || [ -d "/usr/lib/libsfml-graphics.so" ]; then
    echo -e "${GREEN}✓ SFML found (library check)${NC}"
else
    echo -e "${YELLOW}⚠ SFML not found - GUI version will not be built${NC}"
    SFML_WARNING=1
fi

echo ""

# Create build directory
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Configure
echo "========================================"
echo "Configuring project with CMake..."
echo "========================================"
echo ""

if ! cmake -DCMAKE_BUILD_TYPE=Release ..; then
    echo ""
    echo -e "${RED}========================================"
    echo "ERROR: CMake configuration failed!"
    echo "========================================${NC}"
    echo ""
    echo "Possible solutions:"
    echo "1. Install SFML:"
    echo "   Ubuntu/Debian: sudo apt-get install libsfml-dev"
    echo "   Fedora:        sudo dnf install SFML-devel"
    echo "   macOS:         brew install sfml"
    echo "2. Check SETUP.md for detailed instructions"
    echo ""
    exit 1
fi

# Build
echo ""
echo "========================================"
echo "Building project..."
echo "========================================"
echo ""

# Detect number of cores
if [ "$(uname)" == "Darwin" ]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=$(nproc)
fi

if ! make -j${CORES}; then
    echo ""
    echo -e "${RED}========================================"
    echo "ERROR: Build failed!"
    echo "========================================${NC}"
    echo ""
    exit 1
fi

echo ""
echo -e "${GREEN}========================================"
echo "Build successful!"
echo "========================================${NC}"
echo ""

# Check what was built
if [ -f "fleet_commander" ]; then
    echo -e "${GREEN}✓ Built: fleet_commander (Terminal version)${NC}"
fi

if [ -f "fleet_commander_gui" ]; then
    echo -e "${GREEN}✓ Built: fleet_commander_gui (GUI version)${NC}"
else
    if [ $SFML_WARNING -eq 1 ]; then
        echo -e "${YELLOW}⚠ GUI version not built (SFML not found)${NC}"
        echo "  Install SFML to build the GUI version"
    fi
fi

echo ""
echo "To run the game:"
echo "  Terminal: ./fleet_commander"
if [ -f "fleet_commander_gui" ]; then
    echo "  GUI:      ./fleet_commander_gui"
fi
echo ""

# Create convenience run script
cd ..
cat > run.sh << 'RUNSCRIPT'
#!/bin/bash
cd "$(dirname "$0")/build"
if [ -f "fleet_commander_gui" ]; then
    ./fleet_commander_gui
else
    ./fleet_commander
fi
RUNSCRIPT

chmod +x run.sh
echo -e "${GREEN}Created run.sh for easy launching${NC}"
echo ""
