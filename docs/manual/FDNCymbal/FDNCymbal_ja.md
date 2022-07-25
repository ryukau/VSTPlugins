---
lang: ja
...

# FDNCymbal
![](img/fdncymbal.png)

<ruby>FDNCymbal<rt>エフディーエヌ シンバル</rt></ruby> はシンバルのような音を合成するシンセサイザです。エフェクトとして使うこともできます。名前とは裏腹に金属的な質感は FDN (feedback delay network) ではなく Schroeder allpass section によって得られています。 `FDN.Time` の値を小さくすることで、わりとナイスなばちの衝突音が合成できます。シンバルの揺れをシミュレートするためにトレモロもついています。

- [FDNCymbal {{ latest_version["FDNCymbal"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["FDNCymbal"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["FDNCymbal"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["FDNCymbal"] }})
{%- endif %}

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit
- macOS universal binary

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
バージョン 0.1.x と 0.2.x は互換性がありません。

`FDN.Feedback` の値が 0 でないときは信号が発散することがあります。発散したときは `FDN.Feedback` を一番左まで回してください。

`HP Cutoff` の値を短時間に大きく変更すると直流が乗ることがあります。直流が乗ったときは、いったん `HP Cutoff` を上げてから <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> でゆっくりと下げなおしてください。

## エフェクトとしての利用
FDNCymbal はエフェクトとして使うこともできますが、直感的でない設定が必要となっています。

ロード直後の FDNCymbal は初期化時に音が止まることを防ぐためにミュートされています。有効化するためには FDNCymbal に  MIDI ノートオンを送る必要があります。ベロシティによって出力音量が変わるので注意してください。また `Seed` と `FDN` セクションの値を変更したときは新しいノートオンを送るまで更新されません。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/fdncymbal.svg)

## パラメータ
### Gain
出力音量です。

### Smooth
次のパラメータを変更したときに、変更前の値から変更後の値へと移行する秒数です。

- `Gain`
- `FDN.Time`
- `FDN.Feedback`
- `FDN.CascadeMix`
- `Allpass.Mix`
- `Allpass.Stage1.Time`
- `Allpass.Stage1.Feedback`
- `Allpass.Stage2.Time`
- `Allpass.Stage2.Feedback`
- `Tremolo.Mix`
- `Tremolo.Depth`
- `Tremolo.Frequency`
- `Tremolo.DelayTime`

### Stick
`Stick` がオンのとき、ノートオンのたびにばちでシンバルを叩きます。ノートの高さでノイズの密度とトーンの高さが変わります。

Pulse

:   ばちオシレータが出力するインパルスの音量です。

Tone

:   ばちオシレータが出力するサイン波によるトーンの音量です。

Velvet

:   ばちオシレータが出力するベルベットノイズの音量です。

    ベルベットノイズはランダムな間隔で異なる高さのインパルスが出力されるノイズのアルゴリズムです。

Decay

:   ばちオシレータの減衰時間です。

### Random
Seed

:   乱数のシード値です。

Retrigger.Time

:   チェックを外すと、次の 3 つのパラメータでコントロールされる内部的なディレイ時間がノートオンのたびに変わります。

    - `FDN.Time`
    - `Allpass.Stage1.Time`
    - `Allpass.Stage2.Time`

Retrigger.Stick

:   チェックを外すと、ノートオンのたびにばちオシレータの音が変わります。

Retrigger.Tremolo

:   チェックを外すと、トレモロのパラメータがノートオンのたびに変わります。

### FDN
オンのときはFDN (feedback delay network) セクションを信号が通過します。

Time

:   FDN のディレイ時間を調整します。内部的なディレイ時間はノブで設定した値をもとにランダマイズされます。

Feedback

:   FDN セクションのフィードバックです。この値が 0 でないときは信号が発散することがあるので注意してください。

CascadeMix

:   直列につないだ複数の FDN の出力のミックスを調整します。

### Allpass
Mix

:   Schroeder allpass section のミックスを調整します。

### Stage 1 と Stage 2
2 つの Schroeder allpass section が直列につながっています。 Stage 2 では 4 つの Schroeder allpass section が並列接続されています。

Time

:   Schroeder allpass section のディレイ時間の最大値です。ディレイ時間は内部的にランダマイズされます。

Feedback

:   Schroeder allpass section のフィードバックです。

HP Cutoff

:   Schroeder allpass section の出力にかかるハイパスフィルタのカットオフ周波数です。

Tanh

:   チェックを入れると Stage 1 のフィードバックにサチュレーションがかかります。

### Tremolo
Mix

:   トレモロのミックスを調整します。

Depth

:   `Depth` が大きいほどトレモロによる音量の変化が大きくなります。

Frequency

:   トレモロに使われる LFO の周波数です。

DelayTime

:   LFO によって変化するディレイ時間の最大値です。シンバルの揺れによって生じるドップラー効果を表現しています。

### Random (Tremolo)
Depth

:   ノートオンのたびに `Tremolo.Depth` をランダマイズする度合いです。

Freq

:   ノートオンのたびに `Tremolo.Frequency` をランダマイズする度合いです。

Time

:   ノートオンのたびに `Tremolo.DelayTime` をランダマイズする度合いです。

## チェンジログ
{%- for version, logs in changelog["FDNCymbal"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["FDNCymbal"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["FDNCymbal"] %}
- [FDNCymbal {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

旧バージョン向けのプリセットです。

- [バージョン 0.1.* 用のプリセット (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/FDNCymbalPresets.zip)

## ライセンス
FDNCymbal のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
