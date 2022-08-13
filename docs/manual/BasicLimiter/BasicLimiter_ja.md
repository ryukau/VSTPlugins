---
lang: ja
...

# BasicLimiter
![](img/BasicLimiter.png)

<ruby>BasicLimiter<rt>ベーシック リミッタ</rt></ruby> は名前の通りベーシックなシングルバンドリミッタです。目新しい音は出ませんが、トゥルーピークモードはやや贅沢に設計しています。

- [BasicLimiter {{ latest_version["BasicLimiter"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["BasicLimiter"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["BasicLimiter"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["BasicLimiter"] }})
{%- endif %}

自動メイクアップゲイン、サイドチェイン、左右 (L-R) とミッド-サイド (M-S) の切り替えを追加した BasicLimiterAutoMake もあります。ただし CPU 負荷は 1.5 倍強に上がります。

- [BasicLimiterAutoMake {{ latest_version["BasicLimiterAutoMake"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["BasicLimiterAutoMake"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["BasicLimiterAutoMake"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["BasicLimiterAutoMake"] }})
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
アルゴリズムの問題で、入力振幅がとんでもなく大きいと過剰に振幅が制限されることがあります (オーバーリミッティング) 。オーバーリミッティングが起こると、入力振幅が大きくなるほど出力振幅が小さくなります。振幅が `2^53` 、つまり約 319 dB を超えたときに問題が起こることが想定されます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/BasicLimiter.svg)

## パラメータ
Threshold \[dB\]

:   リミッタのしきい値です。

    `True Peak` がオフのとき、出力振幅はしきい値以下に制限されます。

    `True Peak` がオンのとき、出力振幅は完全にしきい値以下に制限されません。これはリアルタイムで使えるアルゴリズムには限界があるからです。

Gate \[dB\]

:   ゲートのしきい値です。 `Attack` で指定した時間を超えて入力振幅が `Gate` の値を下回ると、出力振幅が 0 になります。

Attack \[s\]

:   リミッタのエンベロープを滑らかにするフィルタの遷移時間です。 `Attack` の値はそのままレイテンシに加算されます。

    ドラムなどの振幅の変化が急峻な音にかけるときは 0.02 秒以下に設定することを推奨します。 0.02 秒という値は[継時マスキング](https://web.archive.org/web/20210624083625/http://ccrma.stanford.edu:80/~bosse/proj/node21.html)という人間の耳の聞こえ方の性質に基づいています。

Release \[s\]

:   リミッタのエンベロープが振幅を制限しない状態に戻るときの滑らかさです。

    内部では `Release` で設定された時間の逆数をカットオフ周波数として使っているので、表示されている値はあくまでも目安です。普段は `Sustain` と足し合わせた値が 0.2 秒以下になるよう設定することを推奨します。この値は継時マスキングに基づいています。

Sustain \[s\]

:   リミッタのエンベロープが `Attack` で設定した時間に加えて振幅のピークをホールドする時間です。

    ドラムなどにかけると `Release` よりもダッキングが目立つ傾向があります。歪ませたギターやサステイン中のシンセサイザなど、振幅が一定な音に対してはオートゲインと同じように働くので `Release` よりクリーンな出力が得られます。普段は `Sustain` と足し合わせた値が 0.2 秒以下になるよう設定することを推奨します。サステインを長くすると継時マスキングの曲線から乖離するので注意してください。

Stereo Link

:   `Stereo Link` が 0.0 のとき、左右のリミッタは独立して動作します。 1.0 のとき、すべてのチャンネルの振幅の最大値がすべてのリミッタに入力されます。以下は `Stereo Link` の計算式です。

    ```
    absL = fabs(leftInput).
    absR = fabs(rightInput).

    absMax = max(absL, absR).

    amplitudeL = absL + stereoLink * (absMax - absL).
    amplitudeR = absR + stereoLink * (absMax - absR).
    ```

    細かく設定するときは、まず `Stereo Link` を 0.0 に設定します。すると左右のどちらかに偏った信号が入力されたときに、中央に位置する信号が押し出されて、パンがふらつくように聞こえることがあります。このようなパンのふらつきが聞き取れなくなるまで `Stereo Link` の値を少しづつ増やしていけば、そのうち適切な値にたどり着きます。

True Peak

:   チェックを入れるとトゥルーピークモードがオンになります。トゥルーピークモードがオンのときは 94 サンプルのレイテンシが加わります。

    トゥルーピークモードがオンのときはナイキスト周波数に近い成分を落とすためのローパスフィルタがかかります。サンプリング周波数が 48000 Hz のときに、およそ 18000 Hz 以下の成分についてはローパスフィルタの影響が及ばないように設計しています。

    サンプルピークが `Threshold` で指定した値を超えることがあります。特にサンプルピークが 0 dB を超えたときは `Overshoot` の値が 0 でなくなります。このときは `Threshold` の値を下げてください。

    ナイキスト周波数のトゥルーピークの復元には無限の長さの FIR フィルタが必要です (sinc 補間) 。したがってリアルタイムでの厳密な復元は不可能です。ローパスをかけていたり、トゥルーピークモードでオーバーシュートが起こったりと問題点があるのは、この無理をなんとかしようとしていることが原因です。

Reset Overshoot

:   クリックすると `Overshoot` の値を 0 にリセットします。

    出力のサンプルピークが 0 dB を超えると `Overshoot` の値が 0 でなくなり、 `Reset Overshoot` が点灯します。

### BasicLimiterAutoMake の固有パラメータ
Auto Make Up

:   チェックを入れると自動メイクアップゲインが有効になります。

    自動メイクアップゲインが有効のときに `Threshold` の値が `Auto Make Up Target Gain` を超えると出力振幅が小さくなります。

    自動メイクアップゲインが有効のときに、手やオートメーションで `Threshold` を上げると、オーバーシュートすることがあります。オーバーシュートでの歪みを防ぐためには `Auto Make Up Target Gain` を -0.1 dB 以下に設定することを推奨します。

    入力信号があるときに `Threshold` を動かすのであれば BasicLimiterAutoMake の後にお守りとしてさらにもう一つリミッタを挿入することを推奨します。

Auto Make Up Target Gain

:   自動メイクアップゲインが適用された後の最大振幅です。 `Auto Make Up` の右側に配置されているコントロールで、単位はデシベルです。

    `Channel Type` が `M-S` のときは設定値の +6 dB が最大振幅となります。

Sidechain

:   チェックを入れるとサイドチェインが有効になります。サイドチェインが有効のとき、ソース信号の振幅は `Threshold` に影響されないので `Auto Make Up` は無効になります。

    BasicLimiterAutoMake は 2 つのステレオ入力を備えています。 1 番がソース信号、 2 番がサイドチェイン信号の入力です。ルーティングについてはお使いの DAW のマニュアルを参照してください。

Channel Type

:   ステレオチャンネルの種類を左右 (`L-R`) とミッド-サイド (`M-S`) で切り替えます。

    種類を `M-S` に設定すると、サンプルピークが `Threshold` の 2 倍 (+6.02 dB) に達することがあります。そこで `Auto Make Up` を有効にして `M-S` を使うときは、 `Auto Make Up Target Gain` の値を -6.1 dB 以下に設定することを推奨します。この仕様は `L-R` と `M-S` を切り替えて比較するときに聴感上の音の大きさが同じになることを狙っています。

## チェンジログ
### BasicLimiter
{%- for version, logs in changelog["BasicLimiter"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

### BasicLimiterAutoMake
{%- for version, logs in changelog["BasicLimiterAutoMake"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### BasicLimiter
{%- if old_download_link["BasicLimiter"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["BasicLimiter"] %}
- [BasicLimiter {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

### BasicLimiterAutoMake
{%- if old_download_link["BasicLimiterAutoMake"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["BasicLimiterAutoMake"] %}
- [BasicLimiterAutoMake {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
BasicLimiter のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
