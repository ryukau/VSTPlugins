---
lang: en
...

# L4Reverb
![](img/L4Reverb.png)

L4Reverb is an extended version of LatticeReverb. This time, lattice structure has 4 * 4 * 4 * 4 = 256 sections per channel.

{% for target, download_url in latest_download_url["L4Reverb"].items() %}
- [Download L4Reverb {{ latest_version["L4Reverb"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["L4Reverb"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["L4Reverb"] }})
{%- endif %}

If L4Reverb is too heavy for your machine, L3Reverb is also available. L3Reverb is a trimmed down version of L4Reverb. The lattice structure has 3 * 3 * 3 * 5 = 135 sections per channel.

{% for target, download_url in latest_download_url["L3Reverb"].items() %}
- [Download L3Reverb {{ latest_version["L3Reverb"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["L3Reverb"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["L3Reverb"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## Caution
Output may change with different sample rate or buffer size.

Output may blow up when both `Cross` and `Spread` are not 0.

Output may become loud when `offset` of `*Feed` is not 0, and modulation is enabled.

Output may become loud when following steps are performed.

1. Set some of the `OuterFeed` or `InnerFeed` to close to minimum or maximum.
2. Input signals.
3. Change the value of `OuterFeed` or `InnerFeed` which was set at step 1.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/L4Reverb.svg)

## Parameters

Time

:   Delay time of all-pass filter.

InnerFeed

:   Feedback and feedforward gain of all-pass filters nested inside of lattice structure.

D1Feed

:   Feedback and feedforward gain of lattice structure at lowest layer.

D2Feed

:   Feedback and feedforward gain of lattice structure at 2nd lowest layer.

D3Feed

:   Feedback and feedforward gain of lattice structure at 3rd lowest layer.

D3Feed

:   Feedback and feedforward gain of lattice structure at top layer.

**Tip**: Setting all of the `*Feed` values to negative makes output more wet by blurring  input signal. Especially recommend to fine tuning `D4Feed`.

### Multiplier
Multiplier for `Time` and `*Feed`. Useful to shorten or lengthen reverb.

`Base` values are used in both left and right channel. `Base` value determines the character of reverb.

`Offset` values are maximum ratio between left and right channel. Amount of ratio is randomized. Changing `Offset` spreads reverb to stereo.

```
if (Offset >= 0) {
  valueL = Base
  valueR = Base * (1 - Offset)
}
else {
  valueL = Base * (1 + Offset)
  valueR = Base
}
```

When `Modulation` is checked, `Offset` values will be randomized for each buffer. `Smooth` can be used to change the character of modulation.

### Panic!
Pressing `Panic!` button stops reverb output by setting multiplier of `Time` and `*Feed` to 0.

Useful to stop sounds in case of blow up.

### Mix
Dry

:   Gain of input signal.

Wet

:   Gain of reverb signal.

### Stereo
Cross

:   Feedback amount from an output of lattice structure to the lattice structure on another channel. If the value is 0, signal from other channel will not be mixed.

Spread

:   Mid-side (M-S) signal ratio.

    Following equations are used to calculate mid-side signal.

    ```
    mid  = left + right
    side = left - right

    left  = mid - Spread * (mid - side)
    right = mid - Spread * (mid + side)
    ```

### Misc.
Seed

:    Seed value of random number sequence.

Smooth

:    Transition time to change parameter value to current one. Unit is in second.

## Change Log
### L4Reverb
{%- for version, logs in changelog["L4Reverb"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

### L3Reverb
{%- for version, logs in changelog["L3Reverb"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
### L4Reverb
{%- if old_download_link["L4Reverb"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["L4Reverb"] %}
- [L4Reverb {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

### L3Reverb
{%- if old_download_link["L3Reverb"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["L3Reverb"] %}
- [L3Reverb {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
L4Reverb and L3Reverb is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
