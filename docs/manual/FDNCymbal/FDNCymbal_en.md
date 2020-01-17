# FDNCymbal
![](img/fdncymbal.png)

FDNCymbal is a cymbal sound synthesizer. It can also be used as an effect. Unlike the name, most of metallic texture comes from Schroeder allpass section rather than FDN (feedback delay network). FDN section makes nice impact sound when `FDN.Time` is short. Tremolo is added to simulate wobbling of cymbal.

- [Download FDNCymbal 0.1.3 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/FDNCymbal0.1.3.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/FDNCymbalPresets.zip)

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

## Caution
When `FDN.Feedback` is non zero, it may possibly blow up. If that happens, turn `FDN.Feedback` to leftmost.

When `HP Cutoff` is moving fast, it may output massive DC. To avoid this, use Shift + Mouse Left Drag or turn up Smooth.

## Parameters
### Gain
Output gain.

### Smooth
Time in seconds to move the value of parameters from previous one to changed one. Below is a list of parameter that is affected by `Smooth`.

- `Gain`
- `FDN.Time`
- `FDN.Feedback`
- `FDN.CascadeMix`
- `Allpass.Mix`
- `Allpass.Stage1.Time`
- `Allpass.Stage1.Feedback`
- `Allpass.Stage2.Time`
- `Allpass.Stage2.Feedback`
- `Tremolo.Mix`
- `Tremolo.Depth`
- `Tremolo.Frequency`
- `Tremolo.DelayTime`

### Stick
When `Stick` is on, each note on triggers stick oscillator. Noise density and pitch of tone are related to the pitch of note.

#### Decay
Decay time of stick oscillator.

#### ToneMix
Gain of tone of stick oscillator.

### Random
#### Seed
Value of random seed.

#### Retrigger.Time
When not checked, internal delay time controlled by following 3 parameters will be changed for each note on.

- `FDN.Time`
- `Allpass.Stage1.Time`
- `Allpass.Stage2.Time`

#### Retrigger.Stick
When not checked, stick oscillator output varies for each note.

#### Retrigger.Tremolo
When not checked, tremolo parameters will be changed for each note on.

### FDN
When turned on, the signal go through FDN section.

#### Time
FDN delay time. Internal delay time will be randomized with respect to this value.

#### Feedback
FDN section feedback. Beware that if this value is non zero, it's possible to blow up.

#### CascadeMix
Controls mixing of Cascaded FDN.

### Allpass
#### Mix
Mixing ratio of Schroeder allpass section.

### Stage 1 and Stage 2
Stage 1 is serially connected 8 Schroeder allpass. On stage 2, there are 4 Schroeder allpass section connected in parallel. For each section has serially connected 8 Schroeder allpass.

#### Time
Max delay time of Schroeder allpass section. Internally, delay time will be randomized.

#### Feedback
Feedback of Schroeder allpass section.

#### HP Cutoff
Cutoff frequency of high-pass filter to the output of Schroeder allpass section.

#### Tanh
When checked, stage 1 feedback go through tanh saturator.

### Tremolo
#### Mix
Mixing ratio of tremolo effect.

#### Depth
Maximum change of amplitude with tremolo.

#### Frequency
Tremolo LFO frequency.

#### DelayTime
Maximum delay time. Delay time is modulated by LFO. This delay simulates doppler effect.

### Random (Tremolo)
#### Depth
Amount of randomization to `Tremolo.Depth` for each note on.

#### Freq
Amount of randomization to `Tremolo.Frequency` for each note on.

#### Time
Amount of randomization to `Tremolo.DelayTime` for each note on.

## Change Log
- 0.1.3
  - Fixed a bug that sound stops on Steinberg host.
- 0.1.2
  - Added support for synchronization between multiple GUI instances.
  - Fixed a bug that opening splash screen causes crash.
  - Fixed a bug that crash plugin when reloading.
  - Fixed GUI to follow host automation.
- 0.1.1
  - Fixed note on/off event to be triggered at exact timing.
  - Fixed a bug that `Allpass.Stage1.Feedback` was disabled.
- 0.1.0
  - Initial release.

### Old Versions
- [Download FDNCymbal 0.1.2 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/FDNCymbal0.1.2.zip)

## License
SevenDelay is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
