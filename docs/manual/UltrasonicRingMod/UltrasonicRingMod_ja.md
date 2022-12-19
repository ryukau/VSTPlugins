---
lang: ja
...

# UltrasonicRingMod
![](img/UltrasonicRingMod.png)

<ruby>UltrasonicRingMod<rt>ウルトラソニック リング モッド</rt></ruby> は 64 倍のオーバーサンプリングを行うリングモジュレータで、最高 1 MHz のサイン波による変調をかけることができます。とは言っても単に変調すると可聴域外なので何も聞こえなくなってしまいますが、フィードバックによる位相変調やハードクリッピングができるので、いくらかノイズが出せます。

{% for target, download_url in latest_download_url["UltrasonicRingMod"].items() %}
- [UltrasonicRingMod {{ latest_version["UltrasonicRingMod"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["UltrasonicRingMod"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["UltrasonicRingMod"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
`Mod. Scale` を右へ回すと `Frequency` の高さに比例して振幅が上がります。オーバーサンプリングの影響でやや低めになりますが、振幅が最大で 120 dB 上がるので、リミッタの使用を推奨します。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/UltrasonicRingMod.svg)

## パラメータ
Mix

:   入力と変調された信号を混ぜる割合です。

    信号を混ぜる処理はハードクリッピングの直前に行われます。つまり `Mix` が左いっぱいで入力をバイパスしているときであっても、 `Hardclip` が左いっぱいでなければ、歪みが加えられる可能性があります。

    また、マルチレート処理、つまりオーバーサンプリングによる音の変化はバイパスされません。完全なバイパスを行うときは DAW 側のミキサーを使ってください。

Pre Clip

:   `Hardclip` の直前に乗算されるゲインです。歪みを加えたいときに使えます。

Out

:   出力ゲインです。

    最大値は +60 dB とやや高めなので注意してください。

Frequency

:   変調波の周波数です。

DC Offset

:   変調波に加える直流信号 (direct current, DC) の量です。

    変調波の振幅の範囲は、左いっぱいで `[-1, 1]` 、右いっぱいで `[0, 1]` になります。

Feedback

:   リングモジュレータを通過した信号を変調波の位相へとフィードバックする量です。

Mod. Scale

:   変調波の振幅を `Frequency` の値に比例させる割合です。

Mod. Wrap

:   変調波と、変調波を振幅 1.0 で巻き戻した信号を混ぜる割合です。

    1.0 で巻き戻す計算は `fmod(value, 1.0)` と同じです。

Hardclip

:   リングモジュレータの出力と、リングモジュレータからの出力をハードクリップした信号を混ぜる割合です。

Smoothing \[s\]

:   パラメータの値を変えたときに、変更先の値にたどり着くまでの大まかな時間です。

    内部的には `Smoothing` の値の逆数を平滑化フィルタのカットオフ周波数として使っているので、厳密な時間を表しているわけではない点に注意してください。

### Note
ノートオン・ノートオフによって変調波の周波数を変えることができます。ノートイベントをエフェクトに送る方法については、使用している DAW のマニュアルを参照してください。

Negative, Scale

:   `Scale` は入力されたノートのピッチに掛け合わされる値です。 `Negative` をクリックして点灯させると `Scale` が負の値として内部的に扱われるようになります。

    ノートの音程は C4 を中心とした比率に置き換えられて `Frequency` の値に乗算されます。以下は計算方法です。

    ```
    modulatorFrequency = frequency * 2^((midiNote + offset - 69) / 12).
    ```

Offset \[st.\]

:   入力されたノート番号に加算される値です。単位は半音 (semitone, st.) です。

Slide Time \[s\]

:   ひとつ前の音程から、新しく入力された音程に向かってピッチをスライドさせる大まかな時間です。

    内部的には `Slide Time` の値の逆数を平滑化フィルタのカットオフ周波数として使っているので、厳密な時間を表しているわけではない点に注意してください。

## チェンジログ
{%- for version, logs in changelog["UltrasonicRingMod"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### UltrasonicRingMod
{%- if old_download_link["UltrasonicRingMod"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["UltrasonicRingMod"] %}
- [UltrasonicRingMod {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
UltrasonicRingMod のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
