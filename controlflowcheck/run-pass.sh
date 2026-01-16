#!/bin/bash

set -e

LLVM_BUILD_DIR="/home/xylecrack/LLVM/build/bin"
OPT="${LLVM_BUILD_DIR}/opt"
CLANG="clang"

PASS_PLUGIN="./build/ControlFlowCheck.so"
RUNTIME="./build/runtime/libcfcss_rt.a"

TEST_DIR="tests"
OUTPUT_DIR="artifacts"
EXECUTABLE="${OUTPUT_DIR}/test_prog"

INPUT_FILE="$1"

NAME=$(basename "$INPUT_FILE" | sed 's/\.[^.]*$//')

mkdir -p "$OUTPUT_DIR"
INSTRUMENTED_IR="${OUTPUT_DIR}/${NAME}.cfcss.ll"

if [[ "$INPUT_FILE" == *.c ]]; then
    TARGET_LL="${TEST_DIR}/${NAME}.ll"
    "$CLANG" -S -emit-llvm -O0 -Xclang -disable-O0-optnone "$INPUT_FILE" -o "$TARGET_LL"

elif [[ "$INPUT_FILE" == *.ll ]]; then
    TARGET_LL="$INPUT_FILE"
fi

"$OPT" -load-pass-plugin="$PASS_PLUGIN" -passes=cfcss -S "$TARGET_LL" -o "$INSTRUMENTED_IR"

"$CLANG" "$INSTRUMENTED_IR" "$RUNTIME" -o "$EXECUTABLE"

./"$EXECUTABLE"