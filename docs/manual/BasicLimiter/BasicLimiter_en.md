---
lang: en
...

# BasicLimiter
![](img/basiclimiter.png)

BasicLimiter is a basic single band limiter. The sound is nothing new, but the design of true peak mode is a bit luxurious.

- [Download BasicLimiter {{ latest_version["BasicLimiter"] }} - VST® 3 (github.com)]({{ latest_download_url["BasicLimiter"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["BasicLimiter"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["BasicLimiter"] }})
{%- endif %}

An extended version BasicLimiterAutoMake is also available. Added features are automatic make up gain, sidechain, and switching between left-right (L-R) and mid-side (M-S). Note that CPU load is over 1.5 times heavier than BasicLimiter.

- [Download BasicLimiterAutoMake {{ latest_version["BasicLimiterAutoMake"] }} - VST® 3 (github.com)]({{ latest_download_url["BasicLimiterAutoMake"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["BasicLimiterAutoMake"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["BasicLimiterAutoMake"] }})
{%- endif %}

The package includes following builds:

- Windows 64bit
- Linux 64bit
- macOS universal binary

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely to run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
The algorithm used in BasicLimiter causes over-limiting when input amplitude is extremely high. When over-limiting happens, higher input amplitude turns into lower output amplitude. This problems is expected to happen when input amplitude exceeds `2^53`, or +319 dB.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/basiclimiter.svg)

## Parameters
Threshold \[dB\]

:   Threshold of limiter output amplitude.

    When `True Peak` is off, output amplitude is limited under `Threshold`.

    When `True Peak` is on, output amplitude might exceeds `Threshold`. This is due to the limitation of real-time algorithm.

Gate \[dB\]

:   Threshold of gate. When input amplitude is less than the value of `Gate` for the duration of `Attack`, output amplitude becomes 0.

Attack \[s\]

:   Transition time of smoothing filter which applies to internal envelope. The value of `Attack` also adds to the latency.

    For the sound with sharp transitions like drums, recommend to set `Attack` under 0.02 seconds. This value, 0.02 seconds, is based on a psychoacoustic effect called [temporal masking](https://web.archive.org/web/20210624083625/http://ccrma.stanford.edu:80/~bosse/proj/node21.html).

Release \[s\]

:   Smoothness to reset internal envelope to neutral position.

    Internally, the inverse value of `Release` is used as cutoff frequency. This means that the value of `Release` is not exact. Because of temporal masking, setting the sum of `Release` and `Sustain` under 0.2 seconds is recommended.

Sustain \[s\]

:   Additional peak hold time for the internal envelope.

    `Sustain` causes more ducking when applied to sounds like drums. For sounds like distorted guitar or sustaining synthesizer, `Sustain` might sound cleaner than `Release`, because it works similar to auto-gain. Note that when sustain is longer, it deviates from the curve of temporal masking.

Stereo Link

:   When `Stereo Link` is set to 0.0, limiter for each channel works independently. When set to 1.0, maximum of all the input amplitude is used for both limiter. Following snippet shows the calculation of `Stereo Link`

    ```
    absL = fabs(leftInput).
    absR = fabs(rightInput).

    absMax = max(absL, absR).

    amplitudeL = absL + stereoLink * (absMax - absL).
    amplitudeR = absR + stereoLink * (absMax - absR).
    ```

    When `Stereo Link` is set to 0.0, and input amplitude is leaned to left or right, it may sounds like the pan is wobbling. To reduce wobbling, set the value to 0.0, then gradually increase the value to 1.0. Stop increasing the value when pan wobbling becomes inaudible.

True Peak

:   Enables true peak mode when checked.

    While true peak mode is enabled, lowpass filter is applied to remove the components near nyquist frequency. This lowpass filter is designed to only change the gain over 18000 Hz when sampling rate is 48000 Hz.

    Sample peak might exceeds `Threshold`. Especially when sample peak exceeds 0 dB, the value of `Overshoot` becomes greater than 0. Lower `Threshold` in this case.

    True peak restoration at nyquist frequency requires infinite length FIR filter (sinc interpolation). Therefore, it's impossible to compute in real time. This is the reason that lowpass is applied, and true peak mode overshoots.

Reset Overshoot

:   Resets `Overshoot` to 0 when clicked.

    When output sample peak exceeds 0 dB, the value of `Overshoots` changes to greater than 0, and `Reset Overshoot` will be lit.

### BasicLimiterAutoMake Specific Parameters
Auto Make Up

:   Enable automatic make up gain when checked.

    When `Auto Make Up` is enabled, output amplitude become lower when `Threshold` is greater than `Auto Make Up Target Gain`.

    When `Auto Make Up` is enabled, and `Threshold` is increasing, overshoot may occur. Recommend to set the target gain to -0.1 dB (default) or lower in this case.

    If `Threshold` needs to be changed when input signal is hot, insert another limiter for safe guard.

Auto Make Up Target Gain

:   Maximum amplitude after automatic make up gain is applied. This is a control placed on the right side of `Auto Make Up`. Unit is decibel.

    When `Channel Type` is set to `M-S`, maximum amplitude is +6 dB of `Auto Make Up Target Gain`.

Sidechain

:   When checked, enables sidechain and disables `Auto Make Up`.

    `Auto Make Up` is disabled because source amplitude is not affected by `Threshold` while sidechain is activated.

    BasicLimiterAutoMake has 2 stereo input. No. 1 is source, and No. 2 is sidechain. For routing, refer to your DAW manual.

Channel Type

:   Switch the type of stereo channel between left-right (`L-R`) and mid-side (`M-S`).

    When the type is `M-S`, sample peak becomes `2 * Threshold`, or +6.02 dB over `Threshold`. Therefore, when using `Auto Make Up` with `M-S`, it is recommended to set `Auto Make Up Target Gain` to -6.1 dB or lower. This behavior aims to provide the same loudness when comparing `L-R` and `M-S`.

## Change Log
### BasicLimiter
{%- for version, logs in changelog["BasicLimiter"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

### BasicLimiterAutoMake
{%- for version, logs in changelog["BasicLimiterAutoMake"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
### BasicLimiter
{%- if old_download_link["BasicLimiter"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["BasicLimiter"] %}
- [BasicLimiter {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

### BasicLimiterAutoMake
{%- if old_download_link["BasicLimiterAutoMake"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["BasicLimiterAutoMake"] %}
- [BasicLimiterAutoMake {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
BasicLimiter is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
