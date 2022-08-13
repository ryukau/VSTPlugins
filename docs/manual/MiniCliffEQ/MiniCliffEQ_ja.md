---
lang: ja
...

# MiniCliffEQ
![](img/MiniCliffEQ.png)

<ruby>MiniCliffEQ<rt>ミニ クリフ イーキュー</rt></ruby> はタップ数 2^15 = 32768 の FIR フィルタです。初期バージョンからは改善しましたが、それでもタップ数が多いのでレイテンシがサンプリング周波数 48000 Hz のときに 0.34 秒を超えます。主な用途は直流信号 (DC) の抑制ですが、とても急峻なローパス、ハイパス、ローシェルフ、ハイシェルフフィルタとしても使えます。

- [MiniCliffEQ {{ latest_version["MiniCliffEQ"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["MiniCliffEQ"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["MiniCliffEQ"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["MiniCliffEQ"] }})
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
`2^15 / 2 - 1 = 16383` サンプルのレイテンシが加わります。 S/N 比はおよそ -120 dB です。

## 使い方
MiniCliffEQ の用途としては直流 (DC) の除去と、細かいノイズの検出を考慮しています。線形位相フィルタなのでバンドスプリッタとして使うこともできます。もちろん音を作る用途にも使えますが、レイテンシが大きいので通常のイコライザで足りる場面では使わないことを推奨します。

直流を除去するときは起動直後に `LP Gain` を 2 回 <kbd>ホイールクリック</kbd> して `-inf` に設定してください。 `Cutoff` は普通のスピーカであれば初期値の 20 Hz のままで問題ありません。 10 Hz を下回るとカットオフ周波数の正確さとロールオフの急峻さが損なわれる点に注意してください。 `Cutoff` が 1 Hz のときの振幅特性は 1 Hz で -16.25 dB 、 0.1 Hz で -60 dB です。

細かいノイズの検出を行うときは以下の手順を踏みます。

1. MiniCliffEQ の後にリミッタを挿入する。
2. `LP Gain` を `-inf` に設定する。
3. 入力信号が消えるまで `Cutoff` を上げる。 `Refresh FIR` を忘れずに！
4. `HP Gain` を `144.5` に設定する。

例えば上記の設定に 100 Hz のサイン波を入力して何らかの音が出るときはノイズが乗っていると言えます。ただしゲインを 144.5 dB 上げてようやく聞こえるノイズというのは人間の耳には知覚されることはまずありません。このようなノイズ検出は音の品質を探るというよりも、アルゴリズムを推定するときに使えます。

MiniCliffEQ のフィルタ係数を計算するアルゴリズムは以下の Python 3 のコードと同じです。

```python
import scipy.signal as signal
samplerate = 48000
cutoffHz = 20
fir = signal.firwin(2**15 - 1, cutoffHz, window="nuttall", fs=samplerate)
```

## パラメータ
Refresh FIR

:   `Cutoff` の変更後に `Refresh FIR` ボタンを押すことで FIR フィルタ係数を更新します。 `Refresh FIR` ボタンを押すと内部状態がリセットされて、レイテンシの長さだけ音が止まるので注意してください。

Cutoff \[Hz\]

:   フィルタのカットオフ周波数です。

HP Gain \[dB\]

:   ハイパス出力のゲインです。

LP Gain \[dB\]

:   ローパス出力のゲインです。

## チェンジログ
{%- for version, logs in changelog["MiniCliffEQ"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### MiniCliffEQ
{%- if old_download_link["MiniCliffEQ"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["MiniCliffEQ"] %}
- [MiniCliffEQ {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
MiniCliffEQ のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
