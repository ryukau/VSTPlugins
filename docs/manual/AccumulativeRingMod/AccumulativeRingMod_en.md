---
lang: en
...

# AccumulativeRingMod
![](img/AccumulativeRingMod.png)

AccumulativeRingMod is a ring modulator that uses accumulated amplitude of input signal as the phase. When applied to simple waveforms like a sine wave, it sounds similar to FM.

{% for target, download_url in latest_download_url["AccumulativeRingMod"].items() %}
- [Download AccumulativeRingMod {{ latest_version["AccumulativeRingMod"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["AccumulativeRingMod"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["AccumulativeRingMod"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
In some DAW, main input is fed to sidechain input, when sidechain input is not routed.

- TODO 図

## Usage
AccumulativeRingMod is good at adding variation to simple waveforms such as sine wave and sawtooth wave. It is recommended to lower `Modulation` and Warp -> `Amount` when applying to recorded instrument sounds.

It might make some nice sounds by automating `Modulation` and Warp -> `Amount` with an external envelope generator or LFO. Also, it may sounds better with decaying sound because the amount of modulation changes depending on the amplitude of input signal.

To modulate using a part of pre-mixed sounds, such as snare drum in a drum loop, try following steps.

1. Narrow down target frequency band by changing `Lowpass` and `Highpass`.
2. Increase `Gate` to silence unnecessary parts.
3. Enable `Envelope` to adjust the texture.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/AccumulativeRingMod.svg)

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in AccumulativeRingMod.

- \[dB\] : Decibel.
- \[s\] : Second.
- \[Hz\] : Frequency in Hertz.

### Gain
Output

:   Output gain.

Mix

:   Mixing ratio of input and output.

### Stereo
Link \[Hz\]

:   Cutoff frequency of lowpass filter used to synchronize phase of 2 ring modulators on left (L) and right (R) channels.

    When signal of L and R are different, phase of ring modulator will become out of sync. After that, the difference of the phases stays there even if the L and R signal goes back to the same. Sometimes this results in undesired stereo spread. `Link` can be used to suppress this stereo spread.

    Higher values of `Link` will synchronize faster, lower values will synchronize more slowly. When `Link` is 0, synchronization is disabled.

Cross

:   Amount to swap stereo channels.

    When `Cross` is 0, it stays the same, when it's 0.5 it mixes left and right evenly, and when it's 1 it completely swaps left and right.

Offset

:   Offset between phase of two ring modulators on stereo.

    Setting `Offset` other than 0 spreads stereo image of output.

### Warp
Amount

:   Amount to warp the phase of ring modulator.

    Beware that CPU load increases when Warp -> `Amount` is not 0.

### Misc.
Smoothing \[s\]

:   Time to reach new value after change of parameter.

Oversampling

:   Oversampling ratio.

    Increasing oversampling ratio may suppress gritty noise that appears when `Modulation` value is high. However, CPU load increases with higher oversampling ratio.

### Main Input, Side Chain
Same set of parameters are available for main input and sidechain input.

Modulation

:   Intensity of modulation.

Lowpass \[Hz\]

:   Cutoff frequency of lowpass filter applied to modulation signal.

Highpass \[Hz\]

:   Cutoff frequency of highpass filter applied to modulation signal.

Gate \[dB\]

:   Gate amplitude threshold for modulation signal.

    Modulation stops when amplitude of filtered modulation signal goes below threshold set by `Gate`.

Envelope \[s\]

:   Lighting up `Envelope` button enables envelope follower. The value on right side is release time of envelope.

Asymmetry - X Pre

:   Mixing ratio of raw and full-wave rectified modulation signal before passing filters.

Asymmetry - Y Post

:   Mixing ratio of raw and full-wave rectified modulation signal after passing filters.

## Change Log
{%- for version, logs in changelog["AccumulativeRingMod"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["AccumulativeRingMod"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["AccumulativeRingMod"] %}
- [AccumulativeRingMod {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
AccumulativeRingMod is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
