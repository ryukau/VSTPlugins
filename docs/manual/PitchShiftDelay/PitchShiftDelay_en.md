---
lang: en
...

# PitchShiftDelay
![](img/pitchshiftdelay.png)

PitchShiftDelay is a time domain pitch shifter based on usual delay. The quirk of pitch shifter is somewhat reduced by 16x oversampling. Also the internal buffer length can be changed in real time. PitchShiftDelay doesn't equipped with formant correction. So applying to voice may sounds strange, like Helium is inhaled.

- [Download PitchShiftDelay {{ latest_version["PitchShiftDelay"] }} - VST® 3 (github.com)]({{ latest_download_url["PitchShiftDelay"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["PitchShiftDelay"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["PitchShiftDelay"] }})
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

{{ section["gui_barbox"] }}

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/pitchshiftdelay.svg)

## Parameters
### Delay
Pitch

:   Main pitch shift amount.

    Main shifter multiplies pitch by the value of `Pitch` + LFO amount.

    Because of the 16 fold oversampling, aliasing noise don't affect the output when `Pitch` is below `31` which is equal to `16 * 2 - 1`.

Offset

:   Unison shifter pitch offset from `Pitch`.

    Unison shifter pitch is calculated by following equation:

    ```
    unisonPitch = mainPitch;
    if (Mirror) unisonPitch = max(1 / unisonPitch, 1000);
    if (S2 Reverse) unisonPitch = -unisonPitch;
    unisonPitch += ((To Pitch) + (To Unison)) * lfo;
    ```

Pitch Cross

:   Cross feedback amount between main shifter and unison shifter.

Mirror

:   When checked, it changes unison shifter pitch to `1 / Pitch`.

    Checking `Mirror` after initial state results in that the feedback pitch will always be `Pitch` or `1 / Pitch`. This state will be kept until one of `Offset`, `Pitch Cross`, `To Pitch`, or `To Unison` is changed from default.

S1 Reverse

:   Reverse the playback of main shifter.

    Internally, `S1 Reverse` changes the sign of `Pitch` to negative to moving the read pointer to reverse direction.

S2 Reverse

:   Reverse the playback of unison shifter.

L-R Lean

:   The ratio of `Delay Time` between left and right channels. `L-R Lean` also works when `Channel Type` is `M-S`.

Delay Time \[s\]

:   Buffer length of pitch shifter. Note that the value only accurate when `Pitch` is set to 1.

    The exact delay time depends on the pitch shift amount and buffer length. LFO farther complicates the computation of exact delay time. So the value of `Delay Time` is usually a rough indication rather than exact delay time.

Feedback

:   Feedback gain in amplitude.

Stereo Cross

:   Cross feedback amount between stereo channels. When the value is set to `1.0`, output becomes ping-pong delay.

Channel Type

:   Select input channel type from left-right (`L-R`) or mid-side (`M-S`).

Highpass \[Hz\]

:   Cutoff frequency of highpass filter inserted on feedback path.

### Mix
Dry \[dB\]

:   Input bypass gain.

Wet \[dB\]

:   Output gain of the signal that go through PitchShiftDelay.

Unison Mix

:   Mixing ratio of main shifter and unison shifter.

Smoothing \[s\]

:   Paramter smoothing time in seconds.

    For example, if the value of `Smoothing` is set to `0.01` or something short, the change of parameter almost immediately applies. So it sounds more snappy, but may introduce audible pop noise.

    On the other hand, if thevalue of `Smoothing is set to `1.0` for example, the change of parameter is only slowly followed. So when changing `Pitch`, it may sounds like long slide/portamento.

### LFO
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

Stereo Offset

:   LFO phase offset between stereo channels.

Unison Offset

:   LFO phase offset between main shifter and unison shifter.

Wave Interp.

:   LFO wave form interpolation method.

    - `Step`: Holding.
    - `Linear`: Linear interpolation.
    - `PCHIP`: Monotonic cubic interpolation.

    `Step` makes LFO to act like a sequencer. `Linear` sounds similar to `PCHIP`, and the computation cost is lower. `PCHIP` is the most smooth among all 3.

To Pitch

:   LFO modulation amount to main pitch.

To Unison

:   LFO modulation amount to unison shifter pitch that is added to `To Pitch` amount.

## Change Log
{%- for version, logs in changelog["PitchShiftDelay"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["PitchShiftDelay"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["PitchShiftDelay"] %}
- [PitchShiftDelay {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
PitchShiftDelay is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
