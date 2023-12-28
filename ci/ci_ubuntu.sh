#!/bin/bash
#
# Build script for GitHub Actions.
#

set -e

# Without update, some package will be 404.
sudo apt-get update
sudo apt-get install cmake gcc "libstdc++6" libx11-xcb-dev libxcb-util-dev libxcb-cursor-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libfontconfig1-dev libcairo2-dev libgtkmm-3.0-dev libsqlite3-dev libxcb-keysyms1-dev

# Patch vst3sdk.
# - https://github.com/ryukau/VSTPlugins/issues/48
# - https://github.com/steinbergmedia/vstgui/issues/126
cd lib/vst3sdk/vstgui4/
git apply ../../../ci/linux_patch/cairographicscontext.patch
cd ../../../

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

mv build/VST3/Release "$GITHUB_WORKSPACE"/vst_ubuntu

find "$GITHUB_WORKSPACE"/vst_ubuntu # debug
