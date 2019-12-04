#!/bin/bash
#
# Build script for GitHub Actions.
#

set -e

cd "$HOME" || exit
git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
cd vst3sdk || exit

# Band-aid solution to fix build failure.
sed '79s/10\.10/10.14/' cmake/modules/Global.cmake > cmake/modules/Global.cmake_tmp
mv cmake/modules/Global.cmake_tmp cmake/modules/Global.cmake

mkdir build
cd build || exit
cmake -GXcode \
  -DSMTG_MYPLUGINS_SRC_PATH="$GITHUB_WORKSPACE" \
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE \
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE \
  ..
cmake --build . --config Release

# No idea how to set $HOME as path of actions/upload-artifact@v1.
mv "$HOME"/vst3sdk/build/VST3/Release "$GITHUB_WORKSPACE"/vst_macOS
