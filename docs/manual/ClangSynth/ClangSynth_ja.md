---
lang: ja
...

# ClangSynth
![](img/clangsynth.png)

<ruby>ClangSynth<rt>クラング シンセ</rt></ruby> は金属的なパーカッションの音が得意なシンセサイザです。金属的な質感はフィードバック・ディレイ・ネットワーク (FDN) を用いたディレイ間のクロスフィードバックによるものです。 WaveCymbal や FDNCymbal よりはシンバルに似た音が出ます。

- [ClangSynth 0.0.1 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.9999.0/ClangSynth_0.0.1.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
<!-- - [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.34.0/ParallelCombPresets.zip) -->

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

## 注意
音量の振れ幅が大きいので ClangSynth の後に必ずリミッタを挿入することを推奨します。

FDN のパラメータが短時間に大きく変わると発散することがあります。 LFO やエンベロープを使うときは注意してください。発散はパラメータが時間とともに変わると線形時不変 (linear time invariant, LTI) なシステムではなくなるために起こります。以下は発散の原因となるパラメータの一覧です。

- LFO
  - `Amount > FDN Pitch`
- Envelope
  - `> LP Cut`
  - `> HP Cut`
  - `> FDN Pitch`
  - `> FDN OT +`

`Refresh Wavetable` ボタンを押すとウェーブテーブルが更新されます。更新中は CPU 負荷が上がり、ほぼ確実に音が止まるので注意してください。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/clangsynth.svg)

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は単位の一覧です。

- \[dB\] : デシベル (decibel) 。
- \[s\] : 秒 (second) 。
- \[st.\] : 半音 (semitone) 。
- \[Hz\] : 周波数 (Hertz) 。
- \[rad/pi\] : 正規化した位相 (radian / π) 。

### Gain
Output \[dB\]

:   出力のゲインです。

Attack \[s\]

:   出力直前のゲインをスムーシングする秒数です。値が大きくなるとアタック時間とともに、リリース時間も長くなります。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

    `Attack` は、FDN セクションの `Reset to Note On` がチェックされていないときに、ノートオンでポップノイズが起こることを防ぐために導入されました。 FDN セクションのローパスフィルタのカットオフ周波数を下げているときは、値を小さくしすぎるとポップノイズが目立つことがあります。シンバルなどの高い周波数成分が多い音を作るときは 0 まで値を下げても、ほとんど問題になりません。

Release \[s\]

:   ノートオフの後に続くリリース時間です。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

### Tuning
Octave, Semi, Milli

:   全体の音の高さを変更します。

    - `Octave`: オクターブ
    - `Semi`: 半音
    - `Milli`: 半音の 1 / 1000 。 1 / 10 セント。

ET, A4 \[Hz\]

:   音律を変更します。

    `ET` は Equal Temperament (平均律) の略です。 `ET` が 12 のときは 12 平均律となります。 `ET` を 12 よりも小さくすると無音になる範囲が増えるので注意してください。

    `A4 [Hz]` は、音程 A4 の周波数です。

P.Bend Range

:   ピッチベンドの範囲です。単位は半音です。

### Unison/Chord
nUnison

:   ユニゾンにつかうボイスの数です。

Pan

:   パンを振る幅です。

    初回のノートオンでは、右から左へ向かって、ピッチの低い音から高い音へと順番にパンが振られます。以降はノートオンごとにパンの割り当てを左に回転させます。

Pitch *

:   ユニゾンで変更されるピッチに乗算される値です。

    `Pitch *` が 0 のとき、 `Interval` の値は無効になります。

ET

:   `Interval` のみで使われる平均律の値です。

Interval \[st.\], Cycle At

