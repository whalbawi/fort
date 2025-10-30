#!/usr/bin/env bash

set -euo pipefail

SUBMODULE_PATH="third-party/writing-a-c-compiler-tests"
PATCHES_DIR="patches/writing-a-c-compiler-tests"

cd "$SUBMODULE_PATH"

# Check if there are changes
if git diff-index --quiet HEAD --; then
    echo "No changes to save"
    exit 0
fi

# Get the next patch number from tracking file
cd "../../$PATCHES_DIR"
NEXT_NUM_FILE="NEXT_PATCH_NUM"
if [ ! -f "$NEXT_NUM_FILE" ]; then
    echo "0001" > "$NEXT_NUM_FILE"
fi
NEXT_NUM=$(cat "$NEXT_NUM_FILE")

# Prompt for patch description
echo "Enter a brief description for this patch:"
read -r DESCRIPTION
PATCH_NAME="${NEXT_NUM}-${DESCRIPTION}.patch"
PATCH_FILE="$(pwd)/$PATCH_NAME"

# Increment patch number for next time
printf "%04d" $((10#$NEXT_NUM + 1)) > "$NEXT_NUM_FILE"

cd - > /dev/null

# Create patch
git diff HEAD > "$PATCH_FILE"

echo "Saved patch to: $PATCH_FILE"
echo ""
echo "To apply this patch later:"
echo "  cd $SUBMODULE_PATH && git apply ../../$PATCHES_DIR/$PATCH_NAME"
