---
lang: en
...

# CubicPadSynth
![](img/cubicpadsynth.png)

CubicPadSynth is a wavetable synthesizer which uses PADsynth algorithm to generate oscillator tables. Cubic interpolation is used to get smooth sound even at inaudible low frequency range. LFO waveform can be directly drawn.

- [Download CubicPadSynth {{ latest_version["CubicPadSynth"] }} - VST® 3 (github.com)]({{ latest_download_url["CubicPadSynth"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["CubicPadSynth"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["CubicPadSynth"] }})
{%- endif %}

CubicPadSynth requires CPU which supports AVX or later SIMD instructions.

The package includes following builds:

- Windows 64bit
- Linux 64bit

{{ section["macos_warning"] }}

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely to run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## Caution
Pressing `Refresh LFO` or `Refresh Table` button stop sound. They also stop all midi notes.

## Wavetable Specification
Length of a wavetable is `2^18` samples.

136 band-limited wavetables are generated for each MIDI note. Pitch bending is supported as high as 21100 Hz. 21100 Hz is derived from MIDI note number 136. Frequency of MIDI note number 137 is higher then 22050 Hz. That's why 136 is selected.

When fundamental frequency of a note is lower than 8.18 Hz, some overtones on high frequency may lost. However, cubic interpolation may cause noise which makes those frequency not to be 0.

```
noteToFreq(note) := 440 * Math.pow(2, (note - 69) / 12)
noteToFreq(0) = 8.175798915643707
noteToFreq(136) = 21096.16364242367
noteToFreq(137) = 22350.606811712252
```

Wavetable is represented as 2D array with size of `136 * 2^18`. Bicubic interpolation is used to get the value from the table. Coordinate on time/band-limit axis is determined by phase and frequency of oscillator.

The size of wavetable is large for a synthesizer. [Xrun](https://alsa.opensrc.org/Xruns) may occur if memory is slow. It may also consume more resources for higher notes because access pattern becomes close to random.

- [Alsa Opensrc Org - Independent ALSA and linux audio support site](https://alsa.opensrc.org/Xruns)
- [linux - What are XRuns? - Unix & Linux Stack Exchange](https://unix.stackexchange.com/questions/199498/what-are-xruns)

## PADsynth Algorithm Overview
CubicPadSynth uses PADsynth algorithm, which is originated from [ZynAddSubFX](https://zynaddsubfx.sourceforge.io/). [Yoshimi](http://yoshimi.sourceforge.net/) is also using PADsynth algorithm.

Passing phase randomized spectrum to IDFT (inverse discrete fourier transform) generates a cyclic waveform which is smooth when connecting an end to another end. PADsynth is utilizing this property.

"Profile" function is used to synthesize spectrum. CubicPadSynth uses normal distribution curve for profile, which is same as original PADsynth algorithm. Center frequency of profile can be set arbitrary. In default, CubicPadSynth uses fundamental frequency set by `Base Freq.` and its overtones.

![](img/padsynth.svg)

- [PADsynth algorithm](https://zynaddsubfx.sourceforge.io/doc/PADsynth/PADsynth.htm)

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/cubicpadsynth.svg)

## Parameters
### Main Tab
![](img/cubicpadsynth.png)

#### Tuning
Octave, Semi, Milli

:   Changes master pitch.

    `Milli` is 1/1000 of semitone or 1/10 cent.

ET, A4 [Hz]

:   Changes tuning.

    `ET` stands for equal temperament. Note that when `ET` is less than 12, some notes becomes silent due to frequency becomes too high or too low.

    `A4 [Hz]` is frequency of note A4.

#### Gain
A, D, S, R

:   Gain envelope parameters.

    - `A` : Attack time which is the length from note-on to reaching peak value.
    - `D` : Decay time which is the length from peak value to reaching sustain level.
    - `S` : Sustain level which is the gain after decay.
    - `R` : Release time which is the length from note-off to the gain reaching to 0.

Gain

:   Master output gain.

#### Lowpass
This is a psuedo low-pass filter. It shifts to wavetable which contains less overtone to get low-passed output.

Equation to get cutoff frequency is following.

```
cut = Cutoff * 128
keyFollowRange = Cutoff * (nTable - pitch)
lowpassPitch = (cut + KeyFollow * (keyFollowRange - cut)) - lowpassEnvelope * Amount;
if (lowpassPitch < 0) lowpassPitch = 0

noteToFreq(note) := 440 * Math.pow(2, (note - 69) / 12)
range  = noteToFreq(nTable)
lpFreq = noteToFreq(midiNote + lowpassPitch)
ntFreq = noteToFreq(midiNote)
cutoffFrequency = (1 + (range - lpFreq) / range) * ntFreq
```

128 used to calculate `lpCut` is an arbitrary value which came from MIDI note number range + 1. `nTable` is set to 136.

Cutoff

:   Changes filter cutoff frequency.

KeyFollow

:   When set to right-most, cutoff frequency is set to make the number of overtone to be almost same regardless of pitch of note. When set to left-most, it only use the value of `Cutoff`.

A, D, S, R, Amount

:   Filter envelope parameters. `Amount` changes the amount of modulation to cutoff.

#### Pitch
A, D, S, R, Amount

:   Pitch envelope parameters.

Negative

:   Changes the sign of pitch envelope output.

#### Unison
nUnison

:   Number of voices used by unison.

    To avoid interruption of release, increase the number of `nVoice` in Misc. section. Note that increasing `nVoice` consumes more resources.

Detune, Random Detune

:   `Detune` is the difference of pitch between voices used in a unison.

    When `Random Detune` is checked, amount of detune changes for each note-on.

    ```
    random = RandomDetune ? rand() : 1
    detune = pitch * (1 + random * unisonIndex * Detune)
    ```

GainRnd

:   Amount of randomization of gain for voices used in a unison.

Phase

:   Amount of randomization of phase for voices used in a unison.

    This parameter makes no effect when `Reset` in Phase section is checked.

Spread, Spread Type

:   `Spread` is an amount of stereo spread for a unison.

    `Spread Type` provides options to assign panpot values according to voice pitch.

    - `Alternate L-R`: Alternates `Ascend L -> R` and `Ascend R -> L`.
    - `Alternate M-S`: Alternates `HighOnMid` and `HighOnSide`.
    - `Ascend L -> R`: Ascend pitch from left to right.
    - `Ascend R -> L`: Ascend pitch from right to left.
    - `HighOnMid`: Ascend pitch from side to mid.
    - `HighOnSide`: Ascend pitch from mid to side.
    - `Random`: Randomize pan. May be biased.
    - `RotateL`: Rotate to left for each note-on.
    - `RotateR`: Rotate to right for each note-on.
    - `Shuffle`: Randomly assign pan which is evenly ordered.

#### LFO
Refresh LFO

:   Refresh LFO wavetable based on current value of `LFO Wave`.

    Note that refreshing wavetable stops sound. It also interrupts MIDI notes.

Interpolation

:   Type of LFO wavetable interpolation.

    ![](img/interpolation_type.png)

Tempo, Multiply

:   Sets LFO frequency according to current tempo. Lower numeral represents the length of note. Upper numeral is the number of notes.

    Value of `Multiply` is multiplied to the frequency calculated from `Tempo`.

    ```
    // (60 seconds) * (4 beat) = 240
    lfoFrequency = Multiply * (BPM / 240) / (TempoUpperNumeral / TempoLowerNumeral)
    ```

Retrigger

:   When checked, resets LFO phase for each note-on.

Amount

:   LFO frequency modulation amount.

Lowpass

:   Changes cutoff freequency of low-pass filter for LFO.

LFO Wave

:   LFO waveform.

#### Phase
Phase

:   Initial phase of oscillator.

Reset

:   When checked, resets oscillator phase to the value set by `Phase`.

Random

:   When checked, randomize phase for each note-on. In this case, value of `Phase` becomes range of randomization.

#### Misc.
Smooth

:   Time length to change some parameter value to current one. Unit is in second.

    List of parameters related to `Smooth`. `*` represents wild card.

    - All parameters in Tuning section.
    - Gain
      - `S`
      - `Gain`
    - Lowpass
      - `Cutoff`
      - `KeyFollow`
      - `S`
      - `Amount`
    - Pitch
      - `S`
      - `Amount`
      - `Negative`
    - LFO
      - `Tempo`
      - `Multiply`
      - `Amount`
      - `Lowpass`
    - Phase
      - `Phase`

nVoice

:   Maximum polyphony. Lowering the number of this option reduces CPU load.

Pool

:   When checked, most quiet note is released when the number of active voice is close to maximum polyphony. This can be used to reduce pop noise which occurs on note-on.

### Wavetable Tab
![](img/cubicpadsynth_wavetable_tab.png)

#### Overtone Controls
Gain

:   Gain of profile.

Width

:   Width of profile.

Pitch

:   This value is multiplied to profile center frequency.

Phase

:   Range of randomization for the phase of profile.

#### Pitch
Base Freq.

:   Fundamental frequency of wavetable. Note that if this value is small, master pitch becomes out of tune.

Multiply, Modulo

:   Changes profile center frequency.

    ```
    profileCenterFrequency = mod(
      BaseFreq * profileIndex * overtonePitch * Multiply,
      440 * pow(2, (Modulo - 69) / 12)
    )
    ```

Random

:   When checked, randomize profile center frequency. Ignores values of `Pitch` in overtone control.

#### Spectrum
Expand

:   Scaling factor to shrink/expand the spectrum along to frequency axis.

    ![](img/expand.svg)

Shift

:   Shift spectrum along to frequency axis.

    ![](img/shift.svg)

Comb

:   When this value is higher than 1, it changes the shape of profile like a comb. The value specifies interval between peaks.

    ![](img/comb.png)

Shape

:   Changes profile shapes by using the value of `Shape` as an exponent.

    ```
    shapedProfile = powf(profile, shape);
    ```

Invert

:   Invert spectrum to make the maximum of absolute value to 0, and 0 to the max-abs value. Signs are preserved. Phases aren't considered.

    ```
    maxRe = max(abs(spectrumRe))
    maxIm = max(abs(spectrumIm))
    invertedSpectrumRe = spectrumRe - sgn(spectrumRe) * maxRe
    invertedSpectrumIm = spectrumIm - sgn(spectrumIm) * maxIm
    ```

#### Phase
UniformPhase

:   When checked, phase of a profile becomes an uniform value.

#### Random
Seed

:   Random seed. This value change random number sequence.

#### Modifier
Gain^

:   Exponent for `Gain` in overtone control.

    ```
    profileGain = pow(Gain, Gain^)
    ```

Width\*

:   Multiplier for `Width` in overtone control.

    ```
    profileWidth = Width * (Width*)
    ```

#### Refresh Table
Refresh PADsynth wavetable based on current configuration of Wavetable tab.

Note that refreshing wavetable stops sound. It also interrupts MIDI notes.

## Change Log
{%- for version, logs in changelog["CubicPadSynth"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["CubicPadSynth"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["CubicPadSynth"] %}
- [CubicPadSynth {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
CubicPadSynth is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
