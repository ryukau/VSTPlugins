---
lang: en
...

# ParallelDetune
![](img/ParallelDetune.png)

ParallelDetune is a detuning effect with 8 pitch shifters connected in parallel. It can be used to spread stereo image similar to chorus effect.

{% for target, download_url in latest_download_url["ParallelDetune"].items() %}
- [Download ParallelDetune {{ latest_version["ParallelDetune"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["ParallelDetune"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["ParallelDetune"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/ParallelDetune.svg)

## Note Event Input
ParallelDetune can receive note events to control pitch shift amount.

## Parameters
Characters inside of square brackets \[\] represents unit. Following is a list of units used in ParallelDetune.

- \[s\]: second.
- \[oct.\]: octave.
- \[st.\]: semitone.
- \[cent\]: 1/100 of a semitone.

### Shifter
Dry

:   Gain for bypassing input signal.

Wet

:   Gain for output signal comes out from ParallelDetune.

Feedback

:   Pitch shifter feedback gain.

HP, LP

:   Cutoff frequency of highpass (`HP`) or lowpass (`LP`) filter on feedback path.

Delay Time \[s\]

:   Buffer length of pitch shifter. The value is not exact, but a rough indication.

    When the value of `Shift \[oct.\]` is not 0.0, delay time is doesn't match the displayed value because of the change of buffer playback speed.

    LFO can be applied by turning the knob on the right side of number slider.

Transpose \[st.\]

:   Pitch shift amount used for all the pitch shifters.

    This might be convenient when changing all the pitch shift at once.

    LFO can be applied by turning the knob on the right side of number slider.

Pan. Spread

:   Amount of stereo spread.

    When `Pan. Spread` is 0.0, left and right channels are processed without affecting other. When `Pan. Spread` is 1.0, the output of the pitch shifters with same index on left and right channel are summed, then stereo panning is applied from left to right in order from lowest index to highest.

    When LFO is applied, it rotates the pan. Note that waveform of LFO is fixed to sine wave for `Pan. Spread`.

    Tremolo can be used in combination of `Pan. Spread` to add character to stereo spread.

Tremolo Mix, Tremolo Lean

:   Tremolo parameters.

    Tremolo is disabled when `Tremolo Lean` is set to 12 o'clock, or `Tremolo Mix` is set to leftmost.

    Tremolo is fully applied when `Tremolo Lean` is set to leftmost or rightmost, and `Tremolo Mix` is set to rightmost.

    When `Pan. Spread` is not 0.0, stereo image leans left or right depending on the value of `Tremolo Lean`.

Pitch Shift \[st.\], Fine Tuning \[cent\]

:   Amount of pitch shift.

    `Pitch Shift` is suitable to change large amount of pitch, and `Fine Tuning` is suitable to change small amount. The range of `Pitch Shift` is 1 octave, and the range of `Fine Tuning` is 10 cents.

Gain

:   Gain for each pitch shifter.

Time Multiplier

:   Buffer length for each pitch shifter. The value is relative to `Delay Time`.

HP Offset \[oct.\], LP Offset \[oct.\]

:   Highpass or lowpass filter cutoff frequency for each pitch shifter. The value is relative to the value of `HP` or `LP`.

### LFO
L-R Offset

:   LFO phase offset between stereo channels.

Phase

:   A value that is added to LFO phase.

    To manually control LFO phase, set `Rate` to leftmost to stop LFO, then change the value of `Phase`. Also, the value of `Smoothing` affects the response time when `Phase` is changed.

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

Wave Interp.

:   LFO wave form interpolation method.

    - `Step`: Holding.
    - `Linear`: Linear interpolation.
    - `PCHIP`: Monotonic cubic interpolation.

    `Step` makes LFO to act like a sequencer. `Linear` sounds similar to `PCHIP`, and the computation cost is lower. `PCHIP` is the most smooth among all 3.

LFO Wave

:   LFO waveform.

Smoothing \[s\]

:   Paramter smoothing time in seconds.

    For example, if the value of `Smoothing` is set to `0.01` or something short, the change of parameter almost immediately applies. So it sounds more snappy, but may introduce audible pop noise. On the other hand, if the value of `Smoothing` is set to `1.0` for example, the change of parameter is only slowly followed.

## Change Log
{%- for version, logs in changelog["ParallelDetune"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["ParallelDetune"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["ParallelDetune"] %}
- [ParallelDetune {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
ParallelDetune is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
