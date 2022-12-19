---
lang: ja
...

# UltraSynth
![](img/UltraSynth.png)

<ruby>UltraSynth<rt>ウルトラ シンセ</rt></ruby> は UltrasonicRingMod と同じく 64 倍のオーバーサンプリングを行っていることを除けば、これといった特長のないモノフォニックのシンセサイザです。オーバーサンプリングの倍率が高いので変調をかけた音が得意ですが、 CPU 負荷は高めです。

{% for target, download_url in latest_download_url["UltraSynth"].items() %}
- [UltraSynth {{ latest_version["UltraSynth"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["UltraSynth"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["UltraSynth"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/UltraSynth.svg)

## パラメータ
### Gain
Gain

:   出力ゲインです。

Rect.

:   全波整流した信号を混ぜる比率です。

Sat.

:   平方根によって歪ませた信号を混ぜる比率です。

A, D, S, R

:   出力ゲインにかかるエンベロープのパラメータです。

    - `A`: 立ち上がり時間 (Attack) 。
    - `D`: 減衰時間 (Decay) 。
    - `S`: 持続区間の音量 (Sustain) 。
    - `R`: ノートオフの後に続く減衰時間 (Release) 。

### Oscillator
UltraSynth には 2 つのオシレータが備えられており、それぞれのパラメータが X-Y パッドの縦軸と横軸に対応しています。

Pitch

:   オシレータのピッチです。

    GUI には表示されませんが、パラメータの値の単位はセント (1/100 半音) です。

Wave Shape

:   オシレータの波形です。

    `Saw-Pulse` が 0 のときは [0, 0.5, 1] がそれぞれ [のこぎり波 (上り), 三角波, のこぎり波 (下り)] と対応します。`Saw-Pulse` が 1 のときはパラメータの値が矩形波のデューティ比を表します。

Saw-Pulse

:   オシレータの波形を、のこぎり波と矩形波の間で混ぜる比率です。

    `Saw-Pulse` が 0 のときはのこぎり波、 1 のときは矩形波になります。

X Oct., Y Oct.

:   オクターブ単位のオシレータのピッチです。

Mix

:   2 つのオシレータを混ぜる比率です。

    `Mix` が 0 のときはオシレータ X 、 1 のときはオシレータ Y からの出力だけが聞こえるようになります。

LP\>Osc.X

:   フィルタ出力によってオシレータ X を変調する量です。

Cross PM

:   オシレータを互い違いに変調する量です。

    横軸は Y から X 、縦軸は X から Y への変調量です。

Feedback PM

:   オシレータの自己変調の量です。

    横軸は X から X 、縦軸は Y から Y への変調量です。

### Filter
ローパスフィルタの設定です。

Cut

:   カットオフ周波数です。

Q

:   Q 値あるいはレゾナンスです。

A, D

:   カットオフ周波数にかかるエンベロープのパラメータです。

    - `A`: 立ち上がり時間 (Attack) 。
    - `D`: 減衰時間 (Decay) 。

Env\>Cut

:   エンベロープのかかり具合です。

Key

:   与えられたノートに応じてカットオフ周波数を調整する割合です。

    左いっぱいにするとノートのピッチによらず常に一定、右いっぱいにするとノートのピッチに完全に追従するようになります。

### Tuning
Semitone, Cent

:   全体の音の高さを変更します。

    - `Semitone`: 半音
    - `Cent`: 1 半音の 1 / 100 。

Equal Temp., A4 \[Hz\]

:   音律を変更します。

    `Equal Temp.` は Equal Temperament (平均律) の略です。 `Equal Temp.` が 12 のときは 12 平均律となります。 `Equal Temp.` を 12 よりも小さくすると、内部的なピッチが高すぎるか低すぎるために、音が出なくなる、あるいは変わらなくなる範囲が増えるので注意してください。

    `A4 [Hz]` は、音程 A4 の周波数です。

P.Bend Range \[st.\]

:   ピッチベンドの範囲です。

### LFO
Rate

:   LFO の周期です。

    `Sync.` が無効の時は周波数、有効の時は同期間隔への係数として使われます。例えば `Rate` が 2 のとき、 `Sync.` が無効なら LFO は 2 Hz 、有効なら同期間隔を 1/2 倍に短縮します。

Shape

:   LFO のサイン波を歪ませる量です。

    右いっぱいにすると矩形波に近い波形になります。

Retrigger

:   有効にするとノートオンごとに位相を 0 にリセットします。

Sync.

:   有効にするとテンポに同期します。

Sync. Upper, Sync. Lower

:   `Sync.` の右にある 2 つの縦に重なった数字のことで、 LFO の同期間隔を拍で表しています。

    `Sync. Upper` と `Sync. Lower` は拍を分数であらわしたときの分子と分母であって、拍子記号ではないことに注意してください。例えば 3/4 拍子のときに `Sync. Upper` と `Sync. Lower` がどちらも 1 であれば、 LFO の同期間隔は 1 拍なので、 1 小節の間に LFO は 3 周します。

    以下は同期間隔の計算式です。

    ```
    syncIntervalInBeats = (Sync. Upper) / (Sync. Lower).
    ```

\>Pitch

:   オシレータのピッチへの LFO による変調量です。

\>OscMix

:   `Oscillator -> Mix` への LFO による変調量です。

\>Cutoff

:   フィルタのカットオフ周波数への LFO による変調量です。

\>Pre Sat.

:   `Sat.` による歪みが加えられる前の信号の振幅への、 LFO による変調量です。

\>X Shape, \>Y Shape

:   `Wave Shape` への LFO による変調量です。

### Misc.
Phase Reset

:   有効にすると、ゲイン・エンベロープが休止状態のときにノートオンが与えられたときにオシレータの位相をリセットします。

    ゲイン・エンベロープはリリース中の値が十分に小さくなった時に休止状態へと移行します。つまり、リリースの音がまだ聞こえているときにノートオンを与えると、休止状態ではないのでオシレータの位相はリセットされません。また、リリース時間を長く設定しているときは人間の耳で音が止まっているかどうかを判断することは難しいことに注意してください。

Slide

:   最後に与えられたノートのピッチへとスライドする時間です。

## チェンジログ
{%- for version, logs in changelog["UltraSynth"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["UltraSynth"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["UltraSynth"] %}
- [UltraSynth {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
UltraSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
