---
lang: en
...

# ClangSynth
![](img/clangsynth.png)

ClangSynth is a comb filter which has 4 different delay time and 1 shared buffer. A limiter is inserted on feedback path to make the output somewhat clean. Weird distorsion can be added by modulating delay time using feedback signal amplitude.

- [Download ClangSynth 0.0.1 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.9999.0/ClangSynth_0.0.1.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
<!-- - [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.34.0/ClangSynthPresets.zip) -->

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

## Caution
Output amplitude varies greatly. Recommend to insert limiter after ClangSynth.

FDN may blow up if some parameters change too fast. Use LFO and envelope with caution. When parameter is changed while playing, it breaks the assumption of linear time-invariant (LTI) system which FDN relies on. Following is a list of parameter which potentially blow up FDN.

- LFO
  - `Amount > FDN Pitch`
- Envelope
  - `> LP Cut`
  - `> HP Cut`
  - `> FDN Pitch`
  - `> FDN OT +`

`Refresh Wavetable` button refreshes wavetable. Beware that while refreshing, CPU load becomes high and it will most likely stop the DAW output for a moment.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/clangsynth.svg)

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in ClangSynth.

- \[dB\] : Decibel.
- \[s\] : Second.
- \[st.\] : Semitone.
- \[Hz\] : Hertz.
- \[rad/pi\] : Radian / π.

### Gain
Output \[dB\]

:   Output gain.

Attack \[s\]

:   Output gain smoothing time. Increasing `Attack` also increases release time.

    Note that the value is not exact. It converts to filter cutoff frequency.

    `Attack` is introduced to prevent pop noise at note-on, when `Reset to Note On` is not checked. When lowpass cutoff in FDN is lowered, reducing `Attack` may cause pop noise. If the sound is dense at high frequency, like cymbal, it may be possible to set `Attack` to 0, because pop noise is unnoticeable.

Release \[s\]

:   Release time after note-off.

    Note that the value is not exact. It converts to filter cutoff frequency.

### Tuning
Octave, Semi, Milli

:   Changes master pitch.

    `Milli` is 1/1000 of semitone or 1/10 cent.

ET, A4 \[Hz\]

:   Changes tuning.

    `ET` stands for equal temperament. Note that when `ET` is less than 12, some notes becomes silent due to frequency becomes too high or too low.

    `A4 [Hz]` is frequency of note A4.

P.Bend Range

:   Pitch bend range. Unit is semitone.

### Unison/Chord
nUnison

:   Number of voices a unison uses.

Pan

:   Amount of pan width in a unison.

    At first note-on, pan is allocated from right to left. Rightmost has lowest pitch, and leftmost has highest. After that, the allocation rotates to left for each note-on.

Pitch *

:   Multiplier to unison pitch.

    When the value of `Pitch *` is 0, `Interval` do nothing.

ET

:   Equal temperament fineness specific for `Interval`.

Interval \[st.\], Cycle At

:   `Interval` is the amount of offset to the note pitch.

    `Cycle At` is modulo of `Interval` index.

    For example, considier the following values are set to `Interval`.

    ```
    1, 2, 3, 4
    ```

    The value of `Pitch *` multiplies each `Interval`. Now we consider the value of `Pitch *` is set to 0.1. The values change to the following.

    ```
    0.1, 0.2, 0.3, 0.4
    ```

    The above values turns into a sequence. The sequence starts from index 0, and increases 1 for each voice. When index reaches the value set at `Cycle At`, it goes back to 0. Now, we set `Cycle At` to 1.

    ```
    0.1, 0.2, 0.1, 0.2, ...
    ```

    Then we send note number 60 to the synthesizer. The pitch for each voice becomes following.

    ```
    60,                         // 60
    60 + 0.1,                   // 60.1
    60 + 0.1 + 0.2,             // 60.3
    60 + 0.1 + 0.2 + 0.1,       // 60.4
    60 + 0.1 + 0.2 + 0.1 + 0.2, // 60.6
    ...
    ```

    Below is a code of above procedure.

    ```
    modulo = cycleAt + 1;
    accumulator = 0;
    for index in [0, nUnison) {
      voice[index].pitch = notePitch + pitchMultiplier * accumulator;
      accumulator += interval[index % modulo];
    }
    ```

### Misc.
nVoice

:   Maximum number of voices to play at the same time.

    Decresing `nVoice` reduces the CPU load.

Smoothing \[s\]

:   Parameter smoothing time.

    Note that the value is not exact. It converts to filter cutoff frequency.

### Oscillator
Impulse \[dB\]

