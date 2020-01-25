# IterativeSinCluster
![](img/iterativesincluster.png)

IterativeSinCluster is an additive synthesizer. This synth computes 512 sine waves for each note to make tone cluster. I somehow thought "iterative sin" is a valid term. Correct term is "recursive sine".

- [Download IterativeSinCluster 0.1.3 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/IterativeSinCluster0.1.3.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/IterativeSinClusterPresets.zip)

IterativeSinCluster requires CPU which supports AVX or later SIMD instructions.

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

### Linux Specific
On Ubuntu 18.0.4, those packages are required.

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

If DAW doesn't recognize the plugin, take a look at `Package Requirements` section of the link below and make sure all the VST3 related package is installed.

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

REAPER on Linux may not recognize the plugin. A workaround is to delete a file `~/.config/REAPER/reaper-vstplugins64.ini` and restart REAPER.

## Controls
Knob and slider can do:

- Ctrl + Left Click: Reset value.
- Shift + Left Drag: Fine adjustment.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/iterativesincluster.svg)

## Parameters
### Gain
#### Boost, Gain
Both controls output gain. Peak value of output will be `Boost * Gain`.

It's possible that output gain becomes too small while changing gain of `Note`, `Chord`, `Overtone`. `Boost` can be useful in this situation.

#### ADSR
Parameters for gain envelope.

- `A` : Attack time which is the length from note-on to reaching peak value.
- `D` : Decay time which is the length from peak value to reaching sustain level.
- `S` : Sustain level which is the gain after decay.
- `R` : Release time which is the length from note-off to the gain reaching to 0.

#### Curve
Change attack curve of gain envelope.

### Shelving
Cutoff freqeuncy can be changed by `Semi`, which is relative to a note frequency. For example, if the note is C4 and `Semi` is 12.000, cutoff frequency is set to `C4 + 12 semitone = C5`.

- `Low` shelving changes `Gain` of sine waves **less than or equal to** the cutoff frequency specified by `Semi`.
- `High` shelving changes `Gain` of sine waves **greater than or equal to** the cutoff frequency specified by `Semi`.

### Pitch
#### Add Aliasing
When checked, the synth enables rendering of sine waves over nyquist frequency.

Roughly speaking, nyquist frequency is the highest frequency that can be reconstructed from recorded digital signal. When generating sound, it's possible to set value that is higher than nyquist frequency. However, the result may contain unexpected frequency due to a phenomenon called aliasing. `Add Aliasing` is option to add those aliasing noise.

#### Reverse Semi
When checked, negate sign of `Semi`. For example, 7.000 becomes -7.000.

#### Octave
Note octave.

#### ET
`ET` stands for equal temperament. This parameter can be used to change tuning of `Semi` and `Milli`.

Note that this parameter is not to change tuning of notes sent from plugin host. Only affect to internal parameters.

#### Multiply, Modulo
Change sine wave frequency.

Equation is `noteFrequency * (1 + fmod(Multiply * pitch, Modulo))`. `pitch` is calculated from `Note`, `Chord` and `Overtone`. `fmod(a, b)` is a function that returns reminder of `a / b`.

### Smooth
Time length to change some parameter value to current one. Unit is in second.

List of parameters related to `Smooth`. `*` represents wild card.

- All parameters in `Gain` section.
- `Chord.Pan`
- All parameters in `Chorus` section, except `Key Follow`.

Other parameter uses the value obtained from the timing of note-on for entire duration of a note.

### nVoice
Maximum polyphony. Lowering the number of this option reduces CPU load.

### Random
#### Retrigger
When checked, reset random seed for each note-on.

#### Seed
Random seed. This value change random number sequence.

#### To Gain
Amount of randomization to sine wave gain. Randomization is done for each note-on.

#### To Pitch
Amount of randomization to sine wave pitch. Randomization is done for each note-on.

### Note, Chord, Overtone
Change sine wave gain and pitch.

1 `Note` have 16 `Overtone`. 1 `Chord` consists of 8 `Note`. 4 `Chord` are available to use.

Common parameters for `Note` and `Chord`.

- `Gain` : Sine wave gain.
- `Semi` :  Distance from note frequency. Unit is semitone.
- `Milli` : Distance from note frequency. Unit is 1 / 1000 semitone, or 1 / 10 cent.

Leftmost of `Overtone` is gain of fundamental frequency (1st overtone) of a note. For each bar from left represents 1st, 2nd, 3rd, ... , 16th overtone gain.

Below is the summary of calculation of sine wave frequency.

```
function toneToPitch(semi, milli):
  return 2 ^ (1000 * semi + milli) / (ET * 1000)

for each Chord:
  chordPitch = toneToPitch(Chord.semi, Chord.milli)
  for each Note:
    notePitch = toneToPitch(Note.semi, Note.milli)
    for each Overtone:
      frequency = midiNoteFrequency
        * (1 + mod(Multiply * Overtone * notePitch * chordPitch, Modulo))
```

### Chorus
#### Mix
Change Dry/Wet mix ratio.

#### Freq
Chorus LFO frequency.

#### Depth
Change stereo spread.

#### Range
Amount of modulation from LFO to delay time. There are three delays in this chorus.

#### Time
Delay time.

#### Phase
LFO phase.

#### Offset
Phase difference between delays.

#### Feedback
Feedback of delay.

#### Key Follow
When checked, change delay time relative to note frequency.

## Change Log
- 0.1.3
  - Fixed a bug that was causing noise on PreSonus Studio One 4.6.1.
- 0.1.2
  - Enabled GUI for Linux build.
- 0.1.1
  - Fixed macOS build.
  - Added code paths for AVX and AVX512 instructions.
  - Fixed wobbling knob bug.
- 0.1.0
  - Initial release.

### Old Versions
- [IterativeSinCluster 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/IterativeSinCluster0.1.2.zip)
- [IterativeSinCluster 0.1.1 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/IterativeSinCluster0.1.1.zip)
- [IterativeSinCluster 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/IterativeSinCluster0.1.0.zip)

## License
IterativeSinCluster is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
