---
lang: ja
...

# RingModSpacer
![](img/RingModSpacer.png)

<ruby>RingModSpacer<rt>リング モッド スペーサ</rt></ruby> は激しい歪みを生じるサイドチェインリミッタです。全波整流したサイドチェイン入力によってメインの入力を振幅変調することで、サイドチェインを足し合わせてもクリッピングしないような隙間をメインの入力にこじ開けることができます。当然、隙間をこじ開けられたメインの入力は歪むので使いどころが難しいです。

{% for target, download_url in latest_download_url["RingModSpacer"].items() %}
- [RingModSpacer {{ latest_version["RingModSpacer"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["RingModSpacer"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["RingModSpacer"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 使い方
まず、メイン入力とサイドチェイン入力を用意します。クリーンにしておきたい信号をサイドチェインに入力してください。メインは歪みがかかります。サイドチェイン入力を行う方法はお使いの DAW のマニュアルを参照してください。

次に Input と Side セクションの `Gain` によってそれぞれのピーク振幅を 0 dB まで上げます。この手順によって効果が最大となりますが、飛ばしても問題ありません。

後は `Ring-Sub. Mix` 、 `Attack` 、 `Release` を調整して軽く質感を整えれば設定完了です。

RingModSpacer はあくまでも歪みを加えるエフェクタです。歪みが不要であれば、しきい値 (threshold) を -∞ dB に設定したサイドチェインリミッタのほうが用途に適しています。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/RingModSpacer.svg)

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は単位の一覧です。

- \[dB\] : デシベル (decibel) 。
- \[s\] : 秒 (second) 。

### Gain
Output \[dB\]

:   出力ゲインです。

Side Mix

:   サイドチェイン入力を出力に混ぜる量です。

Ring-Sub. Mix

:   波形に隙間を作る方法をリングモジュレーション方式と差分方式の間で切り替えます。

    `Ring-Sub. Mix` が 0 のときはリングモジュレーション方式、 1 のときは差分方式となります。

### Misc.
Smoothing \[s\]

:   パラメータが変更されたときに、変更先の値に到達するまでにかかる大まかな時間です。

### Input, Side
Gain\[dB\]

:   リミッタの前にかけられる入力ゲイン（メイクアップゲイン）です。

Attack \[s\]

:   リミッタのアタック時間です。

Release \[s\]

:   リミッタのリリース時間です。

## チェンジログ
{%- for version, logs in changelog["RingModSpacer"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["RingModSpacer"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["RingModSpacer"] %}
- [RingModSpacer {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
RingModSpacer のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