:   Gain of impulse which is triggered at note-on.

    Impulse can be used as a spice for cymbal sound. Also, impulse is not affected by velocity, so it can be used to control intensity of impact to cymbal independent to wavetable oscillator.

    TODO プリセットの紹介

Gain \[dB\]

:   Oscillator output gain.

Attack \[s\], Decay \[s\]

:   Oscillator AD envelope attack time and decay time.

    When `Decay` is short, increasing `Attack` almost doesn't change the sound.

    Note that the value is not exact. It converts to filter cutoff frequency.

Octave, Semitone

:   Oscillator pitch tuning in octave or semitone.

Denom. Slope, Rot. Slope, Rot. Offset, Interval

:   Parameters to generate source spectrum for wavetable.

    - `Denom. Slope` : Slope of denominator of frequency amplitude.
    - `Rot. Slope` : Slope of phase rotation.
    - `Rot. Offset` : Offset of phase rotation.
    - `Interval` : Frequency index interval to write a value.

    When `Denom. Slope` is greater than 1, it acts like lowpass filter. At less than 1, it acts like highpass filter. At exactly 1, it generates amplitude of sawtooth wave spectrum.

    `Interval` creates 0 in specrum. For example, when `Interval` is 3, amplitude of index 1, 4, 7, 10, 13, ... becomes non 0, and amplitude of other indices becomes 0. When `Denom. Slope` is 1 and `Interval` is 2, resulting waveform becomes square wave.

    Following is source spectrum generation procedure.

    ```
    for index in [1, numberOfHarmonics] {
        if (index % interval != 0) continue;
        sourceSpectrum[index].amp = 1 / (denominatorSlope * index);
        sourceSpectrum[index].phase = rotationOffset + rotationSlope * index;
    }
    ```

Harmonic HP

:   Frequency index of highpass filter which applies to source spectrum amplitude.

    Amplitude of frequency, which index is below the `Harmonic HP`, will be reduced. When `Harmonic HP` is 0, highpass filter is bypassed.

    Following shows the computation of `Harmonic HP`.

    ```
    for index in [0, harmonicHighpass) {
      sourceSpectrum[index].amp *= index / harmonicHighpass;
    }
    ```

Blur

:   Coefficient of lowpass filter which applies to source spectrum amplitude.

    Reducing `Blur` diffuses frequency amplitude to adjacent indices. It means that the resulting sound becomes close to impulse when ignoring phase rotations.

    When `Blur` is 1, lowpass filter is bypassed.

    TODO 図

OT Amp., Rot. \[rad/pi\]

:   Amplitude and phase rotation to add source spectrum as overtone.

    - `OT Amp.` : Overtone amplitude.
    - `Rot. \[rad/pi\]` : Overtone phase rotation.

    Following shows the procedure to make final spectrum using `OT Amp.` and `Rot.`

    ```
    targetSpectrum.fill(0);

    for i in [0, nOvertone) {
      for k in [0, nFrequency) {
        auto index = (i + 1) * k;
        if (index >= nFrequency) break;
        targetSpectrum[index] += sourceSpectrum[k] * complexFromPolar(otAmp[i], otRot[i]);
      }
    }
    ```

Refresh Wavetable

:   When the button is pressed, wavetable starts refreshing.

    Beware that refreshing increases CPU load, and it will likely stop the sound for a moment.

    Following parameters are only applied after `Refresh Wavetable` is pressed.

    - `Denom. Slope`
    - `Rot. Slope`
    - `Rot. Offset`
    - `Interval`
    - `Harmonic HP`
    - `Blur`
    - `OT Amp.`
    - `Rot. [rad/pi]`

### FDN
FDN

:   When lit, oscillator output goes through FDN.

    It is useful to bypass FDN when checking the raw oscillator output.

Identity

:   Cross feedback amount between delays.

    When `Identitiy` is 0, feedback matrix becomes identity matrix. This means that it behaves as a bunch of comb filters. Increasing `Identity` means to increase cross feedback amount between delays. More cross feedback adds more non-integer harmonics, which sounds more metallic.

Feedback

:   Feedback amount of the entire FDN.

    Even if the `Feedback` is set ot 1, output decays due to the linear interpolation used inside of delay.

Interp. Rate

:   Rate limiting amount to the change of delay time.

Interp. LP \[s\]

:   Inverse of cutoff frequency of lowpass filter, which is used to interpolate delay time.

    On delay time interpolation, the value first goes into lowpass filter, then rate limiter.

Seed

:   Seed value of psuedo-random number generator (PRNG) used for matrix randomization.

Randomize

