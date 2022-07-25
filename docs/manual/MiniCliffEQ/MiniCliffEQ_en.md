---
lang: en
...

# MiniCliffEQ
![](img/minicliffeq.png)

MiniCliffEQ is a linear phase FIR filter with 2^15 = 32768 taps. Latency is improved from initial version, however it still exceeds 0.34 seconds in 48000 Hz sampling rate. The primary purpose is to suppress direct current. It can also be used as very sharp low-pass, high-pass, low-shelf, and high-shelf filter.

- [Download MiniCliffEQ {{ latest_version["MiniCliffEQ"] }} - VST® 3 (github.com)]({{ latest_download_url["MiniCliffEQ"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["MiniCliffEQ"]|length != 0%}
- [Download Presets (github.com)]({{ preset_download_url["MiniCliffEQ"] }})
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
Latency is `2^15 / 2 - 1 = 16383` samples. Signal to noise ratio is around -120 dB.

## Usage
Primary usage of MiniCliffEQ is direct current (DC) suppression. It can also be used for detecting subtle noises. The filter is linear phase, so band-splitting is another application. If possible, it is always better to replace MiniCliffEQ for more lightweight EQ, because the high latency degrades your workflow.

To suppress DC, click `LP Gain` twice by mouse wheel. It sets the value to `-inf`. For ordinary speakers, it is better to stay `Cutoff` at initial value of 20 Hz. When `Cutoff` is set to below 10 Hz, the accuracy of cutoff frequency and steepness of roll-off will degrade. For example, when `Cutoff` is set to 1 Hz, amplitude response is -16.25 dB at 1 Hz, and -60 dB at 0.1 Hz.

To detect subtle noise, follow the steps below:

1. Insert limiter after MiniCliffEQ.
2. Set `LP Gain` to `-inf`.
3. Rise `Cutoff` to eliminate input signal. Don't forget to `Refresh FIR`!
4. Set `HP Gain` to `144.5`.

For example, consider to set `Cutoff` to 200 Hz, and input 100 Hz sine wave to the above setting. If the output is not silent, then the input sine wave contains some noise. Note that the noise that can be heard when applying 144.5 dB gain is definitely not perceived by human ear. This procedure is useful to investigate plugin algorithm, rather than to measure the quality of sound.

The filter design algorithm used in MiniCliffEQ is the same one as following Python 3 code:

```python
import scipy.signal as signal
samplerate = 48000
cutoffHz = 20
fir = signal.firwin(2**15 - 1, cutoffHz, window="nuttall", fs=samplerate)
```

## Parameters
Refresh FIR

:   Click `Refresh FIR` to refresh filter coefficients after changing `Cutoff`. Note that clicking `Refresh FIR` resets internal states, and stops sound for the length of latency.

Cutoff \[Hz\]

:   FIR filter cutoff frequency.

HP Gain \[dB\]

:   Gain of highpass output.

LP Gain \[dB\]

:   Gain of lowpass output.

## Change Log
{%- for version, logs in changelog["MiniCliffEQ"].items() %}
- {{version}}
  {%- for log in logs["en"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## Old Versions
{%- if old_download_link["MiniCliffEQ"]|length == 0 %}
N/A.
{%- else %}
  {%- for x in old_download_link["MiniCliffEQ"] %}
- [MiniCliffEQ {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## License
MiniCliffEQ is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
