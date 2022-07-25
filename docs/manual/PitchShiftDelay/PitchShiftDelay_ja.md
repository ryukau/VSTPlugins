---
lang: ja
...

# PitchShiftDelay
![](img/pitchshiftdelay.png)

<ruby>PitchShiftDelay<rt>ピッチ シフト ディレイ</rt></ruby> はディレイを使った時間領域ピッチシフタです。 16 倍のオーバーサンプリングによってピッチシフタ特有の癖を多少抑えています。また内部のバッファ長をリアルタイムで変更できます。フォルマント補正がないので、声に使うとヘリウムを吸ったときのような音になります。

- [PitchShiftDelay {{ latest_version["PitchShiftDelay"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["PitchShiftDelay"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["PitchShiftDelay"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["PitchShiftDelay"] }})
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

{{ section["gui_barbox"] }}

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/pitchshiftdelay.svg)

## パラメータ
### Delay
Pitch

:   基本となるピッチシフトの倍率です。

    メインシフタは LFO を無視すれば常に `Pitch` で指定された倍率のピッチシフトを行います。

    PitchShiftDelay は 16 倍のオーバーサンプリングを行っているので、 16 * 2 - 1 = 31 倍のピッチシフトまでならエイリアシングノイズは出ません。

Offset

:   ユニゾンシフタのピッチシフトの倍率が `Pitch` からどれだけ離れるかを決める量です。

Pitch Cross

:   メインシフタとピッチシフトの出力をクロスフィードバックする量です。

Mirror

:   チェックを入れるとユニゾンシフタのピッチシフトの倍率を `1 / Pitch` に設定します。

    起動直後の状態から `Mirror` にチェックを入れると `Feedback` を上げても、フィードバック信号のピッチが常に `Pitch` あるいは `1 / Pitch` になります。この状態は `Offset` 、 `Pitch Cross` 、 あるいは LFO セクションの `To Pitch` 、 `To Unison` がデフォルト値である限り保たれます。

S1 Reverse

:   チェックを入れるとメインシフタの出力を逆再生させます。

    内部的にはピッチシフトの倍率の値の符号を負に変えることで読み取りポインタを逆走させています。

S2 Reverse

:   チェックを入れるとユニゾンシフタの出力を逆再生させます。

L-R Lean

:   左右のディレイ時間の比率です。 `Channel Type` が `M-S` のときでも機能します。

Delay Time \[s\]

:   ピッチシフタのバッファの長さです。 `Pitch` の値が 1 のときだけ正確なディレイ時間を表すことに注意してください。

    ピッチシフトの倍率が 1 でないとき、正確なディレイ時間はピッチシフトの倍率とバッファの長さによって変わります。 その上に LFO がかかると正確なディレイ時間を計算することは困難になります。したがって `Delay Time` の値は、正確なディレイ時間というよりも、おおまかな指標です。

Feedback

:   振幅であらわされたフィードバックのゲインです。

Stereo Cross

:   ステレオチャンネル間でクロスフィードバックする量です。 `1.0` にするとピンポンディレイになります。

Channel Type

:   入力信号のステレオチャンネルの種類を左右 (`L-R`) とミッド-サイド (`M-S`) のいずれかから選択します。

Highpass \[Hz\]

:   フィードバック経路に挿入されているハイパスフィルタのカットオフ周波数です。

### Mix
Dry \[dB\]

:   バイパスする入力信号のゲインです。

Wet \[dB\]

:   PitchShiftDelay を通過した出力信号のゲインです。

Unison Mix

:   メインシフタとユニゾンシフタを混ぜる比率です。

Smoothing \[s\]

:   パラメータのスムーシング時間です。

    例えば `Smoothing` の値を `0.01` と短くするとパラメータの変更がほぼ瞬時に適用されます。ただし `Smoothing` の値を小さくするとパラメータ変更時のポップノイズが目立つようになります。

    逆に `Smoothing` の値を `1.0` などと長くするとパラメータの値がゆっくりと切り替わるようになります。特に `Pitch` などを動かしたときはスライドやポルタメントのようになります。

### LFO
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

Stereo Offset

:   ステレオチャンネル間で LFO の位相をずらす量です。

Unison Offset

:   メインシフタとユニゾンシフタの間で LFO の位相をずらす量です。

Wave Interp.

:   LFO の波形の補間方法です。

    - `Step`: ホールド。
    - `Linear`: 線形補間。
    - `PCHIP`: 単調な 3 次補間。

    `Step` を選ぶと LFO をシーケンサのように使えます。 `Linear` は `PCHIP` と似たような音になりますが、計算がすこし速いです。デフォルトの `PCHIP` はサンプル間をだいたい滑らかに補間します。

To Pitch

:   LFO でピッチを変調する量です。

To Unison

:   `To Pitch` で指定した量に加えて LFO でユニゾンシフタのピッチを変調する量です。


## チェンジログ
{%- for version, logs in changelog["PitchShiftDelay"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### PitchShiftDelay
{%- if old_download_link["PitchShiftDelay"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["PitchShiftDelay"] %}
- [PitchShiftDelay {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
PitchShiftDelay のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
