---
lang: en
...

# GlitchSprinkler
![](img/GlitchSprinkler.png)

GlitchSprinkler is a synthesizer specialized to play fast arpeggios. It is using a cheap anti-aliasing trick that is to restrict the oscillator period only to integer sample counts. This anti-aliasing has a side effect to make the pitches out of tune, especially at higher notes, which makes a unique texture.

{% for target, download_url in latest_download_url["GlitchSprinkler"].items() %}
- [Download GlitchSprinkler {{ latest_version["GlitchSprinkler"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["GlitchSprinkler"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["GlitchSprinkler"] }})
{%- endif %}

**Note**: Presets produce the expected sound only if the sampling rate is 48000 Hz.

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
The pitch becomes more out of tune for higher notes. This is a side effect of anti-aliasing used in GlitchSprinkler.

Direct current may appear. Setting `Safety Filter Mix` to 1 applies a high-pass filter with -6 dB/oct slope. If the spike on the transient is too large, it is recommended to use external high-pass filter with steeper slope.

## Mechanism Overview
Note events come from MIDI channel 16 doesn't make any sound. Instead, those ch.16 notes change the pitch of incoming notes. Note that the pitch of the notes can't be changed when they are already being played.

The modification of a note pitch is shown below. It is a multiplication of the note number to a note frequency, which can be used to play integer harmonic series.

```
noteFrequency = baseFrequency * lastNoteNumberOnChannel16;
```

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/GlitchSprinkler.svg)

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in GlitchSprinkler.

- \[dB\] : Decibel.
- \[oct\] : Octave.
- \[st.\] : Ssemitone. 1 semitone is `1 / 12` octave.
- \[cent\] : Cent. 1 cent is `1 / 100` semitone.


### Oscillator
Output \[dB\]

:   Output gain.

Safety Filter Mix

:   Ratio between the direct output and the high-pass filtered output. The value 1 means 100% high-pass filtered output.

    This -6 dB/oct high-pass filter is used to remove direct current (DC). When the DC is high, it may cause an amplitude spike at the beginning of the sound (transient). In this case, it may be better to switching to an external high-pass filter with steeper attenuation.

    Below is an overview of the signal flow:

    ```
    hp = highpass(input);
    output = input + (Safety Filter Mix) * (hp - input);
    ```

Decay to \[dB\]

:   Target gain of the decay envelope after the minimum duration of a note in the arpeggio.

    The minimum duration of a note in the arpeggio depends on the tempo specified on DAW, and the value of `Note / Beat`. The formula is as follows:

    ```
    samplesPerBeat = 60 * sampleRate / tempo;
    noteDurationInSamples = samplesPerBeat / (`Note / Beat`);
    ```

Polyphonic

:   Turn it on when playing chords. Turn it off when playing a monophonic melody.

    When turned on, up to 256 voices can be played simultaneously. When turned off, `nVoice` in the `Unison` section becomes the number of voices played simultaneously.

Release

:   When turned on, the envelope continues to decay even after the note-off.

    When turned off, the sound stops after the waveform completes the current cycle after the note-off. If the pitch is low, the sound will persist for a while, even when `Release` is turned off.

    This is a switch similar to the sustain pedal on a piano.

Soft Envelope

:   When turned on, reduces pop noise at the beginning and end of the sound.

    The pop noise is especially noticeable when `Filter` is on and at low pitches.

Octave

:   Transpose in octave.

Semitone

:   Transpose in semitone. `Tuning` changes the ratio of semitone.

Cent

:   Transpose in cent. Unlike `Semitone`, `Cent` always uses the ratio of 12 equal temperament.

Tuning

:   Musical tunings.

    - `ET N` : N equal temperament.
    - `Just N` : N-limit tuning.
      - `Just 5 Major` : So-called just intonation. Adjusted to reduce the beat of +4 semitones.
      - `Just 5 Minor` : So-called just intonation. Adjusted to reduce the beat of +3 semitones.
    - `MT Pythagorean` : Pythagorean tuning.
    - `MT 1/3 Comma` : 1 / 3 comma mean tone.
    - `MT 1/4 Comma` : 1 / 4 comma mean tone.
    - Discrete N` : 12-tone equal temperament with the sampling rate divided by `N` as the center frequency.

    The pitches of the `Discrete` scales change significantly depending on the sampling rate.

    `Discrete 2` has a similar pitch to `ET 12` when transposed +1 semitone when the sampling rate is 48000 Hz.

Tuning Root \[st.\]

:   Root of non-equal temperament scales.

    For example, suppose we have the following three-tone scale:

    ```
    [1, 5/4, 4/3]
    ```

    When `Tuning Root` is 0, the above scale is used as is.

    When `Tuning Root` is 1, the above scale is shifted to the left by 1, then it is wrapped around to an octave.

    ```
       [           1,            5/4,         4/3]
    -> [         5/4,            4/3,           1] // Left shift.
    -> [(5/4) / (4/5), (4/3) / (4/5), (1) / (4/5)] // Divide all the values with the leftmost ratio.
    -> [            1,         16/15,         4/5]
    -> [            1,         16/15,         8/5] // Wrap around to an octave.
    ```

### Filter
The filter is a resonant low-pass with -6 dB/oct slope. Note that enabling the filter increases CPU load.

Due to the nature of the filter, the resonance will never be higher than around `fs / 6` where `fs` is the sampling rate. In other words, the texture of the resonance changes depending on the sampling rate.

Decay Time Ratio \[dB\]

:   Decay time of the filter envelope, calculated as a ratio from `Decay to` in the `Oscillator` section.

Key Follow

:   Ratio to match the cutoff frequency to the pitch of the note.

    When `Cutoff` is 0 and `Key Follow` is 1, the frequency of the note matches the cutoff frequency when the filter envelope is fully decayed.

    When `Cutoff` is 0 and `Key Follow` is 0, the cutoff frequency will be 20 Hz when the filter envelope is fully decayed.

    The following is the formula for calculating the cutoff frequency (`adjustedCutoffHz`) when the filter envelope is fully decayed.

    ```
    baseCutoffHz = 20 + (Key Follow) * (freqHz - 20);
    adjustedCutoffHz = baseCutoffHz * 2^(Cutoff);
    ```

Cutoff \[oct.\]

:   Cutoff frequency of low-pass filter.

Resonance

:   Resonance of low-pass filter.

Notch \[oct.\]

:   This one changes some character of low-pass filter.

    Technically, this parameter changes the cutoff frequency of all-pass filter placed on the feedback path. On amplitude response, it makes some notch and changes the intensity of the resonance.

### Waveform
In GlitchSprinkler, all waveform parameters except `Pulse Width / Bit Mask` will be fixed at note-on. This is intentional tuning to suppress noise when playing fast arpeggios.

(Waveform Editor), Xn, Yn

:   Control points of the polynomial oscillator.

    Due to the nature of polynomial approximation, the waveform will be wavy. Recommend to use randomization, and avoid manual adjustments as much as possible.

    Below is a list of shortcuts of Waveform Editor. Shortcuts are enabled after left clicking Waveform Editor and only enabled when mouse cursor is stayed on Waveform Editor.

    | Shortcut     | Action                           |
    |--------------|----------------------------------|
    | <kbd>r</kbd> | Randomize                        |
    | <kbd>t</kbd> | Small randomize only for Y axis. |
    | <kbd>0</kbd> | Preset : Silence                 |
    | <kbd>1</kbd> | Preset : Sine                    |
    | <kbd>2</kbd> | Preset : FM A                    |
    | <kbd>3</kbd> | Preset : FM B                    |
    | <kbd>4</kbd> | Preset : Sawtooth                |
    | <kbd>5</kbd> | Preset : Triangle                |
    | <kbd>6</kbd> | Preset : Trapezoid               |
    | <kbd>7</kbd> | Preset : Alternate               |
    | <kbd>8</kbd> | Preset : Pulse                   |
    | <kbd>9</kbd> | Preset : Saturated sine          |

Osc. Sync.

:   Ratio of hard sync.

FM Index

:   Intensity of frequency modulation (FM).

    FM algorithm is 1 operator feedforward. It means that the waveform is computed once, then that computed value is temporary added to phase, then that phase is used to compute the FM waveform.

Saturation \[dB\]

:   Gain before hard clipping.

    This can be used to make square waves.

Pulse Width / Bit Mask

:   Duty ratio of pulse wave. The pulse wave is multiplied to the waveform.

    When `Bitwise And` is on, bitwise and operation is applied to the phase of the oscillator instead of the pulse wave multiplication.

    **Caution** : If `Pulse Width / Bit Mask` is 1, and `PWM` is off, the output becomes silent due to the duty ratio becomes 0.

Modulation Rate

:   Rate of pulse width modulation (PWM).

    Higher the value, the modulation becomes slower. Negative numbers make the modulation faster.

    The modulation rate changes depending on the pitch of the note.

PWM

:   When turned on, pulse width modulation (PWM) will be activated.

    PWM becomes deeper when `Pulse Width / Bit Mask` approaches to 1.

Bidirectional

:   When turned on, triangular wave is used for PWM. When turned off, sawtooth wave is used.

    When `PWM` is off, `Bidirectional` does nothing.

Bitwise And

:   When turned on, the value of `Pulse Width / Bit Mask` will be used to apply bitwise and operation to the phase of the oscillator.

    When `Modulation Rate` is fast, bitwise and operation adds noisy texture to the sound.

### Arpeggio
GlitchSprinkler is designed specifically for playing fast arpeggios, with `Polyphonic` and `Soft Envelope` in the `Oscillator` section turned off, and `nVoice` in the `Unison` section set to 2.

Seed

:   Seed value of the random number. Different `Seed` makes different melody.

Note / Beat

:   Minimum duration of a note in the arpeggio.

    The default setting of 4 will play 4 notes per beat, which means that the minimum duration becomes 16th note.

Loop Length \[beat\]

:   Loop length of arpeggio.

    Setting `Loop Length` to 0 practically disables looping. The actual loop length becomes 2^32 beats in this case.

Duration Variation

:   Range of randomization of the length of the notes in the arpeggio.

    For example, when `Note / Beat` is 4, and `Duration Variation` is 3, the note lengths will be randomly chosen from 1 / 16, 2 / 16, and 3 / 16.

Rest Chance

:   Chance to put a rest in the arpeggio.

    It works well with higher `nVoice`.

    **Caution**: Setting `Rest Chance` to 1 stops the sound, because the note will rest 100% of times.

Scale

:   Musical scale used for arpeggator.

Pitch Drift \[cent\]

:   Amount of randomization of the pitch of a note.

Octave Range

:   Octave range used by the arpeggio.。

    Some scales span more than an octave, so the octave range may be wider than the specified value.

Start From Root

:   When turned on, the first note of the arpeggio will be set to the pitch specified by the note-on.

Reset Modulation

:   When turned on, resets the PWM phase every time a new note is played.

Random FM Index

:   Amount of randomization for `FM Index`. Randomization amount changes for each note.

### Unison
nVoice

:   Number of voices that is played with a note-on.

    Higher values ​​can be more CPU intensive, especially when `Release` is on. To save some CPU load, it's better to reduce `nVoice` when playing higher notes.

Detune \[cent\]

:   Amount of detune for unison voices.

Pan Spread

:   Amount of left/right spread of the unison voices.

Scatter Arpeggio

:   Only effective when `Arpeggio` is on. When turned on, each voice plays a different melody. When turned off, all voices play the same melody.

Gain Sqrt.

:   Changes how the gain is adjusted according to `nVoice`.

    When turned on, the gain for each voice is set to `sqrt(nVoice)`. When turned off, the gain for each voice is set to `1 / (nVoice)`.

    It sounds more natural when turned on, but the peak amplitude may become quite loud.

## Change Log
{%- for version, logs in changelog["GlitchSprinkler"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["GlitchSprinkler"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["GlitchSprinkler"] %}
- [GlitchSprinkler {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
GlitchSprinkler is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
