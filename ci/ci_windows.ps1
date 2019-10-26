#
# Build script for GitHub Actions.
#

$workspace = (Get-Item -Path ".\").FullName
cd $HOME

git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
cd vst3sdk

mkdir build
cd build
cmake -G"Visual Studio 15 2017 Win64" `
  -DSMTG_MYPLUGINS_SRC_PATH="$workspace" `
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE `
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE `
  ..
cmake --build . --config Release

$plugin_dir = "$HOME\vst3sdk\build\VST3\Release"
foreach ($dir in $plugin_dir) {
  attrib.exe -S $dir /D
}
Copy-Item -Path $plugin_dir "$workspace\vst_windows" -Recurse
