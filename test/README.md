# Test
This is kind of a mess. I'll probably forget what I was doing, so wrote this text.

The current goal of the tests is to ensure that:

- The order of parameter is not broken.
- The output of release build is almost equal to the output of previous version.
- The output of first run is almost equal to the output of second run which is rendered after calling `DSPCore*::reset()`.
- The output only contains finite floating point values.

## Setup
Testing requires presets in JSON format. See `presets` directory on the top level of this repository.

### Windows
Install vcpkg.

- [GitHub - microsoft/vcpkg: C++ Library Manager for Windows, Linux, and MacOS](https://github.com/Microsoft/vcpkg)

Install libsndfile (64 bit). Commands in this section are tested on PowerShell.

```ps1
vcpkg install libsndfile:x64-windows
```

Run following command to prepare CMake.

```ps1
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE="/src/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
```

To build, run following command in `test/build` directory. If it exhausts the memory, remove `-j` option.

```ps1
cmake --build . -j
```

## Usage
1. Go back to the commit before the change (`git stash` etc.).
2. Run test.
3. Change directory to `test/build/snd/<PluginName>`.
4. Rename `run1_init` directory to `reference`.
5. Reapply the change (`git stash pop` etc.).
6. Run test.

The tests compares the files in `reference` directory to the other directories.

- `run1_init` is the outputs of first run after initialization.
- `run2_reset` is the outputs of second run after calling `DSPCore*::reset()` without changing parameters.

If the output of current code contains `NaN` or `±inf`, the test reports following error.

```
Error <PresetName>.wav <RunName>: Non-finite value -nan(ind) at channel 0, frame 48000
```

- `<PresetName>` can be found in `presets/json/*.preset.json`.
- `<RunName>` is `init` or `reset`.

If the output of current code is **not** almost equal to previous output, the test reports following error.

```
Error <PresetName>.wav <RunName>: actual 8.89269e-08 and expected 8.89136e-08 are not almost equal at channel 0, frame 952
```

## Notes
Tests are sensitive to compiler options. The output of debug build may not be the same as the output of release build.

On `cl.exe` (MSVC compiler), following options are different on debug and release:

- With and without `/O2`.
- With and without `/fp:fast`.

Even if the test fails, it might sound almost same to the human ear. In this case, [Audacity](https://www.audacityteam.org/) can be used to debug visually. Following are the steps to verify that 2 sounds are almost same.

1. Load sound `A` and sound `B`.
2. Select entire data of `A`.
3. Open Effect menu, then select Invert.
4. Select all data with <kbd>Ctrl</kbd> + <kbd>A</kbd>.
5. Open Tracks menu, then select Mix -> Mix and Render to New Track (<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>M</kbd>).

If the 2 sounds are almost same, the new track made at step 5 should be almost silent. To detect residue, try following steps.

1. Select entire data of the new track.
2. Open Effect menu, then select Amplify.

Amplify is useful when checking the noise that appears on specific timing like note-on.
