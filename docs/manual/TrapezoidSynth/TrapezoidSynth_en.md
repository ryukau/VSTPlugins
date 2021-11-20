---
lang: en
...

# TrapezoidSynth
![](img/trapezoidsynth.png)

TrapezoidSynth is a monophonic synthesizer equipped with 2 trapezoid oscillator. Trapezoid oscillator is based on PTR (Polynomial Transition Regions) and it has a drawback that increasing noise at high frequency. Therefore 8 times oversampled. 2 pitch shifters are added to make some chord.

- [Download TrapezoidSynth 0.1.13 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ResetAndMuteFix/TrapezoidSynth_0.1.13.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/TrapezoidSynthPresets.zip)

The package includes following builds:

- Windows 64bit
- Linux 64bit
- macOS 64bit

macOS build isn't tested because I don't have Mac. If you found a bug, please file a issue to [GitHub repository](https://github.com/ryukau/VSTPlugins) or send email to `ryukau@gmail.com`.

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

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
- `borderLabel`: Line color of parameter section label.
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

Right clicking on controllable object popups a context menu provided by DAW.

## Caution
It is recommended to insert high-pass when setting `SlideType` to `Reset to 0`.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/trapezoidsynth.svg)

## Parameters
### Oscillator Common
Semi

:   Change oscillator pitch in semitone.

Cent

:   Change oscillator pitch in cent. 100 cent = 1 semitone.

Slope

:   Change left and right side of slope of trapezoid.

PW

:   Pulse width. Change length of upper side of trapezoid.

### Oscillator 1
Drift

:   Modulate pitch with white noise.

Feedback

:   Modulate phase of Oscillator 1 with feedback signal.

    ```
    osc1Phase += Feedback * (osc1 + OscMix * (osc2 - osc1))
    ```

### Oscillator 2
Overtone

:   Multiply the frequency of oscillator by number of `Overtone`.

PM

:   Modulate Osillator 1 phase with Oscillator 2 output.

### Envelope Common
Retrigger

:   When checked, reset envelope for each note on. When not checked, reset envelope only if the number of pressed key is changed from 0 to 1 or more.

A

:   Attack time in seconds.

D

:   Decay time in seconds.

S

:   Sustain level.

R

:   Release time in seconds.

Curve

:   Change envelope curve.

### Gain Envelope
Gain

:   Master output gain.

### Filter
Order

:   TrapezoidSynth is using 8 of serially connected first order filter. `Order` changes where to get the signal from the filter.

Cut

:   Cutoff frequency in Hz.

Res

:   Resonance.

Sat

:   Filter saturation.

    ```
    filterInput = tanh(Sat * (input + Res * filterOutput))
    ```

Env>Cut

:   Modulation amount from Filter Envelope to `Cut`.

Key>Cut

:   Modulation amount from note pitch to `Cut`.

+OscMix

:   Modulation amount from mixed signal of Oscillator 1 and Oscillator 2 to `Cut`.

### Filter Envelope
\>Octave

:   Modulation amount from Filter Envelope to pitch in octave.

### Misc
OscMix

:   Ratio of Oscillator 1 and Oscillator 2. When turned to left-most, output becomes only the signal from Oscillator 1.

Octave

:   Change pitch in octave.

Smooth

:   Interpolation time in seconds from previous value to current value. Following parameters are affected by `Smooth`.

    - `Drift`
    - `Slope`
    - `PW` (for all parameter with identical name)
    - `Feedback` (for all parameter with identical name)
    - `PM`
    - `Gain` (for all parameter with identical name)
    - `Cut`
    - `Res`
    - `Sat`
    - `Env>Cut`
    - `Key>Cut`
    - `+OscMix`
    - `OscMix`
    - `>PM`
    - `>Feedback`
    - `>LFO`
    - `>Slope2`
    - `>Shifter1`
    - `Shifter1.Semi`
    - `Shifter1.Cent`
    - `Shifter2.Semi`
    - `Shifter2.Cent`
    - `Freq`
    - `Shape`
    - `>Pitch1`
    - `>Slope1`
    - `>PW1`
    - `>Cut`

### Mod Common
Retrigger

:   When checked, reset envelope for each note on. When not checked, reset envelope only if the number of pressed key is changed from 0 to 1 or more.

Attack

:   Modulation envelope attack time in seconds.

Curve

:   Modulation envelope curve.

### Mod 1
\>PM

