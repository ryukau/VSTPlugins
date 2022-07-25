---
lang: ja
...

# ParallelComb
![](img/parallelcomb.png)

<ruby>ParallelComb<rt>パラレル コム</rt></ruby> は 1 つバッファの異なる 4 つの時点をフィードバックするコムフィルタです。フィードバック経路にリミッタが挟んであるので、それなりにクリーンな音が出ます。フィードバックの振幅でディレイ時間を変調することで、変な歪みを出すこともできます。

- [ParallelComb {{ latest_version["ParallelComb"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["ParallelComb"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["ParallelComb"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["ParallelComb"] }})
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
`Feedback` が 0.25 よりも大きいとき、出力はほぼ確実に発散します。発散するかどうかは `Feedback` と `Delay Time` の値の組み合わせに依存します。発散する状況ではディザリングノイズのような微弱な信号であってもフィードバックの飽和が起こります。フィードバックを確実に止めるためには ParallelComb の直前にゲートを挿入することを推奨します。ただし、ゲートがディザリングを行っていないことを確認してください。

`Feedback` の値を 0 にするとフィードバックが止まります。右下の `Panic!` ボタンを押すことで即座に `Feedback` を 0 にできます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/parallelcomb.svg)

## パラメータ
### Delay
Delay Time \[s\]

:   ディレイ時間の秒数です。最終的なディレイ時間の秒数は以下の式で計算されます。

    ```
    leanL = (L-R Lean) < 0 ? 1 + (L-R Lean) : 1;
    leanR = (L-R Lean) < 0 ? 1              : 1 - (L-R Lean);

    upRate = (16x OverSampling) ? 16 : 1;
    time = (Time Multi.) * upRate * (Delay Time);

    delayTimeL = time * leanL;
    delayTimeR = time * leanR;
    ```

Time Multi.

:   `Delay Time` に乗算される値です。まとめてディレイ時間を変えたいときに便利です。

Interp. Rate

:   ディレイ時間の変動を補間するレートリミッタの 1 サンプルあたりの制限量です。言い換えると `Interp. Rate` は 1 サンプルあたりの `Delay Time` の増分の絶対値の最大です。例えば `Interp. Rate` が 0.1 のときは、 10 サンプル経過でディレイ時間が最大 ±0.1 * 10 = ±1 サンプル変わります。

    `Interp. Rate` の値は `Self Mod.` のキャラクタに影響します。

Interp. LP \[s\]

:   `Delay Time` の補間に使われるローパスフィルタのスムーシング時間です。

    `Interp. LP` の値は `Self Mod.` のキャラクタに影響します。

L-R Lean

:   左右のディレイ時間の比率です。 `Channel Type` が `M-S` のときでも機能します。

Cross

:   フィードバックを左右のチャンネルでクロスする量です。右いっぱいに回すとピンポンディレイになります。

Feedback

:   振幅であらわされたフィードバックのゲインです。

    `Feedback` が `0.25` より小さいとき、出力は減衰してどこかの時点で止まります。 `Feedback` が `0.25` より大きいと出力はほぼ確実に発散します。ただし `Feedback` と `Delay Time` の値の組み合わせによっては、 `Feedback` の値が `0.25` より大きくても出力が減衰することがあります。

Highpass \[Hz\]

:   フィードバック経路に挿入されているハイパスフィルタのカットオフ周波数です。

Limiter R. \[s\]

:   フィードバック経路に挿入されているリミッタのリリース時間です。

    `Limiter R.` を 0 にすると出力がソフトクリッピングします。リリース時間を長くするとフィードバックの歪みが減ってクリーンな音になります。

Self Mod.

:   フィードバック信号の振幅でディレイ時間を変調する量です。

    `Self Mod.` を 1 から 10 の間、 `Feedback` を 0 に設定することで ParallelComb をディストーションやファズのように使えます。

    歪みのキャラクタは `Interp. Rate` と `Interp. LP` の値によって変わります。

Gate Th. \[dB\]

:   ゲートのしきい値となる振幅です。

    ゲートは入力信号の振幅が `Gate Th.` を下回ると閉じます。

Gate R. \[s\]

:   ゲートのリリース時間です。

    `Gate Th.` を `-inf` より大きくしてゲートをかけたときに、どれくらい長く余韻を残すか調整するために使えます。

### Mix

Dry \[dB\]

:   バイパスする入力信号のゲインです。

Wet \[dB\]

:   ParallelComb を通過した出力信号のゲインです。

Channel Type

:   入力信号のステレオチャンネルの種類を左右 (`L-R`) とミッド-サイド (`M-S`) のいずれかから選択します。

16x OverSampling

:   チェックを入れると 16 倍のオーバーサンプリングが有効になります。 CPU 消費が 16 倍 + リサンプラの計算量だけ増えるので注意してください。

Panic!

:   クリックすると `Feedback` を 0 に設定します。

## チェンジログ
{%- for version, logs in changelog["ParallelComb"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### ParallelComb
{%- if old_download_link["ParallelComb"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["ParallelComb"] %}
- [ParallelComb {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
ParallelComb のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
