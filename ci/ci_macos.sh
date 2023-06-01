#!/bin/bash
#
# Build script for GitHub Actions.
#

set -e

cmake -S . -B build -GXcode
cmake --build build --config Release

# Only moving `*.vst3`. `*.dSYM` are also generated but the sizes are too large.
mkdir "$GITHUB_WORKSPACE"/vst_macOS
mv build/VST3/Release/*.vst3 "$GITHUB_WORKSPACE"/vst_macOS
xattr -rc "$GITHUB_WORKSPACE"/vst_macOS
