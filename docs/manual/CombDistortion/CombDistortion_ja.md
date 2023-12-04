---
lang: ja
...

# CombDistortion
![](img/CombDistortion.png)

<ruby>CombDistortion<rt>コム ディストーション</rt></ruby> は自己変調するコムフィルタを使ったディストーションです。そのままの出力はざらざらと耳障りなので、キャビネットのインパルス応答と併せて使うことを想定しています。ノートイベントでコムフィルタのディレイ時間を制御できます。

{% for target, download_url in latest_download_url["CombDistortion"].items() %}
- [CombDistortion {{ latest_version["CombDistortion"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["CombDistortion"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["CombDistortion"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/CombDistortion.svg)

## ノートイベントの入力
CombDistortion はノートイベントによってフィードバックのディレイ時間を制御することができます。入力信号と同じノートイベントを使えば、どことなく音程にあった歪みがつけられます。

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は CombDistortion のパラメータで使われている単位の一覧です。

- \[s\]: 秒 (second) 。
- \[st.\]: 半音 (semitone) 。
- \[Hz\]: 周波数 (Herz) 。

### Comb
Output

:   出力ゲインです。

Mix

:   入力信号と CombDistortion を通過した信号を混ぜる比率です。

    左いっぱいにすると入力信号をバイパスするだけになります。右いっぱいに近づくほどフィードバックが強くかかるようになります。

Feedback

:   フィードバックのゲインです。

    `Mix` が左いっぱいのときは `Feedback` によって音が変わらなくなるので注意してください。

Delay

:   フィードバック信号のディレイ時間です。

AM Mix, AM Gain, AM Invert

:   入力信号によってフィードバック信号に振幅変調をかける量です。

    - `AM Mix`: 変調前の信号と変調後の信号を混ぜる比率。
    - `AM Gain`: 変調波のゲイン。
    - `AM Invert`: チェックを入れると変調波の位相を反転。

    発散を防ぐため、 AM に使われる信号は `tanh` によって値の範囲が制限されます。つまり、入力信号の振幅に応じて変調のかかり方が非線形に変化します。ただし `AM Gain` を大きくすると振幅による違いはほとんど無くなります。

FM Mix, FM Amount, FM Clip

:   入力信号によってフィードバック経路のディレイ時間を変調する量です。

    - `FM Mix`: 変調前の信号と変調後の信号を混ぜる比率。
    - `FM Gain`: 変調波のゲイン。
    - `FM Clip`: 変調波にかけるハードクリッピングの上限の値。

    負のディレイ時間を設定することを避けるため、 FM に使われる信号は `abs` で全波整流されます。

Highpass \[Hz\], Lowpass \[Hz\]

:   フィードバック経路のハイパス (`Highpass`) あるいはローパス (`Lowpass`) フィルタのカットオフ周波数です。

### Note
Origin \[st.\]

:   変調量が 1 倍となる MIDI ノート番号です。

    例えば `Note Origin` が 60 であれば、入力されたノート番号が 60 のときに音が変わらなくなります。

Scaling

:   ノートイベントによるディレイ時間の変調量です。

    1.0 のとき、ノートのピッチに完全に追従します。 -1.0 にするとピッチの高低が逆転します。

Release \[s\]

:   ノートイベントによって変調されたディレイ時間が、ノートオフの時点から `Delay` の値に戻るまでにかかる大まかな時間です。

### Misc.
Smoothing \[s\]

:   パラメータのスムーシング時間です。

    例えば `Smoothing` の値を `0.01` と短くするとパラメータの変更がほぼ瞬時に適用されます。ただし `Smoothing` の値を小さくするとパラメータ変更時のポップノイズが目立つようになります。逆に `Smoothing` の値を `1.0` などと長くするとパラメータの値がゆっくりと切り替わるようになります。

Oversampling

:   オーバーサンプリングの設定です。

    - `1x`: オーバーサンプリングを行いません。
    - `16x Halfway`: 不十分なアップサンプリングによる癖のついた 16 倍のオーバーサンプリングを行います。
    - `16x`: 16 倍のオーバーサンプリングを行います。

## チェンジログ
{%- for version, logs in changelog["CombDistortion"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### CombDistortion
{%- if old_download_link["CombDistortion"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["CombDistortion"] %}
- [CombDistortion {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
CombDistortion のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
