---
lang: ja
...

# IterativeSinCluster
![](img/IterativeSinCluster.png)

<ruby>IterativeSinCluster<rt>イテレイティブ サイン クラスタ</rt></ruby>はノート 1 つあたり 512 のサイン波を計算する加算合成シンセサイザです。一体、何を考えていたのか iterative sin という言葉を名前に使っていますが、アルゴリズムの種類を表す正しい言葉は recursive sine です。

{% for target, download_url in latest_download_url["IterativeSinCluster"].items() %}
- [IterativeSinCluster {{ latest_version["IterativeSinCluster"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["IterativeSinCluster"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["IterativeSinCluster"] }})
{%- endif %}

x86_64 環境で IterativeSinCluster を使うには AVX 以降の SIMD 命令セットをサポートする CPU が必要です。

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/IterativeSinCluster.svg)

## パラメータ
### Gain
Boost, Gain

:   どちらも音量を調整します。出力のピーク値は `Boost * Gain` となります。

    `Note` 、 `Chord` 、 `Overtone` のゲインを調整すると音がかなり小さくなることがあります。そんなときに `Boost` が使えます。

ADSR

:   音量エンベロープのパラメータです。

    - `A` (Attack) : 鍵盤を押した瞬間から最大音量になるまでのアタック時間。
    - `D` (Decay) : 最大音量からサステイン音量になるまでのディケイ時間。
    - `S` (Sustain) : アタックとディケイが終わった後に鍵盤を押し続けているときのサステイン音量。
    - `R` (Release) : 鍵盤から指を離したあとに音量が 0 になるまでのリリース時間。

Curve

:   音量エンベロープのアタックの曲線を変更するパラメータ。

### Shelving
`Low` と `High` の 2 つのシェルビングができます。

カットオフ周波数はノートの音程を基準として、 `Semi` で指定した半音単位で相対的に決まります。例えば音程が C4 、 `Semi` が 12.000 のときは カットオフ周波数は `C4 + 12 半音 = C5` になります。

変更する音量は `Gain` で調整できます。

- `Low` シェルビングは `Semi` で指定したカットオフ周波数**以下**のサイン波の音量を変更します。
- `High` シェルビングは `Semi` で指定したカットオフ周波数**以上**のサイン波の音量を変更します。

### Pitch
Add Aliasing

:   チェックを入れると、ナイキスト周波数より高い周波数が指定されたサイン波もレンダリングします。

    ナイキスト周波数とは、ざっくりいえば録音したデータが再現可能な最高周波数のことです。音を生成するときはナイキスト周波数以上の値を指定することができますが、折り返し（エイリアシング）と呼ばれる現象によって計算結果は予期しない周波数になってしまいます。 `Add Aliasing` は、こうしたエイリアシングノイズを足し合わせるオプションです。

Reverse Semi

:   チェックを入れると、 `Semi` の符号を逆転させます。たとえば `7.000` は `-7.000` として処理されます。

Octave

:   ノートのオクターブです。

ET

:   平均律の値です。この値によって `Semi` と `Milli` の調律を変更できます。例えば `ET` が 12 なら 12 平均律になります。 `ET` は Equal Temperament の略です。

    ホスト側から送られてくるノートの調律を変更するわけではないので注意してください。

Multiply, Modulo

:   サイン波の周波数を変更します。

    計算式は `ノートの周波数 * (1 + fmod(Multiply * pitch, Modulo))` のようになっています。 `pitch` は `Note` 、 `Chord` 、 `Overtone` で指定した値から計算されるピッチです。 `fmod(a, b)` は `a` を `b` で割った余りを計算する関数です。

### Smooth
特定の値を変更したときに、変更前の値から変更後の値に移行する時間（秒）です。

`Smooth` と関連するパラメータのリストです。 `*` はワイルドカードです。

- `Gain` セクションの全て
- `Chord.Pan`
- `Chorus` の `Key Follow` 以外

他のパラメータはノートオンのタイミングで指定されていた値をノートの発音が終わるまで続けて使います。

### nVoice
最大同時発音数です。

### Random
Retrigger

:   チェックを入れると、ノートオンごとに乱数シードをリセットします。

Seed

:   乱数のシード値です。この値を変えると生成される乱数が変わります。

To Gain

:   ノートオンごとに各サイン波の音量をランダマイズする度合いです。

To Pitch

:   ノートオンごとに各サイン波の周波数をランダマイズする度合いです。

### Note, Chord, Overtone
サイン波のピッチと音量を指定します。

1つの `Note` あたり 16 の `Overtone` が発音されます。8つの `Note` で 1 つの `Chord` になります。そして 4 つの `Chord` が使えます。

`Note` と `Chord` に共通のパラメータです。

- `Gain` : サイン波の音量。
- `Semi` : ノートの基本周波数からの距離。単位は半音。
- `Milli` : ノートの基本周波数からの距離。単位は 1 / 1000 半音。 1 / 10 セント。

`Overtone` は左端が基本周波数（第1倍音）の音量で、右に一つ進むごとに第2倍音、第3倍音、 ... 、第16倍音の音量を表しています。

サイン波の周波数の計算式は次のようになっています。

```
function toneToPitch(semi, milli):
  return 2 ^ (1000 * semi + milli) / (ET * 1000)

for each Chord:
  chordPitch = toneToPitch(Chord.semi, Chord.milli)
  for each Note:
    notePitch = toneToPitch(Note.semi, Note.milli)
    for each Overtone:
      frequency = midiNoteFrequency
        * (1 + mod(Multiply * Overtone * notePitch * chordPitch, Modulo))
```

### Chorus
Mix

:   Dry/Wet の比率を調整します。

Freq

:   コーラスの LFO の周波数です。中央から右に回すと正、左に回すと負の周波数を指定できます。

Depth

:   左右の広がりを調整します。

Range

:   LFO によってディレイ時間を変調する度合いです。 3 つのディレイが用意されています。

Time

:   ディレイ時間です。

Phase

:   LFO の位相です。

Offset

:   ディレイの間での位相差です。

Feedback

:   ディレイのフィードバックです。

Key Follow

:   チェックを入れると、ディレイ時間をノートの音程に応じて変更します。

## チェンジログ
{%- for version, logs in changelog["IterativeSinCluster"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["IterativeSinCluster"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["IterativeSinCluster"] %}
- [IterativeSinCluster {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
IterativeSinCluster のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
