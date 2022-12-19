---
lang: ja
...

# L4Reverb
![](img/L4Reverb.png)

<ruby>L4Reverb<rt>エル フォー リバーブ</rt></ruby> は LatticeReverb の拡張版です。今回の格子構造はチャンネル毎に 4 * 4 * 4 * 4 = 256 のセクションが設けてあります。

{% for target, download_url in latest_download_url["L4Reverb"].items() %}
- [L4Reverb {{ latest_version["L4Reverb"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["L4Reverb"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["L4Reverb"] }})
{%- endif %}

もし L4Reverb が重たすぎるときは L3Reverb を試してみてください。 <ruby>L3Reverb<rt>エル スリー リバーブ</rt></ruby> は L4Reverb の軽量版です。格子構造はチャンネル毎に 3 * 3 * 3 * 5 = 135 のセクションが設けてあります。

{% for target, download_url in latest_download_url["L3Reverb"].items() %}
- [L3Reverb {{ latest_version["L3Reverb"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["L3Reverb"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["L3Reverb"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## 注意
サンプリング周波数やバッファサイズによって出力が変わります。

`Cross` と `Spread` の両方が 0 でないときに発散することがあります。

`*Feed` の `offset` が 0 でないときに出力が大きくなることがあります。

次の手順に沿った操作が行われると出力が大きくなることがあります。

1. `OuterFeed` あるいは `InnerFeed` のいくつかを最大値あるいは最小値の近くに設定。
2. 信号を入力。
3. 手順 1. で設定した `OuterFeed` あるいは `InnerFeed` の値を変更する。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/L4Reverb.svg)

## パラメータ

Time

:   オールパスフィルタのディレイ時間です。

InnerFeed

:   格子構造の内側に入れ子になったオールパスフィルタのフィードバック、フィードフォワードの値です。

D1Feed

:   格子構造の入れ子の最下層のフィードバック、フィードフォワードの値です。

D2Feed

:   格子構造の入れ子の底から 2 番目の階層でのフィードバック、フィードフォワードの値です。

D3Feed

:   格子構造の入れ子の底から 3 番目の階層でのフィードバック、フィードフォワードの値です。

D4Feed

:   格子構造の入れ子の最上層でのフィードバック、フィードフォワードの値です。

**Tip**: フィードバック、フィードフォワードの値をすべて負の値にすることで、入力信号の音がぼやけてウェットな質感になります。特に `D4Feed` をうまく調整してみてください。

### Multiplier
`Time` や `*Feed` の値をまとめて変更する係数です。減衰時間をまとめて変えるときなどに使えます。

`Base` は左右のチャンネルで両方で使われる値です。 `Base` によってリバーブの大まかな質感が決まります。

`Offset` は左右のチャンネルの値の比率です。 `Offset` によって左右の広がりを作ることができます。

```
if (Offset >= 0) {
  valueL = Base
  valueR = Base * (1 - Offset)
}
else {
  valueL = Base * (1 + Offset)
  valueR = Base
}
```

`Modulation` にチェックが入っているとき、 `Offset` の値はバッファごとにランダマイズされます。 `Smooth` でモジュレーションの質感を変えることができます。

### Panic!
ボタンを押すと `Time` と `*Feed` の Multiplier を 0 にしてリバーブの出力を止めます。

出力が想定外に大きくなったときなど、音を手早く止めたいときに使えます。

### Mix
Dry

:   バイパスされる入力信号の音量です。

Wet

:   リバーブを通った信号の音量です。

### Stereo
Cross

:   あるチャンネルから、もう片方のチャンネルに格子構造の出力をフィードバックする量です。 0 のときはもう片方のチャンネルからの信号はミックスされません。

Spread

:   ミッドとサイド (M-S) の信号の比率です。

    ミッドとサイドの信号は次の式で計算されます。

    ```
    mid  = left + right
    side = left - right

    left  = mid - Spread * (mid - side)
    right = mid - Spread * (mid + side)
    ```

### Misc.
Seed

:    疑似乱数列のシード値です。

Smooth

:   パラメータを変更したときに、変更前の値から変更後の値へと移行する大まかな秒数です。

## チェンジログ
### L4Reverb
{%- for version, logs in changelog["L4Reverb"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

### L3Reverb
{%- for version, logs in changelog["L3Reverb"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### L4Reverb
{%- if old_download_link["L4Reverb"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["L4Reverb"] %}
- [L4Reverb {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

### L3Reverb
{%- if old_download_link["L3Reverb"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["L3Reverb"] %}
- [L3Reverb {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
L4Reverb と L3Reverb のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
