# SevenDelay

![](img/sevendelay.png)

SevenDelay is a stereo delay with order 7 lagrange interpolated fractional delay and 7x oversampling.

## Font
SevenDelay uses DejaVu fonts. You can download DejaVu fonts from their official web page.

- [DejaVu Fonts](https://dejavu-fonts.github.io/)

## Caution
When the value of `smooth` is near zero and change the value of `tone` too fast, SevenDelay may output very loud click noise. This is caused by the filter which becomes unstable when cutoff frequency moves too fast. Recommendation is not to set `smooth` parameter too low unless you have some specific purpose.

## Parameters
### Delay
#### Time
Delay time. Range is 0.0001 to 8.0.

- If `sync` is on and `time` is less than 1.0, then delay time is set to `time / 16` beats.
- If `sync` is on and `time` is greater or equal to 1.0, then delay time is set to `floor(2 * time) / 32` beats.
- Otherwise, delay time is set to `time` seconds.

#### Feedback
Delay feedback. Range is 0.0 to 1.0.

#### Stereo
L/R stereo offset. Range is -1.0 to 1.0.

- If `stereo` is less than 0.0, then left channel delay time is modified to `timeL * (1.0 + stereo)`.
- Otherwise, right channel delay time is modified to `timeR * (1.0 + stereo)`.

#### Wet
Output volume of delayed signal. Range is 0.0 to 1.0.

#### Dry
Output volume of input signal. Range is 0.0 to 1.0.

#### Sync
Toggle tempo sync.

#### Negative
Toggle negative feedback. This may be useful when delay time is very short.

#### InS/InP and OutS/OutP
Input spread, input pan, output spread and output pan. Range is 0.0 to 1.0.

`*S` controls stereo spread. `*P` controls stereo panning. (`*` means wildcard. In this case, `In` or `Out`.)

These parameter can be used for inverse panning, ping-pong delay etc.

- For inverse panning, set `[InS, InP, OutS, OutP]` to `[0.0, 0.5, 1.0, 0.5]`.
- For ping-pong delay, set `[InS, InP, OutS, OutP]` to `[1.0, 0.5, 0.0, 0.5]`.

```
panL = clamp(2 * pan + spread - 1.0, 0.0, 1.0)
panR = clamp(2 * pan - spread, 0.0, 1.0)

signalL = incomingL + panL * (incomingR - incomingL)
signalR = incomingL + panR * (incomingR - incomingL)
```

#### Tone
SFV allpass filter frequency. Range is 90.0 to 20000.0.

If `tone` is 20000.0, filter will be bypassed.

### LFO
#### Amount
LFO amount. Range is 0.0 to 1.0.

#### Freq
LFO frequency. Range is 0.01 to 100.0.

#### Shape
LFO waveform shape. Range is 0.01 to 10.0.

```
sign = 1 if (phase > π),
      -1 if (phase < π),
       0 if (phase == π)
lfo = sign * abs(sin(phase))^shape
```

#### Phase
LFO initial phase. Range is 0.0 to 2π.

LFO phase is reset to `phase` for each time host starts playing.

#### Hold
Toggle LFO phase hold. This may be useful for live performance.

### Misc.
#### Smooth
Parameter smoothing. Range is 0.0 to 1.0. Unit is seconds.

## License
SevenDelay is licensed under GPLv3 that means you can freely obtain source code and can be use it for any purpose. Huge thanks for Steinberg and people who involved for developing/maintaining the VST3 SDK and open it under GPLv3.

## VST compatible logo
This section is to fullfill the obligation to display the VST compatible logo.

<img src="img/VST_Compatible_Logo_Steinberg_with_TM.svg" alt="The VST compatible logo. VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries." width="240">
