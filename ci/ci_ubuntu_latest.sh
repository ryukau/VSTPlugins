#!/bin/bash
#
# Build script for GitHub Actions.
#

# Without update, some package will be 404.
sudo apt-get update
sudo apt-get install cmake gcc "libstdc++6" libx11-xcb-dev libxcb-util-dev libxcb-cursor-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev libfontconfig1-dev libcairo2-dev libgtkmm-3.0-dev libsqlite3-dev libxcb-keysyms1-dev

cd "$HOME" || exit
git clone https://github.com/steinbergmedia/vst3sdk.git
cd vst3sdk || exit
git submodule update --init --recursive

mkdir build
cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE ..
cmake --build .

# No idea how to set $HOME as path of actions/upload-artifact@v1.
mv "$HOME"/vst3sdk/build/VST3/Release "$GITHUB_WORKSPACE"/vst_ubuntu_latest
