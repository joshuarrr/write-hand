#!/bin/bash

# Kill any running instances of the app
pkill -f WriteHand

# Clean build directory
rm -rf build
mkdir build
cd build

# Configure and build
cmake ..
make -j4

# Install and run
make install
open WriteHand.app
