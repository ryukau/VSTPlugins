---
lang: ja
...

# FDN64Reverb
![](img/FDN64Reverb.png)

<ruby>FDN64Reverb<rt>エフディーエヌ 64 リバーブ</rt></ruby> はフィードバック・ディレイ・ネットワーク (FDN) を 1 つだけ搭載したリバーブです。しかしながら、フィードバック行列の大きさは 64 です。

- [FDN64Reverb {{ latest_version["FDN64Reverb"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["FDN64Reverb"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["FDN64Reverb"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["FDN64Reverb"] }})
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

## 注意
`Feedback` の値が 1 のときに `Highpass Cutoff` の値を 0 にすると入力信号によっては極端な直流が乗ることがあります。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/FDN64Reverb.svg)

## パラメータ
Delay Time \[s\]

:   ディレイ時間の秒数です。最終的なディレイ時間の秒数は以下の式で計算されます。

    ```
    delayTime = (Time Multi.) * (Delay Time) + (Time LFO Amount) * random().
    ```

    ディレイ時間は 1 秒を超えることはないので注意してください。例えば `Delay Time` を最大に設定すると `Time LFO Amount` の値によらず LFO はかかりません。

Time LFO Amount \[s\]

:   ディレイ時間に加算される LFO の量です。例えば `Time LFO Amount` の値が 0.1 ならディレイ時間の最大値は `(Delay Time) + 0.1` 秒になります。

    `Interp. Rate` が 1.0 を超えると変調によるノイズが目立ち始めます。とにかく滑らかな音が欲しいときは `Interp. Rate` を 0.25 以下にすることを推奨します。

Lowpass Cutoff \[Hz\]

:   ディレイの出力にかかるローパスフィルタのカットオフ周波数です。

Highpass Cutoff \[Hz\]

:   ディレイの出力にかかるハイパスフィルタのカットオフ周波数です。

    `Feedback` の値が 1 のときに `Highpass Cutoff` の値を 0 にすると直流が乗ることがあるので注意してください。

### Delay
Time Multi.

:   `Delay Time` に乗算される値です。まとめてディレイ時間を変えたいときに便利です。

Feedback

:   FDN からフィードバックされる量です。別の言い方をすると FDN のフィードバック行列に掛け合わされるスカラーです。

    1.0 に近づくほどリバーブ時間が長くなります。 1.0 のとき、ほとんど減衰しなくなります。このときは `Gate` を使って残響を打ち切ることができます。

Interp. Rate

:   ディレイ時間の変動を補間するレートリミッタの 1 サンプルあたりの制限量です。例えば `Interp. Rate` が 0.1 のときは、 10 サンプル経過でディレイ時間が 0.1 * 10 = 1 サンプル変わります。

Gate \[dB\]

:   ゲートのしきい値です。

    入力振幅が `Gate` の値を一定時間下回ると、 `Stereo Cross` の値を内部的に変調して出力振幅を 0 にします。

    このゲートは Rotation 使用時にフィードバックを打ち切るためにつけた機能です。リリース時間は約 5 ms です。

Matrix

:   フィードバック行列の種類です。この値を変更するとポップノイズがでることがあるので注意してください。

    | GUI 上の略称 | 行列名                     | 追加情報                       | 質       |
    | ------------ | -------------------------- | ------------------------------ | -------- |
    | Ortho.       | 直交行列                   |                                | 良       |
    | S. Ortho.    | 特殊直行行列               |                                | 良       |
    | Circ. Ortho. | 巡回行列                   |                                | 並       |
    | Circ. 4      | 巡回行列                   | 1 行当たりの非ゼロ要素が 4     | 特殊効果 |
    | Circ. 8      | 巡回行列                   | 1 行当たりの非ゼロ要素が 8     | 特殊効果 |
    | Circ. 16     | 巡回行列                   | 1 行当たりの非ゼロ要素が 16    | 特殊効果 |
    | Circ. 32     | 巡回行列                   | 1 行当たりの非ゼロ要素が 32    | 特殊効果 |
    | Upper Tri. + | 上三角行列                 | ランダマイズ範囲が `[0, 1]`    | 低       |
    | Upper Tri. - | 上三角行列                 | ランダマイズ範囲が `[-1, 0]`   | 低       |
    | Lower Tri. + | 下三角行列                 | ランダマイズ範囲が `[0, 1]`    | 低       |
    | Lower Tri. - | 下三角行列                 | ランダマイズ範囲が `[-1, 0]`   | 低       |
    | Schroeder +  | シュローダー・リバーブ行列 | ランダマイズ範囲が `[0, 1]`    | 低       |
    | Schroeder -  | シュローダー・リバーブ行列 | ランダマイズ範囲が `[-1, 0]`   | 低       |
    | Absorbent +  | 吸収オールパス行列         | ランダマイズ範囲が `[0, 1]`    | 並       |
    | Absorbent -  | 吸収オールパス行列         | ランダマイズ範囲が `[-1, 0]`   | 並       |
    | Hadamard     | アダマール行列             | 行列の性質よりランダマイズ無効 | 良       |
    | Conference   | カンファレンス行列         | 行列の性質よりランダマイズ無効 | 良       |

    質は大まかな目安です。ショートディレイによる金属的な質感が出やすいものは低、出にくいものは良としています。特殊効果としているものは一般的なリバーブとしては音が変という意味です。 Rotation の効果は質が低いほど聞こえやすくなります。

Seed

:   フィードバック行列のランダマイズに使われるシード値です。この値を変更するとポップノイズがでることがあるので注意してください。

Change Matrix

:   フィードバック行列をランダマイズするボタンです。押すたびに `Seed` の値が変更されます。この値を変更するとポップノイズがでることがあるので注意してください。

### Mix
Dry \[dB\]

:   FDN をバイパスした入力にかけ合わせられるゲインです。

Wet \[dB\]

:   FDN の出力にかけ合わせられるゲインです。

Stereo Cross

:   左右のチャンネルの FDN の出力をクロスしてフィードバックする量です。 1.0 にすると発散を防ぐために入力を止めてしまうので注意してください。

### Rotation
Speed \[Hz\]

:   FDN への入力ゲインを回転させる速度です。

Offset

:   入力ゲインの回転に使われる波形の初期位相です。

    `Speed` が 0 かつ `Skew` が 0 より大きいときに音を変えることができます。どのような音になるかはフィードバック行列のランダマイズに大きく影響されます。

Skew

:   入力ゲインの回転に使われる波形を変更します。 `Skew` が 0 のときは `Speed` と `Offset` を変えても音が変わらないので注意してください。

    以下は `Skew` と波形の関係についてのプロットです。

    <figure>
    <img src="img/skewplot.svg" alt="Image of plot of relation between `Skew` parameter and rotation waveform. The equation of waveform is `exp(skew * sin(ω * t))`." style="padding-bottom: 12px;"/>
    </figure>

## チェンジログ
{%- for version, logs in changelog["FDN64Reverb"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["FDN64Reverb"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["FDN64Reverb"] %}
- [FDN64Reverb {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
FDN64Reverb のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
