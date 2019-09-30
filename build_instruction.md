# Build Instruction
See also:

- [VST 3 Interfaces: How to use cmake for Building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/cmakeUse.html)
- [VST 3 Interfaces: How to add/create your own VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/addownplugs.html)

## Linux
Tested on clean installed Xubuntu 18.0.4.

Install required packages. See `Package Requirements` section of the link below.

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

Open terminal and execute the command below.

The command creates `$HOME/code` directory. If you alreadly have `$HOME/code`, it might be better to change `code` to some other name.

The command uses wget to download VST3 SDK. You can manualy download VST3 SDK from Steinberg website.

- [Developers | Steinberg](https://www.steinberg.net/en/company/developers.html)

```bash
cd ~
mkdir -p code
cd code
wget https://download.steinberg.net/sdk_downloads/vstsdk3613_08_04_2019_build_81.zip
unzip vstsdk3613_08_04_2019_build_81.zip
cd VST_SDK
git clone https://github.com/ryukau/VSTPlugins.git
cd VST3_SDK
mkdir build; cd build
cmake -DCMAKE_BUILD_TYPE=Release -DSMTG_MYPLUGINS_SRC_PATH="../../VSTPlugins" -DSMTG_ADD_VST3_HOSTING_SAMPLES=FALSE -DSMTG_ADD_VST3_PLUGINS_SAMPLES=FALSE ..
cmake --build .
```

After finishing the command, you can find a link to the plugin in `$HOME/.vst3`. The link is pointing to the plugin directory in `~/code/VST_SDK/VST3_SDK/build/VST3/Release/`.
