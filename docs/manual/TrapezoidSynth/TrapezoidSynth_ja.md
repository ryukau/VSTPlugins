---
lang: ja
...

# TrapezoidSynth
![](img/trapezoidsynth.png)

<ruby>TrapezoidSynth<rt>トラピゾイドシンセ</rt></ruby>は台形オシレータを使ったモノフォニックシンセサイザです。台形オシレータは PTR (Polynomial Transition Regions) という手法に基づいて作ったのですが、ピッチが高くなると逆にノイズが増える欠点があるので 8 倍のオーバーサンプリングをしています。コードを作るために 2 つの AM ピッチシフタを搭載しています。

- [TrapezoidSynth 0.1.11 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/TrapezoidSynth0.1.11.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/TrapezoidSynthPresets.zip)

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit
- macOS 64bit

Mac を持っていないので、 macOS ビルドはテストできていません。もしバグを見つけたときは [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか、 `ryukau@gmail.com` までメールを送っていただければ対応します。

Linux ビルドは Ubuntu 18.0.4 でビルドしています。また Bitwig と REAPER で動作確認を行っています。もし Ubuntu 18.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

## インストール
### プラグイン
名前が `.vst3` で終わるディレクトリを OS ごとに決められた位置に配置してください。

- Windows では `/Program Files/Common Files/VST3/` に配置します。
- Linux では `$HOME/.vst3/` に配置します。
- macOS では `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/` に配置します。

DAW によっては上記とは別に VST3 をインストールできるディレクトリを提供していることがあります。詳しくは利用している DAW のマニュアルを参照してください。

### プリセット
解凍して出てきたディレクトリを OS ごとに決められた位置に配置すると使えるようになります。

- Windows : `/Users/$USERNAME/Documents/VST3 Presets/Uhhyou`
- Linux : `$HOME/.vst3/presets/Uhhyou`
- macOS : `/Users/$USERNAME/Library/Audio/Presets/Uhhyou`

プリセットディレクトリの名前はプラグインと同じである必要があります。 `Uhhyou` ディレクトリが無いときは作成してください。

### Windows
プラグインが DAW に認識されないときは C++ redistributable をインストールしてみてください。インストーラは次のリンクからダウンロードできます。ファイル名は `vc_redist.x64.exe` です。

- [The latest supported Visual C++ downloads](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

### Linux
Ubuntu 18.0.4 では次のパッケージのインストールが必要です。

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

もし DAW がプラグインを認識しないときは、下のリンクの `Package Requirements` を参考にして VST3 に必要なパッケージがすべてインストールされているか確認してみてください。

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

REAPER の Linux 版がプラグインを認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

## 色の設定
初回設定時は手動で次のファイルを作成してください。

- Windows では `/Users/ユーザ名/AppData/Roaming/UhhyouPlugins/style/style.json` 。
- Linux では `$XDG_CONFIG_HOME/UhhyouPlugins/style/style.json` 。
  - `$XDG_CONFIG_HOME` が空のときは `$HOME/.config/UhhyouPlugins/style/style.json` 。
- macOS では `/Users/$USERNAME/Library/Preferences/UhhyouPlugins/style/style.json` 。

既存の色のテーマを次のリンクに掲載しています。 `style.json` にコピペして使ってください。

- [LV2Plugins/style/themes at master · ryukau/LV2Plugins · GitHub](https://github.com/ryukau/LV2Plugins/tree/master/style/themes)

`style.json` の設定例です。

```json
{
  "fontPath": "",
  "foreground": "#000000",
  "foregroundButtonOn": "#000000",
  "foregroundInactive": "#8a8a8a",
  "background": "#ffffff",
  "boxBackground": "#ffffff",
  "border": "#000000",
  "borderCheckbox": "#000000",
  "borderLabel": "#000000",
  "unfocused": "#dddddd",
  "highlightMain": "#0ba4f1",
  "highlightAccent": "#13c136",
  "highlightButton": "#fcc04f",
  "highlightWarning": "#fc8080",
  "overlay": "#00000088",
  "overlayHighlight": "#00ff0033"
}
```

16 進数カラーコードを使っています。

- 6 桁の色は RGB 。
- 8 桁の色は RGBA 。

プラグインはカラーコードの 1 文字目を無視します。よって `?102938` や `\n11335577` も有効なカラーコードです。

2 文字目以降のカラーコードの値に `0-9a-f` 以外の文字を使わないでください。

以下は設定できる色の一覧です。設定に抜けがあるとデフォルトの色が使われます。

- `fontPath`: フォント (*.ttf) の絶対パス。VST 3 版では実装されていません。
- `foreground`: 文字の色。
- `foregroundButtonOn`: オンになっているボタンの文字の色。 `foreground` か `boxBackground` のいずれかと同じ値にすることを推奨します。
- `foregroundInactive`: 非アクティブなタブの文字の色。
- `background`: 背景色。x
- `boxBackground`: 矩形の UI 部品の内側の背景色。
- `border`: <ruby>縁<rt>ふち</rt></ruby>の色。
- `borderCheckbox`: チェックボックスの縁の色。
- `borderLabel`: パラメータセクションのラベルの左右の直線の色。
- `unfocused`: つまみがフォーカスされていないときの色。
- `highlightMain`: フォーカスされたときの色。スライダの値の表示にも使用されます。
- `highlightAccent`: フォーカスされたときの色。一部のプラグインをカラフルにするために使用されます。
- `highlightButton`: ボタンがフォーカスされたときの色。
- `highlightWarning`: 変更に注意を要する UI がフォーカスされたときの色。
- `overlay`: オーバーレイの色。
- `overlayHighlight`: フォーカスを示すオーバーレイの色。

## 操作
つまみとスライダーでは次の操作ができます。

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

## 注意
`SlideType` を `Reset to 0` にすると極端に低い周波数が出るので、ハイパスフィルタをかけることを推奨します。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/trapezoidsynth.svg)

## パラメータ
### Oscillator 共通
Semi

:   ピッチを半音単位で変更します。

Cent

:   ピッチをセント単位で変更します。 100 セント = 1 半音です。

Slope

:   台形の両端の傾きです。

PW

:   台形の上辺の長さです。 Pulse Width の略です。

### Oscillator 1
Drift

:   ホワイトノイズでピッチを変調します。

Feedback

:   Oscillator 1 の位相をフィードバックで変調します。

    ```
    osc1Phase += Feedback * (osc1 + OscMix * (osc2 - osc1))
    ```

### Oscillator 2
Overtone

:   周波数を `Overtone` 倍にします。

PM

:   Oscillator 1 の位相を Oscillator 2 の出力で変調します。

### Envelope 共通
Retrigger

:   チェックを入れると鍵盤が押されるたびにエンベロープをリセットします。チェックが外れているときは、同時に押している鍵盤の数が 0 から 1 以上になったときだけエンベロープをリセットします。

A

:   アタック時間の秒数です。アタックは音が最大音量に到達するまでのエンベロープの区間です。

D

:   ディケイ時間の秒数です。ディケイはアタックが終わった後にエンベロープが減衰する区間です。

S

:   サステインの大きさです。サステインはディケイが終わったのエンベロープの出力の大きさです。

R

:   リリース時間の秒数です。エンベロープは、鍵盤から指を離した時点（ノートオフ）からリリース状態に移行して、出力が 0 になるまで減衰します。

Curve

:   エンベロープの特性を変更します。

### Gain Envelope
Gain

:   出力音量です。

### Filter
Order

:   フィルタの次数です。 TrapezoidSynth では 1 次フィルタを 8 個直列につないでいます。 `Order` の値によってフィルタから信号を取り出す位置を変更しています。

Cut

:   カットオフ周波数です。

Res

:   レゾナンスの強さです。

Sat

:   フィルタのサチュレーションの強さです。

    ```
    filterInput = tanh(Sat * (input + Res * filterOutput))
    ```

Env>Cut

:   Filter Envelope による `Cut` の変調量です。

Key>Cut

:   鍵盤の高さによる `Cut` の変調量です。

+OscMix

:   Oscillator 1 と Oscillator 2 をミックスした信号による `Cut` の変調量です。

### Filter Envelope
\>Octave

:   Filter Envelope の出力を使ってピッチをオクターブ単位で変調する量です。

### Misc
OscMix

:   Osillator 1 と Oscillator 2 の比率です。左いっぱいに回すと Oscillator 1 の音だけになります。

Octave

:   ピッチをオクターブ単位で変更します。

Smooth

:   パラメータを変更したときに変更前の値から変更後の値に移行する秒数です。次のパラメータに影響します。

    - `Drift`
    - `Slope`
    - `PW` （同名のパラメータすべて）
    - `Feedback` （同名のパラメータすべて）
    - `PM`
    - `Gain` （同名のパラメータすべて）
    - `Cut`
    - `Res`
    - `Sat`
    - `Env>Cut`
    - `Key>Cut`
    - `+OscMix`
    - `OscMix`
    - `>PM`
    - `>Feedback`
    - `>LFO`
    - `>Slope2`
    - `>Shifter1`
    - `Shifter1.Semi`
    - `Shifter1.Cent`
    - `Shifter2.Semi`
    - `Shifter2.Cent`
    - `Freq`
    - `Shape`
    - `>Pitch1`
    - `>Slope1`
    - `>PW1`
    - `>Cut`

### Mod 共通
Retrigger

:   チェックを入れると鍵盤が押されるたびにエンベロープをリセットします。チェックが外れているときは、同時に押している鍵盤の数が 0 から 1 以上になったときだけエンベロープをリセットします。

Attack

:   モジュレーションエンベロープのアタック時間の秒数です。

Curve

:   モジュレーションエンベロープの特性です。

### Mod 1
\>PM

:   `Oscillator2.PM` をモジュレーションエンベロープ 1 で変調します。

### Mod 2
\>Feedback

:   `Oscillator1.Feedback` をモジュレーションエンベロープ 2 で変調します。

\>LFO

:   `LFO.Freq` をモジュレーションエンベロープ 2 で変調します。

\>Slope2

:   `Oscillator2.Slope` をモジュレーションエンベロープ 2 で変調します。

\>Shifter1

:   Shifter 1 のピッチをモジュレーションエンベロープ 2 で変調します。

### Shifter 共通
Semi

:   半音単位でピッチシフトの量を変更します。

Cent

:   セント単位でピッチシフトの量を変更します。

Gain

:   ピッチシフトした信号を足し合わせる大きさです。

### LFO
LFOType

:   LFO の波形の種類を次の 4 つから選択できます。

    - `Sin` : サイン波です。 出力は `Shape` 乗されます。
    - `Saw` : 鋸歯波です。 `Shape` によって三角波にもできます。
    - `Pulse` : 矩形波です。 `Shape` によってデューティ比を変更できます。
    - `Noise` : ブラウンノイズです。 `Shape` によってステップあたりの移動量を変更できます。

Tempo

:   LFO の周波数をテンポシンクします。

Freq

:   LFO の周波数です。

Shape

:   LFO の波形を変更します。

\>Pitch

:   LFO による Oscillator 1 のピッチの変調量です。

\>Slope1

:   LFO による `Oscillator1.Slope` の変調量です。

\>PW1

:   LFO による `Oscillator1.PW` の変調量です。

\>PW1

:   LFO による `Filter.Cut` の変調量です。

### Slide
SlideType

:   ピッチスライドの種類を変更できます。

    - `Always` : 常に最後に演奏されたノートのピッチからスライドします。
    - `Sustain` : 2 つ以上の鍵盤が同時に押されたときだけスライドします。
    - `Reset to 0` : 同時に押している鍵盤の数が 0 から 1 、あるいは 1 から 0 になるときは 0 Hz にスライドします。それ以外の場合は他の 2 つと同じようにスライドします。

Time

:   ピッチスライドにかかる時間です。

Offset

:   Oscillator 1 のスライド時間に対する Oscillator 2 のスライド時間の比率です。

## チェンジログ
- 0.1.11
  - Process context requirements を実装。
  - `Slide->Time` の値が無視されて、 `Smooth` の値を使うようになっていたバグを修正。1.10 では正しく修正されていなかった。
- 0.1.10
  - DSP が初期化されているかどうかのチェックを追加。
  - `Slide->Time` の値が無視されて、 `Smooth` の値を使うようになっていたバグを修正。
- 0.1.9
  - カラーコンフィグを追加。
- 0.1.8
  - パラメータの補間を可変サイズのオーディオバッファでも機能する以前の手法に巻き戻した。
- 0.1.7
  - 文字列の描画でクラッシュするバグを修正。
- 0.1.6
  - プラグインタイトルをクリックすると表示されるポップアップの表示方法の変更。
- 0.1.5
  - PreSonus Studio One 4.6.1 で出力にノイズが乗るバグを修正。
- 0.1.4
  - Linux ビルドの GUI を有効化。
  - プラグインの分類を `kFxInstrument` から `kInstrumentSynth` に修正。
- 0.1.3
  - Ableton Live 10.1.6 で特定のノブが揺れ戻るバグを修正。
- 0.1.2
  - 複数の GUI インスタンス間で表示を同期するように変更。
  - スプラッシュスクリーンを開くとクラッシュするバグを修正。
  - GUI がホストのオートメーションに応じて更新されるように修正。
- 0.1.1
  - オシレータの実装の誤りを修正。
- 0.1.0
  - 初期リリース。

### 旧バージョン
- [TrapezoidSynth 0.1.10 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/TrapezoidSynth0.1.10.zip)
- [TrapezoidSynth 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/TrapezoidSynth0.1.9.zip)
- [TrapezoidSynth 0.1.8 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/TrapezoidSynth0.1.8.zip)
- [TrapezoidSynth 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/TrapezoidSynth0.1.7.zip)
- [TrapezoidSynth 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/TrapezoidSynth0.1.5.zip)
- [TrapezoidSynth 0.1.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/TrapezoidSynth0.1.4.zip)
- [TrapezoidSynth 0.1.3 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/TrapezoidSynth0.1.3.zip)
- [TrapezoidSynth 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/TrapezoidSynth0.1.2.zip)

## ライセンス
TrapezoidSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
