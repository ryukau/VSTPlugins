---
lang: ja
...

# FeedbackPhaser
![](img/FeedbackPhaser.png)

<ruby>FeedbackPhaser<rt>フィードバック フェイザ</rt></ruby> は LFO ではなくフィードバックによってオールパスフィルタのカットオフ周波数を変調するフェイザです。主に歪み系の音が出ます。

{% for target, download_url in latest_download_url["FeedbackPhaser"].items() %}
- [FeedbackPhaser {{ latest_version["FeedbackPhaser"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["FeedbackPhaser"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["FeedbackPhaser"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
FeedbackPhaser の後に必ずリミッタを挿入することを推奨します。 Feedback -> `Amount` と `Safety Clip` の値によっては出力振幅が +140 dB を超えることがあります。

## 使い方
### 起動時の設定
以下は、セットアップ手順の例です。

1. FeedbackPhaser を起動します。
2. 入力の振幅に応じて `Output` を下げます。
3. Feedback -> `Amount` を変更します。
4. Modulation -> `Amount` を変更します。
5.`Safety Clip` を変更します。

出力振幅は上記の手順でほぼ決まります。先に音量を決めておけば他のパラメータの変更がスムーズになります。

Modulation -> `Type` によって Modulation -> `Amount` の効きが変わるので注意してください。出力振幅の変化につながります。

### フィードバックの発散の抑制
`Safety Clip` は、出力振幅を制限するための最も重要なパラメータです。 フィードバックの振幅が `Safety Clip` の振幅を超える場合、 Feedback セクションの：

```
Output is clean.
```

が、以下に変わります。

```
Too much feedback.
```

"Too much feedback." が表示されているときは内部的に歪みが生じています。歪みを避けるときは `Safety Clip` を上げて、 `Output` を下げます。

`Safety Clip` を上げても歪みを止められないことがあります。 この場合 Feedback -> `Amount` と Modulation -> `Amount` を下げてください。

### オートメーションのかけ方
以下のパラメータをオートメーションすると面白い音になるかもしれません。

- Allpass
  - `Spread`.
  - `Center`.
- Modulation
  - `Mod. Asym.`.

また、ノートイベントによってオールパスフィルタのカットオフ周波数を制御できます。ノートイベントをプラグインに送る方法は、お使いの DAW のマニュアルを参照してください。

Allpass -> `Stage` を増やすとフィードバックのレゾナンスがより強くなります。 ただし、入力信号の質感も薄まります。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/FeedbackPhaser.svg)

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は単位の一覧です。

- \[dB\] : デシベル (decibel) 。
- \[s\] : 秒 (second) 。
- \[st.\] : 半音 (semitone) 。
- \[Hz\] : 周波数 (Hertz) 。

### Gain
Output \[dB\]

:   出力ゲインです。

Highpass \[Hz\]

:   出力の直前に通過するハイパスフィルタのカットオフ周波数です。

    このフィルタは直流の除去を目的としています。

Mix

:   入出力の混合比です。

    `Mix` の値によって Feedback -> `Amount` の強さが変わります。

### Feedback
Amount

:   フィードバックの強さです。

    Feedback -> `Amount` の絶対値が大きいときはフィードバック信号がハードクリップされることがあります。

Safety Clip \[dB\]

:   フィードバック信号の振幅が `Safety Clip` を超えると、フィードバック信号は内部でハードクリップされます。

    FeedbackPhaser では、フィードバックによる発散を抑えるために内部的にハードクリッピングが行われています。ハードクリッピングがかかるとフィードバックに歪みが加わって音が変わります。

    クリーンな音を出したいときは Feedback セクションの `Output is clean.` という表示が `Too much feedback.` に変わらなくなるまで `Safety Clip` の値を上げてください。ただし、出力振幅が跳ね上がるので、 `Safety Clip` を上げた分だけ Gain セクションの `Output` を下げることを推奨します。

Highpass \[Hz\]

:   フィードバック信号が通過するハイパスフィルタのカットオフ周波数です。

### Allpass
Stage

:   直列につなぐオールパスフィルタの数です。

Spread

:   オールパスフィルタのカットオフ周波数の分布を変更する値です。

    `Spread` が 0 のとき、すべてのカットオフ周波数は `Center` と同じになります。 `Spread` が 1 のとき、カットオフ周波数は通過する順に `Center * 1` 、 `Center * 2` 、 `Center * 3` 、 ... と高くなっていきます。

    以下はオールパスフィルタのカットオフ周波数の計算式です。

    ```
    allpassCutoffHz = modulation * Center * (1 - Spread * (index - 1)).
    ```

Center \[Hz\]

:   変調がかかっていないときのオールパスフィルタのカットオフ周波数です。

### Modulation
Type

:   フィードバックによる変調のスケーリングの種類です。

    - `Exp. Mul.` : `Center * 2^modulation` として指数関数を使ったスケーリングを行います。
    - `Lin. Mul.` : `Center * modulation` として変調波を乗算します。
    - `Add` : `Center + modulation` として変調波を加算します。

    上のリストの式はそのまま実装で使われている形ではないことに注意してください。上式に加えて不正な値を避けるため、値の範囲の制限などが行われます。

Amount

:   フィードバックによる変調の量です。

    変調はオールパスフィルタのカットオフ周波数に対してかかります。

Mod. Asym.

:   そのままの変調信号と、全波整流した変調信号を混ぜる比率です。

Post LP \[Hz\]

:   変調後のオールパスフィルタのカットオフ周波数が通過する、ローパスフィルタのカットオフ周波数です。

### Note
Transpose \[st.\]

:   中心となるノートを変更する値です。

To Allpass

:   ノートのピッチの倍率です。

    `To Allpass` が 1 のときにノートのピッチが表す比率がそのまま使われます。

    FeedbackPhaser が使うのはノートが表す相対的なピッチであり、ノートが表す絶対的な周波数が使われることはない点に注意してください。

Slide \[s\]

:   あるノートのノートオフを受け取る前に、別のノートのノートオンを受け取ったときにピッチをスライドする時間です。

Release \[s\]

:   最後のノートのピッチから `Center` に戻るまでにかかる大まかな時間です。

### Misc.
Side Chain

:   点灯させるとサイドチェイン入力を変調信号として使います。

Smoothing \[s\]

:   パラメータが変更されたときに、変更先の値に到達するまでにかかる大まかな時間です。

Oversampling

:   オーバーサンプリングの倍率です。

    オーバーサンプリングの倍率を上げると変調が強いときに乗る、ざらざらとしたノイズが消えることがあります。ただし CPU 負荷は倍率に応じて上がります。

## チェンジログ
{%- for version, logs in changelog["FeedbackPhaser"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["FeedbackPhaser"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["FeedbackPhaser"] %}
- [FeedbackPhaser {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
FeedbackPhaser のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
