---
lang: ja
...

# EsPhaser
![](img/esphaser.png)

<ruby>EsPhaser<rt>エス フェイザ</rt></ruby> は最大 4096 の 2 次 Thiran オールパスフィルタを直列につなぐことができるフェイザです。 EnvelopedSine のフェイザと同じアルゴリズムを使っています。

- [EsPhaser {{ latest_version["EsPhaser"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["EsPhaser"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["EsPhaser"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["EsPhaser"] }})
{%- endif %}

EsPhaser の利用には AVX 以降の SIMD 命令セットをサポートする CPU が必要です。

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit

{{ section["macos_warning"] }}

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
サンプリング周波数によって出力が変わります。

`Stage` を最大にすると CPU 消費がとても大きくなります。

`Smooth` の値が小さいときに `Cas. Offset` を変更すると、大きな音が出ることがあります。 `Cas. Offset` を変更するときは <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> の利用と、 EsPhaser のあとにリミッタをインサートすることを推奨します。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/esphaser.svg)

## パラメータ
Mix

:   Dry : Wet の信号比です。右いっぱいにすると Dry : Wet = 0 : 1 になります。

Freq

:   LFO の周波数です。

Spread

:   16 個の LFO の間で周波数をずらす量です。

    1 サンプル当たりの LFO の位相の進み `deltaPhase` は次の式で計算されます。

    ```
    deltaPhase = 2 * pi * Freq / ((1 + LfoIndex * Spread) * sampleRate)
    ```

Feedback

:   フィードバックの大きさです。 12 時にするとフィードバックなし、左に回すと負のフィードバック、右に回すと正のフィードバックとなります。

Range

:   LFO によって変更するオールパスフィルタの特性の幅です。

Min

:   LFO によって変調するオールパスフィルタの特性の最小値です。

Cas. Offset

:   16 個の LFO の間で位相をずらす量です。

L/R Offset

:   左右の LFO の位相差です。

Phase

:   LFO の位相です。オートメーションで音を作りたいときに使えます。 `Freq` を左いっぱいに回すことで LFO の周波数を 0 にできます。

    最終的な LFO の位相は次の式で計算されます。

    ```
    LfoPhaseOffset = Phase + (L/R Offset) + LfoIndex * (Cas. Offset)
    ```

Stage

:   オールパスフィルタを直列につなぐ個数です。 `Stage` の値に比例して CPU 消費が増えるので注意してください。

Smooth

:   パラメータを変更したときに、変更前の値から変更後の値へと移行する秒数です。 `Stage` 以外のパラメータに有効です。

## チェンジログ
{%- for version, logs in changelog["EsPhaser"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["EsPhaser"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["EsPhaser"] %}
- [EsPhaser {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
EsPhaser のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
