---
lang: ja
...

# DoubleLoopCymbal
![](img/DoubleLoopCymbal.png)

<ruby>DoubleLoopCymbal<rt>ダブル ループ シンバル</rt></ruby> はハイハットのような音が出るシンセサイザです。オールパスループというディレイのつなぎ方を使って金属的な質感を出しています。

{% for target, download_url in latest_download_url["DoubleLoopCymbal"].items() %}
- [DoubleLoopCymbal {{ latest_version["DoubleLoopCymbal"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["DoubleLoopCymbal"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["DoubleLoopCymbal"] }})
{%- endif %}

**注意**: プリセットはサンプリング周波数が 48000 Hz でなければ想定された音が出ません。

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
音量の変化が大きいのでリミッタと併用してください。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/DoubleLoopCymbal.svg)

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は単位の一覧です。

- \[dB\] : デシベル (decibel) 。
- \[s\] : 秒 (second) 。
- \[Hz\] : 周波数 (Hertz) 。
- \[sample\] : サンプル数。 1 サンプルは `1 / (サンプリング周波数)` 秒。
- \[st.\] : 半音 (semitone)。 1 半音は `1 / 12` オクターブ。

### Mix & Options
Output \[dB\]

:   出力ゲインです。

2x Sampling

:   チェックすると 2 倍のオーバーサンプリングを有効にします。

Release

:   チェックを入れると、

    - ノートオフの後も音が止まらなくなります。
    - `Closing Noise` セクションの設定がすべて無視されます。

Fixed Noise

:   チェックを入れるとノートオンごとの音の揺れを抑えます。

    内部的にはノイズ信号の合成に使われる乱数のシードをノートオンごとにリセットします。

Spread

:   ステレオスプレッダのかかり具合です。値が 1 に近づくほど左右の広がりがでます。

Split \[Hz\]

:   ステレオスプレッダのクロスオーバー周波数です。

    大まかには指定した周波数以上でのみステレオスプレッダの効果が出ます。ただしクロスオーバーフィルタの性質によって指定した周波数以下であっても効果が完全に無効とはなりません。

External Input \[dB\]

:   外部入力のゲインです。

    DoubleLoopCymbal は外部入力に金属的な質感を加えるエフェクタとして使うことができます。

    **重要**: エフェクタとして使うときでも DoubleLoopCymbal にノートイベントを送ってください。ノートが演奏されていないとき、外部入力はミュートされます。

    ノートイベントはハイハットのオープン・クローズと似た表現を行うために使われます。例えば、片手で MIDI キーボードを抑えながら、もう片方の手で机などを叩いてマイクに音を入れるといったセットアップを想定しています。

    マイクからの入力を使うのであれば外部入力を事前にイコライザで調整することを推奨します。プラスチックのような固い素材の衝突であればそのまま励起信号として使えますが、手で机をたたいた音などは低い周波数が強すぎることがあります。

    DoubleLoopCymbal が生成する励起信号 (ノイズ) を外部入力に置き換えるときは以下のパラメータをすべて最小 (-inf dB) にしてください。

    - `Impact Noise` -> `Gain`
    - `Half Closed Noise` -> `Gain`
    - `Closing Noise` -> `Gain`

    外部入力やノートイベントをルーティングする方法はお使いの DAW のマニュアルを参照してください。

### Tuning
Note -> Pitch

:   ノートの音程によるピッチの変調量です。

    `Note -> Pitch` の値によらず、音程が C4 (MIDI ノート番号でいうと 60) のときにピッチの変調量が 0 となります。

    `Note -> Pitch` の値が 0.0 のとき、ノートのピッチは無視されます。

    `Note -> Pitch` の値が 1.0 のときに音程がピッチにそのまま反映されます。ただしドラムの音ははっきりとした音程を持たないのでチューニングがあっていないように聞こえることがあります。また、実装の都合により音が高くなるほどチューニングが狂います。

    `Note -> Pitch` の値が -1.0 のとき、音程の上下が逆転します。

Transpose \[st.\]

:   半音単位でノートの音程をトランスポーズします。

    **注意**: `Note -> Pitch` が 0.0 のときは無効です。

Pitch Bend Range \[st.\]

:   ピッチベンドの範囲です。単位は半音です。

Slide Time \[s\]

:   2 つ以上のノートが同時に演奏されたときにピッチをスライドする大まかな時間です。

### Velocity Map
以下は略語と対応するセクションの一覧です。

- `Imp.` -> Impulse Noise
- `HC` -> Half Closed Noise
- `Cl.` -> Closing Noise
- `AP` -> Allpass Loop

> Imp. Gain \[dB\]

:   ノートオン・ベロシティによって `Impact Noise` セクションの `Gain` を変える量です。

    MIDI キーボードなどで演奏するときは -30 dB あたりまで範囲を狭めたほうが弾きやすいかもしれません。

> Imp. Highpass

:   ノートオン・ベロシティによって `Impact Noise` セクションの `Highpass` を変える量です。

    - 値が 0 より大きいと、ベロシティが大きくなるにつれてカットオフ周波数が高くなります。
    - 値が 0 より小さいと、ベロシティが大きくなるにつれてカットオフ周波数が低くなります。

> HC Density

:   ノートオン・ベロシティによって `Half Closed Noise` セクションの `Density` を変える量です。

    `> Half Closed Highpass` と `> Modulation` との組み合わせによって、ベロシティが大きいときにクラッシュシンバルのような音となるように変化させることを狙っています。ただし、本物のクラッシュシンバルのような音は出ません。

    `Half Closed Noise` セクションの `Decay` が短いときは、あまり効果がありません。

> HC Highpass

:   ノートオン・ベロシティによって `Half Closed Noise` セクションの `Highpass` を変える量です。

Note-off Velocity

:   ハイハットのクローズに使うベロシティの種類です。

    チェックを入れるとノートオフ・ベロシティ、チェックを外すとノートオン・ベロシティが使われます。以下は `Note-off Velocity` によって影響を受けるパラメータの一覧です。

    - `> Cl. Gain`
    - `> Cl. Duration`
    - `> Cl. Highpass`

> Cl. Gain \[dB\]

:   `Note-off Velocity` で指定されたベロシティによって `Closing Noise` セクションの `Gain` を変える量です。

> Cl. Release

:   `Note-off Velocity` で指定されたベロシティによって `Closing Noise` セクションの `Release Ratio` を変える量です。

> Cl. Highpass

:   ノートオン・ベロシティによって `Closing Noise` セクションの `Highpass` を変える量です。

> AP Modulation

:   ノートオン・ベロシティによって `Allpass Loop` セクションの `Modulation` を変える量です。

### Impact Noise
スティックとシンバルの衝突を模倣するノイズです。

Seed

:   `Seed` を変えると全体の質感が大きく変わります。

Texture Mix

:   ノートオンごとにシードがリセットされる乱数と、そうでない乱数を混ぜる割合です。

    値が 1.0 のときにシードがリセットされる乱数のみとなり、音の揺らぎが抑えられます。 `Fixed Noise` と似たような効果がありますが、 `Half Closed Noise` と `Closing Noise` に影響を与えない点が異なります。

Gain \[dB\]

:   ノイズ信号のゲインです。

Decay \[s\]

:   ノイズ信号の減衰時間です。

Highpass \[Hz\]

:   ノイズ信号が通るハイパスフィルタのカットオフ周波数です。

### Half Closed Noise
ハイハットのハーフクローズを模倣するノイズです。

Gain \[dB\]

:   ノートオン直後のノイズ信号のゲインです。

    ノイズ信号は時間とともに `Sustain` で指定したゲインに向かって減衰します。

Decay \[s\]

:   ノイズ信号の減衰時間です。

Sustain \[dB\]

:   `Decay` で指定した時間が経過したあとのノイズ信号のゲインです。

Pulse Duration \[s\]

:   1 回の衝突あたりのノイズの減衰時間です。

    `Pulse Duration` と `Density` の値がともに小さいとき、カラカラとした質感がでます。

Density \[Hz\]

:   大まかな衝突頻度です。

    `Density` の値が大きいとき、衝突間隔が長くなったようなカラカラという質感に近づきます。値が小さいときはサーッという質感に近づきます。

Highpass \[Hz\]

:   ノイズ信号が通るハイパスフィルタのカットオフ周波数です。

### Closing Noise
ハイハットのクローズを模倣するノイズです。クローズは `Mix & Options` セクションの `Release` が無効のときだけ行われます。

Gain \[dB\]

:   ノイズ信号のゲインです。

Attack \[s\]

:   ノイズ信号がおおよそ最大振幅に到達するまでの時間です。

    `Attack` の値を大きくするとリリース時間が長くなり、最大振幅が小さくなります。

Release Ratio

:   ノイズ信号の大まかなリリース時間です。

    **注意**: `Attack` が小さいときに `Release Ratio` を上げると振幅がとても大きくなることがあります。

Highpass \[Hz\]

:   ノイズ信号が通るハイパスフィルタのカットオフ周波数です。

### Allpass Loop
金属的な質感はこのセクションで調整できます。オールパスループは板というよりは輪になったワイヤに近いので、どちらかと言えばシンバルよりもトライアングルに似た音になりがちです。

Character

:   内部的なディレイ時間を変えて質感を調整するパラメータです。

    値が 0 のときは音程がはっきりした音、 1 のときは音程がはっきりしない音になります。

    値が 0 のときのディレイ時間の比率は固定されています。値が 1 のときのディレイ時間の比率は `Impact Noise` セクションの `Seed` に応じてランダムに決められます。

Pitch Ratio \[st.\]

:   2 つのオールパスループのディレイ時間をずらす量です。

    ディレイ時間がずらされるのは 2 つ目のオールパスループのみです。

Modulation \[sample\]

:   フィードバック信号によるディレイ時間の変調量です。

    **警告**: `Modulation` の値が 0 でなければ発散によって大きな音が出ることがあります。

Delay Count 1

:   1 つ目のオールパスループのディレイの数です。

Delay Count 2

:   2 つ目のオールパスループのディレイの数です。

Feed 1

:   1 つ目のオールパスループのオールパスフィルタの係数です。

    エフェクタのディレイのフィードバックと似たように扱えます。値が 1 あるいは -1 に近づくほど音が長く伸びるようになります。ちょうど 1 あるいはちょうど -1 にしてしまうと音が減衰しなくなります。

Feed 2

:   2 つ目のオールパスループのオールパスフィルタの係数です。

Mix Spike

:   大まかにノイズ信号を混ぜる比率です。

    値が 0 のときにノイズ信号はほぼ消えます。 1 のときはオールパスループの出力とノイズ信号が足し合わさったような音になります。

Mix Alt. Sign

:   オールパスループの出力の質感を調整します。

    値が 0 のとき、ループ内のすべてのオールパスの出力を単純に加算します。

    ```
    loopOutput = ap1 + ap2 + ap3 + ap4 + ap5 + ...
    ```

    値が 1 のとき、ループ内のオールパスごとに符号を反転した上で加算を行います。

    ```
    loopOutput = ap1 - ap2 + ap3 - ap4 + ap5 - ...
    ```

### Filter
オールパスループ内のフィルタに関する設定です。

High Shelf Cutoff \[Hz\]

:   ハイシェルフフィルタのカットオフ周波数です。

    シンバルの音を出すときは `High Shelf Cutoff` を 10000 Hz あたり、 `High Shelf Gain` を -1 から 0 dB の間にしてください。

High Shelf Gain \[dB\]

:   ハイシェルフフィルタのゲインです。

Low Shelf Cutoff \[Hz\]

:   ローシェルフフィルタのカットオフ周波数です。

Low Shelf Gain \[dB\]

:   ローシェルフフィルタのゲインです。

    **注意**: 0 dB にすると直流が乗ることがあります。

## チェンジログ
{%- for version, logs in changelog["DoubleLoopCymbal"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["DoubleLoopCymbal"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["DoubleLoopCymbal"] %}
- [DoubleLoopCymbal {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
DoubleLoopCymbal のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