:   Modulation amount from modulation envelope 1 to `Oscillator2.PM`.

### Mod 2
\>Feedback

:   Modulation amount from modulation envelope 2 to `Oscillator1.Feedback`.

\>LFO

:   Modulation amount from modulation envelope 2 to `LFO.Freq`.

\>Slope2

:   Modulation amount from modulation envelope 2 to `Oscillator2.Slope`.

\>Shifter1

:   Modulation amount from modulation envelope 2 to Shifter 1 pitch.

### Shifter Common
Semi

:   Change amount of pitch shift in semitone.

Cent

:   Change amount of pitch shift pitch in cent. 100 cent = 1 semitone.

Gain

:   Output gain of pitch shifter.

### LFO
LFOType

:   4 waveforms are available for LFO.

    - `Sin` : Sine wave. Output will be power of the value of `Shape`.
    - `Saw` : Sawtooth wave. Can be turned into triangle with `Shape`.
    - `Pulse` : Pulse wave. Duty ratio can be controlled by `Shape`.
    - `Noise` : Brown noise. Wandering of noise can be changed by `Shape`.

Tempo

:   Synchronize LFO frequency to tempo.

Freq

:   LFO frequency.

Shape

:   Change LFO wave shape.

\>Pitch

:   Modulation amount from LFO to Oscillator 1 pitch.

\>Slope1

:   Modulation amount from LFO to `Oscillator1.Slope`.

\>PW1

:   Modulation amount from LFO to `Oscillator1.PW`.

\>PW1

:   Modulation amount from LFO to `Filter.Cut`.

### Slide
SlideType

:   3 type of pitch slide is available.

    - `Always` : Always slide from the pitch of last note.
    - `Sustain` : Only slide when 2 or more keys are pressed.
    - `Reset to 0` : When number of pressed key is changed from 0 to 1, or 1 to 0, pitch will be slide to 0 Hz. Otherwise, slide behaves same as other 2 options.

Time

:   Time of pitch sliding.

Offset

:   Oscillator 2 slide time with respect to Oscillator 1 slide time.

## Change Log
- 0.1.13
  - Fixed to reset properly.
  - Fixed `bypass` parameter behavior. This fixes playing all the notes at the moment of unmute, even if host sends note to plugin while muting. This bug was only happening on the hosts which respect VST 3 `bypass` parameter.
- 0.1.12
  - Implemented process context requirements.
- 0.1.11
  - Fixed `Slide->Time` is ignored and set as same as `Smooth`. Fix in 1.10 was incorrect.
- 0.1.10
  - Added check that DSP is initialized or not.
  - Fixed `Slide->Time` is ignored and set as same as `Smooth`.
- 0.1.9
  - Added color configuration.
- 0.1.8
  - Reverted parameter smoother to the old one which works with variable size audio buffer.
- 0.1.7
  - Fixed a bug that cause crash when drawing string.
- 0.1.6
  - Changed display method for pop-up which shows up by clicking plugin title.
- 0.1.5
  - Fixed a bug that was causing noise on PreSonus Studio One 4.6.1.
- 0.1.4
  - Enabled GUI for Linux build.
  - Fixed plugin type from `kFxInstrument` to `kInstrumentSynth`.
- 0.1.3
  - Fixed wobbling knob bug.
- 0.1.2
  - Added support for synchronization between multiple GUI instances.
  - Fixed a bug that opening splash screen causes crash.
  - Fixed GUI to follow host automation.
- 0.1.1
  - Fixed mistake in oscillator implementation.
- 0.1.0
  - Initial release.

### Old Versions
- [TrapezoidSynth 0.1.12 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/TrapezoidSynth0.1.12.zip)
- [TrapezoidSynth 0.1.10 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/TrapezoidSynth0.1.10.zip)
- [TrapezoidSynth 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/TrapezoidSynth0.1.9.zip)
- [TrapezoidSynth 0.1.8 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/TrapezoidSynth0.1.8.zip)
- [TrapezoidSynth 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/TrapezoidSynth0.1.7.zip)
- [TrapezoidSynth 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/TrapezoidSynth0.1.5.zip)
- [TrapezoidSynth 0.1.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/TrapezoidSynth0.1.4.zip)
- [TrapezoidSynth 0.1.3 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/TrapezoidSynth0.1.3.zip)
- [TrapezoidSynth 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/TrapezoidSynth0.1.2.zip)

## License
TrapezoidSynth is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