:   `Interval` はユニゾンで使われるボイスのピッチを、ノートオンで送られてきたピッチからどれだけずらすかを調整する値です。

    `Cycle At` は `Interval` の値を巻き戻すインデックスです。

    例えば `Interval` が以下の値に設定されているとします。

    ```
    1, 2, 3, 4
    ```

    各  `Interval` には `Pitch *` の値が乗算されます。仮に `Pitch *` の値が 0.1 とすると以下の値へと変換されます。

    ```
    0.1, 0.2, 0.3, 0.4
    ```

    ここで `Cycle At` が 1 に設定されているとすると、以下のようにインデックス 1 で巻き戻しながら繰り返します。

    ```
    0.1, 0.2, 0.1, 0.2, ...
    ```


    さらにノートオンで MIDI ノート番号 60 のピッチが入力されると、以下のようにピッチがボイスに割り当てられます。

    ```
    60,                         // 60
    60 + 0.1,                   // 60.1
    60 + 0.1 + 0.2,             // 60.3
    60 + 0.1 + 0.2 + 0.1,       // 60.4
    60 + 0.1 + 0.2 + 0.1 + 0.2, // 60.6
    ...
    ```

    以下は計算の手続きです。

    ```
    modulo = cycleAt + 1;
    accumulator = 0;
    for index in [0, nUnison) {
      voice[index].pitch = notePitch + pitchMultiplier * accumulator;
      accumulator += interval[index % modulo];
    }
    ```

### Misc.
nVoice

:   最大同時発音数です。

    CPU 負荷を下げるときは `nVoice` の値を下げると効果的です。

Smoothing \[s\]

:   パラメータのスムーシング時間です。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

### Oscillator
Impulse \[dB\]

:   ノートオンと同時に発音されるインパルスの振幅です。

    インパルスはシンバルの音を作るときの味付けに使えます。またインパルスはベロシティに影響されないので、ばちとシンバルの衝突の強弱を演奏時に調整するときにも便利です。

    TODO プリセットの紹介

Gain \[dB\]

:   オシレータ出力のゲインです。

Attack \[s\], Decay \[s\]

:   オシレータの AD エンベロープのアタック時間とディケイ時間です。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

    ディケイ時間が短いときは `Attack` を長くしても音はほとんど変わりません。

Octave, Semitone

:   オクターブあるいは半音単位のオシレータのチューニングです。

Denom. Slope, Rot. Slope, Rot. Offset, Interval

:   オシレータのウェーブテーブルの基となるソース・スペクトラムを合成するパラメータです。

    - `Denom. Slope` : 振幅の分子の係数。
    - `Rot. Slope` : 位相の傾き。
    - `Rot. Offset` : 位相の切片。
    - `Interval` : 書き込む間隔。

    `Denom. Slope` の値が 1 より大きいときはローパス、 1 より小さいときはハイパスのように機能します。値が 1 のときは、のこぎり波と同じ振幅特性になります。

    `Interval` を 1 より大きくするとソース・スペクトラムが 0 となる箇所を作り出します。例えば `Interval` の値が 3 なら、インデックス 1, 4, 7, 10, 13, ... の値だけが書き込まれて、それ以外のインデックスでの振幅は 0 となります。 `Denom. Slope` の値が 1 かつ `Interval` の値が 2 なら、矩形波が生成されます。

    以下は `Denom. Slope` 、 `Rot. Slope` 、 `Rot. Offset` 、 `Interval` を使ったソース・スペクトラムの計算方法です。

    ```
    for index in [1, numberOfHarmonics] {
        if (index % interval != 0) continue;
        sourceSpectrum[index].amp = 1 / (denominatorSlope * index);
        sourceSpectrum[index].phase = rotationOffset + rotationSlope * index;
    }
    ```

Harmonic HP

:   ソース・スペクトラムの振幅に適用されるハイパスフィルタです。

    `Harmonic HP` で指定された値よりインデックスが小さい低周波成分の振幅を下げます。値が 0 のときはハイパスフィルタはかかりません。

    以下は `Harmonic HP` の計算方法です。

    ```
    for index in [0, harmonicHighpass) {
      sourceSpectrum[index].amp *= index / harmonicHighpass;
    }
    ```

Blur

:   ソース・スペクトラムの振幅を通過させるローパスフィルタの係数です。

    `Blur` の値を下げると周波数領域で振幅の値が周囲のインデックスへと拡散するので、位相の回転を無視すると、時間領域に戻したときにはインパルスに近い音になります。

    `Blur` が 1 のときはローパスフィルタはかかりません。

    TODO 図

OT Amp., Rot. \[rad/pi\]

