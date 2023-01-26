---
lang: ja
...

# LongPhaser
![](img/LongPhaser.png)

<ruby>LongPhaser<rt>ロング フェイザ</rt></ruby> は遅延時間を長くできる Schroeder オールパスフィルタを使ったフェイザです。ディレイとフェイザを足して 2 で割ったような音が出ます。 2 で割った物足りなさを補うために LFO やノートイベントで変調をかけることができます。

{% for target, download_url in latest_download_url["LongPhaser"].items() %}
- [LongPhaser {{ latest_version["LongPhaser"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["LongPhaser"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["LongPhaser"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/LongPhaser.svg)

## ノートイベントの入力
LongPhaser はノートイベントによってオールパスフィルタのディレイ時間を制御することができます。

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は LongPhaser のパラメータで使われている単位の一覧です。

- \[s\]: 秒 (second) 。

### Allpass
Output

:   出力ゲインです。

Mix

:   入力信号とフェイザを通過した信号を混ぜる比率です。

    左いっぱいにすると入力信号をバイパスするだけになります。右いっぱいに近づくほど `OuterFeed` が強くかかるようになります。

OuterFeed

:   直列オールパスセクションを囲む部分のフィードバックのゲインです。

InnerFeed

:   直列オールパスセクションに含まれるオールパスフィルタのフィードバックとフィードフォワードのゲインです。

Inner Mod.

:   LFO による `InnerFeed` の変調量です。

Time Spread

:   オールパスフィルタのディレイ時間の倍率を `[1, 1, 1, ...]` と `[1/1, 1/2, 1/3, ...]` の間で調整します。

    フェイザは複数のオールパスフィルタを直列に繋いだエフェクタです。 `Time Spread` が左いっぱいのときは、このオールパスフィルタのディレイ時間をすべて同じにします。 `Time Spread` が右いっぱいのときは、オールパスフィルタのディレイ時間が前から順に 1/1 倍、 1/2 倍、 1/3 倍、と設定されます。

Base Time \[s\]

:   オールパスフィルタのディレイ時間です。

LFO > Time

:   LFO によってオールパスフィルタのディレイ時間を変調する量です。

    表示されている値の単位は `Mod. Type` によって変わります。

Input > Time

:   入力信号によってオールパスフィルタのディレイ時間を変調する量です。

    負のディレイ時間を設定することを避けるため、変調に使われる信号は `abs` で全波整流されます。

    表示されている値の単位は `Mod. Type` によって変わります。

Mod. Type

:   `LFO > Time` と `Input > Time` による、ディレイ時間への変調のかけ方です。

    - `Multiply`: `exp2` を通した変調信号をディレイ時間に乗算します。 `Multiply` が選択されているとき、 `Input > Time` と `LFO > Time` の値は変調量をオクターブで表します。
    - `Add`: 変調信号をディレイ時間にそのまま加算します。 `Add` が選択されているとき、 `Input > Time` と `LFO > Time` の値は変調量を秒数で表します。

Interp. Rate

:   ディレイ時間の変動を補間するレートリミッタの 1 サンプルあたりの制限量です。例えば `Interp. Rate` が 0.1 のときは、 10 サンプル経過でディレイ時間が 0.1 * 10 = 1 サンプル変わります。

Stage

:   直列に接続するオールパスフィルタの数です。

### LFO
L-R Offset

:   ステレオチャンネル間で LFO の位相をずらす量です。

Phase

:   LFO の位相に加算される値です。

    `Rate` を左いっぱいに回して LFO の位相を止めているときに `Phase` の値を変更することで、 LFO の位相を制御することができます。また `Smoothing` の値によって `Phase` を動かしたときの応答速度を変えることができます。

Sync.

:   チェックを入れるとテンポ同期を有効にします。また同期間隔が変わったときに再生開始時点から導かれる位相へと同期します。

    チェックが外れているときは 120 BPM に同期した状態と同じになります。ただし、同期間隔が変わったときに位相を調整しなくなります。

Tempo Upper

:   テンポ同期が有効な時の同期間隔を表す分数の分子です。

    `1/1` のときに 1 小節、 4/4拍子であれば `1/4` のときに 1 拍で LFO が 1 周します。 `Rate` が乗算されて周期が変わる点に注意してください。

    以下は同期間隔の計算式です。

    ```
    syncInterval = (Rate) * (Tempo Upper) / (Tempo Lower);
    ```

Tempo Lower

:   テンポ同期が有効な時の同期間隔を表す分数の分母です。

    `Rate` が乗算されて周期が変わる点に注意してください。

Rate

:   同期間隔に乗算される係数です。

    `Tempo Upper` と `Tempo Lower` を変えずに LFO の同期間隔を変えたいときに使えます。

Wave Interp.

:   LFO の波形の補間方法です。

    - `Step`: ホールド。
    - `Linear`: 線形補間。
    - `PCHIP`: 単調な 3 次補間。

    `Step` を選ぶと LFO をシーケンサのように使えます。 `Linear` は `PCHIP` と似たような音になりますが、計算がすこし速いです。デフォルトの `PCHIP` はサンプル間をだいたい滑らかに補間します。

LFO Wave

:   LFO の波形です。

### Misc.
Note Origin

:   変調量が 1 倍となる MIDI ノート番号です。

    例えば `Note Origin` が 60 であれば、入力されたノート番号が 60 のときに音が変わらなくなります。

Note Scale

:   ノートイベントによるディレイ時間の変調量です。

    1.0 のとき、ノートのピッチに完全に追従します。 -1.0 にするとピッチの高低が逆転します。

Smoothing \[s\]

:   パラメータのスムーシング時間です。

    例えば `Smoothing` の値を `0.01` と短くするとパラメータの変更がほぼ瞬時に適用されます。ただし `Smoothing` の値を小さくするとパラメータ変更時のポップノイズが目立つようになります。逆に `Smoothing` の値を `1.0` などと長くするとパラメータの値がゆっくりと切り替わるようになります。

2x Sampling

:   チェックを入れると 2 倍のオーバーサンプリングを行います。

## チェンジログ
{%- for version, logs in changelog["LongPhaser"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### LongPhaser
{%- if old_download_link["LongPhaser"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["LongPhaser"] %}
- [LongPhaser {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
LongPhaser のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
