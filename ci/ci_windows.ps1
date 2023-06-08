#
# Build script for GitHub Actions.
#

$ErrorActionPreference = "Stop"

cmake -S . -B build -G"Visual Studio 17 2022" -A x64
cmake --build build --config Release

# https://gitlab.com/gitlab-org/gitlab-runner/issues/3194#note_196458158
if (!$?) { Exit $LASTEXITCODE }

$plugin_dir = "build\VST3\Release"
foreach ($dir in $plugin_dir) {
  attrib.exe -S $dir /D
}
Copy-Item -Path $plugin_dir vst_windows -Recurse

tree /A /F vst_windows # debug
