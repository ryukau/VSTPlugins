---
lang: en
...

# SevenDelay
![](img/sevendelay.png)

SevenDelay is a stereo delay with 7th order lagrange interpolated fractional delay and 7x oversampling.

- [Download SevenDelay 0.1.11 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/SevenDelay0.1.11.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/SevenDelayPresets.zip)

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

- Ctrl + Left Click: Reset value.
- Shift + Left Drag: Fine adjustment.

## Caution
When the value of `Smooth` is near zero and change the value of `Allpass Cut` too fast, SevenDelay may output very loud click noise. This is caused by the filter which becomes unstable when cutoff frequency moves too fast. Recommendation is not to set `Smooth` parameter too low unless you have some specific purpose.

When the value of `Time` is set to minimum and the value of `Feedback` is set to maximum, SevenDelay outputs direct current (DC). To remove DC, Set `DC Kill` to be greater than 1.0.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/sevendelay.svg)

## Parameters
### Delay
Time

:   Delay time. Range is 0.0001 to 8.0.

    - If `Sync` is on and `Time` is less than 1.0, then delay time is set to `Time / 16` beats.
    - If `Sync` is on and `Time` is greater or equal to 1.0, then delay time is set to `floor(2 * Time) / 32` beats.
    - Otherwise, delay time is set to `Time` seconds.

Feedback

:   Delay feedback. Range is 0.0 to 1.0.

Stereo

:   L/R stereo offset. Range is -1.0 to 1.0.

    - If `Stereo` is less than 0.0, then left channel delay time is modified to `timeL * (1.0 + Stereo)`.
    - Otherwise, right channel delay time is modified to `timeR * (1.0 + Stereo)`.

Wet

:   Output volume of delayed signal. Range is 0.0 to 1.0.

Dry

:   Output volume of input signal. Range is 0.0 to 1.0.

Sync

:   Toggle tempo sync.

Negative

:   Toggle negative feedback. This may be useful when delay time is very short.

In/Out Spread/Pan

:   Panning controls. Range is 0.0 to 1.0.

    `In/Out Spread` controls stereo spread. `In/Out Pan` controls stereo panning.

    These parameter can be used for inverse panning, ping-pong delay etc.

    - For inverse panning, set `[InSpread, InPan, OutSpread, OutPan]` to `[0.0, 0.5, 1.0, 0.5]`.
    - For ping-pong delay, set `[InSpread, InPan, OutSpread, OutPan]` to `[1.0, 0.5, 0.0, 0.5]`.

    ```
    panL = clamp(2 * pan + spread - 1.0, 0.0, 1.0)
    panR = clamp(2 * pan - spread, 0.0, 1.0)

    signalL = incomingL + panL * (incomingR - incomingL)
    signalR = incomingL + panR * (incomingR - incomingL)
    ```

Allpass Cut

:   SFV allpass filter cutoff frequency. Range is 90.0 to 20000.0.

    If `Allpass Cut` is 20000.0, filter will be bypassed.

Allpass Q

:   SFV allpass filter resonance. Range is 0.00001 to 1.0.

DC Kill

:   Cutoff frequency of biquad highpass filter. Range is 1.0 to 120.0.

    `DC Kill` removes direct current from delay feedback when the value is set to be greater than 1.0.

Smooth

:   Parameter smoothing. Range is 0.0 to 1.0. Unit is seconds.

    Some parameter may produce noise when the value is changed too fast. Increasing `Smooth` reduces those noise.

### LFO
To Time

:   LFO modulation amount to delay time. Range is 0.0 to 1.0.

To Allpass

:   LFO modulation amount to allpass cutoff frequency. Range is 0.0 to 1.0.

Freq

:   LFO frequency. Range is 0.01 to 100.0.

Shape

:   LFO waveform shape. Range is 0.01 to 10.0.

    ```
    sign = 1 if (phase > π),
          -1 if (phase < π),
           0 if (phase == π)
    lfo = sign * abs(sin(phase))^shape
    ```

Phase

:   LFO initial phase. Range is 0.0 to 2π.

    LFO phase is reset to `Phase` for each time host starts playing.

Hold

:   Toggle LFO phase hold. This may be useful for live performance.

## Change Log
- 0.1.11
  - Added color configuration.
- 0.1.10
  - Reverted parameter smoother to the old one which works with variable size audio buffer.
- 0.1.9
  - Fixed a bug that cause crash when drawing string.
- 0.1.8
  - Changed display method for pop-up which shows up by clicking plugin title.
- 0.1.7
  - Fixed to stop sounds when deactivated.
- 0.1.6
  - Enabled GUI for Linux build.
- 0.1.5
  - Fixed wobbling knob bug.
- 0.1.4
  - Added support for synchronization between multiple GUI instances.
  - Fixed a bug that opening splash screen causes crash.
  - Fixed a bug that crash plugin when reloading.
  - Fixed GUI to follow host automation.
- 0.1.3
  - Fixed smoothing algorithm to prevent blow up.
  - Fixed a bug that LFO Hold on/off was inverted.
  - Removed a modulation from LFO to allpass mixing.
- 0.1.2
  - Changed internal parameter structure.
  - Changed UI looks.
- 0.1.1
  - Disabled GUI for Linux build. This is a makeshift change while tracking down a bug related to VSTGUI.
  - Changed tuning of `To Allpass` of LFO.
  - Changed minimal value of `DC Kill` to 5.0 Hz.
- 0.1.0
  - Initial release.

### Old Versions
- [SevenDelay 0.1.10 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/SevenDelay0.1.10.zip)
- [SevenDelay 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/SevenDelay0.1.9.zip)
- [SevenDelay 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/SevenDelay0.1.7.zip)
- [SevenDelay 0.1.6 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/SevenDelay0.1.6.zip)
- [SevenDelay 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/SevenDelay0.1.5.zip)
- [Download SevenDelay 0.1.4 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/SevenDelay0.1.4.zip)

## License
SevenDelay is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
