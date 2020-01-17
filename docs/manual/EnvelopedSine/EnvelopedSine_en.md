# EnvelopedSine
![](img/envelopedsine.png)

EnvelopedSine is an additive synthesizer that computes 64 sine waves for each note. Difference to IterativeSinCluster is that this synth has AD envelope and saturator for each oscillator. EnvelopedSine is better suited for percussive sounds.

- [Download EnvelopedSine 0.1.0 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/EnvelopedSine0.1.0.zip) <img
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

Linux build is built on Ubuntu 18.0.4 and tested on Bitwig 3.0.3 and Reaper 5.983. Both seems to have problem to display GUI, therefore GUI is currently disabled for Linux build.

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

### Linux Specific
On Ubuntu 18.0.4, those packages are required.

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

If DAW doesn't recognize the plugin, take a look at `Package Requirements` section of the link below and make sure all the VST3 related package is installed.

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

REAPER 5.983 on Linux may not recognize the plugin. A workaround is to delete a file `~/.config/REAPER/reaper-vstplugins64.ini` and restart REAPER.

## Controls
Knob and slider can do:

- Ctrl + Left Click: Reset value.
- Shift + Left Drag: Fine adjustment.

`Octave` 、 `Seed` などで使われている数値スライダーでは、上記に加えて次の操作ができます。

- ホイールクリック : 最小値、最大値の切り替え。

Overtone controls (`Attack`, `Decay`, `Gain`, `Saturation`) have some keyboard shortcuts. Shortcuts are only enabled after left clicking overtone control. Cheat sheet can be popped up by clicking plugin title on bottom left.

| Input                                   | Control                                 |
| --------------------------------------- | --------------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Left Click</kbd> | Reset to Default                        |
| <kbd>Right Drag</kbd>                   | Draw Line                               |
| <kbd>a</kbd>                            | Sort Ascending Order                    |
| <kbd>d</kbd>                            | Sort Descending Order                   |
| <kbd>f</kbd>                            | Low-pass Filter                         |
| <kbd>F</kbd>                            | High-pass Filter                        |
| <kbd>i</kbd>                            | Invert Value (Preserve current minimum) |
| <kbd>I</kbd>                            | Invert Value (Minimum to 0)             |
| <kbd>n</kbd>                            | Normalize (Preserve current minimum)    |
| <kbd>N</kbd>                            | Normalize (Minimum to 0)                |
| <kbd>h</kbd>                            | Emphasize High                          |
| <kbd>l</kbd>                            | Emphasize Low                           |
| <kbd>p</kbd>                            | Permute                                 |
| <kbd>r</kbd>                            | Randomize                               |
| <kbd>R</kbd>                            | Sparse Randomize                        |
| <kbd>s</kbd>                            | Subtle Randomize                        |
| <kbd>,</kbd> (Comma)                    | Rotate Back                             |
| <kbd>.</kbd> (Period)                   | Rotate Forward                          |
| <kbd>1</kbd>                            | Decrease Odd                            |
| <kbd>2</kbd>-<kbd>9</kbd>               | Decrease 2n-9n                          |

## Parameters
### Overtone
4 big controls on top right.

#### Attack, Decay
Gain envelope attack and decay for each oscillator.

#### Gain
Gain for each oscillator.

#### Saturation
Saturation gain for each oscillator.

### Gain
#### Boost, Gain
Both controls output gain. Peak value of output will be `Boost * Gain`.

### Pitch
#### Add Aliasing
When checked, the synth enables rendering of sine waves over nyquist frequency.

Roughly speaking, nyquist frequency is the highest frequency that can be reconstructed from recorded digital signal. When generating sound, it's possible to set value that is higher than nyquist frequency. However, the result may contain unexpected frequency due to a phenomenon called aliasing. `Add Aliasing` is option to add those aliasing noise.

#### Octave
Note octave.

#### Multiply, Modulo
Change sine wave frequency.

Equation is `noteFrequency * (1 + fmod(Multiply * pitch, Modulo))`. `pitch` is calculated from note pitch and overtone index. `fmod(a, b)` is a function that returns reminder of `a / b`.

#### Expand
Scaling factor for overtone controls.

![](img/envelopedsine_expand.svg)

#### Shift
Amount of right shift to overtone controls.

![](img/envelopedsine_shift.svg)

### Random
Randomize parameters. These are more effective when used with `Unison`.

#### Retrigger
When checked, reset random seed for each note-on.

#### Seed
Random seed. This value change random number sequence.

#### To Gain, To Attack, To Decay, To Sat.
Amount of randomization to overtone controls. Equation is `value * random`. Range of `random` is `[0.0, 1.0)`.

#### To Pitch
Amount of randomization to pitch.

#### To Phase
Amount of randomization to phase.

### Misc.
#### Smooth
Time length to change some parameter value to current one. Unit is in second.

List of parameters related to `Smooth`. `*` represents wild card.

- All parameters in `Gain` section.
- All parameters in `Phaser` section, except `nStages`.

Other parameter uses the value obtained from the timing of note-on for entire duration of a note.

#### nVoices
Maximum polyphony. Lowering the number of this option reduces CPU load.

#### Unison
When checked, unison is enabled.

When unison is enabled, 1 note uses 2 voices. 1 voice is placed on left and other is placed on right. Combining `Unison` with `Random.To Phase`, `Random.To Pitch`, etc. can be used to make sound with stereo spread.

### Modifier
#### Attack*, Decay*
Multiplier for `Attack`/`Decay` in overtone control section.

#### Declick
When checked, it reduces click noise that occurs when the value of `Attack` and/or `Decay` is 0.

#### Gain^
Exponent to `Gain` in overtone control. Following is the equation of gain of an oscillator.

```
Gain.Boost * Gain.Gain * pow(Overtone.Gain, Modifier.Gain^)
```

#### Sat. Mix
Mixing ratio of dry/wet signal of saturation. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

### Phaser
#### nStages
Number of all-pass filter.

#### Mix
Mixing ratio of dry/wet signal of phaser. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

#### Freq
LFO frequency.

#### Feedback
Amount of feedback. Feedback is disabled when the knob is pointing to 12 o'clock. It becomes negative feedback when turned to left and positive feedback when turned to right.

#### Range
Range of all-pass filter modulation by LFO.

#### Min
Minimum value of all-pass filter modulation by LFO.

#### Offset
LFO phase offset.

#### Phase
LFO phase. This can be used to make sound with automation. Turning `Freq` to leftmost sets LFO frequency to 0.

## Change Log
- 0.1.0
  - Initial release.

### Old Versions
Not available.

## License
IterativeSinCluster is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
