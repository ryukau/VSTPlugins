---
lang: en
...

# WaveCymbal
![](img/WaveCymbal.png)

WaveCymbal is an attempt to make a cymbal synthesizer with banded wave-guide model. The result is more like dragging bunch of empty cans or thin metal plate on asphalt rather than cymbal.

- [Download WaveCymbal {{ latest_version["WaveCymbal"] }} - VST® 3 (github.com)]({{ latest_download_url["WaveCymbal"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["WaveCymbal"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["WaveCymbal"] }})
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
Parameters that turns red when pointing can drastically change output gain. Changing those parameters with Shift + Left Drag is recommended. To prevent sudden clipping, always insert limiter after WaveCymbal.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/WaveCymbal.svg)

## Parameters
### Gain
Output gain.

### Excitation
Short delays that turns impulse to tone.

Feedback

:   Feedback of excitor delays.

Time

:   Delay time of excitor delays. Beware that when the value of this parameter is high, output will be loud.

### Objects
nCymbal

:   Number of cymbal-ish objects.

nString

:   Number of Karplus-Strong string per cymbal object.

### Wave
Damping

:   Damping of 1D wave simulation. Beware that when the value of this parameter is high, output will be loud.

PulsePosition

:   Position where oscillator input causes wave.

PulseWidth

:   Width of wave caused by oscillator input.

### Collision
When turns on, it sounds like a rubbing on surface of thin light metal plate. When `nCymbal` is 1, this doesn't change output sound.

Distance

:   Distance between cymbals. Collision will more likely to occur when turning this knob to left.

### Random
Seed

:   Random seed. Output will be the same for each note while checking `Retrigger`.

Amount

:   Amount of random number effects to delay time and band-pass cutoff. Beware that when the value of this parameter is low, output may be loud.

### String
MinHz

:   Lower bound of Karplus-Strong string frequency.

MaxHz

:   Upper bound of Karplus-Strong string frequency.

Decay

:   Decay of Karplus-Strong string. Decay time will be longer when turning this knob to left.

Q

:   Band-pass filter Q.

### Oscillator
Retrigger

:   When checking, each note on resets random seed.

OscType

:   Oscillator type.

    - `Off` : Stop making noise for each note on. This option will be used when using WaveCymbal as FX.
    - `Impulse` : Outputs impulse for each note on. Simulates hit of cymbal with stick.
    - `Sustain` : Outputs impulse train with pitch of note. Simulates drawing of bow on surface of cymbal.
    - `Velvet Noise` : Outputs velvet noise of a density respect to pitch of note. Simulates less uniform drawing than `Sustain`.
    - `Brown Noise` : Outputs brown noise of a brightness respect to pitch of note. Kind of like dragging cymbal on asphalt.

Bandpass Cutoff Distribution

:   - `Log` : Sound becomes relatively natural.
    - `Linear` : Leans sound to higher frequency.

### Smoothness
Transition time from previous value to current value in seconds. `Gain`, `Excitation.Time`, `Random.Amount`, `Bandpass.MinCutoff`, `Bandpass.MaxCutoff` will be affected. Slide effect of `OscType.Sustain` can be tuned by `Smoothness`.

## Change Log
{%- for version, logs in changelog["WaveCymbal"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["WaveCymbal"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["WaveCymbal"] %}
- [WaveCymbal {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
WaveCymbal is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
