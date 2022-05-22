---
lang: ja
...

# FDNCymbal
![](img/fdncymbal.png)

<ruby>FDNCymbal<rt>エフディーエヌ シンバル</rt></ruby> はシンバルのような音を合成するシンセサイザです。エフェクトとして使うこともできます。名前とは裏腹に金属的な質感は FDN (feedback delay network) ではなく Schroeder allpass section によって得られています。 `FDN.Time` の値を小さくすることで、わりとナイスなばちの衝突音が合成できます。シンバルの揺れをシミュレートするためにトレモロもついています。

- [FDNCymbal 0.2.7 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/FDNCymbal_0.2.7.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BarBoxFocusFix/FDNCymbalPresets.zip)

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit

2022-05-14 の時点ではユニバーサルバイナリのビルドに失敗するので、 macOS ビルドはパッケージに含まれていません。将来的に対応したいですが、お財布の問題で M1 mac 入手のめどが立たないので時期は未定です。

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

## 連絡先
何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか `ryukau@gmail.com` までお気軽にどうぞ。

[paypal.me/ryukau](paypal.me/ryukau) から開発資金を投げ銭することもできます。現在の目標は macOS と ARM ポートのための M1 mac の購入資金を作ることです。 🤑💸💻

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

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

## 注意
バージョン 0.1.x と 0.2.x は互換性がありません。

`FDN.Feedback` の値が 0 でないときは信号が発散することがあります。発散したときは `FDN.Feedback` を一番左まで回してください。

`HP Cutoff` の値を短時間に大きく変更すると直流が乗ることがあります。直流が乗ったときは、いったん `HP Cutoff` を上げてから <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> でゆっくりと下げなおしてください。

## エフェクトとしての利用
FDNCymbal はエフェクトとして使うこともできますが、直感的でない設定が必要となっています。

ロード直後の FDNCymbal は初期化時に音が止まることを防ぐためにミュートされています。有効化するためには FDNCymbal に  MIDI ノートオンを送る必要があります。ベロシティによって出力音量が変わるので注意してください。また `Seed` と `FDN` セクションの値を変更したときは新しいノートオンを送るまで更新されません。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/fdncymbal.svg)

## パラメータ
### Gain
出力音量です。

### Smooth
次のパラメータを変更したときに、変更前の値から変更後の値へと移行する秒数です。

- `Gain`
- `FDN.Time`
- `FDN.Feedback`
- `FDN.CascadeMix`
- `Allpass.Mix`
- `Allpass.Stage1.Time`
- `Allpass.Stage1.Feedback`
- `Allpass.Stage2.Time`
- `Allpass.Stage2.Feedback`
- `Tremolo.Mix`
- `Tremolo.Depth`
- `Tremolo.Frequency`
- `Tremolo.DelayTime`

### Stick
`Stick` がオンのとき、ノートオンのたびにばちでシンバルを叩きます。ノートの高さでノイズの密度とトーンの高さが変わります。

Pulse

:   ばちオシレータが出力するインパルスの音量です。

Tone

:   ばちオシレータが出力するサイン波によるトーンの音量です。

Velvet

:   ばちオシレータが出力するベルベットノイズの音量です。

    ベルベットノイズはランダムな間隔で異なる高さのインパルスが出力されるノイズのアルゴリズムです。

Decay

:   ばちオシレータの減衰時間です。

### Random
Seed

:   乱数のシード値です。

Retrigger.Time

:   チェックを外すと、次の 3 つのパラメータでコントロールされる内部的なディレイ時間がノートオンのたびに変わります。

    - `FDN.Time`
    - `Allpass.Stage1.Time`
    - `Allpass.Stage2.Time`

Retrigger.Stick

:   チェックを外すと、ノートオンのたびにばちオシレータの音が変わります。

Retrigger.Tremolo

:   チェックを外すと、トレモロのパラメータがノートオンのたびに変わります。

### FDN
オンのときはFDN (feedback delay network) セクションを信号が通過します。

Time

:   FDN のディレイ時間を調整します。内部的なディレイ時間はノブで設定した値をもとにランダマイズされます。

Feedback

:   FDN セクションのフィードバックです。この値が 0 でないときは信号が発散することがあるので注意してください。

CascadeMix

:   直列につないだ複数の FDN の出力のミックスを調整します。

### Allpass
Mix

:   Schroeder allpass section のミックスを調整します。

