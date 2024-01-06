# Build Instruction
Building this repository requires C++20 compatible compiler.

**If this instruction doesn't work, please open issue.**

## General
Following plugins are only available for x86_64 or aarch64.

- CubicPadSynth
- EnvelopedSine
- EsPhaser
- IterativeSinCluster

To stop building these plugins, comment out these plugins in top level `CMakeLists.txt`.

To target a specific x86_64 SIMD instructions, see `common/cmake/simd_x86_64_and_aarch64.cmake`.

## Linux (Ubuntu)
Install required packages. See `Package Requirements` section of the link below.

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

Open terminal and run following command. The command creates `~/code/vst` directory. If you alreadly have `~/code`, it might be better to change `code` in below command to some other name.

```bash
mkdir -p ~/code/vst
cd ~/code/vst

git clone --recursive https://github.com/ryukau/VSTPlugins.git

cd VSTPlugins

# Patch vst3sdk.
# - https://github.com/ryukau/VSTPlugins/issues/48
# - https://github.com/steinbergmedia/vstgui/issues/126
git apply --directory=lib/vst3sdk/vstgui4/ ci/linux_patch/cairographicscontext.patch

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Plugins are built into `~/code/vst/VSTPlugins/build/VST3/Release`. To install plugins, copy them to `$HOME/.vst3`.

### Building on Fedora 32
Required packages are listed below.

```bash
dnf install              \
  make                   \
  cmake                  \
  gcc-c++                \
  sqlite-devel           \
  gtkmm30-devel          \
  xcb-util-devel         \
  xcb-util-cursor-devel  \
  xcb-util-keysyms-devel \
  libxkbcommon-x11-devel \
```

### Building on Debian
Required packages are listed below.

```bash
apt install \
  build-essential \
  cmake \
  libx11-dev \
  libx11-xcb-dev \
  libxcb-util-dev \
  libxcb-cursor-dev \
  libxcb-keysyms1-dev \
  libxcb-xkb-dev \
  libxkbcommon-dev \
  libxkbcommon-x11-dev \
  libfontconfig1-dev \
  libcairo2-dev \
  libfreetype6-dev \
  libpango1.0-dev \
  libgtkmm-3.0-dev \
  libsqlite3-dev
```

## Windows
Install following applications.

- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/)
- [Visual Studio](https://visualstudio.microsoft.com/)

When installing Visual Studio, make sure to check C++ development environment. I use community edition to build plugin localy.

Open Git Bash and run following command. The command creates `~/code/vst` directory. If you alreadly have `~/code`, it might be better to change `code` in below command to some other name.

```bash
mkdir -p ~/code/vst
cd ~/code/vst

git clone --recursive https://github.com/ryukau/VSTPlugins.git

cd VSTPlugins

cmake -S . -B build -G"Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Plugins are built into `~/code/vst/VSTPlugins/build/VST3/Release`. To install plugins, copy them to `/c/Program Files/Common Files/VST3` in Git Bash path, or `C:\Program Files\Common Files\VST3` in Windows path. Administrator privilege is required.

## macOS
Note that I don't have mac, so I can't test this instruction. This instruction is based on build script for [GitHub Actions](https://help.github.com/en/actions/automating-your-workflow-with-github-actions). If you find something wrong, feel free to open issue.

Install following applications.

- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/)
- [Xcode](https://developer.apple.com/xcode/)

Open terminal and run following command.

The command creates `~/code/vst` directory. If you alreadly have `~/code`, it might be better to change `code` in below command to some other name.

```bash
mkdir -p ~/code/vst
cd ~/code/vst

git clone --recursive https://github.com/ryukau/VSTPlugins.git

cd VSTPlugins

cmake -S . -B build -GXcode
cmake --build build --config Release
```

Plugins are built into `~/code/vst/VSTPlugins/build/VST3/Release`. To install plugins, copy them to `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/`.

If CMake is installed from Homebrew, `cmake -S . -B build -GXcode` may fail. In this case, try running following command.

```
sudo xcode-select --reset
```

## Reference
Steinberg's vst3sdk repository on GitHub.

- [GitHub - steinbergmedia/vst3sdk: VST 3 Plug-In SDK](https://github.com/steinbergmedia/vst3sdk)

See also:

- [VST 3 Interfaces: How to use cmake for Building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/cmakeUse.html)
- [VST 3 Interfaces: How to add/create your own VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/addownplugs.html)
