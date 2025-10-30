#!/usr/bin/env bash
# Apply all patch files to the submodule

set -euo pipefail

SUBMODULE_PATH="third-party/writing-a-c-compiler-tests"
PATCHES_DIR="patches/writing-a-c-compiler-tests"

if [ ! -d "$PATCHES_DIR" ]; then
    echo "No patches directory found"
    exit 0
fi

cd "$SUBMODULE_PATH"

# Apply patches in numerical order using ls -v for version sorting
for patch in $(ls -v ../../$PATCHES_DIR/*.patch 2>/dev/null); do
    if [ -f "$patch" ]; then
        echo "Applying $(basename "$patch")..."
        git apply "$patch"
    fi
done

echo "All patches applied successfully"
