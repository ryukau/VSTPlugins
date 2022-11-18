# Uhhyou Plugins VST 3
This repository is publishing VST 3 plugins under GPLv3.

- [Uhhyou Plugins (Documentation)](https://ryukau.github.io/VSTPlugins/)

Packaged plugins are available at [Releases · ryukau/VSTPlugins](https://github.com/ryukau/VSTPlugins/releases).

You can fund the development via [paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau). Primary goal is to get ARM mac for macOS port.

## Build Instruction
See [build_instruction.md](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

## Audio Demo
Album on the link below is almost entirely made from Uhhyou Plugins.

- [Animals | Ryukau](https://ryukau.bandcamp.com/album/animals)

## Experimental Plugins
Following plugins are experimental, or in α stage. They lack character and/or parameter tuning. Breaking changes might be introduced.

- FoldShaper, ModuloShaper, OddPowShaper, SoftClipper
  - Naive waveshapers with 16-fold oversampling.
- MatrixShifter
  - AM frequency shifter and chorus in a plugin.

## Files and Directories
- `.github/workflows` and `ci`: Continuous integration (CI) scripts for GitHub Actions.
- `common`: Common components across plugins. Including GUI widgets, VST 3 controller, and some DSP code.
- `docs`: Contents of [documentation](https://ryukau.github.io/VSTPlugins/).
- `lib`: External libraries.
- `License`: License texts.
- `package`: Data and scripts to create zip packages from artifacts made by GitHub Actions.
- `presets`: Plugin presets and related scripts.
- `test`: Tests to check reset behavior.

Others are plugin directories. Following is a bit of documentation about source codes under `<PluginName>/source`.

- DSP codes are under `dsp`.
- Plugin specific widgets are under `gui`.
- GUI is defined in `editor.*`.
- Parameters are defined in `parameter.*`.

## License
GPLv3. Complete licenses are available under `License` directory.
