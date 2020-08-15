#
# Build script for GitHub Actions.
#

$ErrorActionPreference = "Stop"

$uhhyou_dir = "$GITHUB_WORKSPACE\VSTPlugins"

cd $GITHUB_WORKSPACE

git clone `
  https://github.com/steinbergmedia/vst3sdk.git `
  vst3sdk

git sudmodule update --init vst3sdk/base
git sudmodule update --init vst3sdk/cmake
git sudmodule update --init vst3sdk/pluginterfaces
git sudmodule update --init vst3sdk/public.sdk
git sudmodule update --init vst3sdk/vstgui4

tree

mkdir build
cd build

echo "///////////////////////////// in build"

cmake -G "Visual Studio 16 2019" -A x64 `
  -DSMTG_MYPLUGINS_SRC_PATH="$uhhyou_dir" `
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE `
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE `
  ../vst3sdk
cmake --build . -j --config Release

# https://gitlab.com/gitlab-org/gitlab-runner/issues/3194#note_196458158
if (!$?) { Exit $LASTEXITCODE }

$plugin_dir = "$GITHUB_WORKSPACE\build\VST3\Release"
Copy-Item -Path $plugin_dir "$GITHUB_WORKSPACE\vst_windows" -Recurse
