---
lang: ja
...

# SevenDelay
![](img/SevenDelay.png)

<ruby>SevenDelay<rt>セブン ディレイ</rt></ruby> は7次のラグランジュ補間による分数ディレイと7倍のオーバーサンプリングを使ったステレオディレイです。

{% for target, download_url in latest_download_url["SevenDelay"].items() %}
- [SevenDelay {{ latest_version["SevenDelay"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["SevenDelay"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["SevenDelay"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

## 注意
`Smooth` の値が 0 に近く、 `Allpass Cut` の値を速く動かしすぎたときに、非常に大きな音量のクリックノイズが出力されることがあります。これは `Allpass Cut` で使われているフィルタのカットオフ周波数が、速く、大きく、動きすぎると出力が発散する場合があることが原因です。特別な目的がない限り、`Smooth` パラメータを低く設定しすぎないことを推奨します。

`Time` の値が最小かつ `Feedback` の値が最大に設定されると直流信号が出ることがあります。 `DC Kill` を 1.0 より大きな値に設定することで直流信号を除去できます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/SevenDelay.svg)

## パラメータ
### Delay
Time

:   ディレイ時間。範囲は 0.0001 から 8.0 です。

    - もし `Sync` が有効で `Time` が 1.0 より小さいときは、ディレイ時間が `Time / 16` 拍に設定されます。
    - もし `Sync` が有効で `Time` が 1.0 以上のときは、ディレイ時間が `floor(2 * Time) / 32` 拍に設定されます。
    - それ以外のときは、ディレイ時間が `time` 秒に設定されます。

Feedback

:   ディレイのフィードバック。範囲は 0.0 から 1.0 です。

Stereo

:   左右のディレイ時間のオフセット。範囲は -1.0 から 1.0 です。

    - もし `Stereo` が 0.0 より小さいときは、左チャンネルのディレイ時間が `timeL * (1.0 + Stereo)` に変更されます。
    - それ以外のときは、右チャンネルのディレイ時間が `timeR * (1.0 - Stereo)` に変更されます。

Wet

:   ディレイ信号の出力音量。範囲は 0.0 から 1.0 。

Dry

:   入力信号の出力音量。範囲は 0.0 から 1.0 。

Sync

:   テンポシンクの切り替え。

Negative

:   負のフィードバックの切り替え。ディレイ時間がとても短いときに役立つかもしれません。

Spread/Pan

:   入力の広がり (In Spread) 、入力のパン (In Pan) 、出力の広がり (Out Spread) 、出力のパン (Out Pan) 。範囲は 0.0 から 1.0 です。

    `In Spread` 、 `Out Spread` はステレオの広がりを制御します。 `In Pan` 、 `Out Pan` はステレオのパンニングを制御します。

    これらのパラメータはパンニングの逆転やピンポンディレイを作るときに使えます。

    - パンニングの逆転を行うには `[InSpread, InPan, OutSpread, OutPan]` を `[0.0, 0.5, 1.0, 0.5]` に設定します。
    - ピンポンディレイにするには `[InSpread, InPan, OutSpread, OutPan]` を `[1.0, 0.5, 0.0, 0.5]` に設定します。

    ```
    signalL = inL + spread * (inR - inL)
    signalR = inL + (1.0f - spread) * (inR - inL)

    if (pan < 0.5f) {
      outL = (0.5f + pan) * signalL + (0.5f - pan) * signalR,
      outR = signalR * 2.0f * pan,
    }
    else {
      outL = signalL * (2.0f - 2.0f * pan)
      outR = (pan - 0.5f) * signalL + (1.5f - pan) * signalR
    }
    ```

Allpass Cut

:   SVF オールパスフィルタのカットオフ周波素。範囲は 90.0 から 20000.0 です。

    `tone` が 20000.0 のとき、フィルタはバイパスされます。

Allpass Q

:   SVF オールパスフィルタのレゾナンス。範囲は 0.00001 から 1.0 です。

    値が大きいほどレゾナンスが強くなります。

DC Kill

:   ハイパスフィルタのカットオフ周波数。範囲は 5.0 から 120.0 です。

    `DC Kill` を 5.0 より大きく設定すればディレイのフィードバックから直流信号を取り除くことができます。

Smooth

:   パラメータ平滑化の度合い。範囲は 0.0 から 1.0 で、単位は秒です。

    パラメータによっては値が急激に変化するとノイズが出ることがあります。 `Smooth` の値を大きめにすることで、値の変化を緩やかにしてノイズを減らすことができます。

### LFO
To Time

:   LFO によるディレイ時間の変調量。範囲は 0.0 から 1.0 です。

To Allpass

:   LFO によるオールパスフィルタのカットオフ周波数の変調量。範囲は 0.0 から 1.0 です。

Frequency

:   LFO の周波数。範囲は 0.01 から 100.0 。

Shape

:   LFO の波形。範囲は 0.01 から 10.0 。

    ```
    sign = 1 if (phase > π),
          -1 if (phase < π),
           0 if (phase == π)
    lfo = sign * abs(sin(phase))^shape
    ```

Phase

:   LFO の位相の初期値。範囲は 0.0 から 2π 。

    LFO の位相はホストが演奏を開始するたびに `Phase` の値にリセットされます。

Hold

:   LFO の位相のホールドの切り替え。ライブ演奏などで役に立つかもしれません。

## チェンジログ
{%- for version, logs in changelog["SevenDelay"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
{%- if old_download_link["SevenDelay"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["SevenDelay"] %}
- [SevenDelay {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
SevenDelay のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
