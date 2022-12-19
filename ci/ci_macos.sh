#!/bin/bash
#
# Build script for GitHub Actions.
#

set -e

cd "$HOME" || exit
git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
cd vst3sdk || exit

mkdir build
cd build || exit
cmake -GXcode \
  -DCMAKE_BUILD_TYPE=Release \
  -DSMTG_MYPLUGINS_SRC_PATH="$GITHUB_WORKSPACE" \
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE \
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE \
  -DSMTG_CREATE_PLUGIN_LINK=FALSE \
  ..
cmake --build . -j2 --config Release

# Only moving `*.vst3`. `*.dSYM` are also generated but the sizes are too large.
mkdir "$GITHUB_WORKSPACE"/vst_macOS
mv "$HOME"/vst3sdk/build/VST3/Release/*.vst3 "$GITHUB_WORKSPACE"/vst_macOS
xattr -rc "$GITHUB_WORKSPACE"/vst_macOS