:   ソース・スペクトラムを、さらに倍音間隔で足し合わせるときの振幅と位相です。

    - `OT Amp.` : 加算する倍音の振幅。
    - `Rot. \[rad/pi\]` : 加算する倍音の位相。

    以下は `OT Amp.` と `Rot.` による最終的なスペクトラムの計算方法です。

    ```
    targetSpectrum.fill(0);

    for i in [0, nOvertone) {
      for k in [0, nFrequency) {
        auto index = (i + 1) * k;
        if (index >= nFrequency) break;
        targetSpectrum[index] += sourceSpectrum[k] * complexFromPolar(otAmp[i], otRot[i]);
      }
    }
    ```

Refresh Wavetable

:   ボタンを押すとウェーブテーブルを更新します。

    更新中は CPU 負荷が上がり、ほぼ確実に音が止まるので注意してください。

    Oscillator セクションの以下のパラメータは `Refresh Wavetable` ボタンを押すまで更新されません。

    - `Denom. Slope`
    - `Rot. Slope`
    - `Rot. Offset`
    - `Interval`
    - `Harmonic HP`
    - `Blur`
    - `OT Amp.`
    - `Rot. [rad/pi]`

### FDN
FDN

:   点灯しているときはオシレータ出力が FDN を通過します。

    オシレータの出力を確認するときには無効にすると便利です。

Identity

:   FDN 内のクロス・フィードバックの量を変更します。

    `Identity` の値が 0 から離れるほど、フィードバック行列が単位行列から離れてクロス・フィードバックが増えます。クロス・フィードバックが増えると非整数次倍音が増えて金属的な質感が出ます。

Feedback

:   FDN のフィードバック量です。

    ディレイ内部の線形補間の影響で、 `Feedback` の値が 1 のときでも少しづつ減衰が起こります。

Interp. Rate

:   ディレイ時間が変更されたときのレート制限の量です。

Interp. LP \[s\]

:   ディレイ時間が変更されたときの補間に使われるローパスフィルタのカットオフ周波数の逆数です。

    ディレイ時間の補間は、ローパスフィルタ、レート制限の順で行われます。

Seed

:   フィードバック行列のランダマイズに使われるシード値です。

Randomize

:   ノートオンごとにフィードバック行列をランダマイズする割合です。

    内部的には行列をランダマイズする基となる値の組を 2 つ持っています。 1 つの組は再生開始の時点で固定されます。もう 1 つの組はノートオンごとに生成されます。この 2 つの組を混ぜる割合が `Randomize` です。

    `Randomize` が 0 のときは再生開始の時点で固定される組だけを使うので、ノートオンによって倍音が変わりません。 `Randomize` が 1 のときはノートオンのたびにフィードバック行列が変わるので、倍音も変わります。

OT +, OT *, OT Offset, OT Modulo, OT Random

:   FDN のディレイ時間を、ノートオンで提供されるピッチの倍音として設定するパラメータです。

    - `OT +` : 倍音のインデックスが進むたびに加算される値。
    - `OT *` : 倍音のインデックスが進むたびに 1 つ前の値に乗算される値。
    - `OT Offset` : 倍音を高いほうに向かってシフトする量。
    - `OT Modulo` : `OT +` と `OT *` から計算される値を、巻き戻す点。
    - `OT Random` : ノートオンごとに倍音をランダマイズする量。

    `OT +` と `OT Offset` を整数でない値に設定することで打楽器のような音に近づきます。

    `OT *` と `OT Modulo` はやや不自然な倍音になります。

    以下は倍音の計算方法です。

    ```
    // At note on.
    for idnex in [0, fdnMatrixSize) {
        overtoneRandomness[idx] = randomUniform(-1, 1) * otRandom;
    }

    // For each sample.
    overtone = 1;
    for idnex in [0, fdnMatrixSize) {
      ot = otOffset + (1 + overtoneRandomness[index]) * overtone;
      fdn.delayTime[index] = sampleRate / (ot * noteFrequency);
      overtone = overtone * otMul + otAdd;
      overtone = fmod(overtone, 1 + otModulo);
    }
    ```

Reset at Note ON

:   チェックを入れるとノートオンのたびに FDN の状態をリセットします。

    リセットによる CPU 負荷のスパイクに注意してください。音が止まるなどの問題が出るときは `nVoice` の値を小さくすることで CPU 負荷を軽減できます。

Cutoff \[st.\]

:   ディレイの出力が通るローパスフィルタとハイパスフィルタのカットオフ周波数です。

Q

