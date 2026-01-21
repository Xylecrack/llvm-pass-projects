#!/bin/bash
set -e

OUT_DIR="./tests/ir"

process_file() {
    local CFILE="$1"
    local BASENAME=$(basename "$CFILE" .c)
    local OPTFILE="${OUT_DIR}/${BASENAME}.opt.ll"

    echo "[+] Compiling $CFILE to LLVM IR..."
    clang -S -emit-llvm "$CFILE" -o "$OPTFILE" -Xclang -disable-O0-optnone

    echo "[+] Running opt passes on $OPTFILE..."
    /home/xylecrack/LLVM/build/bin/opt -passes="mem2reg,loop-simplify,instcombine,indvars" "$OPTFILE" -S -o "$OPTFILE"

    echo "[+] Done. Optimized IR written to $OPTFILE"
    echo
}

if [ $# -eq 0 ]; then
    for file in tests/src/*.c; do
        process_file "$file"
    done
elif [ $# -eq 1 ]; then
    process_file "$1"
fi
