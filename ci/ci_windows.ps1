#
# Build script for GitHub Actions.
#

$ErrorActionPreference = "Stop"

$uhhyou_dir = "$GITHUB_WORKSPACE\VSTPlugins"

cd $GITHUB_WORKSPACE

git clone --recursive `
  https://github.com/steinbergmedia/vst3sdk.git `
  vst3sdk

mkdir build
cd Build
cmake -G"Visual Studio 16 2019" `
  -DSMTG_MYPLUGINS_SRC_PATH="$uhhyou_dir" `
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE `
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE `
  ../vst3sdk
cmake --build . -j --config Release

# https://gitlab.com/gitlab-org/gitlab-runner/issues/3194#note_196458158
if (!$?) { Exit $LASTEXITCODE }

$plugin_dir = "$GITHUB_WORKSPACE\build\VST3\Release"
Copy-Item -Path $plugin_dir "$uhhyou_dir\vst_windows" -Recurse