### Stage 1 と Stage 2
2 つの Schroeder allpass section が直列につながっています。 Stage 2 では 4 つの Schroeder allpass section が並列接続されています。

Time

:   Schroeder allpass section のディレイ時間の最大値です。ディレイ時間は内部的にランダマイズされます。

Feedback

:   Schroeder allpass section のフィードバックです。

HP Cutoff

:   Schroeder allpass section の出力にかかるハイパスフィルタのカットオフ周波数です。

Tanh

:   チェックを入れると Stage 1 のフィードバックにサチュレーションがかかります。

### Tremolo
Mix

:   トレモロのミックスを調整します。

Depth

:   `Depth` が大きいほどトレモロによる音量の変化が大きくなります。

Frequency

:   トレモロに使われる LFO の周波数です。

DelayTime

:   LFO によって変化するディレイ時間の最大値です。シンバルの揺れによって生じるドップラー効果を表現しています。

### Random (Tremolo)
Depth

:   ノートオンのたびに `Tremolo.Depth` をランダマイズする度合いです。

Freq

:   ノートオンのたびに `Tremolo.Frequency` をランダマイズする度合いです。

Time

:   ノートオンのたびに `Tremolo.DelayTime` をランダマイズする度合いです。

## チェンジログ
- 0.2.7
  - フォントを Tinos に変更。
- 0.2.6
  - リセットが正しく行われるように修正。
  - `bypass` パラメータの挙動を修正。この修正によって、ホストがミュート中のプラグインにノートを送り続けても、ミュート解除とともにそれまでに送られたノートがすべて再生されなくなった。このバグは VST 3 の `bypass` パラメータを正しく実装しているホストでのみ発生していた。
- 0.2.5
  - Process context requirements を実装。
- 0.2.4
  - DSP が初期化されているかどうかのチェックを追加。
- 0.2.3
  - カラーコンフィグを追加。
- 0.2.2
  - パラメータの補間を可変サイズのオーディオバッファでも機能する以前の手法に巻き戻した。
- 0.2.1
  - 文字列の描画でクラッシュするバグを修正。
- 0.2.0
  - プラグインタイトルをクリックすると表示されるポップアップの表示方法の変更。
  - `Pulse` パラメータの追加。
  - `Velvet` パラメータの追加。
  - `ToneMix` の名前を `Tone` に変更。値の範囲も [0, 0.002] から [0, 0.02] に変更。
  - サイン波の周波数を割り当てるアルゴリズムを変更。
  - ベルベットノイズオシレータのインパルスの間隔がランダムで無かったバグを修正。
- 0.1.5
  - 非アクティブ化で音が止まるように修正。
  - PreSonus Studio One 4.6.1 で出力にノイズが乗るバグを修正。
- 0.1.4
  - Linux ビルドの GUI を有効化。
- 0.1.3
  - Steinberg のホストで音が止まるバグを修正。
- 0.1.2
  - 複数の GUI インスタンス間で表示を同期するように変更。
  - スプラッシュスクリーンを開くとクラッシュするバグを修正。
  - リロード時にクラッシュするバグを修正。
  - GUI がホストのオートメーションに応じて更新されるように修正。
- 0.1.1
  - ノートオン・オフが正確なタイミングでトリガされるように修正。
  - `Allpass.Stage1.Feedback` が無効になっていたバグを修正。
- 0.1.0
  - 初期リリース。

### 旧バージョン
- [FDNCymbal 0.2.6 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ResetAndMuteFix/FDNCymbal_0.2.6.zip)
- [FDNCymbal 0.2.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/FDNCymbal0.2.5.zip)
- [FDNCymbal 0.2.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/FDNCymbal0.2.4.zip)
- [FDNCymbal 0.2.3 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/FDNCymbal0.2.3.zip)
- [FDNCymbal 0.2.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/FDNCymbal0.2.2.zip)
- [FDNCymbal 0.2.1 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/FDNCymbal0.2.1.zip)
- [FDNCymbal 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/FDNCymbal0.1.5.zip)
- [FDNCymbal 0.1.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/FDNCymbal0.1.4.zip)
- [FDNCymbal 0.1.3 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/FDNCymbal0.1.3.zip)
- [FDNCymbal 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/FDNCymbal0.1.2.zip)

旧バージョン向けのプリセットです。

- [バージョン 0.1.* 用のプリセット (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/FDNCymbalPresets.zip)

## ライセンス
FDNCymbal のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
