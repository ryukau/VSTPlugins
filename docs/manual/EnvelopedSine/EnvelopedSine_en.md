---
lang: en
...

# EnvelopedSine
![](img/envelopedsine.png)

EnvelopedSine is an additive synthesizer that computes 64 sine waves for each note. Difference to IterativeSinCluster is that this synth has AD envelope and saturator for each oscillator. EnvelopedSine is better suited for percussive sounds.

- [Download EnvelopedSine 0.1.8 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L4Reverb0.1.0/EnvelopedSine0.1.8.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/EnvelopedSinePresets.zip)

EnvelopedSine requires CPU which supports AVX or later SIMD instructions.

The package includes following builds:

- Windows 64bit
- Linux 64bit
- macOS 64bit

macOS build isn't tested because I don't have Mac. If you found a bug, please file a issue to [GitHub repository](https://github.com/ryukau/VSTPlugins) or send email to `ryukau@gmail.com`.

Linux build is built on Ubuntu 18.0.4 and tested on Bitwig 3.1.2 and Reaper 6.03. Bitwig 3.1.2 seems to have a bug that occasionally blackouts GUI.

## Installation
### Plugin
Place `*.vst3` directory to:

- `/Program Files/Common Files/VST3/` for Windows.
- `$HOME/.vst3/` for Linux.
- `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/` for macOS.

DAW may provides additional VST3 directory. For more information, please refer to the manual of the DAW.

### Presets
Extract preset zip, then place preset directory to the OS specific path:

- Windows : `/Users/$USERNAME/Documents/VST3 Presets/Uhhyou`
- Linux : `$HOME/.vst3/presets/Uhhyou`
- macOS : `/Users/$USERNAME/Library/Audio/Presets/Uhhyou`

Preset directory name must be the same as the plugin. Make `Uhhyou` directory if it does not exist.

### Windows Specific
If DAW doesn't recognize the plugin, try installing C++ redistributable (`vc_redist.x64.exe`). Installer can be found in the link below.

- [The latest supported Visual C++ downloads](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

### Linux Specific
On Ubuntu 18.0.4, those packages are required.

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

If DAW doesn't recognize the plugin, take a look at `Package Requirements` section of the link below and make sure all the VST3 related package is installed.

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

REAPER on Linux may not recognize the plugin. A workaround is to delete a file `~/.config/REAPER/reaper-vstplugins64.ini` and restart REAPER.

## Color Configuration
At first time, create color config file to:

- `/Users/USERNAME/AppData/Roaming/UhhyouPlugins/style/style.json` on Windows.
- `$XDG_CONFIG_HOME/UhhyouPlugins/style/style.json` on Linux.
  - If `$XDG_CONFIG_HOME` is empty, make `$HOME/.config/UhhyouPlugins/style/style.json`.
- `/Users/$USERNAME/Library/Preferences/UhhyouPlugins/style/style.json` on macOS.

Below is a example of `style.json`.

```json
{
  "fontPath": "",
  "foreground": "#ffffff",
  "foregroundButtonOn": "#000000",
  "foregroundInactive": "#8a8a8a",
  "background": "#353d3e",
  "boxBackground": "#000000",
  "border": "#808080",
  "borderCheckbox": "#808080",
  "unfocused": "#b8a65c",
  "highlightMain": "#368a94",
  "highlightAccent": "#2c8a58",
  "highlightButton": "#a77842",
  "highlightWarning": "#8742a7",
  "overlay": "#ffffff88",
  "overlayHighlight": "#00ff0033"
}
```

Hex color codes are used.

- 6 digit color is RGB.
- 8 digit color is RGBA.

First letter `#` is conventional. Plugins ignore the first letter of color code, thus `?102938`, `\n11335577` are valid.

Do not use characters outside of `0-9a-f` for color value.

- `fontPath`: Absolute path to *.ttf font file. Not implemented in VST 3 version.
- `foreground`: Text color.
- `foregroundButtonOn`: Text color of active toggle button. Recommend to use the same value of `foreground` or `boxBackground`.
- `foregroundInactive`: Text color of inactive components. Currently, only used for TabView.
- `background`: Background color.
- `boxBackground`: Background color of inside of box shaped components (Barbox, Button, Checkbox, OptionMenu, TextKnob, VSlider).
- `border`: Border color of box shaped components.
- `borderCheckbox`: Border color of CheckBox.
- `unfocused`: Color to fill unfocused components. Currently, only used for knobs.
- `highlightMain`: Color to indicate focus is on a component. Highlight colors are also used for value of slider components (BarBox and VSlider).
- `highlightAccent`: Same as `highlightMain`. Used for cosmetics.
- `highlightButton`: Color to indicate focus is on a button.
- `highlightWarning`: Same as `highlightMain`, but only used for parameters which requires extra caution.
- `overlay`: Overlay color. Used to overlay texts and indicators.
- `overlayHighlight`: Overlay color to highlight current focus.

## Controls
Knob and slider can do:

- <kbd>Ctrl</kbd> + <kdb>Left Click</kbd>: Reset value.
- <kbd>Shift</kbd> + <kbd>Left Drag</kbd>: Fine adjustment.

There is an additional control for number sliders used for `Octave`, `Seed` etc.

- <kbd>Middle Click</kbd> : Flip minimum and maximum.

Control with many blue vertical bars (BarBox) have some keyboard shortcuts. BarBox is used by `Attack`, `Decay`, `Gain` and `Saturation`. Shortcuts are only enabled after left clicking overtone control. Cheat sheet can be popped up by clicking plugin title on bottom left.

| Input                                   | Control                               |
| --------------------------------------- | ------------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Left Drag</kbd>  | Reset to Default                      |
| <kbd>Shift</kbd> + <kbd>Left Drag</kbd> | Naive Draw (Skip bars between frames) |
| <kbd>Right Drag</kbd>                   | Draw Line                             |
| <kbd>a</kbd>                            | Alternate Sign                        |
| <kbd>d</kbd>                            | Reset Everything to Default           |
| <kbd>D</kbd>                            | Toggle Min/Mid/Max                    |
| <kbd>e</kbd>                            | Emphasize Low                         |
| <kbd>E</kbd>                            | Emphasize High                        |
| <kbd>f</kbd>                            | Low-pass Filter                       |
| <kbd>F</kbd>                            | High-pass Filter                      |
| <kbd>i</kbd>                            | Invert Value (Preserve minimum)       |
| <kbd>I</kbd>                            | Invert Value (Minimum to 0)           |
| <kbd>n</kbd>                            | Normalize (Preserve minimum)          |
| <kbd>N</kbd>                            | Normalize (Minimum to 0)              |
| <kbd>p</kbd>                            | Permute                               |
| <kbd>r</kbd>                            | Randomize                             |
| <kbd>R</kbd>                            | Sparse Randomize                      |
| <kbd>s</kbd>                            | Sort Descending Order                 |
| <kbd>S</kbd>                            | Sort Ascending Order                  |
| <kbd>t</kbd>                            | Subtle Randomize (Random walk)        |
| <kbd>T</kbd>                            | Subtle Randomize (Converge to 0)      |
| <kbd>z</kbd>                            | Undo                                  |
| <kbd>Z</kbd>                            | Redo                                  |
| <kbd>,</kbd> (Comma)                    | Rotate Back                           |
| <kbd>.</kbd> (Period)                   | Rotate Forward                        |
| <kbd>1</kbd>                            | Decrease                              |
| <kbd>2</kbd>-<kbd>9</kbd>               | Decrease 2n-9n                        |

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/envelopedsine.svg)

## Parameters
### Overtone
4 big controls on top right.

Attack, Decay

:   Gain envelope attack and decay for each oscillator.

Gain

:   Gain for each oscillator.

Saturation

:   Saturation gain for each oscillator.

### Gain
Boost, Gain

:   Both controls output gain. Peak value of output will be `Boost * Gain`.

### Pitch
Add Aliasing

:   When checked, the synth enables rendering of sine waves over nyquist frequency.

    Roughly speaking, nyquist frequency is the highest frequency that can be reconstructed from recorded digital signal. When generating sound, it's possible to set value that is higher than nyquist frequency. However, the result may contain unexpected frequency due to a phenomenon called aliasing. `Add Aliasing` is option to add those aliasing noise.

Octave

:   Note octave.

Multiply, Modulo

:   Change sine wave frequency.

    Equation is `noteFrequency * (1 + fmod(Multiply * pitch, Modulo))`. `pitch` is calculated from note pitch and overtone index. `fmod(a, b)` is a function that returns reminder of `a / b`.

Expand

:   Scaling factor for overtone controls.

    ![](img/envelopedsine_expand.svg)

Shift

:   Amount of right shift to overtone controls.

    ![](img/envelopedsine_shift.svg)

### Random
Randomize parameters. These are more effective when used with `Unison`.

Retrigger

:   When checked, reset random seed for each note-on.

Seed

:   Random seed. This value change random number sequence.

To Gain, To Attack, To Decay, To Sat.

:   Amount of randomization to overtone controls. Equation is `value * random`. Range of `random` is `[0.0, 1.0)`.

To Pitch

:   Amount of randomization to pitch.

To Phase

:   Amount of randomization to phase.

### Misc.
Smooth

:   Time length to change some parameter value to current one. Unit is in second.

    List of parameters related to `Smooth`. `*` represents wild card.

    - All parameters in `Gain` section.
    - All parameters in `Phaser` section, except `nStages`.

    Other parameter uses the value obtained from the timing of note-on for entire duration of a note.

nVoices

:   Maximum polyphony. Lowering the number of this option reduces CPU load.

Unison

:   When checked, unison is enabled.

    When unison is enabled, 1 note uses 2 voices. 1 voice is placed on left and other is placed on right. Combining `Unison` with `Random.To Phase`, `Random.To Pitch`, etc. can be used to make sound with stereo spread.

### Modifier
Attack\*, Decay\*

:   Multiplier for `Attack`/`Decay` in overtone control section.

Declick

:   When checked, it reduces click noise that occurs when the value of `Attack` and/or `Decay` is 0.

Gain^

:   Exponent to `Gain` in overtone control. Following is the equation of gain of an oscillator.

    ```
    Gain.Boost * Gain.Gain * pow(Overtone.Gain, Modifier.Gain^)
    ```

Sat. Mix

:   Mixing ratio of dry/wet signal of saturation. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

### Phaser
nStages

:   Number of all-pass filter.

Mix

:   Mixing ratio of dry/wet signal of phaser. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

Freq

:   LFO frequency.

Feedback

:   Amount of feedback. Feedback is disabled when the knob is pointing to 12 o'clock. It becomes negative feedback when turned to left and positive feedback when turned to right.

Range

:   Range of all-pass filter modulation by LFO.

Min

:   Minimum value of all-pass filter modulation by LFO.

Offset

:   LFO phase offset.

Phase

:   LFO phase. This can be used to make sound with automation. Turning `Freq` to leftmost sets LFO frequency to 0.

## Change Log
- 0.1.8
  - Added undo/redo to BarBox.
- 0.1.7
  - Added color configuration.
- 0.1.6
  - Reverted parameter smoother to the old one which works with variable size audio buffer.
- 0.1.5
  - Changed BarBox shortcuts to be disabled when mouse cursor is on the outside of BarBox.
- 0.1.4
  - Fixed a bug that cause crash when drawing string.
- 0.1.3
  - Changed display method for pop-up which shows up by clicking plugin title.
- 0.1.2
  - Fixed a bug that was causing noise on PreSonus Studio One 4.6.1.
- 0.1.1
  - Enabled GUI for Linux build.
  - Added shortcut cheat sheet to splash screen.
- 0.1.0
  - Initial release.

### Old Versions
- [EnvelopedSine 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/EnvelopedSine0.1.7.zip)
- [EnvelopedSine 0.1.6 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/EnvelopedSine0.1.6.zip)
- [EnvelopedSine 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BarBoxFocusFix/EnvelopedSine0.1.5.zip)
- [EnvelopedSine 0.1.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/EnvelopedSine0.1.4.zip)
- [EnvelopedSine 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/EnvelopedSine0.1.2.zip)
- [EnvelopedSine 0.1.1 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/EnvelopedSine0.1.1.zip)
- [EnvelopedSine 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/EnvelopedSine0.1.0.zip)

## License
EnvelopedSine is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
