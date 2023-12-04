---
lang: en
...

# NarrowingDelay
![](img/NarrowingDelay.png)

NarrowingDelay is a delay with serially connected pitch shifter and frequency shifter on feedback path. The spectrum of input signal can be narrowed or expanded for each feedback. It is more suitable for sound without stable pitch. Note event can be used to control frequency shift amount.

{% for target, download_url in latest_download_url["NarrowingDelay"].items() %}
- [Download NarrowingDelay {{ latest_version["NarrowingDelay"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["NarrowingDelay"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["NarrowingDelay"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/NarrowingDelay.svg)

## Note Event Input
NarrowingDelay can receive note events to control the amount of frequency shift. Following the equation to calculate the amount of frequency shift.

```
# `note` is MIDI note number.
(Frequency Shift) = 2^((note - 69) / 12) * ((Value of Shift [Hz]) + lfo).
```

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in NarrowingDelay.

- \[s\]: Second.
- \[oct.\]: Octave.
- \[Hz\]: Herz.

### Shifter
Dry

:   Gain for bypassing input signal.

Wet

:   Gain for output signal comes out from NarrowingDelay.

Feedback

:   Feedback gain.

HP, LP

:   Cutoff frequency of highpass (`HP`) or lowpass (`LP`) filter on feedback path.

Delay Time \[s\]

:   Buffer length of pitch shifter. The value is not exact, but a rough indication.

    When the value of `Shift \[oct.\]` is not 0.0, delay time is doesn't match the displayed value because of the change of buffer playback speed.

Shift \[oct.\], Shift \[Hz\]

:   `Shift [oct.]` is the amount of pitch shift, and `Shift [Hz]` is the amount of frequency shift.

    To input frequencies, pitch shift performs multiplication, and frequency shift performs addition. It can be written as following equation.

    ```
    (Output Frequency) ~= (Input Frequency) * 2^(Pitch Shift) + (Frequency Shift).
    ```

    This means that when the amount of pitch shift is less than 0.0, spectrum becomes narrower. And when the amount of pitch shift is greater than 0.0, spectrum becomes broader than the original. NarrowingDelay was created with the idea that if the fundamental frequencies of the input and output are matched by frequency shifting after narrowing the spectrum, it might produce an interesting sound.

    The frequency shift algorithm used in NarrowingDelay can't decrease frequency. Therefore a procedure that decreasing frequency after broadening spectrum can't be used to match fundamental frequency.

LFO

:   LFO modulation amount. Following 3 parameters can be targeted.

    - `Delay Time [s]`
    - `Shift [oct.]`
    - `Shift [Hz]`

### LFO
L-R Offset

:   LFO phase offset between stereo channels.

Phase

:   A value that is added to LFO phase.

    To manually control LFO phase, set `Rate` to leftmost to stop LFO, then change the value of `Phase`. Also, the value of `Smoothing` affects the response time when `Phase` is changed.

Clip, Skew

:   LFO waveform parameters. Computation is done as following.

    ```
    # Range of `phase` is [0, 1).
    wave = sin(2 * π * phase^Skew).
    lfo = clamp(Clip * wave, T(-1), T(1)).
    ```

Sync.

:   When checked, it enables tempo synchronization (tempo sync). LFO phase also synchronize to the exact position derived from sync interval, and time passed from the start of playback.

    When not checked, LFO behaves as same as if synchronizing to 120 BPM. It also disables LFO phase sync.

Tempo Upper

:   Numerator of sync interval.

    LFO synchronizes to 1 bar when `(Tempo Upper) / (Tempo Lower)` is `1/1`. `1/4` synchronizes to 1 beat when time signature of `4/4`. Note that `Rate` multiplies the sync interval.

    Following is the equation to calculate sync interval.

    ```
    syncInterval = (Rate) * (Tempo Upper) / (Tempo Lower);
    ```

Tempo Lower

:   Denominator of sync interval.

    Note that `Rate` multiplies the sync interval.

Rate

:   Multiplier to sync interval.

    `Rate` is convenient when changing sync interval while keeping the values of `Tempo Upper` and `Tempo Lower`.

Smoothing \[s\]

:   Paramter smoothing time in seconds.

    For example, if the value of `Smoothing` is set to `0.01` or something short, the change of parameter almost immediately applies. So it sounds more snappy, but may introduce audible pop noise. On the other hand, if the value of `Smoothing` is set to `1.0` for example, the change of parameter is only slowly followed.

Oversampling

:   Oversampling ratio.

    - `1x`: Disables oversampling. This reduces CPU load, but increases aliasing noise. However, aliasing noise might be preferred in some cases to add a character to sound.
    - `2x`: Enables 2-fold oversampling.
    - `8x`: Enables 8-fold oversampling. CPU load becomes high, but aliasing noise will be reduced. This might be preferred when expanding spectrum.

## Change Log
{%- for version, logs in changelog["NarrowingDelay"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["NarrowingDelay"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["NarrowingDelay"] %}
- [NarrowingDelay {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
NarrowingDelay is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
