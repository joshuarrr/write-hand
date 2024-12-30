#!/bin/bash

# Stop on error
set -e

# Get number of CPU cores
CORES=$(sysctl -n hw.ncpu)

# Create and enter build directory
mkdir -p build
cd build

# Configure with CMake if needed
if [ ! -f "CMakeCache.txt" ]; then
  cmake ..
fi

# Build just the app (skip installation/bundling for faster iteration)
cmake --build . --target WriteHand -j${CORES}

# Run the app directly from build directory
./WriteHand.app/Contents/MacOS/WriteHand
