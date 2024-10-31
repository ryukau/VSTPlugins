# Uhhyou Plugins VST 3
This repository is publishing VST 3 plugins under GPLv3.

- [Uhhyou Plugins (Documentation)](https://ryukau.github.io/VSTPlugins/)

The plugins are available at [release page](https://github.com/ryukau/VSTPlugins/releases) (github.com).

- `UhhyouPlugins*.zip` contains everything.
- Other zip files are individual plugins.

To install from a zip package, refer to [a section on the manual](https://ryukau.github.io/VSTPlugins/manual/SevenDelay/SevenDelay_en.html#installation) (`docs/manual/common/contact_installation_guiconfig_*.md`).

To build, refer to [./build_instruction.md](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

To report a bug, open an [issue](https://github.com/ryukau/VSTPlugins/issues) (github.com).

Questions, suggestions, usage tips, and for anything else, feel free to open [dicussion](https://github.com/ryukau/VSTPlugins/discussions) (github.com).

You can fund the development via [paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau).

## Audio Demo
Album on the link below is almost entirely made from Uhhyou Plugins.

- [Animals | Ryukau](https://ryukau.bandcamp.com/album/animals)

## Experimental Plugins
Following plugins are experimental, or in α stage. They lack character and/or parameter tuning. Breaking changes might be introduced.

- FoldShaper, ModuloShaper, OddPowShaper, SoftClipper
  - Naive waveshapers with 16-fold oversampling.
- MatrixShifter
  - AM frequency shifter and chorus in a plugin.

## Git Branches
Following branches are active.

- `master`: Release is published from here.
- `develop`: Changes are gathered here before being released on `master`.
- `docs`: Documentation is published from here.

This branch structure is specific to GitHub Actions and GitHub Pages. When a release tag is pushed, a CI action is triggered to upload zip files. `docs` is separated to verify the links to those zip files before publishing the updated documentation.

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
