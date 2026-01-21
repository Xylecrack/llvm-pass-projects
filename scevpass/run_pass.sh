#!/bin/bash
set -e

PASS_PLUGIN="./build/SCEVArrayAccess.so"
PASS_NAME="array-access"
TESTS_DIR="./tests/ir"
OUT_DIR="./tests/out"
PASS_CMD="/home/xylecrack/LLVM/build/bin/opt -load-pass-plugin $PASS_PLUGIN -passes=$PASS_NAME"
LOGFILE="pass_output.log"

mkdir -p "$OUT_DIR"

if [ ! -f "$PASS_PLUGIN" ]; then
    echo "[!] Pass plugin not found: $PASS_PLUGIN"
    exit 1
fi

> "$LOGFILE"

for file in "$TESTS_DIR"/*.ll; do
    BASENAME=$(basename "$file")
    OUTFILE="$OUT_DIR/${BASENAME%.ll}.out.ll"

    {
        echo ""
        echo "--- $BASENAME ---"
        echo ""
        $PASS_CMD "$file" -S -o "$OUTFILE"
    } >> "$LOGFILE" 2>&1
done

echo "[+] Done. Combined log written to $LOGFILE"
