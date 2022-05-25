---
lang: ja
...

# FDN64Reverb
![](img/fdn64reverb.png)

<ruby>FDN64Reverb<rt>エフディーエヌ 64 リバーブ</rt></ruby> はフィードバック・ディレイ・ネットワーク (FDN) を 1 つだけ搭載したリバーブです。しかしながら、フィードバック行列の大きさは 64 です。

- [FDN64Reverb 0.1.3 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins_0_31_0/FDN64Reverb_0.1.3.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/FDN64ReverbPresets.zip)

FDN64Reverb の利用には AVX 以降の SIMD 命令セットをサポートする CPU が必要です。

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit

2022-05-14 の時点ではユニバーサルバイナリのビルドに失敗するので、 macOS ビルドはパッケージに含まれていません。将来的に対応したいですが、お財布の問題で M1 mac 入手のめどが立たないので時期は未定です。

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

## 連絡先
何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか `ryukau@gmail.com` までお気軽にどうぞ。

[paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau) から開発資金を投げ銭することもできます。現在の目標は macOS と ARM ポートのための M1 mac の購入資金を作ることです。 💸💻

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

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

REAPER の Linux 版がプラグインを認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

### macOS
**重要**: 現バージョンのパッケージは macOS 未対応です。

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

**重要**: `fontFamily` 、 `fontBold` 、 `fontItalic` で設定したフォントファミリ名とスタイルの組み合わせが `*.vst3/Contents/Resources/Fonts` 以下のいずれかの `*.ttf` ファイルに含まれていないときは VSTGUI が指定するデフォルトフォントが使用されます。

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
操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

つまみとスライダーでは次の操作ができます。

- <kbd>Ctrl</kbd> + <kbd>左クリック</kbd> : 値のリセット。
- <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> : 細かい値の変更。

数値スライダーでは、上記に加えて次の操作ができます。

- <kbd>ホイールクリック</kbd> : 最小値、中央値、最大値の切り替え。

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

<kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> のスナップは一部の BarBox だけで有効になっています。特定の BarBox にスナップを追加したいという要望があれば、お気軽に [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を開いてください。

<kbd>Shift</kbd> + <kbd>ホイールドラッグ</kbd> による 1 つのバーを編集は、マウスホイールが押された時点でカーソルの下にあるバーだけを編集します。マウスホイールが押されている間はカーソルの左右の位置に関わらず、選択したバーのみを編集できます。

<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>ホイールドラッグ</kbd> によるロックの切り替えでは、マウスホイールが押された時点でカーソルの下にあるバーの反対の状態が残り全てに適用されます。例えばカーソルの下のバーがアクティブだったときはロックに切り替えます。

## 注意
`Feedback` の値が 1 のときに `Highpass Cutoff` の値を 0 にすると入力信号によっては極端な直流が乗ることがあります。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/fdn64reverb.svg)

## パラメータ
Delay Time \[s\]

:   ディレイ時間の秒数です。最終的なディレイ時間の秒数は以下の式で計算されます。

    ```
    delayTime = (Time Multi.) * (Delay Time) + (Time LFO Amount) * random().
    ```

    ディレイ時間は 1 秒を超えることはないので注意してください。例えば `Delay Time` と `Time LFO Amount` を最大に設定すると LFO はかかりません。

Time LFO Amount \[s\]

:   ディレイ時間に加算される LFO の量です。例えば `Time LFO Amount` の値が 0.1 ならディレイ時間の最大値は `(Delay Time) + 0.1` 秒になります。

    `Interp. Rate` が 1.0 を超えると変調によるノイズが目立ち始めます。とにかく滑らかな音が欲しいときは `Interp. Rate` を 0.25 以下にすることを推奨します。

Lowpass Cutoff \[Hz\]

:   ディレイの出力にかかるローパスフィルタのカットオフ周波数です。

Highpass Cutoff \[Hz\]

:   ディレイの出力にかかるハイパスフィルタのカットオフ周波数です。

    `Feedback` の値が 1 のときに `Highpass Cutoff` の値を 0 にすると直流が乗ることがあるので注意してください。

### Delay
Time Multi.

:   `Delay Time` に乗算される値です。まとめてディレイ時間を変えたいときに便利です。

Feedback

:   FDN からフィードバックされる量です。別の言い方をすると FDN のフィードバック行列に掛け合わされるスカラーです。

    1.0 に近づくほどリバーブ時間が長くなります。 1.0 すると、ほとんど減衰しなくなります。このときは `Gate` を使って残響を打ち切ることができます。

Interp. Rate

:   ディレイ時間の変動を補間するレートリミッタの、 1 サンプルあたりの制限量です。例えば `Interp. Rate` が 0.1 のときは、 10 サンプル経過でディレイ時間が 0.1 * 10 = 1 サンプル変わります。

Gate \[dB\]

