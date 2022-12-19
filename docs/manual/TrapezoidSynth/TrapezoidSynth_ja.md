---
lang: ja
...

# TrapezoidSynth
![](img/TrapezoidSynth.png)

<ruby>TrapezoidSynth<rt>トラピゾイド シンセ</rt></ruby> は台形オシレータを使ったモノフォニックシンセサイザです。台形オシレータは PTR (Polynomial Transition Regions) という手法に基づいて作ったのですが、ピッチが高くなると逆にノイズが増える欠点があるので 8 倍のオーバーサンプリングをしています。コードを作るために 2 つの AM ピッチシフタを搭載しています。

{% for target, download_url in latest_download_url["TrapezoidSynth"].items() %}
- [TrapezoidSynth {{ latest_version["TrapezoidSynth"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["TrapezoidSynth"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["TrapezoidSynth"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
`SlideType` を `Reset to 0` にすると極端に低い周波数が出るので、ハイパスフィルタをかけることを推奨します。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/TrapezoidSynth.svg)

## パラメータ
### Oscillator 共通
Semi

:   ピッチを半音単位で変更します。

Cent

:   ピッチをセント単位で変更します。 100 セント = 1 半音です。

Slope

:   台形の両端の傾きです。

PW

:   台形の上辺の長さです。 Pulse Width の略です。

### Oscillator 1
Drift

:   ホワイトノイズでピッチを変調します。

Feedback

:   Oscillator 1 の位相をフィードバックで変調します。

    ```
    osc1Phase += Feedback * (osc1 + OscMix * (osc2 - osc1))
    ```

### Oscillator 2
Overtone

:   周波数を `Overtone` 倍にします。

PM

:   Oscillator 1 の位相を Oscillator 2 の出力で変調します。

### Envelope 共通
Retrigger

:   チェックを入れると鍵盤が押されるたびにエンベロープをリセットします。チェックが外れているときは、同時に押している鍵盤の数が 0 から 1 以上になったときだけエンベロープをリセットします。

A

:   アタック時間の秒数です。アタックは音が最大音量に到達するまでのエンベロープの区間です。

D

:   ディケイ時間の秒数です。ディケイはアタックが終わった後にエンベロープが減衰する区間です。

S

:   サステインの大きさです。サステインはディケイが終わったのエンベロープの出力の大きさです。

R

:   リリース時間の秒数です。エンベロープは、鍵盤から指を離した時点（ノートオフ）からリリース状態に移行して、出力が 0 になるまで減衰します。

Curve

:   エンベロープの特性を変更します。

### Gain Envelope
Gain

:   出力音量です。

### Filter
Order

:   フィルタの次数です。 TrapezoidSynth では 1 次フィルタを 8 個直列につないでいます。 `Order` の値によってフィルタから信号を取り出す位置を変更しています。

Cut

:   カットオフ周波数です。

Res

:   レゾナンスの強さです。

Sat

:   フィルタのサチュレーションの強さです。

    ```
    filterInput = tanh(Sat * (input + Res * filterOutput))
    ```

Env>Cut

:   Filter Envelope による `Cut` の変調量です。

Key>Cut

:   鍵盤の高さによる `Cut` の変調量です。

+OscMix

:   Oscillator 1 と Oscillator 2 をミックスした信号による `Cut` の変調量です。

### Filter Envelope
\>Octave

:   Filter Envelope の出力を使ってピッチをオクターブ単位で変調する量です。

### Misc
OscMix

:   Osillator 1 と Oscillator 2 の比率です。左いっぱいに回すと Oscillator 1 の音だけになります。

Octave

:   ピッチをオクターブ単位で変更します。

Smooth

:   パラメータを変更したときに変更前の値から変更後の値に移行する秒数です。次のパラメータに影響します。

    - `Drift`
    - `Slope`
    - `PW` （同名のパラメータすべて）
    - `Feedback` （同名のパラメータすべて）
    - `PM`
    - `Gain` （同名のパラメータすべて）
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

### Mod 共通
Retrigger

:   チェックを入れると鍵盤が押されるたびにエンベロープをリセットします。チェックが外れているときは、同時に押している鍵盤の数が 0 から 1 以上になったときだけエンベロープをリセットします。

Attack

:   モジュレーションエンベロープのアタック時間の秒数です。

Curve

:   モジュレーションエンベロープの特性です。

### Mod 1
\>PM

:   `Oscillator2.PM` をモジュレーションエンベロープ 1 で変調します。

### Mod 2
\>Feedback

:   `Oscillator1.Feedback` をモジュレーションエンベロープ 2 で変調します。

\>LFO

:   `LFO.Freq` をモジュレーションエンベロープ 2 で変調します。

\>Slope2

:   `Oscillator2.Slope` をモジュレーションエンベロープ 2 で変調します。

\>Shifter1

:   Shifter 1 のピッチをモジュレーションエンベロープ 2 で変調します。

### Shifter 共通
Semi

:   半音単位でピッチシフトの量を変更します。

Cent

:   セント単位でピッチシフトの量を変更します。

Gain

:   ピッチシフトした信号を足し合わせる大きさです。

### LFO
LFOType

:   LFO の波形の種類を次の 4 つから選択できます。

    - `Sin` : サイン波です。 出力は `Shape` 乗されます。
    - `Saw` : 鋸歯波です。 `Shape` によって三角波にもできます。
    - `Pulse` : 矩形波です。 `Shape` によってデューティ比を変更できます。
    - `Noise` : ブラウンノイズです。 `Shape` によってステップあたりの移動量を変更できます。

Tempo

:   LFO の周波数をテンポシンクします。

Freq

:   LFO の周波数です。

Shape

:   LFO の波形を変更します。

\>Pitch

:   LFO による Oscillator 1 のピッチの変調量です。

\>Slope1

:   LFO による `Oscillator1.Slope` の変調量です。

\>PW1

:   LFO による `Oscillator1.PW` の変調量です。

\>PW1

:   LFO による `Filter.Cut` の変調量です。

### Slide
SlideType

:   ピッチスライドの種類を変更できます。

    - `Always` : 常に最後に演奏されたノートのピッチからスライドします。
    - `Sustain` : 2 つ以上の鍵盤が同時に押されたときだけスライドします。
    - `Reset to 0` : 同時に押している鍵盤の数が 0 から 1 、あるいは 1 から 0 になるときは 0 Hz にスライドします。それ以外の場合は他の 2 つと同じようにスライドします。

Time

:   ピッチスライドにかかる時間です。

Offset

:   Oscillator 1 のスライド時間に対する Oscillator 2 のスライド時間の比率です。

## チェンジログ
{%- for version, logs in changelog["TrapezoidSynth"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["TrapezoidSynth"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["TrapezoidSynth"] %}
- [TrapezoidSynth {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
TrapezoidSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
