---
lang: en
...

# EnvelopedSine
![](img/EnvelopedSine.png)

EnvelopedSine is an additive synthesizer that computes 64 sine waves for each note. Difference to IterativeSinCluster is that this synth has AD envelope and saturator for each oscillator. EnvelopedSine is better suited for percussive sounds.

- [Download EnvelopedSine {{ latest_version["EnvelopedSine"] }} - VST® 3 (github.com)]({{ latest_download_url["EnvelopedSine"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["EnvelopedSine"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["EnvelopedSine"] }})
{%- endif %}

EnvelopedSine requires CPU which supports AVX or later SIMD instructions.

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

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/EnvelopedSine.svg)

## Parameters
### Overtone
4 big controls on top right.

Attack, Decay

:   Gain envelope attack and decay for each oscillator.

Gain

:   Gain for each oscillator.

Saturation

:   Saturation gain for each oscillator.

### Gain
Boost, Gain

:   Both controls output gain. Peak value of output will be `Boost * Gain`.

### Pitch
Add Aliasing

:   When checked, the synth enables rendering of sine waves over nyquist frequency.

    Roughly speaking, nyquist frequency is the highest frequency that can be reconstructed from recorded digital signal. When generating sound, it's possible to set value that is higher than nyquist frequency. However, the result may contain unexpected frequency due to a phenomenon called aliasing. `Add Aliasing` is option to add those aliasing noise.

Octave

:   Note octave.

Multiply, Modulo

:   Change sine wave frequency.

    Equation is `noteFrequency * (1 + fmod(Multiply * pitch, Modulo))`. `pitch` is calculated from note pitch and overtone index. `fmod(a, b)` is a function that returns reminder of `a / b`.

Expand

:   Scaling factor for overtone controls.

    ![](img/EnvelopedSine_expand.svg)

Shift

:   Amount of right shift to overtone controls.

    ![](img/EnvelopedSine_shift.svg)

### Random
Randomize parameters. These are more effective when used with `Unison`.

Retrigger

:   When checked, reset random seed for each note-on.

Seed

:   Random seed. This value change random number sequence.

To Gain, To Attack, To Decay, To Sat.

:   Amount of randomization to overtone controls. Equation is `value * random`. Range of `random` is `[0.0, 1.0)`.

To Pitch

:   Amount of randomization to pitch.

To Phase

:   Amount of randomization to phase.

### Misc.
Smooth

:   Time length to change some parameter value to current one. Unit is in second.

    List of parameters related to `Smooth`. `*` represents wild card.

    - All parameters in `Gain` section.
    - All parameters in `Phaser` section, except `nStages`.

    Other parameter uses the value obtained from the timing of note-on for entire duration of a note.

nVoices

:   Maximum polyphony. Lowering the number of this option reduces CPU load.

Unison

:   When checked, unison is enabled.

    When unison is enabled, 1 note uses 2 voices. 1 voice is placed on left and other is placed on right. Combining `Unison` with `Random.To Phase`, `Random.To Pitch`, etc. can be used to make sound with stereo spread.

### Modifier
Attack\*, Decay\*

:   Multiplier for `Attack`/`Decay` in overtone control section.

Declick

:   When checked, it reduces click noise that occurs when the value of `Attack` and/or `Decay` is 0.

Gain^

:   Exponent to `Gain` in overtone control. Following is the equation of gain of an oscillator.

    ```
    Gain.Boost * Gain.Gain * pow(Overtone.Gain, Modifier.Gain^)
    ```

Sat. Mix

:   Mixing ratio of dry/wet signal of saturation. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

### Phaser
nStages

:   Number of all-pass filter.

Mix

:   Mixing ratio of dry/wet signal of phaser. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

Freq

:   LFO frequency.

Feedback

:   Amount of feedback. Feedback is disabled when the knob is pointing to 12 o'clock. It becomes negative feedback when turned to left and positive feedback when turned to right.

Range

:   Range of all-pass filter modulation by LFO.

Min

:   Minimum value of all-pass filter modulation by LFO.

Offset

:   LFO phase offset.

Phase

:   LFO phase. This can be used to make sound with automation. Turning `Freq` to leftmost sets LFO frequency to 0.

## Change Log
{%- for version, logs in changelog["EnvelopedSine"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["EnvelopedSine"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["EnvelopedSine"] %}
- [EnvelopedSine {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
EnvelopedSine is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
