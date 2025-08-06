#!/bin/bash
set -e

PASS_PLUGIN="./build/SCEVArrayAccess.so"
PASS_NAME="array-access"
EXAMPLES_DIR="./examples"
PASS_CMD="opt -load-pass-plugin $PASS_PLUGIN -passes=$PASS_NAME -disable-output"
LOGFILE="pass_output.log"


if [ ! -f "$PASS_PLUGIN" ]; then
    echo "[!] Pass plugin not found: $PASS_PLUGIN"
    exit 1
fi


> "$LOGFILE"

    echo "[+] Running $PASS_NAME pass on all .ll files in $EXAMPLES_DIR"
    for file in "$EXAMPLES_DIR"/*.ll; do
        BASENAME=$(basename "$file")

        {
            echo ""
            echo "================================================================================"
            echo "===> Processing File: $BASENAME"
            echo "================================================================================"
            echo ""
            $PASS_CMD < "$file"
        } >> "$LOGFILE" 2>&1
    done
    echo "[+] Done. Combined log written to $LOGFILE"

