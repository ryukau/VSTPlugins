---
lang: en
...

# UltrasonicRingMod
![](img/UltrasonicRingMod.png)

UltrasonicRingMod is a ring modulator with 64-fold oversampling. Maximum modulator frequency is 1 MHz. That said, just modulating with ultrasonic frequency doesn't produce anything audible. Therefore, phase modulation and hard clipping are added to make some noise in audible frequency.

{% for target, download_url in latest_download_url["UltrasonicRingMod"].items() %}
- [Download UltrasonicRingMod {{ latest_version["UltrasonicRingMod"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["UltrasonicRingMod"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["UltrasonicRingMod"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
Amplitude increases by 120 dB maximum when `Mod. Scale` is turned to right. Recommend to use limiter.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/UltrasonicRingMod.svg)

## Parameters
Mix

:   Mixing ratio of input and modulated signal.

    Note that mixing is done before hard clipping. In other words, hard clipping is applied even if `Mix` is set to leftmost, and `Hardclip` is not set to leftmost.

    UltrasonicRingMod do not bypass multi-rate processing (oversampling). To bypass completely, use mixer on DAW.

Pre Clip

:   A gain right before `Hardclip`.

Out

:   Output gain.

    Beware that maximum gain is +60 dB.

Frequency

:   Modulator frequency.

DC Offset

:   Amplitude of direct current (DC) added to modulator.

    Range of amplitude becomes `[-1, 1]` at leftmost, and `[0, 1]` at rightmost.

Feedback

:   Amount of feedback from ring modulator output to modulator phase.

Mod. Scale

:   Ratio to make amplitude of modulator proportional to `Frequency`.

Mod. Wrap

:   Mixing ratio of raw and wrapped-at-1 modulator signal.

    Wrapped-at-1 is same operation as `fmod(value, 1.0)`.

Hardclip

:   Mixing ratio of raw and hard-clipped ring modulator output.

Smoothing \[s\]

:   Parameter smoothing time in second.

    Internally, inverse of `Smoothing` is used as cutoff frequency. Therefore the time is not exact, but a rough estimate.

### Note
Modulator frequency can be controled by note-on. To send note events, please refer to the manual of DAW you are using.

Negative, Scale

:   `Scale` is a multiplier to note. When `Negative` is lit, it negates the value of `Scale`.

    Provided note multiplies the value of `Frequency`. Following is the equation to calculate modulator frequency.

    ```
    sign = negative ? -1 : 1.
    modulatorFrequency = frequency * 2^(sign * scale * (midiNote + offset - 69) / 12).
    ```

Offset \[st.\]

:   A value added to input note number. Unit is semitone.

Slide Time \[s\]

:   Pitch slide time from previous note to current note.

    Internally, inverse of `Slide Time` is used as cutoff frequency. Therefore the time is not exact, but a rough estimate.

## Change Log
{%- for version, logs in changelog["UltrasonicRingMod"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["UltrasonicRingMod"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["UltrasonicRingMod"] %}
- [UltrasonicRingMod {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
UltrasonicRingMod is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
