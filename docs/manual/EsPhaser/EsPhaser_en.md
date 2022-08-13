---
lang: en
...

# EsPhaser
![](img/EsPhaser.png)

EsPhaser is a phaser with up to 4096 stages of order 2 Thiran all-pass filters. This is the same phaser used in EnvelopedSine.

- [Download EsPhaser {{ latest_version["EsPhaser"] }} - VST® 3 (github.com)]({{ latest_download_url["EsPhaser"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["EsPhaser"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["EsPhaser"] }})
{%- endif %}

EsPhaser requires CPU which supports AVX or later SIMD instructions.

The package includes following builds:

- Windows 64bit
- Linux 64bit

{{ section["macos_warning"] }}

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely to run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
When stage is set to 4096, it will be CPU intensive.

Output varies in different sample rate.

Output may be loud when changing Cas. Offset. Use  <kbd>Shift</kbd> + <kbd>Left Drag</kbd> to slowly change the value, or insert limiter to prevent hard clipping.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/EsPhaser.svg)

## Parameters
Stages

:   Number of all-pass filter.

Mix

:   Mixing ratio of dry/wet signal of phaser. `Dry : Wet` becomes `0 : 1` when turned the knob to rightmost.

Freq

:   LFO frequency.

Spread

:   Spread frequency between LFOs.

    Equation for difference of LFO phase in 1 sample:

    ```
    deltaPhase = 2 * pi * Freq / ((1 + LfoIndex * Spread) * sampleRate)
    ```

Feedback

:   Amount of feedback. Feedback is disabled when the knob is pointing to 12 o'clock. It becomes negative feedback when turned to left and positive feedback when turned to right.

Range

:   Range of all-pass filter modulation by LFO.

Min

:   Minimum value of all-pass filter modulation by LFO.

Cas. Offset

:   Phase offset between 16 LFO.

L/R Offset

:   LFO phase offset between L/R channels.

Phase

:   LFO phase. This can be used to make sound with automation. Turning `Freq` to leftmost sets LFO frequency to 0.

    Equation for phase offset:

    ```
    LfoPhaseOffset = Phase + (L/R Offset) + LfoIndex * (Cas. Offset)
    ```

## Change Log
{%- for version, logs in changelog["EsPhaser"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["EsPhaser"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["EsPhaser"] %}
- [EsPhaser {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
EsPhaser is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
