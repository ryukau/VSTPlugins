---
lang: en
...

# ClangSynth
![](img/ClangSynth.png)

ClangSynth is a synthesizer suitable for percussion sounds. Each voice equips an 8 * 8 feedback delay network (FDN) as a resonator. Oscillator is static wavetalble with controls specialized to make pulse train.

{% for target, download_url in latest_download_url["ClangSynth"].items() %}
- [Download ClangSynth {{ latest_version["ClangSynth"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["ClangSynth"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["ClangSynth"] }})
{%- endif %}

ClangCymbal is a spin off of ClangSynth. FDN size is expended to 64 * 64 to provide more rich harmonics. However it is monophonic due to increased CPU load. Oscillator is changed for noise/pulse mix which is more suitable to make cymbal sounds. It can also make sound of manhole dragged on asphalt.

{% for target, download_url in latest_download_url["ClangCymbal"].items() %}
- [Download ClangCymbal {{ latest_version["ClangCymbal"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["ClangCymbal"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["ClangCymbal"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

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

### ClangSynth
![](img/ClangSynth.svg)

### ClangCymbal
![](img/ClangCymbal.svg)

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in ClangSynth.

- \[dB\] : Decibel.
- \[s\] : Second.
- \[st.\] : Semitone.
- \[Hz\] : Hertz.
- \[rad/pi\] : Radian / π.

### ClangSynth
#### Gain
Output \[dB\]

:   Output gain.

Attack \[s\]

:   Output gain smoothing time. Increasing `Attack` also increases release time.

    Note that the value is not exact. It converts to filter cutoff frequency.

    `Attack` is introduced to prevent pop noise at note-on, when `Reset to Note On` is not checked. When lowpass cutoff in FDN is lowered, reducing `Attack` may cause pop noise. If the sound is dense at high frequency, like cymbal, it may be possible to set `Attack` to 0, because pop noise is unnoticeable.

Release \[s\]

:   Release time after note-off.

    Note that the value is not exact. It converts to filter cutoff frequency.

#### Tuning
Octave, Semi, Milli

:   Changes master pitch.

    `Milli` is 1/1000 of semitone or 1/10 cent.

ET, A4 \[Hz\]

:   Changes tuning.

    `ET` stands for equal temperament. Note that when `ET` is less than 12, some notes becomes silent due to frequency becomes too high or too low.

    `A4 [Hz]` is frequency of note A4.

P.Bend Range

:   Pitch bend range. Unit is semitone.

#### Unison/Chord
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

#### Misc.
nVoice

:   Maximum number of voices to play at the same time.

    Decresing `nVoice` reduces the CPU load.

Smoothing \[s\]

:   Parameter smoothing time.

    Note that the value is not exact. It converts to filter cutoff frequency.

#### Oscillator
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

#### FDN
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

#### LFO
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

#### Envelope
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

### ClangCymbal
This section only lists the parameters different to ClangSynth.

#### Tremolo
Intention of tremolo is to crudely imitate the distance from vibrating object to microphone. Modulation signal is sine wave.

Mix

:   Mixing ratio of input and delay output.

    When the value is 0.0, input is bypassed.

Depth

:   Modulation amount to the gain of delay output.

Delay \[s\]

:   Maximum delay time.

Delay Offset

:   Amount of DC offset which adds to the delay time modulation signal.

    On top of `Misc.->Smoothing`, extra smoothing is applied to `Delay Offset`.

Rate \[Hz\]

:   Frequency of modulation sine wave.

#### Misc.
Pitch slide options are the difference to ClangSynth.

Slide \[s\]

:   Pitch slide time.

Slide Type

:   3 types of pitch slide are available.

    - `Always` : Always slide from the pitch of last note.
    - `Sustain` : Only slide when 2 or more keys are pressed.
    - `Reset to 0` : When number of pressed key is changed from 0 to 1, or 1 to 0, pitch will be slide to 0 Hz. Otherwise, slide behaves same as other 2 options. On release, slide time follows the value of `Gain->Release`, instead of `Misc.->Slide`.

2x Sampling

:   When checked, it enables 2-fold oversampling.

    Recommend to turn on for better metallic texture. However it doubles the CPU load.

    When turned off, it reduces CPU load. However it also degrades metallic texture because of the high frequency loss caused by linear interpolation used in delays inside of FDN. It might be better to use external equalizer to compensate the high frequency loss.

#### Oscillator
ClangCymbal uses mix of noise and pulse train as an oscillator.

Noise generation step is following:

1. Generate naive pulse train.
2. Input 1 to integrator.
3. Multiply 2 to normal distribution noise.

Integrator here is not the exact one used in theory, but behaves similarly.

Pulse train is generated by BLIT (band-limited impulse train) algorithm. The naive pulse train used in noise generation is not related to this one.

Noise/Pulse

:   Mixing ratio of noise and pulse train.

LP Cutoff \[st.\]

:   Lowpass filter cutoff frequency.

LP Q

:   Lowpass filter Q.

LP Key

:   Key follow amount to `LP Cutoff`.

    When `LP Cutoff` is set to 0, `Q` is set to maximum, and `LP Key` is set to 1, the oscillator outputs sine wave.

Density \[Hz\]

:   Density of noise and pulse train.

    To match the tuning to FDN, set the value of `Density` to the same value of `Tuning->A4`, then set `Density key` to 1.

Density Key

:   Key follow amount to `Density`.

Noise Decay

:   Decay time of a noise burst.

    When the value of `Noise Decay` is small, it sounds like pulse train. However, the naive pulse train used for noise generator is not anti-aliased. Therefore it has different texture to BLIT.

Bounce

:   Amount of bouncing effect.

    The bouncing effect is tied to AD envelope output, and bouncing interval becomes shorter when AD envelope amplitude becomes lower. The effect is disabled when `Bounce` is 0.0.

    `Bounce` can be used to simulate the bounce of a stick on a surface.

Bounce Curve

:   The curve parameter of `Bounce` effect.

    Increasing `Bounce Curve` shorten the interval of bounce.

Jitter

:   Randomness of pulse interval.

Amp. Rand.

:   Randomness of the amplitude of a naive pulse on noise generator.

#### FDN
Individual control for each filter is added compared to ClangSynth. This increases flexibility, and CPU load.

Cutoff Slope

:   Amount of increment or decrement to the filter cutoff. The amount is proportional to the order of harmonics.

Q Slope

:   Amount of increment or decrement to the filter Q. The amount is proportional to the order of harmonics.

LP/HP Cut Offset \[st.\]

:   The value added to filter cutoff frequency, which is calculated from `Cutoff` and `Cutoff Slope`.

    On the control, the order of harmonics becomes lower towards left, and higher towards right. Note that higher harmonics may affects lower frequency, and lower harmonics may affects higher frequency. This is due to the cross feedback on FDN.

LP/HP Q Offset

:   The value added to filter Q, which is calculated from `Q` and `Q Slope`.

    As same as `Cut Offset`, left side of control represents lower harmonics and right side represents higher harmonics.

#### Envelope
\> Jitter

:   Modulation amount to `Oscillator->Jitter`.

\> Noise/Pulse

:   Modulation amount to `Oscillator->Noise/Pulse`.

## Change Log
### ClangSynth
{%- for version, logs in changelog["ClangSynth"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

### ClangCymbal
{%- for version, logs in changelog["ClangCymbal"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
### ClangSynth
{%- if old_download_link["ClangSynth"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["ClangSynth"] %}
- [ClangSynth {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

### ClangCymbal
{%- if old_download_link["ClangCymbal"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["ClangCymbal"] %}
- [ClangCymbal {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
ClangSynth is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