:   Ratio of feedback matrix randomization for each note-on.

    Internally, ClangSynth is holding two pairs of matrix randomization seed values. One of the pair is fixed when DAW starts playing. Another is generated at each note-on. `Randomize` is the ratio to mix those 2 pairs.

    When `Randomize` is 0, harmonics of FDN only depends on `Seed` value, and it doesn't changed by note-on. When `Randomize` is 1, each note-on changes the harmonics of FDN.

OT +, OT *, OT Offset, OT Modulo, OT Random

:   This group of parameters changes delay times as overtone of note pitch.

    - `OT +` : Incrementation to previous harmonic value.
    - `OT *` : Multiplier to previous harmonic value.
    - `OT Offset` : Amount of harmonic shift.
    - `OT Modulo` : Modulo of the harmonic value calculated from `OT +` and `OT *`.
    - `OT Random` : Harmonics randomization amount for each note-on.

    Setting `OT +` and `OT Offset` as non-integer value makes the resulting sound close to some percussion.

    `OT *` and `OT Modulo` creates unnatural harmonics.

    Followins is the procedure to determine delay times.

    ```
    // At note on.
    for idnex in [0, fdnMatrixSize) {
        overtoneRandomness[idx] = randomUniform(-1, 1) * otRandom;
    }

    // For each sample.
    overtone = 1;
    for idnex in [0, fdnMatrixSize) {
      ot = otOffset + (1 + overtoneRandomness[index]) * overtone;
      fdn.delayTime[index] = sampleRate / (ot * noteFrequency);
      overtone = overtone * otMul + otAdd;
      overtone = fmod(overtone, 1 + otModulo);
    }
    ```

Reset at Note ON

:   When checked, each note-on resets FDN internal state.

    Beware the CPU load spike caused by reset. If the sound stops at note-on, decreasing `nVoice` might help to reduce CPU load.

Cutoff \[st.\]

:   Cutoff frequency of lowpass or highpass filter.

Q

:   Q factor of lowpass or highpass filter.

Key Follow

:   When checked, cutoff frequency changes by following the pitch of a note.

### LFO
LFO Wave

:   The waveform of LFO wavetable.

    Wavetable is shared between all voices.

Retrigger

:   When `Retrigger` is checked, note-on resets LFO phase.

    When `Retrigger` is not checked, all the voices uses same LFO phase.

    If 1 voice represents 1 object, check `Retrigger`. If note-on represents impact to the same object again and again, do not check `Retrigger`.

Wave Interp.

:   LFO wave form interpolation method.

    - `Step`: Holding.
    - `Linear`: Linear interpolation.
    - `PCHIP`: Monotonic cubic interpolation.

    `Step` makes LFO to act like a sequencer. `Linear` sounds similar to `PCHIP`, and the computation cost is lower. `PCHIP` is the most smooth among all 3.

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

Pitch Amount

:   Modulation amount to oscillator or FDN pitch. Unit is semitone.

    `> Osc. Pitch` row modulates oscillator, and `> FDN Pitch` row modulates FDN.

Pitch Alignment

:   Modulation amount from LFO to pitch only uses the multiple of `Alignment`. For example, if current `Amount` is 12.345 and `Alignment` is 6, actual modulation amount becomes 12.

    When `Alignment` is 0, the value of `Amount` is used directly without alignment.

    Following is procedure of LFO to pitch modulation.

    ```
    if (alignment == 0)
      modulation = amount * lfo;
    else
      modulation = alignment * floor(lfo * amount / alignment + 0.5);
    ```

### Envelope
Envelope Wave

:   The waveform of envelope wavetable.

    Wavetable is shared between all voices.

Time \[s\]

:   Duration of envelope from the start of a note.

    When envelope reaches the end, all the modulation amounts becomes 0.

Wave Interp.

:   Envelope wave form interpolation method.

    This parameter works as same as `Wave Interp.` in LFO. Refer to the `Wave Interp.` in LFO section for details.

> LP Cut

:   Modulation amount to cutoff frequency of FDN lowpass filters. Unit is semitone.

> HP Cut

:   Modulation amount to cutoff frequency of FDN highpass filters. Unit is semitone.

> Osc. Pitch

:   Modulation amount to oscillator pitch. Unit is semitone.

> FDN Pitch

:   Modulation amount to FDN pitch. Unit is semitone.

> FDN OT +

:   Modulation amount to `OT +` in FDN section.

    `> FDN OT +` changes pitch using overtone structure, while `> FDN Pitch` moves entire pitch.

## Change Log
- 0.0.1
  - β release.

## Old Versions
Old version is not available.

## License
ClangSynth is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
