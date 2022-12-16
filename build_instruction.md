# Build Instruction
Building this repository requires C++17 compatible compiler and CPU with AVX or later instruction set support. Some plugins use [vector class library](https://github.com/vectorclass/version2) (vcl) to write SIMD code, and vcl version 2 requires C++17 support.

**If this instruction doesn't work, please open issue.**

## General
Following plugins are only available for x86_64 or aarch64.

- CubicPadSynth
- EnvelopedSine
- EsPhaser
- IterativeSinCluster

To disable building them, comment out those plugins in top level `CMakeLists.txt`.

To target a specific x86_64 SIMD instructions, see `common/cmake/simd_x86_64_and_aarch64.cmake`.

## Linux
Install required packages. See `Package Requirements` section of the link below.

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

Open terminal and run following command. The command creates `~/code/vst` directory. If you alreadly have `~/code`, it might be better to change `code` in below command to some other name.

```bash
mkdir -p ~/code/vst
cd ~/code/vst

git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
git clone --recursive https://github.com/ryukau/VSTPlugins.git

# Patch vst3sdk.
# - https://github.com/ryukau/VSTPlugins/issues/3
cp \
  VSTPlugins/ci/linux_patch/cairocontext.cpp \
  vst3sdk/vstgui4/vstgui/lib/platform/linux/cairocontext.cpp

mkdir vst3sdk/build
cd vst3sdk/build

cmake -DCMAKE_BUILD_TYPE=Release -DSMTG_MYPLUGINS_SRC_PATH="../../VSTPlugins" -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE ..
cmake --build . -j
```

After finishing the command, you can find a link to the plugin in `$HOME/.vst3`. The link is pointing to the plugin directory in `~/code/vst/VST_SDK/VST3_SDK/build/VST3/Release/`.

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

### Building for non x86_64 CPU
Following plugins are only available for x86_64 because of SIMD instructions.

- CubicPadSynth
- EnvelopedSine
- EsPhaser
- IterativeSinCluster

There's a switch `DISABLE_X86_64_PLUGINS` to disable the build of above plugins. Following is an example `cmake` command.

```bash
cmake \
  -DDISABLE_X86_64_PLUGINS=TRUE \
  -DCMAKE_BUILD_TYPE=Release \
  -DSMTG_MYPLUGINS_SRC_PATH="../../VSTPlugins" \
  -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE \
  -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE \
  ..
```

- [Build failure on aarch64 · Issue #21 · ryukau/VSTPlugins · GitHub](https://github.com/ryukau/VSTPlugins/issues/21)

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

git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
git clone --recursive https://github.com/ryukau/VSTPlugins.git

mkdir vst3sdk/build
cd vst3sdk/build

cmake -G"Visual Studio 16 2019" -A x64 -DSMTG_MYPLUGINS_SRC_PATH="../../VSTPlugins" -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE ..
cmake --build . -j --config Release
```

Plugins are built into `~/code/vst3sdk/build/VST3/Release`. In default, link of plugins are made to `/c/Program Files/Common Files/VST3` in Git Bash path, or `C:\Program Files\Common Files\VST3` in Windows path.

If the plugin already exists in `C:\Program Files\Common Files\VST3`, following error may appear.

```
error MSB3073: mklink "C:\Program Files\Common Files\VST3\SomePlugin.vst3"  C:\<somepath>\vst3sdk\build\VST3\Release\SomePlugin.vst3
```

In this case, add `-DSMTG_CREATE_PLUGIN_LINK=FALSE` to cmake option.

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

git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
git clone --recursive https://github.com/ryukau/VSTPlugins.git

mkdir vst3sdk/build
cd vst3sdk/build

cmake -GXcode -DSMTG_MYPLUGINS_SRC_PATH="../../VSTPlugins" -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE ..
cmake --build . -j --config Release
```

Plugins are built into `~/code/vst3sdk/build/VST3/Release`. To install plugins, copy them to `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/`.

## Reference
Steinberg's vst3sdk repository on GitHub.

- [GitHub - steinbergmedia/vst3sdk: VST 3 Plug-In SDK](https://github.com/steinbergmedia/vst3sdk)

See also:

- [VST 3 Interfaces: How to use cmake for Building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/cmakeUse.html)
- [VST 3 Interfaces: How to add/create your own VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/addownplugs.html)
