#!/bin/bash
set -e
BUILD_DIR="build"

if [[ "$1" == "clean" ]]; then
    echo "Performing clean rebuild..."
    rm -rf "$BUILD_DIR"
    mkdir "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DLLVM_DIR=/home/xylecrack/LLVM/build/lib/cmake/llvm ..
else
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    if [ ! -f CMakeCache.txt ]; then
        cmake -DLLVM_DIR=/home/xylecrack/LLVM/build/lib/cmake/llvm ..
    fi
fi

make
