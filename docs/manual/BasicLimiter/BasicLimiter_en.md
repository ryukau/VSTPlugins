---
lang: en
...

# BasicLimiter
![](img/basiclimiter.png)

BasicLimiter is a basic single band limiter. The sound is nothing new, but the design of true peak mode is a bit luxurious.

- [Download BasicLimiter 0.1.4 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/BasicLimiter_0.1.4.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/BasicLimiterPresets.zip)

An extended version BasicLimiterAutoMake is also available. Added features are automatic make up gain, sidechain, and switching between left-right (L-R) and mid-side (M-S). Note that CPU load is over 1.5 times heavier than BasicLimiter.

- [Download BasicLimiterAutoMake 0.1.4 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/BasicLimiterAutoMake_0.1.4.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">

BasicLimiter and BasicLimiterAutoMake requires CPU which supports AVX or later SIMD instructions.

The package includes following builds:

- Windows 64bit
- Linux 64bit

On 2022-05-15, macOS is not supported. Current code fails to build as macOS universal binary. I'm planning to support macOS in future. However due to my financial situation, I can't purchase M1 mac for now.

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

## Contact
Feel free to contact me on [GitHub repository](https://github.com/ryukau/VSTPlugins/commits/master) or `ryukau@gmail.com.`

You can fund the development through [paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau).  Current goal is to purchase M1 mac for macOS and ARM port. 💸💻

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
**Important**: Current package do not contain mac OS build.

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

**Important**: If the combination of `fontFamily`, `fontBold`, `fontItalic` is not exists in custom font path, default font of VSTGUI is used.

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

## Caution
The algorithm used in BasicLimiter causes over-limiting when input amplitude is extremely high. When over-limiting happens, higher input amplitude turns into lower output amplitude. This problems is expected to happen when input amplitude exceeds `2^53`, or 319 in decibel.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/basiclimiter.svg)

## Parameters
Threshold \[dB\]

:   Threshold of limiter output amplitude.

    When `True Peak` is off, output amplitude is limited under `Threshold`.

    When `True Peak` is on, output amplitude might exceeds `Threshold`. This is due to the limitation of real-time algorithm.

Gate \[dB\]

:   Threshold of gate. When input amplitude is less than the value of `Gate` for the duration of `Attack`, output amplitude becomes 0.

Attack \[s\]

:   Transition time of smoothing filter which applies to internal envelope. The value of `Attack` also adds to the latency.

    For the sound with sharp transitions, like drums, recommend to set `Attack` under 0.02 seconds. This 0.02 seconds comes from a psychoacoustic effect called [temporal masking](https://ccrma.stanford.edu/~bosse/proj/node21.html).

Release \[s\]

:   Smoothness to reset internal envelope to neutral position.

    Internally, the inverse value of `Release` is used as cutoff frequency. This means that the indicated value is not exact. For usual case, recommend to set the sum of `Release` and `Sustain` under 0.2 seconds. This is based on temporal masking.

Sustain \[s\]

:   Additional peak hold time for the internal envelope.

    `Sustain` causes more ducking when applied to sounds like drums. For sounds like distorted guitar or sustaining synthesizer, `Sustain` might sounds cleaner than `Release`, because it works similar to auto-gain. Note that the sustain on limiter envelope doesn't match the curve of temporal masking.

Stereo Link

:   When `Stereo Link` is set to 0.0, limiter for each channel works independently. When set to 1.0, maximum of all the input amplitude is used for both limiter. Following snippet shows the calculation of `Stereo Link`

    ```
    absL = fabs(leftInput).
    absR = fabs(rightInput).

    absMax = max(absL, absR).

    amplitudeL = absL + stereoLink * (absMax - absL).
    amplitudeR = absR + stereoLink * (absMax - absR).
    ```

    When `Stereo Link` is set to 0.0, and input amplitude is leaned to left or right, it may sounds like the pan is wobbling. So for fine tuning, set the value to 0.0, gradually increase the value to 1.0, then stop when the wobbling is gone.

True Peak

:   Enables true peak mode when checked.

    While true peak mode is enabled, lowpass filter is applied to remove the components near nyquist frequency. Also, sample peak might exceeds `Threshold`. Especially when sample peak exceeds 0 dB, the value of `Overshoot` becomes greater than 0. Lower `Threshold` in this case.

    In real time processing, restoration of true peak around nyquist frequency is almost impossible. That's why the lowpass is applied, and this true peak mode causes overshoot. If the aim is to eliminate overshoot at any cost, the lowpass will either change the gain of audible frequency, or burn out your CPU. The lowpass on BasicLimiter is designed to only change the gain over 18000 Hz.

Reset Overshoot

:   Resets `Overshoot` to 0 when clicked.

    When output sample peak exceeds 0 dB, the value of `Overshoots` changes to greater than 0, and `Reset Overshoot` will be lit.

### BasicLimiterAutoMake Specific Parameters
Auto Make Up

:   Enable automatic make up gain when checked.

    When `Auto Make Up` is enabled, output amplitude become lower when `Threshold` is greater than `Auto Make Up Target Gain`.

    When `Auto Make Up` is enabled, and `Threshold` is increasing, overshoot may occur. Recommend to set the target gain to -0.1 dB (default) or lower in this case. If `Threshold` needs to be changed while signal is hot, insert another limiter for safe guard.

Auto Make Up Target Gain

:   Maximum amplitude after automatic make up gain is applied. This is a control placed on the right side of `Auto Make Up`. Unit is decibel.

    When `Channel Type` is set to `M-S`, maximum amplitude is +6 dB of `Auto Make Up Target Gain`.

Sidechain

:   Enable sidechain when checked. Also disable `Auto Make Up` when checked, because source signal amplitude is not affected by `Threshold` while sidechain is applied.

    BasicLimiterAutoMake has 2 stereo input. No. 1 is source input, and No. 2 is sidechain input. For routing, please refer to your DAW manual.

Channel Type

:   Switch the type of stereo channel between left-right (`L-R`) and mid-side (`M-S`).

    When the type is set to `M-S`, sample peak becomes `Threshold` + 6 dB. Therefore, when using `Auto Make Up` with `M-S`, it is recommended to set `Auto Make Up Target Gain` to -6.1 dB or lower. This behavior aims to provide the same loudness when comparing `L-R` and `M-S`.

## Change Log
### BasicLimiter
- 0.1.4
  - Added custom font options for `style.json`.
  - Changed to disable `Gate` when the value is set to -inf dB.
- 0.1.3
  - Updated VSTGUI from 4.10 to 4.11.
- 0.1.2
  - Initial release.

#### BasicLimiterAutoMake
- 0.1.4
  - Initial release.

### Old Versions
### BasicLimiter
- [BasicLimiter 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/BasicLimiter_0.1.2.zip)
-
#### BasicLimiterAutoMake
Currently, there aren't any old versions.

## License
BasicLimiter is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
