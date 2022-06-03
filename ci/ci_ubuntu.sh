#!/bin/bash
#
# Build script for GitHub Actions.
#

set -e

# Without update, some package will be 404.
sudo apt-get update
sudo apt-get install cmake gcc "libstdc++6" libx11-xcb-dev libxcb-util-dev libxcb-cursor-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libfontconfig1-dev libcairo2-dev libgtkmm-3.0-dev libsqlite3-dev libxcb-keysyms1-dev

cd "$HOME" || exit
git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
cd vst3sdk || exit

# Patch vst3sdk.
# - https://github.com/ryukau/VSTPlugins/issues/3
patch \
  vstgui4/vstgui/lib/platform/linux/cairocontext.cpp \
  "$GITHUB_WORKSPACE"/ci/linux_patch/cairocontext.cpp.diff

# - https://github.com/steinbergmedia/vstgui/issues/249
patch \
  vstgui4/vstgui/lib/platform/linux/x11frame.cpp \
  "$GITHUB_WORKSPACE"/ci/linux_patch/x11frame.cpp.diff

mkdir build
cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release \
  -DSMTG_MYPLUGINS_SRC_PATH="$GITHUB_WORKSPACE" \
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE \
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE \
  -DSMTG_CREATE_PLUGIN_LINK=FALSE \
  ..
cmake --build .

mv "$HOME"/vst3sdk/build/VST3/Release "$GITHUB_WORKSPACE"/vst_ubuntu
