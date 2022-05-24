---
lang: ja
...

# IterativeSinCluster
![](img/iterativesincluster.png)

<ruby>IterativeSinCluster<rt>イテレイティブ サイン クラスタ</rt></ruby>はノート 1 つあたり 512 のサイン波を計算する加算合成シンセサイザです。一体、何を考えていたのか iterative sin という言葉を名前に使っていますが、アルゴリズムの種類を表す正しい言葉は recursive sine です。

- [IterativeSinCluster 0.1.15 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/IterativeSinCluster_0.1.15.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/IterativeSinClusterPresets.zip)

IterativeSinCluster を使うには CPU が AVX 以降の SIMD 命令セットをサポートしている必要があります。

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
つまみとスライダーでは次の操作ができます。

- <kbd>Ctrl</kbd> + <kbd>左クリック</kbd> : 値のリセット。
- <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> : 細かい値の変更。

`Gain` 、 `Semi` などで使われている数値スライダーでは、上記に加えて次の操作ができます。

- <kbd>ホイールクリック</kbd> : 最小値、最大値の切り替え。

`Overtone` では次の操作ができます。

- <kbd>Ctrl</kbd> + <kbd>左ドラッグ</kbd> : 値のリセット。
- <kbd>ホイールドラッグ</kbd> : 直線の描画。

操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/iterativesincluster.svg)

## パラメータ
### Gain
Boost, Gain

:   どちらも音量を調整します。出力のピーク値は `Boost * Gain` となります。

    `Note` 、 `Chord` 、 `Overtone` のゲインを調整すると音がかなり小さくなることがあります。そんなときに `Boost` が使えます。

ADSR

:   音量エンベロープのパラメータです。

    - `A` (Attack) : 鍵盤を押した瞬間から最大音量になるまでのアタック時間。
    - `D` (Decay) : 最大音量からサステイン音量になるまでのディケイ時間。
    - `S` (Sustain) : アタックとディケイが終わった後に鍵盤を押し続けているときのサステイン音量。
    - `R` (Release) : 鍵盤から指を離したあとに音量が 0 になるまでのリリース時間。

Curve

:   音量エンベロープのアタックの曲線を変更するパラメータ。

### Shelving
`Low` と `High` の 2 つのシェルビングができます。

カットオフ周波数はノートの音程を基準として、 `Semi` で指定した半音単位で相対的に決まります。例えば音程が C4 、 `Semi` が 12.000 のときは カットオフ周波数は `C4 + 12 半音 = C5` になります。

変更する音量は `Gain` で調整できます。

- `Low` シェルビングは `Semi` で指定したカットオフ周波数**以下**のサイン波の音量を変更します。
- `High` シェルビングは `Semi` で指定したカットオフ周波数**以上**のサイン波の音量を変更します。

### Pitch
Add Aliasing

:   チェックを入れると、ナイキスト周波数より高い周波数が指定されたサイン波もレンダリングします。

    ナイキスト周波数とは、ざっくりいえば録音したデータが再現可能な最高周波数のことです。音を生成するときはナイキスト周波数以上の値を指定することができますが、折り返し（エイリアシング）と呼ばれる現象によって計算結果は予期しない周波数になってしまいます。 `Add Aliasing` は、こうしたエイリアシングノイズを足し合わせるオプションです。

Reverse Semi

:   チェックを入れると、 `Semi` の符号を逆転させます。たとえば `7.000` は `-7.000` として処理されます。

Octave

:   ノートのオクターブです。

ET

:   平均律の値です。この値によって `Semi` と `Milli` の調律を変更できます。例えば `ET` が 12 なら 12 平均律になります。 `ET` は Equal Temperament の略です。

    ホスト側から送られてくるノートの調律を変更するわけではないので注意してください。

Multiply, Modulo

:   サイン波の周波数を変更します。

    計算式は `ノートの周波数 * (1 + fmod(Multiply * pitch, Modulo))` のようになっています。 `pitch` は `Note` 、 `Chord` 、 `Overtone` で指定した値から計算されるピッチです。 `fmod(a, b)` は `a` を `b` で割った余りを計算する関数です。

### Smooth
特定の値を変更したときに、変更前の値から変更後の値に移行する時間（秒）です。

`Smooth` と関連するパラメータのリストです。 `*` はワイルドカードです。

- `Gain` セクションの全て
- `Chord.Pan`
- `Chorus` の `Key Follow` 以外

他のパラメータはノートオンのタイミングで指定されていた値をノートの発音が終わるまで続けて使います。

### nVoice
最大同時発音数です。

### Random
Retrigger

:   チェックを入れると、ノートオンごとに乱数シードをリセットします。

Seed

:   乱数のシード値です。この値を変えると生成される乱数が変わります。

To Gain

:   ノートオンごとに各サイン波の音量をランダマイズする度合いです。

To Pitch

:   ノートオンごとに各サイン波の周波数をランダマイズする度合いです。

