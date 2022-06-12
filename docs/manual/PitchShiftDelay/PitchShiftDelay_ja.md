---
lang: ja
...

# PitchShiftDelay
![](img/pitchshiftdelay.png)

<ruby>PitchShiftDelay<rt>ピッチ シフト ディレイ</rt></ruby> はディレイを使った時間領域ピッチシフタです。 16 倍のオーバーサンプリングによってピッチシフタ特有の癖を多少抑えています。また内部のバッファ長をリアルタイムで変更できます。フォルマント補正がないので、声に使うとヘリウムを吸ったときのような音になります。

- [PitchShiftDelay 0.2.0 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.34.0/PitchShiftDelay_0.2.0.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
<!-- - [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.34.0/PitchShiftDelayPresets.zip) -->

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit
- macOS universal binary

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

## 連絡先
何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか `ryukau@gmail.com` までお気軽にどうぞ。

[paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau) から開発資金を投げ銭することもできます。現在の目標はよりよい macOS サポートのための M1 mac の購入資金を作ることです。 💸💻

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

もし DAW がプラグインを認識しないときは、下のリンクの `Package Requirements` を参考にして VST3 に必要なパッケージがすべてインストールされているか確認してください。

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

REAPER の Linux 版がプラグインを認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

### macOS
**注意**: この節は英語で macOS を使用しているユーザからの報告によって作成されました。日本語でのエラーメッセージが分からなかったので曖昧な書き方になっています。

プラグインの初回起動時に「破損している」という趣旨のメッセージが表示されることがあります。この場合は、ターミナルを開いて、解凍した `.vst3` ディレクトリに次のコマンドを適用してみてください。

```sh
xattr -rc /path/to/PluginName.vst3
```

プラグインは署名されていない (unsigned) 、または公証されていない (un-notarized) アプリケーションとして認識されることがあります。この場合は以下の手順を試してみてください。

1. ターミナルを開いて `sudo spctl --master-disable` を実行。
2. システム環境設定 → セキュリティとプライバシー → 一般 → ダウンロードしたアプリケーションの実行許可、を開いて 「全てのアプリケーションを許可」 を選択。

上記の手順を実行するとシステムのセキュリティが弱くなるので注意してください。元に戻すには以下の手順を実行してください。

1. システム環境設定 → セキュリティとプライバシー → 一般 → ダウンロードしたアプリケーションの実行許可、を開いて 「App Store と認証済みの開発元からのアプリケーションを許可」 を選択。
2. ターミナルを開いて `sudo spctl --master-enable` を実行。

#### 参考リンク
- [How to Fix App “is damaged and can’t be opened. You should move it to the Trash” Error on Mac](https://osxdaily.com/2019/02/13/fix-app-damaged-cant-be-opened-trash-error-mac/)
- [Allowing unsigned/un-notarized applications/plugins in Mac OS | Venn Audio](https://www.vennaudio.com/allowing-unsigned-un-notarized-applications-plugins-in-mac-os/)
- [Safely open apps on your Mac - Apple Support](https://support.apple.com/en-us/HT202491)

## GUI の見た目の設定
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
  "fontFamily": "Tinos",
  "fontBold": true,
  "fontItalic": true,
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

### フォントオプション
以下はフォントオプションの一覧です。

- `fontFamily`: フォントファミリ名。
- `fontBold`: ボールドスタイル (太字) を `true` で有効、 `false` で無効。
- `fontItalic`: イタリックスタイル (斜体) を `true` で有効、 `false` で無効。

カスタムフォントを使用するには、プラグインディレクトリの `*.vst3/Contents/Resources/Fonts` に `*.ttf` ファイルを配置します。

**注意**: `fontFamily` 、 `fontBold` 、 `fontItalic` で設定したフォントファミリ名とスタイルの組み合わせが `*.vst3/Contents/Resources/Fonts` 以下のいずれかの `*.ttf` ファイルに含まれていないときは VSTGUI が指定するデフォルトフォントが使用されます。

`fontFamily` が長さ 0 の文字列 `""` のときはフォールバックとして [`"Tinos"`](https://fonts.google.com/specimen/Tinos) に設定されます。長さが 1 以上かつ、存在しないフォントファミリ名が指定されると VSTGUI が指定するデフォルトフォントが使用されます。

ボールドあるいはイタリック以外のスタイルは VSTGUI がサポートしていないので動作確認していません。該当する例としては Noto フォントの Demi Light や、 Roboto フォントの Thin や Black などがあります。

### 色のオプション
16 進数カラーコードを使っています。

- 6 桁の色は RGB 。
- 8 桁の色は RGBA 。

プラグインはカラーコードの 1 文字目を無視します。よって `?102938` や `\n11335577` も有効なカラーコードです。

2 文字目以降のカラーコードの値に `0-9a-f` 以外の文字を使わないでください。

以下は設定できる色の一覧です。設定に抜けがあるとデフォルトの色が使われます。

- `foreground`: 文字の色。
- `foregroundButtonOn`: オンになっているボタンの文字の色。 `foreground` か `boxBackground` のいずれかと同じ値にすることを推奨します。
- `foregroundInactive`: 非アクティブなタブの文字の色。
- `background`: 背景色。
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
つまみと数値スライダでは次の操作ができます。

- <kbd>Ctrl</kbd> + <kbd>左クリック</kbd> : 値のリセット。
- <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> : 細かい値の変更。
- <kbd>ホイールクリック</kbd> : 最小値、デフォルト値、最大値の切り替え。

青い縦棒が並んだコントロール (BarBox) ではショートカットが使えます。ショートカットは BarBox を左クリックしてフォーカスすると有効になります。フォーカス後にマウスカーソルを BarBox の領域外に移動させると、ショートカットが一時的に無効になります。ショートカットによって変更されるパラメータはカーソルの位置によって変更できます。

左下のプラグイン名をクリックすると、よく使いそうな一部のショートカットを見ることができます。利用できる全てのショートカットを次の表に掲載しています。

| 入力                                                             | 操作                                     |
| ---------------------------------------------------------------- | ---------------------------------------- |
| <kbd>左ドラッグ</kbd>                                            | 値の変更                                 |
| <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd>                         | 値の変更 (スナップ)                      |
| <kbd>Ctrl</kbd> + <kbd>左ドラッグ</kbd>                          | デフォルト値にリセット                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd>       | 値の変更 (フレーム間の補間が無効)        |
| <kbd>ホイールドラッグ</kbd>                                      | 直線の描画                               |
| <kbd>Shift</kbd> + <kbd>ホイールドラッグ</kbd>                   | 1 つのバーを編集                         |
| <kbd>Ctrl</kbd> + <kbd>ホイールドラッグ</kbd>                    | デフォルト値にリセット                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>ホイールドラッグ</kbd> | ロックの切り替え                         |
| <kbd>a</kbd>                                                     | 符号を交互に入れ替え                     |
| <kbd>d</kbd>                                                     | すべての値をデフォルト値にリセット       |
| <kbd>D</kbd>                                                     | 最小値・中央値・最大値の切り替え         |
| <kbd>e</kbd>                                                     | 低域の強調                               |
| <kbd>E</kbd>                                                     | 高域の強調                               |
| <kbd>f</kbd>                                                     | ローパスフィルタ                         |
| <kbd>F</kbd>                                                     | ハイパスフィルタ                         |
| <kbd>i</kbd>                                                     | 値の反転 (最小値を保存)                  |
| <kbd>I</kbd>                                                     | 値の反転 (最小値を 0 に設定)             |
| <kbd>l</kbd>                                                     | マウスカーソル下のバーのロックの切り替え |
| <kbd>L</kbd>                                                     | 全てのバーのロックを切り替え             |
| <kbd>n</kbd>                                                     | 最大値を 1 に正規化 (最小値を保存)       |
| <kbd>N</kbd>                                                     | 最大値を 1 に正規化 (最小値を 0 に設定)  |
| <kbd>p</kbd>                                                     | ランダムに並べ替え                       |
| <kbd>r</kbd>                                                     | ランダマイズ                             |
| <kbd>R</kbd>                                                     | まばらなランダマイズ                     |
| <kbd>s</kbd>                                                     | 降順にソート                             |
| <kbd>S</kbd>                                                     | 昇順にソート                             |
| <kbd>t</kbd>                                                     | 少しだけランダマイズ (ランダムウォーク)  |
| <kbd>T</kbd>                                                     | 少しだけランダマイズ (0 に収束)          |
| <kbd>z</kbd>                                                     | アンドゥ                                 |
| <kbd>Z</kbd>                                                     | リドゥ                                   |
| <kbd>,</kbd> (Comma)                                             | 左に回転                                 |
| <kbd>.</kbd> (Period)                                            | 右に回転                                 |
| <kbd>1</kbd>                                                     | すべての値を低減                         |
| <kbd>2</kbd>-<kbd>9</kbd>                                        | インデックスが 2n-9n の値を低減          |

<kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> のスナップは一部の BarBox だけで有効になっています。特定の BarBox にスナップを追加したいという要望があれば、気軽に [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を開いてください。

<kbd>Shift</kbd> + <kbd>ホイールドラッグ</kbd> による 1 つのバーを編集は、マウスホイールが押された時点でカーソルの下にあるバーだけを編集します。マウスホイールが押されている間はカーソルの左右の位置に関わらず、選択したバーのみを編集できます。

<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>ホイールドラッグ</kbd> によるロックの切り替えでは、マウスホイールが押された時点でカーソルの下にあるバーの反対の状態が残り全てに適用されます。例えばカーソルの下のバーがアクティブだったときはロックに切り替えます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/pitchshiftdelay.svg)

## パラメータ
### Delay
Pitch

:   基本となるピッチシフトの倍率です。

    メインシフタは LFO を無視すれば常に `Pitch` で指定された倍率のピッチシフトを行います。

    PitchShiftDelay は 16 倍のオーバーサンプリングを行っているので、 16 * 2 - 1 = 31 倍のピッチシフトまでならエイリアシングノイズは出ません。

Offset

:   ユニゾンシフタのピッチシフトの倍率が `Pitch` からどれだけ離れるかを決める量です。

Pitch Cross

:   メインシフタとピッチシフトの出力をクロスフィードバックする量です。

Mirror

:   チェックを入れるとユニゾンシフタのピッチシフトの倍率を `1 / Pitch` に設定します。

    起動直後の状態から `Mirror` にチェックを入れると `Feedback` を上げても、フィードバック信号のピッチが常に `Pitch` あるいは `1 / Pitch` になります。この状態は `Offset` 、 `Pitch Cross` 、 あるいは LFO セクションの `To Pitch` 、 `To Unison` がデフォルト値である限り保たれます。

S1 Reverse

:   チェックを入れるとメインシフタの出力を逆再生させます。

    内部的にはピッチシフトの倍率の値の符号を負に変えることで読み取りポインタを逆走させています。

S2 Reverse

:   チェックを入れるとユニゾンシフタの出力を逆再生させます。

L-R Lean

:   左右のディレイ時間の比率です。 `Channel Type` が `M-S` のときでも機能します。

Delay Time \[s\]

:   ピッチシフタのバッファの長さです。 `Pitch` の値が 1 のときだけ正確なディレイ時間を表すことに注意してください。

    ピッチシフトの倍率が 1 でないとき、正確なディレイ時間はピッチシフトの倍率とバッファの長さによって変わります。 その上に LFO がかかると正確なディレイ時間を計算することは困難になります。したがって `Delay Time` の値は、正確なディレイ時間というよりも、おおまかな指標です。

Feedback

:   振幅であらわされたフィードバックのゲインです。

Stereo Cross

:   ステレオチャンネル間でクロスフィードバックする量です。 `1.0` にするとピンポンディレイになります。

Channel Type

:   入力信号のステレオチャンネルの種類を左右 (`L-R`) とミッド-サイド (`M-S`) のいずれかから選択します。

Highpass \[Hz\]

:   フィードバック経路に挿入されているハイパスフィルタのカットオフ周波数です。

### Mix
Dry \[dB\]

:   バイパスする入力信号のゲインです。

Wet \[dB\]

:   PitchShiftDelay を通過した出力信号のゲインです。

Unison Mix

:   メインシフタとユニゾンシフタを混ぜる比率です。

Smoothing \[s\]

:   パラメータのスムーシング時間です。

    例えば `Smoothing` の値を `0.01` と短くするとパラメータの変更がほぼ瞬時に適用されます。ただし `Smoothing` の値を小さくするとパラメータ変更時のポップノイズが目立つようになります。

    逆に `Smoothing` の値を `1.0` などと長くするとパラメータの値がゆっくりと切り替わるようになります。特に `Pitch` などを動かしたときはスライドやポルタメントのようになります。

### LFO
Sync.

:   チェックを入れるとテンポ同期を有効にします。また同期間隔が変わったときに再生開始時点から導かれる位相へと同期します。

    チェックが外れているときは 120 BPM に同期した状態と同じになります。ただし、同期間隔が変わったときに位相を調整しなくなります。

Tempo Upper

:   テンポ同期が有効な時の同期間隔を表す分数の分子です。

    `1/1` のときに 1 小節、 4/4拍子であれば `1/4` のときに 1 拍で LFO が 1 周します。 `Rate` が乗算されて周期が変わる点に注意してください。

    以下は同期間隔の計算式です。

    ```
    syncInterval = (Rate) * (Tempo Upper) / (Tempo Lower);
    ```

Tempo Lower

:   テンポ同期が有効な時の同期間隔を表す分数の分母です。

    `Rate` が乗算されて周期が変わる点に注意してください。

Rate

:   同期間隔に乗算される係数です。

    `Tempo Upper` と `Tempo Lower` を変えずに LFO の同期間隔を変えたいときに使えます。

Stereo Offset

:   ステレオチャンネル間で LFO の位相をずらす量です。

Unison Offset

:   メインシフタとユニゾンシフタの間で LFO の位相をずらす量です。

Wave Interp.

:   LFO の波形の補間方法です。

    - `Step`: ホールド。
    - `Linear`: 線形補間。
    - `PCHIP`: 単調な 3 次補間。

    `Step` を選ぶと LFO をシーケンサのように使えます。 `Linear` は `PCHIP` と似たような音になりますが、計算がすこし速いです。デフォルトの `PCHIP` はサンプル間をだいたい滑らかに補間します。

To Pitch

:   LFO でピッチを変調する量です。

To Unison

:   `To Pitch` で指定した量に加えて LFO でユニゾンシフタのピッチを変調する量です。


## チェンジログ
- 0.2.0
  - 初期リリース。

## 旧バージョン
### PitchShiftDelay
旧バージョンはありません。

## ライセンス
PitchShiftDelay のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
