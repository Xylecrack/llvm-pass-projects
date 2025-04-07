#!/bin/bash
set -e

PASS_PLUGIN="./build/SCEVArrayAccess.so"
PASS_NAME="array-access"
EXAMPLES_DIR="./examples"
PASS_CMD="opt -load-pass-plugin $PASS_PLUGIN -passes=$PASS_NAME -disable-output"
LOGFILE="pass_output.log"

# Check if plugin exists
if [ ! -f "$PASS_PLUGIN" ]; then
    echo "[!] Pass plugin not found: $PASS_PLUGIN"
    exit 1
fi

# Clear previous log file
> "$LOGFILE"

echo "[?] Do you want to run the pass on:"
echo "1) All files in $EXAMPLES_DIR"
echo "2) A specific file"
read -p "Enter choice (1 or 2): " choice

if [ "$choice" == "1" ]; then
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

elif [ "$choice" == "2" ]; then
    read -p "Enter full path to the .ll file: " file
    if [ ! -f "$file" ]; then
        echo "[!] File not found: $file"
        exit 1
    fi
    BASENAME=$(basename "$file")

    {
        echo ""
        echo "================================================================================"
        echo "===> Processing File: $BASENAME"
        echo "================================================================================"
        echo ""
        $PASS_CMD < "$file"
    } >> "$LOGFILE" 2>&1

    echo "[+] Done. Log saved to $LOGFILE"

else
    echo "[!] Invalid choice. Exiting."
    exit 1
fi