### Note, Chord, Overtone
サイン波のピッチと音量を指定します。

1つの `Note` あたり 16 の `Overtone` が発音されます。8つの `Note` で 1 つの `Chord` になります。そして 4 つの `Chord` が使えます。

`Note` と `Chord` に共通のパラメータです。

- `Gain` : サイン波の音量。
- `Semi` : ノートの基本周波数からの距離。単位は半音。
- `Milli` : ノートの基本周波数からの距離。単位は 1 / 1000 半音。 1 / 10 セント。

`Overtone` は左端が基本周波数（第1倍音）の音量で、右に一つ進むごとに第2倍音、第3倍音、 ... 、第16倍音の音量を表しています。

サイン波の周波数の計算式は次のようになっています。

```
function toneToPitch(semi, milli):
  return 2 ^ (1000 * semi + milli) / (ET * 1000)

for each Chord:
  chordPitch = toneToPitch(Chord.semi, Chord.milli)
  for each Note:
    notePitch = toneToPitch(Note.semi, Note.milli)
    for each Overtone:
      frequency = midiNoteFrequency
        * (1 + mod(Multiply * Overtone * notePitch * chordPitch, Modulo))
```

### Chorus
Mix

:   Dry/Wet の比率を調整します。

Freq

:   コーラスの LFO の周波数です。中央から右に回すと正、左に回すと負の周波数を指定できます。

Depth

:   左右の広がりを調整します。

Range

:   LFO によってディレイ時間を変調する度合いです。 3 つのディレイが用意されています。

Time

:   ディレイ時間です。

Phase

:   LFO の位相です。

Offset

:   ディレイの間での位相差です。

Feedback

:   ディレイのフィードバックです。

Key Follow

:   チェックを入れると、ディレイ時間をノートの音程に応じて変更します。

## チェンジログ
- 0.1.15
  - `style.json` でカスタムフォントを設定するオプションを追加。
- 0.1.14
  - VSTGUI を 4.10 から 4.11 にアップデート。
- 0.1.13
  - フォントを Tinos に変更。
- 0.1.12
  - リセットが正しく行われるように修正。
  - `bypass` パラメータの挙動を修正。この修正によって、ホストがミュート中のプラグインにノートを送り続けても、ミュート解除とともにそれまでに送られたノートがすべて再生されなくなった。このバグは VST 3 の `bypass` パラメータを正しく実装しているホストでのみ発生していた。
- 0.1.11
  - Process context requirements を実装。
- 0.1.10
  - DSP が初期化されているかどうかのチェックを追加。
- 0.1.9
  - BarBox に アンドゥ・リドゥの機能を追加。
- 0.1.8
  - カラーコンフィグを追加。
- 0.1.7
  - パラメータの補間を可変サイズのオーディオバッファでも機能する以前の手法に巻き戻した。
  - ピッチベンドを有効化。
- 0.1.6
  - BarBox にフォーカスした後、マウスカーソルを領域外に動かしている間はショートカットが無効になるように変更。
- 0.1.5
  - 文字列の描画でクラッシュするバグを修正。
- 0.1.4
  - プラグインタイトルをクリックすると表示されるポップアップの表示方法の変更。
- 0.1.3
  - PreSonus Studio One 4.6.1 で出力にノイズが乗るバグを修正。
- 0.1.2
  - Linux ビルドの GUI を有効化。
- 0.1.1
  - macOS ビルドの修正。
  - AVX, AVX512 命令セットのコードパスを追加。
  - Ableton Live 10.1.6 で特定のノブが揺れ戻るバグを修正。
- 0.1.0
  - 初期リリース。

### 旧バージョン
- [IterativeSinCluster 0.1.13 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/IterativeSinCluster_0.1.13.zip)
- [IterativeSinCluster 0.1.12 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ResetAndMuteFix/IterativeSinCluster_0.1.12.zip)
- [IterativeSinCluster 0.1.11 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/IterativeSinCluster0.1.11.zip)
- [IterativeSinCluster 0.1.10 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/IterativeSinCluster0.1.10.zip)
- [IterativeSinCluster 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L4Reverb0.1.0/IterativeSinCluster0.1.9.zip)
- [IterativeSinCluster 0.1.8 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/IterativeSinCluster0.1.8.zip)
- [IterativeSinCluster 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/IterativeSinCluster0.1.7.zip)
- [IterativeSinCluster 0.1.6 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BarBoxFocusFix/IterativeSinCluster0.1.6.zip)
- [IterativeSinCluster 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/IterativeSinCluster0.1.5.zip)
- [IterativeSinCluster 0.1.3 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/IterativeSinCluster0.1.3.zip)
- [IterativeSinCluster 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/IterativeSinCluster0.1.2.zip)
- [IterativeSinCluster 0.1.1 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/IterativeSinCluster0.1.1.zip)
- [IterativeSinCluster 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/IterativeSinCluster0.1.0.zip)

## ライセンス
IterativeSinCluster のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
