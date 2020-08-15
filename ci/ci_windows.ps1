#
# Build script for GitHub Actions.
#

$ErrorActionPreference = "Stop"

cd $GITHUB_WORKSPACE

git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
cd vst3sdk

mkdir build
cd build

# No idea what's happening, but this cmake command stucks.
cmake -G"Visual Studio 16 2019" `
  -DSMTG_MYPLUGINS_SRC_PATH="$GITHUB_WORKSPACE\VSTPlugins" `
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE `
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE `
  ..
cmake --build . -j --config Release

# https://gitlab.com/gitlab-org/gitlab-runner/issues/3194#note_196458158
if (!$?) { Exit $LASTEXITCODE }

$plugin_dir = "$HOME\vst3sdk\build\VST3\Release"
foreach ($dir in $plugin_dir) {
  attrib.exe -S $dir /D
}
Copy-Item -Path $plugin_dir "$GITHUB_WORKSPACE\vst_windows" -Recurse
