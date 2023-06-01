#!/bin/bash
#
# Build script for GitHub Actions.
#

set -e

# Without update, some package will be 404.
sudo apt-get update
sudo apt-get install cmake gcc "libstdc++6" libx11-xcb-dev libxcb-util-dev libxcb-cursor-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libfontconfig1-dev libcairo2-dev libgtkmm-3.0-dev libsqlite3-dev libxcb-keysyms1-dev

# Patch vst3sdk.
# - https://github.com/ryukau/VSTPlugins/issues/3
cp \
  ci/linux_patch/cairocontext.cpp \
  lib/vst3sdk/vstgui4/vstgui/lib/platform/linux/cairocontext.cpp

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

mv build/VST3/Release "$GITHUB_WORKSPACE"/vst_ubuntu
