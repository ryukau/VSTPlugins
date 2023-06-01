#
# Build script for GitHub Actions.
#

$ErrorActionPreference = "Stop"

# Disable to build VST3Inspector.
Copy-Item `
  "ci\windows_patch\inspectorapp\CMakeLists.txt" `
  "lib\vst3sdk\public.sdk\samples\vst-hosting\inspectorapp\CMakeLists.txt"

# SMTG_PLUGIN_TARGET_USER_PATH must be set for GitHub Actions. Because cmake can't
# reach the default path which is `C:/Program Files/Common Files/VST3`.
mkdir target
cmake -S . -B build -G"Visual Studio 17 2022" -A x64 `
  -DSMTG_PLUGIN_TARGET_USER_PATH=target `

cmake --build build --config Release

# https://gitlab.com/gitlab-org/gitlab-runner/issues/3194#note_196458158
if (!$?) { Exit $LASTEXITCODE }

$plugin_dir = "build\VST3\Release"
foreach ($dir in $plugin_dir) {
  attrib.exe -S $dir /D
}
Copy-Item -Path $plugin_dir vst_windows -Recurse
