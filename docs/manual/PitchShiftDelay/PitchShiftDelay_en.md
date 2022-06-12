---
lang: en
...

# PitchShiftDelay
![](img/pitchshiftdelay.png)

PitchShiftDelay is a time domain pitch shifter based on usual delay. The quirk of pitch shifter is somewhat reduced by 16x oversampling. Also the internal buffer length can be changed in real time. PitchShiftDelay doesn't equipped with formant correction. So applying to voice may sounds strange, like Helium is inhaled.

- [Download PitchShiftDelay 0.2.0 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.34.0/PitchShiftDelay_0.2.0.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
<!-- - [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.34.0/PitchShiftDelayPresets.zip) -->

The package includes following builds:

- Windows 64bit
- Linux 64bit
- macOS universal binary

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely to run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

## Contact
Feel free to contact me on [GitHub repository](https://github.com/ryukau/VSTPlugins/commits/master) or `ryukau@gmail.com.`

You can fund the development through [paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau).  Current goal is to purchase M1 mac for better macOS support. 💸💻

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

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

REAPER on Linux may not recognize the plugin. A workaround is to delete a file `~/.config/REAPER/reaper-vstplugins64.ini` and restart REAPER.

### macOS Specific
When trying to run plugin first time, following message may appear on macOS.

```
<PluginName>.vst3 is damaged and can't be opened. You should move it to
the Trash"
```

In this case, open terminal and try running following command to unzipped `.vst3` directory.

```sh
xattr -rc /path/to/PluginName.vst3
```

Plugin may be considered as unsigned/un-notarized application. In this case, try following the steps below.

1. Open terminal and run `sudo spctl --master-disable`.
2. Go to System Preferences → Security and Privacy → General → Allow apps downloaded from, then select "Anywhere".

Beware that steps above degrades security of your system. To revert the settings, follow the steps below.

1. Go to System Preferences → Security and Privacy → General → Allow apps downloaded from, then select option to "App Store and identified developers".
2. Open terminal and run `sudo spctl --master-enable`.

#### Reference
- [How to Fix App “is damaged and can’t be opened. You should move it to the Trash” Error on Mac](https://osxdaily.com/2019/02/13/fix-app-damaged-cant-be-opened-trash-error-mac/)
- [Allowing unsigned/un-notarized applications/plugins in Mac OS | Venn Audio](https://www.vennaudio.com/allowing-unsigned-un-notarized-applications-plugins-in-mac-os/)
- [Safely open apps on your Mac - Apple Support](https://support.apple.com/en-us/HT202491)

## GUI Style Configuration
At first time, create color config file to:

- `/Users/USERNAME/AppData/Roaming/UhhyouPlugins/style/style.json` on Windows.
- `$XDG_CONFIG_HOME/UhhyouPlugins/style/style.json` on Linux.
  - If `$XDG_CONFIG_HOME` is empty, make `$HOME/.config/UhhyouPlugins/style/style.json`.
- `/Users/$USERNAME/Library/Preferences/UhhyouPlugins/style/style.json` on macOS.

Below is a example of `style.json`.

```json
{
  "fontFamily": "Tinos",
  "fontBold": true,
  "fontItalic": true,
  "foreground": "#000000",
  "foregroundButtonOn": "#000000",
  "foregroundInactive": "#8a8a8a",
  "background": "#ffffff",
  "boxBackground": "#ffffff",
  "border": "#000000",
  "borderCheckbox": "#000000",
  "borderLabel": "#000000",
  "unfocused": "#dddddd",
  "highlightMain": "#0ba4f1",
  "highlightAccent": "#13c136",
  "highlightButton": "#fcc04f",
  "highlightWarning": "#fc8080",
  "overlay": "#00000088",
  "overlayHighlight": "#00ff0033"
}
```

### Font Options
Following is a list of font options.

- `fontFamily`: Font family name.
- `fontBold`: Enable **bold** style when `true`, disable when `false`.
- `fontItalic`: Enable *italic* style when `true`, disable when `false`.

To use custom font, place `*.ttf` file into custom font path: `*.vst3/Contents/Resources/Fonts`.

**Notice**: If the combination of `fontFamily`, `fontBold`, `fontItalic` is not exists in custom font path, default font of VSTGUI is used.

If `fontFamily` is set to empty string `""`, then [`"Tinos"`](https://fonts.google.com/specimen/Tinos) is used as fallback. If the length is greater than 1 and the font family name doesn't exists, default font of VSTGUI is used.

Styles other than bold, italic or bold-italic are not supported by VSTGUI. For example, "Thin", "Light", "Medium", and "Black" weights cannot be used.

### Color Options
Hex color codes are used.

- 6 digit color is RGB.
- 8 digit color is RGBA.

First letter `#` is conventional. Plugins ignore the first letter of color code, thus `?102938`, `\n11335577` are valid.

Do not use characters outside of `0-9a-f` for color value.

Following is a list of color options. If an option is missing, default color will be used.

- `foreground`: Text color.
- `foregroundButtonOn`: Text color of active toggle button. Recommend to use the same value of `foreground` or `boxBackground`.
- `foregroundInactive`: Text color of inactive components. Currently, only used for TabView.
- `background`: Background color.
- `boxBackground`: Background color of inside of box shaped components (Barbox, Button, Checkbox, OptionMenu, TextKnob, VSlider).
- `border`: Border color of box shaped components.
- `borderCheckbox`: Border color of CheckBox.
- `borderLabel`: Line color of parameter section label.
- `unfocused`: Color to fill unfocused components. Currently, only used for knobs.
- `highlightMain`: Color to indicate focus is on a component. Highlight colors are also used for value of slider components (BarBox and VSlider).
- `highlightAccent`: Same as `highlightMain`. Used for cosmetics.
- `highlightButton`: Color to indicate focus is on a button.
- `highlightWarning`: Same as `highlightMain`, but only used for parameters which requires extra caution.
- `overlay`: Overlay color. Used to overlay texts and indicators.
- `overlayHighlight`: Overlay color to highlight current focus.

## Controls
Knob and number slider can do:

- <kbd>Ctrl</kbd> + <kdb>Left Click</kbd>: Reset value.
- <kbd>Shift</kbd> + <kbd>Left Drag</kbd>: Fine adjustment.
- <kbd>Middle Click</kbd> : Rotate through minimum, default and maximum value.

Control with many blue vertical bars (BarBox) have some keyboard shortcuts. `LFO Wave` on Main tab and `Gain`, `Width`, `Pitch`, `Phase` on Wavetable tab are using BarBox. Shortcuts are enabled after left clicking BarBox and mouse cursor is on the inside of BarBox. Cheat sheet is available on Infomation tab.

| Input                                                      | Control                            |
| ---------------------------------------------------------- | ---------------------------------- |
| <kbd>Left Drag</kbd>                                       | Change Value                       |
| <kbd>Shift</kbd> + <kbd>Left Drag</kbd>                    | Change Value (Snapped)             |
| <kbd>Ctrl</kbd> + <kbd>Left Drag</kbd>                     | Reset to Default                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Left Drag</kbd>  | Change Value (Skip Between Frames) |
| <kbd>Right Drag</kbd>                                      | Draw Line                          |
| <kbd>Shift</kbd> + <kbd>Right Drag</kbd>                   | Edit One Bar                       |
| <kbd>Ctrl</kbd> + <kbd>Right Drag</kbd>                    | Reset to Default                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Right Drag</kbd> | Toggle Lock                        |
| <kbd>a</kbd>                                               | Alternate Sign                     |
| <kbd>d</kbd>                                               | Reset Everything to Default        |
| <kbd>D</kbd>                                               | Toggle Min/Mid/Max                 |
| <kbd>e</kbd>                                               | Emphasize Low                      |
| <kbd>E</kbd>                                               | Emphasize High                     |
| <kbd>f</kbd>                                               | Low-pass Filter                    |
| <kbd>F</kbd>                                               | High-pass Filter                   |
| <kbd>i</kbd>                                               | Invert Value (Preserve minimum)    |
| <kbd>I</kbd>                                               | Invert Value (Minimum to 0)        |
| <kbd>l</kbd>                                               | Toggle Lock under Mouse Cursor     |
| <kbd>L</kbd>                                               | Toggle Lock for All                |
| <kbd>n</kbd>                                               | Normalize (Preserve minimum)       |
| <kbd>N</kbd>                                               | Normalize (Minimum to 0)           |
| <kbd>p</kbd>                                               | Permute                            |
| <kbd>r</kbd>                                               | Randomize                          |
| <kbd>R</kbd>                                               | Sparse Randomize                   |
| <kbd>s</kbd>                                               | Sort Descending Order              |
| <kbd>S</kbd>                                               | Sort Ascending Order               |
| <kbd>t</kbd>                                               | Subtle Randomize (Random walk)     |
| <kbd>T</kbd>                                               | Subtle Randomize (Converge to 0)   |
| <kbd>z</kbd>                                               | Undo                               |
| <kbd>Z</kbd>                                               | Redo                               |
| <kbd>,</kbd> (Comma)                                       | Rotate Back                        |
| <kbd>.</kbd> (Period)                                      | Rotate Forward                     |
| <kbd>1</kbd>                                               | Decrease                           |
| <kbd>2</kbd>-<kbd>9</kbd>                                  | Decrease 2n-9n                     |

Snapping is not available for all BarBox. If you'd like to have snapping for certain BarBox, feel free to open issue to [GitHub repository](https://github.com/ryukau/VSTPlugins).

Edit One Bar with <kbd>Shift</kbd> + <kbd>Right Drag</kbd> holds a bar under the cursor when mouse right button is pressed. Then only changes that one bar while holding down mouse right button.

Toggle Lock with <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Right Drag</kbd> behaves as line edit. When right mouse button (RMB) is pressed, it holds the opposite state of the bar below mouse cursor, then use the state for the rest of bars. For example, if RMB is pressed on a locked bar, dragging unlocks bars while holding down RMB.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/pitchshiftdelay.svg)

## Parameters
### Delay
Pitch

:   Main pitch shift amount.

    Main shifter multiplies pitch by the value of `Pitch` + LFO amount.

    Because of the 16 fold oversampling, aliasing noise don't affect the output when `Pitch` is below `31` which is equal to `16 * 2 - 1`.

Offset

:   Unison shifter pitch offset from `Pitch`.

    Unison shifter pitch is calculated by following equation:

    ```
    unisonPitch = mainPitch;
    if (Mirror) unisonPitch = max(1 / unisonPitch, 1000);
    if (S2 Reverse) unisonPitch = -unisonPitch;
    unisonPitch += ((To Pitch) + (To Unison)) * lfo;
    ```

Pitch Cross

:   Cross feedback amount between main shifter and unison shifter.

Mirror

:   When checked, it changes unison shifter pitch to `1 / Pitch`.

    Checking `Mirror` after initial state results in that the feedback pitch will always be `Pitch` or `1 / Pitch`. This state will be kept until one of `Offset`, `Pitch Cross`, `To Pitch`, or `To Unison` is changed from default.

S1 Reverse

:   Reverse the playback of main shifter.

    Internally, `S1 Reverse` changes the sign of `Pitch` to negative to moving the read pointer to reverse direction.

S2 Reverse

:   Reverse the playback of unison shifter.

L-R Lean

:   The ratio of `Delay Time` between left and right channels. `L-R Lean` also works when `Channel Type` is `M-S`.

Delay Time \[s\]

:   Buffer length of pitch shifter. Note that the value only accurate when `Pitch` is set to 1.

    The exact delay time depends on the pitch shift amount and buffer length. LFO farther complicates the computation of exact delay time. So the value of `Delay Time` is usually a rough indication rather than exact delay time.

Feedback

:   Feedback gain in amplitude.

Stereo Cross

:   Cross feedback amount between stereo channels. When the value is set to `1.0`, output becomes ping-pong delay.

Channel Type

:   Select input channel type from left-right (`L-R`) or mid-side (`M-S`).

Highpass \[Hz\]

:   Cutoff frequency of highpass filter inserted on feedback path.

### Mix
Dry \[dB\]

:   Input bypass gain.

Wet \[dB\]

:   Output gain of the signal that go through PitchShiftDelay.

Unison Mix

:   Mixing ratio of main shifter and unison shifter.

Smoothing \[s\]

:   Paramter smoothing time in seconds.

    For example, if the value of `Smoothing` is set to `0.01` or something short, the change of parameter almost immediately applies. So it sounds more snappy, but may introduce audible pop noise.

    On the other hand, if thevalue of `Smoothing is set to `1.0` for example, the change of parameter is only slowly followed. So when changing `Pitch`, it may sounds like long slide/portamento.

### LFO
Sync.

:   When checked, it enables tempo synchronization (tempo sync). LFO phase also synchronize to the exact position derived from sync interval, and time passed from the start of playback.

    When not checked, LFO behaves as same as if synchronizing to 120 BPM. It also disables LFO phase sync.

Tempo Upper

:   Numerator of sync interval.

    LFO synchronizes to 1 bar when `(Tempo Upper) / (Tempo Lower)` is `1/1`. `1/4` synchronizes to 1 beat when time signature of `4/4`. Note that `Rate` multiplies the sync interval.

    Following is the equation to calculate sync interval.

    ```
    syncInterval = (Rate) * (Tempo Upper) / (Tempo Lower);
    ```

Tempo Lower

:   Denominator of sync interval.

    Note that `Rate` multiplies the sync interval.

Rate

:   Multiplier to sync interval.

    `Rate` is convenient when changing sync interval while keeping the values of `Tempo Upper` and `Tempo Lower`.

Stereo Offset

:   LFO phase offset between stereo channels.

Unison Offset

:   LFO phase offset between main shifter and unison shifter.

Wave Interp.

:   LFO wave form interpolation method.

    - `Step`: Holding.
    - `Linear`: Linear interpolation.
    - `PCHIP`: Monotonic cubic interpolation.

    `Step` makes LFO to act like a sequencer. `Linear` sounds similar to `PCHIP`, and the computation cost is lower. `PCHIP` is the most smooth among all 3.

To Pitch

:   LFO modulation amount to main pitch.

To Unison

:   LFO modulation amount to unison shifter pitch that is added to `To Pitch` amount.

## Change Log
- 0.2.0
  - Initial release.

## Old Versions
Old version is not available.

## License
PitchShiftDelay is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
