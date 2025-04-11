#!/bin/bash
set -e
BUILD_DIR="build"
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -G Ninja -DLLVM_DIR=/usr/lib/llvm-19/lib/cmake/llvm ..
ninja
