---
lang: en
...

# TrapezoidSynth
![](img/TrapezoidSynth.png)

TrapezoidSynth is a monophonic synthesizer equipped with 2 trapezoid oscillator. Trapezoid oscillator is based on PTR (Polynomial Transition Regions) and it has a drawback that increasing noise at high frequency. Therefore 8 times oversampled. 2 pitch shifters are added to make some chord.

{% for target, download_url in latest_download_url["TrapezoidSynth"].items() %}
- [Download TrapezoidSynth {{ latest_version["TrapezoidSynth"] }} `{{ target }}` - VST 3 (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["TrapezoidSynth"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["TrapezoidSynth"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## Controls
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## Caution
It is recommended to insert high-pass when setting `SlideType` to `Reset to 0`.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/TrapezoidSynth.svg)

## Parameters
### Oscillator Common
Semi

:   Change oscillator pitch in semitone.

Cent

:   Change oscillator pitch in cent. 100 cent = 1 semitone.

Slope

:   Change left and right side of slope of trapezoid.

PW

:   Pulse width. Change length of upper side of trapezoid.

### Oscillator 1
Drift

:   Modulate pitch with white noise.

Feedback

:   Modulate phase of Oscillator 1 with feedback signal.

    ```
    osc1Phase += Feedback * (osc1 + OscMix * (osc2 - osc1))
    ```

### Oscillator 2
Overtone

:   Multiply the frequency of oscillator by number of `Overtone`.

PM

:   Modulate Osillator 1 phase with Oscillator 2 output.

### Envelope Common
Retrigger

:   When checked, reset envelope for each note on. When not checked, reset envelope only if the number of pressed key is changed from 0 to 1 or more.

A

:   Attack time in seconds.

D

:   Decay time in seconds.

S

:   Sustain level.

R

:   Release time in seconds.

Curve

:   Change envelope curve.

### Gain Envelope
Gain

:   Master output gain.

### Filter
Order

:   TrapezoidSynth is using 8 of serially connected first order filter. `Order` changes where to get the signal from the filter.

Cut

:   Cutoff frequency in Hz.

Res

:   Resonance.

Sat

:   Filter saturation.

    ```
    filterInput = tanh(Sat * (input + Res * filterOutput))
    ```

Env>Cut

:   Modulation amount from Filter Envelope to `Cut`.

Key>Cut

:   Modulation amount from note pitch to `Cut`.

+OscMix

:   Modulation amount from mixed signal of Oscillator 1 and Oscillator 2 to `Cut`.

### Filter Envelope
\>Octave

:   Modulation amount from Filter Envelope to pitch in octave.

### Misc
OscMix

:   Ratio of Oscillator 1 and Oscillator 2. When turned to left-most, output becomes only the signal from Oscillator 1.

Octave

:   Change pitch in octave.

Smooth

:   Interpolation time in seconds from previous value to current value. Following parameters are affected by `Smooth`.

    - `Drift`
    - `Slope`
    - `PW` (for all parameter with identical name)
    - `Feedback` (for all parameter with identical name)
    - `PM`
    - `Gain` (for all parameter with identical name)
    - `Cut`
    - `Res`
    - `Sat`
    - `Env>Cut`
    - `Key>Cut`
    - `+OscMix`
    - `OscMix`
    - `>PM`
    - `>Feedback`
    - `>LFO`
    - `>Slope2`
    - `>Shifter1`
    - `Shifter1.Semi`
    - `Shifter1.Cent`
    - `Shifter2.Semi`
    - `Shifter2.Cent`
    - `Freq`
    - `Shape`
    - `>Pitch1`
    - `>Slope1`
    - `>PW1`
    - `>Cut`

### Mod Common
Retrigger

:   When checked, reset envelope for each note on. When not checked, reset envelope only if the number of pressed key is changed from 0 to 1 or more.

Attack

:   Modulation envelope attack time in seconds.

Curve

:   Modulation envelope curve.

### Mod 1
\>PM

:   Modulation amount from modulation envelope 1 to `Oscillator2.PM`.

### Mod 2
\>Feedback

:   Modulation amount from modulation envelope 2 to `Oscillator1.Feedback`.

\>LFO

:   Modulation amount from modulation envelope 2 to `LFO.Freq`.

\>Slope2

:   Modulation amount from modulation envelope 2 to `Oscillator2.Slope`.

\>Shifter1

:   Modulation amount from modulation envelope 2 to Shifter 1 pitch.

### Shifter Common
Semi

:   Change amount of pitch shift in semitone.

Cent

:   Change amount of pitch shift pitch in cent. 100 cent = 1 semitone.

Gain

:   Output gain of pitch shifter.

### LFO
LFOType

:   4 waveforms are available for LFO.

    - `Sin` : Sine wave. Output will be power of the value of `Shape`.
    - `Saw` : Sawtooth wave. Can be turned into triangle with `Shape`.
    - `Pulse` : Pulse wave. Duty ratio can be controlled by `Shape`.
    - `Noise` : Brown noise. Wandering of noise can be changed by `Shape`.

Tempo

:   Synchronize LFO frequency to tempo.

Freq

:   LFO frequency.

Shape

:   Change LFO wave shape.

\>Pitch

:   Modulation amount from LFO to Oscillator 1 pitch.

\>Slope1

:   Modulation amount from LFO to `Oscillator1.Slope`.

\>PW1

:   Modulation amount from LFO to `Oscillator1.PW`.

\>PW1

:   Modulation amount from LFO to `Filter.Cut`.

### Slide
SlideType

:   3 type of pitch slide is available.

    - `Always` : Always slide from the pitch of last note.
    - `Sustain` : Only slide when 2 or more keys are pressed.
    - `Reset to 0` : When number of pressed key is changed from 0 to 1, or 1 to 0, pitch will be slide to 0 Hz. Otherwise, slide behaves same as other 2 options.

Time

:   Time of pitch sliding.

Offset

:   Oscillator 2 slide time with respect to Oscillator 1 slide time.

## Change Log
{%- for version, logs in changelog["TrapezoidSynth"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["TrapezoidSynth"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["TrapezoidSynth"] %}
- [TrapezoidSynth {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
TrapezoidSynth is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
