# Build Instruction
**Note: Currently, these instruction is not tested.**

Building this repository requires C++17 compatible compiler and CPU with AVX2 support in default.

Some plugin uses [vector class library](https://github.com/vectorclass/version2) (vcl) to write AVX2 code, and vcl version 2 requires C++17 support.

If you'd like to build without C++17 and/or AVX2 instructions, change `IterativeSinCluster/CMakeLists.txt`. Code path without AVX2 is available.

In `IterativeSinCluster/CMakeLists.txt`, there is following lines. Delete lines that suits your requirement.

```cmake
set(CMAKE_CXX_STANDARD 17) # Delete this line to disable C++17.
if(MSVC)
    add_definitions(/arch:AVX2) # Delete this line to disable AVX2 support on windows.
elseif(UNIX)
    if(APPLE)
        # macOS build doesn't explicitly use AVX2 instructions. Just -O3 provided by vst3sdk.
    else()
        # Delete next line to disable AVX2 support on Linux.
        add_compile_options(-O3 -mavx2 -mfma)
    endif()
endif()
```

## Linux
Install required packages. See `Package Requirements` section of the link below.

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

Open terminal and run following command. The command creates `~/code/vst` directory. If you alreadly have `~/code`, it might be better to change `code` in below command to some other name.

```bash
mkdir -p ~/code/vst
cd ~/code/vst

git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
git clone --recursive https://github.com/ryukau/VSTPlugins.git

mkdir vst3sdk/build
cd vst3sdk/build

cmake -DCMAKE_BUILD_TYPE=Release -DSMTG_MYPLUGINS_SRC_PATH="../../VSTPlugins" -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE ..
cmake --build .
```

After finishing the command, you can find a link to the plugin in `$HOME/.vst3`. The link is pointing to the plugin directory in `~/code/VST_SDK/VST3_SDK/build/VST3/Release/`.

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
cmake --build . --config Release
```

Plugins are built into `~/code/vst3sdk/build/VST3/Release`. To install plugins, copy them to `/c/Program Files/Common Files/VST3` in Git Bash path, or `C:\Program Files\Common Files\VST3` in Windows path.

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
cmake --build . --config Release
```

Plugins are built into `~/code/vst3sdk/build/VST3/Release`. To install plugins, copy them to `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/`.

## Reference
Steinberg's vst3sdk repository on GitHub.

- [GitHub - steinbergmedia/vst3sdk: VST 3 Plug-In SDK](https://github.com/steinbergmedia/vst3sdk)

See also:

- [VST 3 Interfaces: How to use cmake for Building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/cmakeUse.html)
- [VST 3 Interfaces: How to add/create your own VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/addownplugs.html)
