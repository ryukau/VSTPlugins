# Uhhyou Plugins VST 3
This repository is publishing VST 3 plugins under GPLv3.

- [Uhhyou Plugins (Documentation)](https://ryukau.github.io/VSTPlugins/)

Packaged plugins are available at [Releases · ryukau/VSTPlugins](https://github.com/ryukau/VSTPlugins/releases).

LV2 versions are also available. [GitHub - ryukau/LV2Plugins](https://github.com/ryukau/LV2Plugins)

## Build Instruction
See [build_instruction.md](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md) for building plugin.

Building on latest macOS will likely fail. See [macosarmbuild.md](https://github.com/ryukau/VSTPlugins/blob/master/macarmbuild.md) for more details.

## x86_64 CPU Dispatching
To use appropriate SIMD instruction set at runtime, CPU dispatching is implemented. See the note below for the technique used in this repository. It was written for LV2 repository, but the same thing applies to this repository.

- [LV2Plugins/code_walkthrough.md at master · ryukau/LV2Plugins · GitHub](https://github.com/ryukau/LV2Plugins/blob/master/docs/dev_note/code_walkthrough.md#cpu-dispatching)

## Files and Directories
- `.github/workflows` and `ci`: Continuous integration (CI) scripts for GitHub Actions.
- `common`: Common components across plugins. Including GUI widgets, VST 3 controller, and some DSP code.
- `docs`: Contents of [documentation](https://ryukau.github.io/VSTPlugins/).
- `lib`: External libraries.
- `License`: License texts.
- `presets`: Plugin presets and related scripts.
- `test`: Tests to check reset behavior.

Others are plugin directories. Following is a bit of documentation about source codes under `<PluginName>/source`.

- DSP codes are under `dsp`.
- Plugin specific widgets are under `gui`.
- GUI is defined in `editor.*`.
- Parameters are defined in `parameter.*`.

## License
GPLv3. Complete licenses are available under `License` directory.
