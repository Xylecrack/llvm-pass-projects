#!/bin/bash
set -e
BUILD_DIR="build"

if [[ "$1" == "clean" ]]; then
    echo "Performing clean rebuild..."
    rm -rf "$BUILD_DIR"
    mkdir "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -G Ninja -DLLVM_DIR=/usr/lib/llvm-19/lib/cmake/llvm ..
else
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    if [ ! -f CMakeCache.txt ]; then
        cmake -G Ninja -DLLVM_DIR=/usr/lib/llvm-19/lib/cmake/llvm ..
    fi
fi

ninja
