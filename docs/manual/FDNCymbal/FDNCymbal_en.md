---
lang: en
...

# FDNCymbal
![](img/FDNCymbal.png)

FDNCymbal is a cymbal sound synthesizer. It can also be used as an effect. Unlike the name, most of metallic texture comes from Schroeder allpass section rather than FDN (feedback delay network). FDN section makes nice impact sound when `FDN.Time` is short. Tremolo is added to simulate wobbling of cymbal.

{% for target, download_url in latest_download_url["FDNCymbal"].items() %}
- [Download FDNCymbal {{ latest_version["FDNCymbal"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["FDNCymbal"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["FDNCymbal"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
There's no compatibility between version 0.1.x and 0.2.x.

When `FDN.Feedback` is non zero, it may possibly blow up. If that happens, turn `FDN.Feedback` to leftmost.

When `HP Cutoff` is moving fast, it may output massive DC. To avoid this, use Shift + Mouse Left Drag or turn up Smooth.

## Use as Effect
FDNCymbal can be use as effect, but it's not intuitive.

When loaded, FDNCymbal is muted to prevent unexpected blow up at initialization. Therefore user must send MIDI note-on to FDNCymbal. Note that output volume could be changed by velocity. `Seed` and parameters under `FDN` section are only refreshed when FDNCymbal receives a new note.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/FDNCymbal.svg)

## Parameters
### Gain
Output gain.

### Smooth
Time in seconds to move the value of parameters from previous one to changed one. Below is a list of parameter that is affected by `Smooth`.

- `Gain`
- `FDN.Time`
- `FDN.Feedback`
- `FDN.CascadeMix`
- `Allpass.Mix`
- `Allpass.Stage1.Time`
- `Allpass.Stage1.Feedback`
- `Allpass.Stage2.Time`
- `Allpass.Stage2.Feedback`
- `Tremolo.Mix`
- `Tremolo.Depth`
- `Tremolo.Frequency`
- `Tremolo.DelayTime`

### Stick
When `Stick` is on, each note on triggers stick oscillator. Noise density and pitch of tone are related to the pitch of note.

Pulse

:   Gain of impulse of stick oscillator.

Tone

:   Gain of sine wave tone of stick oscillator.

Velvet

:   Gain of velvet noise of stick oscillator.

    Velvet noise is a noise algorithm which output different height of impulses with randomized interval.

Decay

:   Decay time of stick oscillator.

### Random
Seed

:   Value of random seed.

Retrigger.Time

:   When not checked, internal delay time controlled by following 3 parameters will be changed for each note on.

    - `FDN.Time`
    - `Allpass.Stage1.Time`
    - `Allpass.Stage2.Time`

Retrigger.Stick

:   When not checked, stick oscillator output varies for each note.

Retrigger.Tremolo

:   When not checked, tremolo parameters will be changed for each note on.

### FDN
When turned on, the signal go through FDN section.

Time

:   FDN delay time. Internal delay time will be randomized with respect to this value.

Feedback

:   FDN section feedback. Beware that if this value is non zero, it's possible to blow up.

CascadeMix

:   Controls mixing of Cascaded FDN.

### Allpass
Mix

:   Mixing ratio of Schroeder allpass section.

### Stage 1 and Stage 2
Stage 1 is serially connected 8 Schroeder allpass. On stage 2, there are 4 Schroeder allpass section connected in parallel. For each section has serially connected 8 Schroeder allpass.

Time

:   Max delay time of Schroeder allpass section. Internally, delay time will be randomized.

Feedback

:   Feedback of Schroeder allpass section.

HP Cutoff

:   Cutoff frequency of high-pass filter to the output of Schroeder allpass section.

Tanh

:   When checked, stage 1 feedback go through tanh saturator.

### Tremolo
Mix

:   Mixing ratio of tremolo effect.

Depth

:   Maximum change of amplitude with tremolo.

Frequency

:   Tremolo LFO frequency.

DelayTime

:   Maximum delay time. Delay time is modulated by LFO. This delay simulates doppler effect.

### Random (Tremolo)
Depth

:   Amount of randomization to `Tremolo.Depth` for each note on.

Freq

:   Amount of randomization to `Tremolo.Frequency` for each note on.

Time

:   Amount of randomization to `Tremolo.DelayTime` for each note on.

## Change Log
{%- for version, logs in changelog["FDNCymbal"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["FDNCymbal"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["FDNCymbal"] %}
- [FDNCymbal {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

Presets for old versions.

- [Presets for version 0.1.* (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/FDNCymbalPresets.zip)

## License
FDNCymbal is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
