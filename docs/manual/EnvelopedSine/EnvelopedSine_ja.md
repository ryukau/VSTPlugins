---
lang: ja
...

# EnvelopedSine
![](img/envelopedsine.png)

<ruby>EnvelopedSine<rt>エンベロープド サイン</rt></ruby>はノート 1 つあたり 64 のサイン波を計算する加算合成シンセサイザです。各サイン波に AD エンベロープとサチュレータがついているので IterativeSinCluster よりもパーカッシブな音が得意です。

- [EnvelopedSine 0.1.10 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/EnvelopedSine0.1.10.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/EnvelopedSinePresets.zip)

EnvelopedSine を使うには CPU が AVX 以降の SIMD 命令セットをサポートしている必要があります。

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

Reference:
- [How to Fix App “is damaged and can’t be opened. You should move it to the Trash” Error on Mac](https://osxdaily.com/2019/02/13/fix-app-damaged-cant-be-opened-trash-error-mac/)
- [Allowing unsigned/un-notarized applications/plugins in Mac OS | Venn Audio](https://www.vennaudio.com/allowing-unsigned-un-notarized-applications-plugins-in-mac-os/)
- [Safely open apps on your Mac - Apple Support](https://support.apple.com/en-us/HT202491)


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
操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

つまみとスライダーでは次の操作ができます。

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

`Octave` 、 `Seed` などで使われている数値スライダーでは、上記に加えて次の操作ができます。

- ホイールクリック : 最小値、最大値の切り替え。

`Attack` 、 `Decay` 、 `Gain` 、 `Saturation` を操作する、青い縦棒が並んだインターフェイス (BarBox) ではショートカットが使えます。ショートカットは BarBox を左クリックしてフォーカスした上で、マウスカーソルが BarBox の領域内にあるときだけ有効になります。左下にあるプラグインのタイトルをクリックするとショートカットの一覧を見ることができます。

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

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/envelopedsine.svg)

## パラメータ
### Overtone
右上にある 4 つの大きなコントロールです。

Attack, Decay

:   各オシレータの音量エンベロープのアタックとディケイです。

Gain

:   各オシレータの音量です。

Saturation

:   各オシレータのサチュレーションの大きさです。

### Gain
Boost, Gain

:   どちらも音量を調整します。出力のピーク値は `Boost * Gain` となります。

### Pitch
Add Aliasing

:   チェックを入れると、ナイキスト周波数より高い周波数が指定されたサイン波もレンダリングします。

    ナイキスト周波数とは、ざっくりいえば録音したデータが再現可能な最高周波数のことです。音を生成するときはナイキスト周波数以上の値を指定することができますが、折り返し（エイリアシング）と呼ばれる現象によって計算結果は予期しない周波数になってしまいます。 `Add Aliasing` は、こうしたエイリアシングノイズを足し合わせるオプションです。

Octave

:   ノートのオクターブです。

Multiply, Modulo

:   サイン波の周波数を変更します。

    計算式は `ノートの周波数 * (1 + fmod(Multiply * pitch, Modulo))` のようになっています。 `pitch` はノートの音程と倍音のインデックスから計算される係数です。 `fmod(a, b)` は `a` を `b` で割った余りを計算する関数です。

Expand

:   倍音コントロールのインデックスを拡大・縮小するスケーリング係数です。

    ![](img/envelopedsine_expand.svg)

Shift

:   倍音コントロールのインデックスを右シフトする量です。

    ![](img/envelopedsine_shift.svg)

### Random
ランダマイズに関するパラメータです。 `Unison` にチェックを入れた状態で使うと効果的です。

Retrigger

:   チェックを入れると、ノートオンごとに乱数列をリセットします。

Seed

:   乱数のシード値です。この値を変えると生成される乱数が変わります。

To Gain, To Attack, To Decay, To Sat.

:   倍音コントロールの値をランダマイズします。ランダマイズの式は `value * random` です。 `random` の値の範囲は `[0.0, 1.0)` です。

To Pitch

:   ピッチをランダマイズします。

To Phase

:   位相をランダマイズします。

### Misc.
Smooth

:   特定の値を変更したときに、変更前の値から変更後の値に移行する時間（秒）です。

    `Smooth` と関連するパラメータのリストです。 `*` はワイルドカードです。

    - `Gain` セクションの全て
    - `Phaser` セクションの `nStages` 以外

    他のパラメータはノートオンのタイミングで指定されていた値を、ノートの発音が終わるまで使い続けます。

nVoices

:   最大同時発音数です。

Unison

:   チェックを入れるとユニゾンを有効にします。

    ユニゾンが有効になると 1 つのノートについて左右のチャンネルに 1 つずつボイスを配置します。 `Random.To Phase` や `Random.To Pitch` と組み合わせることで広がりのある音を作ることができます。

### Modifier
Attack\*, Decay\*

:   倍音コントロールの `Attack` と `Decay` にかけ合わせる値です。まとめて長さを変えたいときに使えます。

Declick

:   チェックを入れると、倍音コントロールの `Attack` あるいは `Decay` の値が 0 のときに生じるプチノイズを低減します。各オシレータのピッチに応じた値を足し合わせます。

Gain^

:   倍音コントロールの `Gain` をべき乗する値です。 1 つのオシレータの最終的なゲインは次の式で計算されます。

    ```
    Gain.Boost * Gain.Gain * pow(Overtone.Gain, Modifier.Gain^)
    ```

Sat. Mix

:   サチュレーションの Dry : Wet の信号比です。右いっぱいにすると Dry : Wet = 0 : 1 になります。

### Phaser
nStages

:   オールパスフィルタの数です。

Mix

:   フェイザの Dry : Wet の信号比です。右いっぱいにすると Dry : Wet = 0 : 1 になります。

Freq

:   LFO の周波数です。

Feedback

:   フィードバックの大きさです。 12 時にするとフィードバックなし、左に回すと負のフィードバック、右に回すと正のフィードバックとなります。

Range

:   LFO によって変更するオールパスフィルタの特性の幅です。

Min

:   LFO によって変調するオールパスフィルタの特性の最小値です。

Offset

:   左右の LFO の位相差です。

Phase

:   LFO の位相です。オートメーションで音を作りたいときに使えます。 `Freq` を左いっぱいに回すことで LFO の周波数を 0 にできます。

## チェンジログ
- 0.1.10
  - Process context requirements を実装。
  - BarBox の機能が LV2 版と同等になるように更新。
    - 1 つのバーを編集を追加。
    - ロックを追加。
    - 内部的なマウスホイールの感度を追加。
    - スナップを追加 (未使用) 。
    - 直線の描画での開始点の値をアンカーポイントに固定するように変更。
- 0.1.9
  - DSP が初期化されているかどうかのチェックを追加。
- 0.1.8
  - BarBox に アンドゥ・リドゥの機能を追加。
- 0.1.7
  - カラーコンフィグを追加。
- 0.1.6
  - パラメータの補間を可変サイズのオーディオバッファでも機能する以前の手法に巻き戻した。
- 0.1.5
  - BarBox にフォーカスした後、マウスカーソルを領域外に動かしている間はショートカットが無効になるように変更。
- 0.1.4
  - 文字列の描画でクラッシュするバグを修正。
- 0.1.3
  - プラグインタイトルをクリックすると表示されるポップアップの表示方法の変更。
- 0.1.2
  - PreSonus Studio One 4.6.1 で出力にノイズが乗るバグを修正。
- 0.1.1
  - Linux ビルドの GUI を有効化。
  - スプラッシュスクリーンにショートカット一覧を追加。
- 0.1.0
  - 初期リリース。

### 旧バージョン
- [EnvelopedSine 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/EnvelopedSine0.1.9.zip)
- [EnvelopedSine 0.1.8 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L4Reverb0.1.0/EnvelopedSine0.1.8.zip)
- [EnvelopedSine 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/EnvelopedSine0.1.7.zip)
- [EnvelopedSine 0.1.6 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/EnvelopedSine0.1.6.zip)
- [EnvelopedSine 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BarBoxFocusFix/EnvelopedSine0.1.5.zip)
- [EnvelopedSine 0.1.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/EnvelopedSine0.1.4.zip)
- [EnvelopedSine 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/EnvelopedSine0.1.2.zip)
- [EnvelopedSine 0.1.1 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/EnvelopedSine0.1.1.zip)
- [EnvelopedSine 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/EnvelopedSine0.1.0.zip)

## ライセンス
EnvelopedSine のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