:   ゲートのしきい値です。

    入力振幅が `Gate` の値を一定時間下回ると、 `Stereo Cross` の値を内部的に変調して出力振幅を 0 にします。このゲートは Rotation 使用時にフィードバックを打ち切るためにつけた機能なので、かなり素早く閉じます。

Matrix

:   フィードバック行列の種類です。この値を変更するとポップノイズがでることがあるので注意してください。

    | GUI 上の略称 | 行列名                     | 追加情報                           | 質       |
    | ------------ | -------------------------- | ---------------------------------- | -------- |
    | Ortho.       | 直交行列                   |                                    | 良       |
    | S. Ortho.    | 特殊直行行列               |                                    | 良       |
    | Circ. Ortho. | 巡回行列                   |                                    | 並       |
    | Circ. 4      | 巡回行列                   | 1 行当たりの非ゼロ要素を 4 に制限  | 特殊効果 |
    | Circ. 8      | 巡回行列                   | 1 行当たりの非ゼロ要素を 8 に制限  | 特殊効果 |
    | Circ. 16     | 巡回行列                   | 1 行当たりの非ゼロ要素を 16 に制限 | 特殊効果 |
    | Circ. 32     | 巡回行列                   | 1 行当たりの非ゼロ要素を 32 に制限 | 特殊効果 |
    | Upper Tri. + | 上三角行列                 | ランダマイズの範囲が `[0, 1]`      | 低       |
    | Upper Tri. - | 上三角行列                 | ランダマイズの範囲が `[-1, 0]`     | 低       |
    | Lower Tri. + | 下三角行列                 | ランダマイズの範囲が `[0, 1]`      | 低       |
    | Lower Tri. - | 下三角行列                 | ランダマイズの範囲が `[-1, 0]`     | 低       |
    | Schroeder +  | シュローダー・リバーブ行列 | ランダマイズの範囲が `[0, 1]`      | 低       |
    | Schroeder -  | シュローダー・リバーブ行列 | ランダマイズの範囲が `[-1, 0]`     | 低       |
    | Absorbent +  | 吸収オールパス行列         | ランダマイズの範囲が `[0, 1]`      | 並       |
    | Absorbent -  | 吸収オールパス行列         | ランダマイズの範囲が `[-1, 0]`     | 並       |
    | Hadamard     | アダマール行列             | 行列の性質よりランダマイズ無効     | 良       |
    | Conference   | カンファレンス行列         | 行列の性質よりランダマイズ無効     | 良       |

    質は大まかな目安です。ショートディレイによる金属的な質感が出やすいものは低、出にくいものは良としています。特殊効果としているものは一般的なリバーブとしては音が変という意味です。 Rotation の効果は質が低いほど聞こえやすくなります。

Seed

:   フィードバック行列のランダマイズに使われるシード値です。この値を変更するとポップノイズがでることがあるので注意してください。

Change Matrix

:   フィードバック行列をランダマイズするボタンです。押すたびに `Seed` の値が変更されます。この値を変更するとポップノイズがでることがあるので注意してください。

### Mix
Dry \[dB\]

:   FDN をバイパスした入力にかけ合わせられるゲインです。

Wet \[dB\]

:   FDN の出力にかけ合わせられるゲインです。

Stereo Cross

:   左右のチャンネルの FDN の出力をクロスしてフィードバックする量です。 1.0 にすると発散を防ぐために入力を止めてしまうので注意してください。

### Rotation
Speed \[Hz\]

:   FDN への入力ゲインを回転させる速度です。

Offset

:   入力ゲインの回転に使われる波形の初期位相です。

    `Speed` が 0 かつ `Skew` が 0 より大きいときに音を変えることができます。どのような音になるかはフィードバック行列のランダマイズに大きく影響されます。

Skew

:   入力ゲインの回転に使われる波形を変更します。 `Skew` が 0 のときは `Speed` と `Offset` を変えても音が変わらないので注意してください。

    以下は `Skew` と波形の関係についてのプロットです。

    <figure>
    <img src="img/skewplot.svg" alt="Image of plot of relation between `Skew` parameter and rotation waveform. The equation of waveform is `exp(skew * sin(ω * t))`." style="padding-bottom: 12px;"/>
    </figure>

## チェンジログ
- 0.1.3
  - プラグインが許可するチャネル数の条件を修正。この修正は REAPER のサイドチェインで意図しないミュートがかかることを防ぐために行った。
- 0.1.2
  - `style.json` でカスタムフォントを設定するオプションを追加。
- 0.1.1
  - VSTGUI を 4.10 から 4.11 にアップデート。
- 0.1.0
  - 初期リリース。

## 旧バージョン
- [FDN64Reverb 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/FDN64Reverb_0.1.2.zip)
- [FDN64Reverb 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/FDN64Reverb_0.1.0.zip)

## ライセンス
FDN64Reverb のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