:   ディレイの出力が通るローパスフィルタとハイパスフィルタの Q です。

Key Follow

:   チェックを入れるとフィルタのカットオフ周波数をノートのピッチに応じて変更します。

### LFO
LFO Wave

:   LFO のウェーブテーブルの波形です。

    ウェーブテーブルの波形はすべてのボイスで共有されます。

Retrigger

:   チェックを入れるとノートオンされたボイスの LFO の位相をリセットします。

    チェックを外すとすべてのボイスが同じ位相を使います。

    1 つのボイスが 1 つの物体を表すのであれば、チェックを入れたほうがそれらしく聞こえます。ノートオンによって同じ物体を叩く、例えば 8 人が両手にばちをもって同じ中華鍋を叩いているとするなら、チェックを外してください。

Wave Interp.

:   LFO の波形の補間方法です。

    - `Step`: ホールド。
    - `Linear`: 線形補間。
    - `PCHIP`: 単調な 3 次補間。

    `Step` を選ぶと LFO をシーケンサのように使えます。 `Linear` は `PCHIP` と似たような音になりますが、計算がすこし速いです。デフォルトの `PCHIP` はサンプル間をだいたい滑らかに補間します。

Sync.

:   チェックを入れるとテンポ同期を有効にします。また同期間隔が変わったときに再生開始時点から導かれる位相へと同期します。

    チェックが外れているときは 120 BPM に同期した状態と同じになります。ただし、同期間隔が変わったときに位相を調整しなくなります。

Tempo Upper

:   テンポ同期が有効な時の同期間隔を表す分数の分子です。

    `1/1` のときに 1 小節、 4/4拍子であれば `1/4` のときに 1 拍で LFO が 1 周します。 `Rate` が乗算されて周期が変わる点に注意してください。

    以下は同期間隔の計算方法です。

    ```
    syncInterval = (Rate) * (Tempo Upper) / (Tempo Lower);
    ```

Tempo Lower

:   テンポ同期が有効な時の同期間隔を表す分数の分母です。

    `Rate` が乗算されて周期が変わる点に注意してください。

Rate

:   同期間隔に乗算される係数です。

    `Tempo Upper` と `Tempo Lower` を変えずに LFO の同期間隔を変えたいときに使えます。

Pitch Amount

:   LFO によるピッチの変調量です。単位は半音です。

    `> Osc. Pitch` の行のパラメータはオシレータ、 `> FDN Pitch` の行のパラメータは FDN への変調です。

Pitch Alignment

:   LFO のよるピッチの変調量は `Alignment` で指定された値の倍数だけを使います。例えば現在の変調量が 12.345 で `Alignment` が 6 とすると、実際の変調量は 12 になります。

    `Alignment` が 0 のときは `Amount` の値を直接使います。

    以下は変調量の計算方法です。

    ```
    if (alignment == 0)
      modulation = amount * lfo;
    else
      modulation = alignment * floor(lfo * amount / alignment + 0.5);
    ```

### Envelope
Envelope Wave

:   エンベロープのウェーブテーブルの波形です。

    ウェーブテーブルの波形はすべてのボイスで共有されます。

Time \[s\]

:   ノートオンからエンベロープの終了までにかかる時間です。

    エンベロープが終了すると変調量はすべて 0 になります。

Wave Interp.

:   エンベロープのウェーブテーブルの補間方法です。

    LFO と同じく `Step` 、 `Linear` 、 `PCHIP` の 3 つの補間方法から選択できます。補間方法の詳細は LFO セクションの `Wave Interp.` を参照してください。

> LP Cut

:   FDN のローパスフィルタのカットオフ周波数への変調量です。単位は半音です。

> HP Cut

:   FDN のハイパスフィルタのカットオフ周波数への変調量です。単位は半音です。

> Osc. Pitch

:   オシレータのピッチの変調量です。単位は半音です。

> FDN Pitch

:   FDN のピッチの変調量です。単位は半音です。

> FDN OT +

:   FDN セクションの `OT +` への変調量です。

    `> FDN Pitch` とは異なり、倍音に基づいたピッチの変調を行います。

## チェンジログ
- 0.0.1
  - β リリース。

## 旧バージョン
### ClangSynth
旧バージョンはありません。

## ライセンス
ClangSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
