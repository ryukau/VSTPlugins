---
lang: en
...

# FeedbackPhaser
![](img/FeedbackPhaser.png)

FeedbackPhaser is a phaser but the modulation source is feedback signal rather than an LFO. It sounds more like a distortion rather than a phaser most of times.

{% for target, download_url in latest_download_url["FeedbackPhaser"].items() %}
- [Download FeedbackPhaser {{ latest_version["FeedbackPhaser"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["FeedbackPhaser"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["FeedbackPhaser"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
It is recommended to always insert limiter after FeedbackPhaser. Output amplitude can exceed +140 dB, depending on the values of Feedback `Amount` and `Safety Clip`.

## Usage
### Quick Start
Below is an example setup steps.

1. Launch FeedbackPhaser.
2. Lower `Output` depending on the amplitude of input.
3. Change Feedback `Amount`.
4. Change Modulation `Amount`.
5. Change `Safety Clip`.

Output amplitude is mostly determined by above steps. After that, start changing other parameters.

Note that the character of Modulation `Amount` changes depending on Modulation `Type`. It consequently changes the output amplitude.

### Taming Feedback
`Safety Clip` is the most important parameter to limit output amplitude. if feedback amplitude exceeds `Safety Clip` amplitude, the text indicator:

```
Output is clean.
```

changes to:

```
Too much feedback.
```

Distortion will be added when "Too much feedback." is displayed. If distortion is not desired, increase `Safety Clip`, and decrease `Output`.

Sometimes increasing `Safety Clip` can't stop distortion. In this case, reduce Feedback `Amount` and/or Modulation `Amount`.

### Where to Automate
Following parameters might be fun to automate.

- Allpass
  - `Spread`.
  - `Center`.
- Modulation
  - `Mod. Asym.`.

Also, note event can be used to control Allpass `Center`.

More Allpass `Stage` adds more resonance for feedback. However, it also dilutes the texture of input signal.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/FeedbackPhaser.svg)

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in FeedbackPhaser.

- \[dB\] : Decibel.
- \[s\] : Second.
- \[st.\] : Semitone.
- \[Hz\] : Frequency in Hertz.

### Gain
Output \[dB\]

:   Output gain.

Highpass \[Hz\]

:   Cutoff frequency of highpass filter that is applied right before output.

    This highpass filter is used to remove direct current.

Mix

:   Mixing ratio of input and output.

    Intensity of Feedback -> `Amount` changes depending on value of `Mix`.

### Feedback
Amount

:   Amount of feedback.

    Feedback signal may be hard clipped when absolute value of Feedback -> `Amount` is high.

Safety Clip \[dB\]

:   When amplitude of feedback signal exceeds `Safety Clip`, feedback signal will be hard clipped internally.

    FeedbackPhaser applies hard clipping on feedback signal to prevent blowing up. Distortion will occur when hard clipping is engaged.

    To get clean sound, increase `Safety Clip` until text indicator in the Feedback section stays to `Output is clean.`, and no longer changes to `Too much feedback.` Beware that increasing `Safety Clip` sometimes drastically increases output amplitude. To compensate, turn down `Output` at the same time.

Highpass \[Hz\]

:   Cutoff frequency of highpass filter that is applied to feedback signal.

### Allpass
Stage

:   Number of allpass filters to serially connect.

Spread

:   Value to determine allpass filter cutoff frequency distribution.

    When `Spread` is 0, all of cutoff frequencies become as same as `Center`. When `Spread` is 1, cutoff frequencies becomes higher for later index, as in `Center * 1`, `Center * 2`, `Center * 3`, and so on.

    Below is the formula to calculate cutoff frequency of allpass filter.

    ```
    allpassCutoffHz = modulation * Center * (1 - Spread * (index - 1)).
    ```

Center \[Hz\]

:   Cutoff frequency of allpass filters when not modulated.

### Modulation
Type

:   Type of modulation scaling.

    - `Exp. Mul.` : Exponentially multiplies modulation amount as `Center * 2^modulation`.
    - `Lin. Mul.` : Linearly multiplies modulation amount as `Center * modulation`.
    - `Add` : Adds modulation amount as `Center + modulation`.

    The formulas on the list above aren't directly used in implementation. Range restrictions and other things are also performed to avoid invalid values.

Amount

:   Amount of modulation by feedback signal.

    Modulation is applied to cutoff frequency of allpass filter.

Mod. Asym.

:   Mixing ratio between raw and full-wave rectified modulation signal.

Post LP \[Hz\]

:   Lowpass filter cutoff frequency.

    This lowpass filter applies to allpass filter cutoff frequencies after modulation.

### Note
Transpose \[st.\]

:   Note event transpose amount in semitones.

To Allpass

:   Multiplier to pitch of a note.

    When `To Allpass` is 1, pitch is used without scaling.

    Note that FeedbackPhaser uses relative pitch represented by note, not absolute frequency represented by note.

Slide \[s\]

:   Pitch slide time when receiving a note-on before receiving a note-off of another note.

Release \[s\]

:   Approximate time to return to `Center` from pitch of last note.

### Misc.
Side Chain

:   When lit, uses sidechain input as modulating signal.

Smoothing \[s\]

:   Time to reach new value after change of parameter.

Oversampling

:   Oversampling ratio.

    Increasing oversampling ratio may suppress gritty noise that appears when modulation is high. However, CPU load increases with higher oversampling ratio.

## Change Log
{%- for version, logs in changelog["FeedbackPhaser"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["FeedbackPhaser"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["FeedbackPhaser"] %}
- [FeedbackPhaser {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
FeedbackPhaser is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
