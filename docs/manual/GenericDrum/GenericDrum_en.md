---
lang: en
...

# GenericDrum
![](img/GenericDrum.png)

GenericDrum is a generic drum synthesizer using delays. This one can make convincing snare and bass drum sounds. Cymbal sounds can be made, but it's not good as good as more lower drum sounds.

{% for target, download_url in latest_download_url["GenericDrum"].items() %}
- [Download GenericDrum {{ latest_version["GenericDrum"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["GenericDrum"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["GenericDrum"] }})
{%- endif %}

**Important**: Presets may not produce the expected sound if sample rate is not 48000 Hz.

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
Recommend to use with external limiter. Gain changes greatly depending on parameter values and note velocity.

Oscillation or blow up may occur depending on the parameter settings. Oscillation is a state in which the amplitude remains constant and the sound does not stop. Blow up (or divergence) is a state in which the sound continues to grow louder over time.

Blow up can result in amplitudes of up to +40 dB. GenericDrum automatically lowers the feedback when it detects blow up. This mechanism is activated when primary or secondary membrane output exceeds +40 dB.

Enabling `Prevent Blow Up` can mostly prevent the blow up caused by collisions between wire and primary. However, the texture of the sound may also change significantly.

Even with `Prevent Blow Up` enabled, combinations of short delay time, high bandpass Q, and high feedback may cause blow up. Below is a list of relevant parameters.

- `Note -> Pitch`
- `Cross Feedback Gain`
- `Cross Feedback Ratio`
- `Delay`
- `BP Q`

## Mechanism Overview
GenericDrum is not a physical model. Because the corresponding physical phenomena are not described. So what is this? It's a work of art that uses delays to imitate the sound of drums, like using a violin to imitate bird songs. Art here means that a human set the parameters from experimentations, mostly relying on their senses.

GenericDrum is good at making the sound of drums that have 1 or 2 membranes on a cylinder, such as bass drums and snare drums, but It's not good at the sound of thin metal plates like cymbals. It also does not support expressions like rubbing the drum membrane, or expressions that stop the sound with an impact, such as closing a hi-hat.

To imitate the sound of a drum, it's better to start from a structure similar to a real drum. GenericDrum can be roughly divided into the following parts.

- Impact
- Wire
- Primary Membrane
- Secondary Membrane

Drums produce sound by hitting a membrane with a hand or a stick. Impact section generates a noise that resembles the impact of a hit. The sound is white noise followed by a lowpass. `Echo` can imitate the clanking sound made by collisions between a drum and something like a stand.

Wire is a part intended for the snare drum, and it collides only to primary. It consists of delays connected in series. `Collision Distance` controls the distance between wire and primary. `Rattle-Squeak Mix` changes the texture of the collision. It can produce a sound similar to a cuíca, especially when the value is set closer to the squeak side (1.0), but there is no mechanism to control the rubbing.

An impact hits both the primary and secondary membranes at the same time. This behavior is unrealistic, but makes a nice sound. One membrane consists of a feedback delay network (FDN). A bandpass filter is placed for each feedback path on the FDN. The texture of membranes can be changed by the combination of bandpass cutoff, bandpass Q, and delay times. In particular, lowering the Q produces a brighter sound.

A nonlinearity of thin plate vibrations is imitated by modulating delay time in FDN with their feedback signal. When a plate vibrates, it stretches or shrinks which add a bit more tension to the plate. For sound, more tension means higher pitch. So my idea was to reduce the delay time of FDN depending on the amplitude of feedback signal.

For collisions, perfectly elastic collisions of 2 equal masses is used. The sound decays within FDN, because of `Cross Feedback Gain` and bandpass filters. So, there is no need to consider energy loss of collisions. In addition, the amplitude after the collision is stored and gradually returned to the system, in order to resemble a sound of a snare drum. A delay can approximate a wave propagation, but it can't represent the state during propagation. In consequence, collisions that occur during propagation also can't be represented. The fact that collisions do not occur during wave propagation becomes more noticeable when the value of `Delay` is lowered.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/GenericDrum.svg)

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in GenericDrum.

- \[dB\] : Decibel.
- \[s\] : Second.
- \[Hz\] : Frequency in Hertz.
- \[sample\] : Time in samples. 1 sample is `1 / (sampling rate)` seconds.
- \[oct\] : Octave.
- \[st.\] : Ssemitone. 1 semitone is `1 / 12` octave.
- \[cent\] : Cent. 1 cent is `1 / 100` semitone.

### Mix & Options
Output \[dB\]

:   Output gain.

Highpass \[Hz\]

:   A highpass filter that can be passed just before the output. The number slider controls cutoff frequency of highpass.

    **Warning**: Enabling or disabling `Highpass` may cause a pop noise.

2x Sampling \[Hz\]

:   Performs 2-fold oversampling when enabled.

    **Warning**:  Enabling or disabling `2x Sampling` may cause a pop noise.

Normalize Gain

:   When enabled, the gain is automatically adjusted according to the following values.

    - Note pitch.
    - `Noise Lowpass` in Impact section.

Reset Seed at Note-on

:   When enabled, resets the random number seed on every note-on. This seed is used for noises of impact and collision.

    When disabled, the sound becomes inconsistent. It might be useful for a kind of "humanization" effect.

Prevent Blow Up

:   When enabled, it tries to prevent blow up by lowering internal gains around wire.

    When wire blows up, it makes a loud explosion-like sound. `Prevent Blow Up` suppresses this explosion, but as a side effect it tends to result in a less powerful sound without rattling texture. To bring back rattling, try changing parameters on the list below. They are in Wire sections.

    - Raise `Decay`.
    - Lower `Collision Distance`.
    - Lower `Rattle-Squeak Mix`.

    Note that if `Prevent Blow Up` is disabled after making the above changes, the system will be likely in an unstable state that is easy to blow up.

### Stereo Unison
Enabling `Stereo Unison` will double the number of drums. This means that 2 different instances will be placed in each channels. Also the CPU load will be doubled.

Stereo Balance

:   Adjust the gain of the left and right channels.

    Unlike panning, the left and right channels won't be mixed.

Stereo Merge

:   Adjust the stereo spread.

### External Input
There are two cases when using external input.

- Using audio from a microphone.
- Using audio from other synthesizers.

When using audio from a microphone, enable `Automatic Trigger` and adjust the `Trigger Threshold`.

When using audio from other synthesizers (source), it's recommended to share the note events to GenericDrum, so that envelopes in GenericDrum starts precisely at the timing of note-ons. When sharing note events, `Automatic Trigger` is better to be disabled.

Please refer to your DAW's manual for external audio input and note event routing.

External Gain \[dB\]

:   Gain of external input.

Trigger Threshold \[dB\]

:   Threshold amplitude of automatic trigger.

Automatic Trigger

:   Engages automatic triggering when enabled.

### Tuning
The Tuning section contains note event and pitch bend related parameters.

Note -> Pitch

:   Amount of pitch modulation applied from the pitch of a note-on event.

    Regardless of the value of `Note -> Pitch`, the amount of pitch modulation will be 0 when the pitch is A3 (57 in MIDI note number).

    When the value of `Note -> Pitch` is 0.0, the pitch of the note is ignored.

    When the value of `Note -> Pitch` is 1.0, the pitch of a note is directly applied to the pitch of the synthesizer. Some drum sounds do not have a clear pitch, so it may sound out of tune in some cases. Also, higher the pitch, more out of tune it becomes. This is because delays doesn't use interpolations to be more efficient, and to avoid lowpass in feedback path.

    When the value of `Note -> Pitch` is -1.0, the pitch is reversed.

Semitone

:   Transposes the pitch of the note in semitone steps.

    **Note**: No effect when `Note -> Pitch` is 0.0.

Cent

:   Transposes the pitch of notes in cents.

    **Note**: No effect when `Note -> Pitch` is 0.0.

Pitch Bend Range \[st.\]

:   Pitch bend range in semitones.

Slide Time \[s\]

:   Pitch slide time.

    The pitch starts sliding when 2 or more notes are played at the same time.

### Impact
Impact section contains parameters that imitates the collision between the a stick and a membrane.

Seed

:   A random number seed.

    Changing the seed allows you to change the sound while maintaining the similar texture.

    GenericDrum has two random number generators for generating collision noise and for setting internal parameters. For noise generation, you can reset the seed on each note-on using `Reset Seed at Note-on`.

Noise Decay \[s\]

:   Decay time of impact noise.

Noise Lowpass \[Hz\]

:   Cutoff frequency of a low-pass filter applied to impact noise.

    When `Normalize Gain` is enabled, the gain is reduced when `Noise Lowpass` becomes higher.

Echo \[Hz\]

:   Frequency that represents the total length of echoes applied to the noise.

    Echo has 4 delays with feedback, so the value specified in `Echo` doesn't represent the exact frequency.

    Echo is provided to represent the clanking sound caused by the collision between drum and drum stand.

### Wire
Wire section contains parameters that imitates a snare wire.

Impact-Wire Mix

:   Mixing ratio between impact noise and wire output.

    0.0 only mixes impact noise, and 1.0 only mixes wire output.

Membrane-Wire Mix

:   Mixing ratio between primary membrane and wire output.

    0.0 only mixes primary membrane output, and 1.0 only mixes wire output.

Frequency \[Hz\]

:   Frequency of the entire wire.

    Wire has four sections, so the value specified in `Frequency` doesn't represetn the exact frequency.

Decay \[s\]

:   Decay time of the wire output.

    **Warning**: When `Prevent Blow Up` is disabled, increasing the value of `Decay` will make it easier to blow up.

Collision Distance

:   Threshold value used to determine collision between wire and primary membrane.

    **Warning**: When the value of `Collision Distance` is small, there may be a temporary blow up until sufficient gain reduction by `Decay` is applied. Increasing this value reduces the chance of blow up.

Rattle-Squeak Mix

:   Texture of collision.

    0.0 gives a full rattling texture, 1.0 gives a full squeak texture. Rattle here means a sound like a snare wire, and squeak means a sound like friction on rubbing.

    **Warning**: When `Prevent Blow Up` is disabled, and the value of `Rattle-Squeak Mix` is close to 0.0, the system becomes more unstable which means easier to blow up.

### Membrane Tone
Membrane Tone section contains parameters related to the feedbacks of FDNs. An FDN produces a sound of a membrane. The values in the Membrane Tone section are shared by both the primary and secondary membranes.

Cross Feedback Gain

:   Gain that adjusts how fast the vibration decays.

    **Warning**: When `Cross Feedback Gain` approaches 1.0, oscillation or blow up is likely to happen.

Cross Feedback Ratio

:   Adjusts the tone of membranes.

    The value of `Cross Feedback Ratio` is converted to a feedback matrix used in both FDNs. Householder transformation is used.

    When all `Cross Feedback Ratio` values are 0.0, the feedbacks won't be crossed into other delays. In this case, the pitch of the sound becomes easier to hear, but the texture of the sound becomes more like a 1D string vibration, rather than 2D plate vibration.

    When all `Cross Feedback Ratio` values are 1.0, the texture becomes similar to the vibration of a thin plate. The texture varies depending on the values in Primary Membrane section.

    To prevent oscillation or blow up without changing the `Cross Feedback Gain` or `Collision Distance`, changing `Cross Feedback Ratio` is an option.

### Pitch Texture
Pitch Texture section contains parameters related to the internal pitch of FDNs that represents membranes.

Delay Time Spread

:   Spread width of the FDN delay time distribution.

BP Cut Spread

:   Width of the cutoff frequency distribution of the bandpass filters in FDN.

Pitch Random \[cent\]

:   Amount of randomization to change the pitches of the delays.

### Pitch Envelope
Pitch envelope modulates delay times and bandpass cutoff frequencies in FDNs.

Attack \[s\]

:   Attack time of the pitch envelope.

    **Note**: Attack time won't be longer than the value of `Decay`.

Decay \[s\]

:   Decay time of the pitch envelope.

Amount \[oct\]

:   Amount of modulation by the pitch envelope.

### Primary Membrane
Pitch Type

:   Type of FDN delay time distribution.

    **Note**: Do not select `- Reserved * -`. `- Reserved * -` is an empty option reserved for adding a different `Pitch Type` later. Fallback to `Harmonic` when `- Reserved * -` is selected.

Delay \[Hz\]

:   Reference value for the FDN delay time.

    In FDN, there is always one feedback path with the frequency specified as the value of `Delay`, but it's only a guideline and does not represent the exact pitch. The reason is the chaos caused by cross feedbacks. So the actual pitch depends on the `Cross Feedback Ratio` setting.

Delay Modulation \[sample\]

:   Amount of the delay time modulation with respect to the amplitude of a feedback signal.

    **Warning**: Increasing `Delay Modulation` increase the chance of oscillation or blow up.

BP Cut \[oct\]

:   Bandpass cutoff frequency with respect to the frequency set by `Delay`.

    Raising `BP Cut` is effective when making high-pitched sounds like cymbals.

    Lowering `BP Cut` is effective to smooth out the noise caused by `Delay Modulation`.

BP Q

:   Bandpass Q value. Q means quality factor in filter.

    When `BP Q` is low, the bandwidth becomes wider. This results in a higher and longer sound, but it also becomes easier to oscillate.

    When `BP Q` is high, the bandwidth becomes narrower. This results in a lower and shorter sound.

    **Warning**: Increasing `BP Q` increase the chance of oscillation or blow up.

### Secondary Membrane
Secondary membrane inherits most of the values from the Primary Membrane section.

Mix

:   Mixing ratio between primary and secondary membrane. When set to 1.0, only the output from the secondary will be mixed.

    **Note**: Collisions will be calculated even if `Mix` is set to 0.0 or 1.0. To stop collision, increase the value of `Collision Distance`.

Pitch Offset \[oct\]

:   Secondary membrane pitch with respect to primary.

    When set to 0.0, the related values are copied from Primary Membrane section.

    Adjusting `Pitch Offset` and `Q Offset` is similar to detuning one side of a snare drum.

Q Offset \[oct\]

:   Bandpass Q of secondary membrane with respect to primary.

Collision Distance

:   Threshold value used to determine collision between primary and secondary membrane.

    **Warning**: Oscillation may occur when the value of `Collision Distance` is small. To prevent oscillation, increase the value.

## Change Log
{%- for version, logs in changelog["GenericDrum"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["GenericDrum"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["GenericDrum"] %}
- [GenericDrum {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
GenericDrum is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
