---
lang: ja
...

# SyncSawSynth
![](img/syncsawsynth.png)

<ruby>SyncSawSynth<rt>シンクソウシンセ</rt></ruby>は10次までのPTR鋸歯波オシレータが使える32ボイスのポリフォニックシンセサイザです。主にハードシンクと周波数変調で電池が切れかけのおもちゃのような音を作る用途に向いています。もちろん普通の音も出ます。

- [SyncSawSynth 0.1.15 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/SyncSawSynth0.1.15.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/SyncSawSynthPresets.zip)

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
つまみとスライダーでは次の操作ができます。

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

## 注意
アタックの長さによって音量エンベロープのピーク値が変わります。音量エンベロープのアタックが小さいとき、ピークは通常より少し高くなることがあります。

Dirty Filter にチェックを入れるとノートオンによってプチノイズが出ることがあります。

パラメータの組み合わせによっては直流信号が乗るときがあります。ディストーションなどをかけると面白い音になることがあるので、直流は切っていません。安全のためには SyncSawSynth の後にハイパスフィルタを通すことが望ましいです。またオシロスコープで出力を監視することをお勧めします。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。
![](img/syncsawsynth.svg)

## 用語について
SyncSawSynth のオシレータは内部に 2 つのオシレータを持っています。 1 つは波形生成のために使われ、もう 1 つはハードシンクをトリガするために使われています。ハードシンクに関する用語として、波形生成のためのオシレータのことをスレーブ、トリガに使われるオシレータのことをマスタと呼んでいます。

## パラメータ
### Osc
Gain

:   オシレータの音量。範囲は 0.0 から 1.0 。

Semi

:   オシレータのピッチ。範囲は -24.0 から 24.0 。単位は半音。

    値は `floor` 関数で整数に丸められます。例えば 6.3 なら 6 、 -11.5 なら -12 になります。

Cent

:   オシレータのピッチ。範囲は -100.0 から 100.0 。単位はセント。

Sync と SyncType

:   `Sync` の値は `SyncType` の値によって意味が変わります。 `Sync` の値の範囲は 0.01 から 16.0 です。

    `SyncType` は次の4つから選択できます。

    - `Off`
    - `Ratio`
    - `Fixed-Master`
    - `Fixed-Slave`

    `Off` を選ぶとマスタ周波数を 0 Hz にすることでハードシンクを無効にします。ただし `Modulation` の `Osc*->Sync1` が 0 より大きいときはマスタ位相が動くのでハードシンクがかかることがあります。 `SyncType` が `Off` のときは、 `Sync` の値は使われません。

    `Ratio` を選ぶと音程をマスタ周波数に使います。スレーブ周波数はマスタ周波数に `Sync` の値をかけ合わせた値になります。また `Ratio` のときだけ `To Freq/Sync` の変調先が入れ替わります。

    `Fixed-Master` を選ぶと音程をスレーブ周波数に使います。マスタ周波数は `Sync` の値のみによって決まります。

    `Fixed-Slave` を選ぶと音程をマスタ周波数に使います。スレーブ周波数は `Sync` の値のみによって決まります。

    `Fixed-Master` と `Fixed-Slave` で使われる `Sync` の値を周波数に変換する式は次の通りです。

    ```
    frequency = 2 * Sync^3
    ```

OscType

:   オシレータの波形。

    0 から 10 次までの PTR 鋸歯波オシレータとサイン波が選べます。

Phase

:   オシレータの位相。範囲は 0.0 から 1.0 。

    `Lock` にチェックを入れるとノートオンのたびに位相がリセットされます。

    `Osc2` の `Invert` にチェックを入れると、 `Osc2` の出力の正負の符号を変えることで位相を反転します。

Unison

:   チェックを入れるとノートごとに位相の異なる同じ高さの音を重ねます。CPU負荷も2倍になります。

Number of Voice

:   最大同時発音数。この値を減らすとCPU負荷を減らすことができます。

### Modulation
Osc1->Sync1

:   `Osc1` の出力による `Osc1` のマスタ周波数の変調。範囲は 0.0 から 1.0 。

Osc1->Freq2

:   `Osc1` の出力による `Osc2` のスレーブ周波数の変調。範囲は 0.0 から 1.0 。

Osc2->Sync1

:   `Osc2` の出力による `Osc1` のマスタ周波数の変調。範囲は 0.0 から 1.0 。

Attack

:   変調用の AD エンベロープのアタック。範囲は 0.0 から 4.0 。単位は秒。

    AD エンベロープの式です。

    ```
    env(t) := t^a * exp(-b * t)

    t: 時間。
    a, b: 適当な定数。
    ```

Curve

:   変調用の AD エンベロープの特性。範囲は 1.0 から 96.0 。

    `Curve` の値に応じてディケイの長さが決まります。

To Freq1 (AD エンベロープ)

:   AD エンベロープによる `Osc1` のスレーブ周波数の変調。範囲は 0.0 から 16.0 。

To Sync1 (AD エンベロープ)

:   AD エンベロープによる `Osc1` のマスタ周波数の変調。範囲は 0.0 から 16.0 。

To Freq2 (AD エンベロープ)

:   AD エンベロープによる `Osc２` のスレーブ周波数の変調。範囲は 0.0 から 16.0 。

To Sync2 (AD エンベロープ)

:   AD エンベロープによる `Osc２` のマスタ周波数の変調。範囲は 0.0 から 16.0 。

LFO

:   LFO の周波数。範囲は 0.01 から 20.0 。単位は Hz 。

NoiseMix

:   LFO とピンクノイズのミックスの比率。範囲は 0.0 から 1.0 。

    値が 0.0 のときに LFO のみ、 1.0 のときにピンクノイズのみになります。

    ピンクノイズによる変調はスパイクを生じることがあるので注意してください。スパイクを避けるには `To Sync/Freq` の値を 1.0 より小さく設定してください。スパイクは0dBを超えないようにクリップされています。

To Freq1 (LFO/ノイズ)

:   LFO/ノイズによる `Osc1` のスレーブ周波数の変調。範囲は 0.0 から 16.0 。

To Sync1 (LFO/ノイズ)

:   LFO/ノイズによる `Osc1` のマスタ周波数の変調。範囲は 0.0 から 16.0 。

To Freq2 (LFO/ノイズ)

:   LFO/ノイズによる `Osc２` のスレーブ周波数の変調。範囲は 0.0 から 16.0 。

To Sync2 (LFO/ノイズ)

:   LFO/ノイズによる `Osc２` のマスタ周波数の変調。範囲は 0.0 から 16.0 。

### Gain
Gain

:   シンセサイザの出力音量。範囲は 0.0 から 1.0 。

A

:   音量エンベロープのアタックの長さ。範囲は 0.0001 から 16.0 。単位は秒。

D

:   音量エンベロープのディケイの長さ。範囲は 0.0001 から 16.0 。単位は秒。

S

:   音量エンベロープのサステインの大きさ。範囲は 0.0 から 1.0 。

R

:   音量エンベロープのリリースの長さ。範囲は 0.0001 から 16.0 。単位は秒。

Curve

:   音量エンベロープの特性。範囲は 0.0 から 1.0 。

    素の音量エンベロープと tanh によって飽和させたエンベロープとを線形補間しています。

    ```
    gainEnv(t) := expEnv(t) * (tanh(3 * Curve * expEnv(t)) - expEnv(t))
    ```

### Filter
Cut

:   フィルタのカットオフ周波数。範囲は 20.0 から 20000.0 。単位は Hz 。

Res

:   2 次フィルタのレゾナンス。範囲は 0.001 から 1.0 。

    値を最小にすると音量がかなり小さくなります。

Feed

:   フィルタセクションのフィードバック。範囲は 0.0 から 1.0 。

    直列につないだ 4 つの 2 次フィルタの出力を 1 つ目のフィルタにフィードバックしています。

Sat

:   フィルタのサチュレーション。範囲は 0.01 から 8.0 。

    フィードバックと入力を足し合わせた値をウェーブシェーパに入力するときにかけ合わせる値です。

    ```
    filterIn = shaper(Sat * (input - Feed * filterOut))
    ```

Dirty Buffer

:   チェックを外すとノートオンのたびにフィルタのバッファをクリアします。

    チェックを入れている間はノートオンのたびに音の始まりにプチノイズが乗ることがあります。

FilterType

:   フィルタの種類。

    - `LP` : ローパス
    - `HP` : ハイパス
    - `BP` : バンドパス
    - `Notch` : ノッチ (バンドストップ)
    - `Bypass` : フィルタの迂回

ShaperType

:   フィルタのサチュレーションに使うウェーブシェーパの種類。

    ```
    HardClip(x) := clamp(x, -1.0, 1.0)
    Tanh(x)     := tanh(x)
    ShaperA(x)  := sin(2 * pi * x) / (1 + 10 * x * x)
    ShaperB(x)  := 0.7439087749328765 * x^3 * exp(-abs(x))
    ```

A

:   フィルタエンベロープのアタックの長さ。範囲は 0.0001 から 16.0 。単位は秒。

D

:   フィルタエンベロープのディケイの長さ。範囲は 0.0001 から 16.0 。単位は秒。

S

:   フィルタエンベロープのサステインの大きさ。範囲は 0.0 から 1.0 。

R

:   フィルタエンベロープのリリースの長さ。範囲は 0.0001 から 16.0 。単位は秒。

To Cut

:   フィルタエンベロープによるカットオフ周波数の変調。範囲は -1.0 から 1.0 。

To Res

:   フィルタエンベロープによるレゾナンスの変調。範囲は 0.0 から 1.0 。

Key->Cut

:   ノートの MIDI ノート番号によるカットオフ周波数の変調。範囲は -1.0 から 1.0 。

Key->Feed

:   ノートの MIDI ノート番号によるフィードバックの変調。範囲は -1.0 から 1.0 。

## チェンジログ
- 0.1.15
  - Process context requirements を実装。
- 0.1.14
  - DSP が初期化されているかどうかのチェックを追加。
- 0.1.13
  - カラーコンフィグを追加。
- 0.1.12
  - パラメータの補間を可変サイズのオーディオバッファでも機能する手法に変更。
- 0.1.11
  - 文字列の描画でクラッシュするバグを修正。
- 0.1.10
  - プラグインタイトルをクリックすると表示されるポップアップの表示方法の変更。
- 0.1.9
  - PreSonus Studio One 4.6.1 で出力にノイズが乗るバグを修正。
- 0.1.8
  - Linux ビルドの GUI を有効化。
- 0.1.7
  - 複数の GUI インスタンス間で表示を同期するように変更。
  - スプラッシュスクリーンを開くとクラッシュするバグを修正。
  - GUI がホストのオートメーションに応じて更新されるように修正。
- 0.1.6
  - ノートオン・オフが正確なタイミングでトリガされるように修正。
- 0.1.5
  - スムーシングのアルゴリズムの値が決められた範囲を超えるバグを修正。
- 0.1.4
  - semi コントロールのノブが番号を表示するように変更。
- 0.1.3
  - ユニゾンパラメータを追加。
  - 最大同時発音数パラメータを追加。
  - 内部のパラメータ構造を変更。
  - UI の見た目を変更。
  - フィルタと数学関数の高速化。
- 0.1.1
  - ピッチベンドに対応。
  - double 型の PTR を追加。
  - 音量エンベロープをアタック中にリリースすると、リリースがサステインの大きさから始まるバグを修正。
- 0.1.0
  - 初期リリース。

### 旧バージョン
- [SyncSawSynth 0.1.14 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/SyncSawSynth0.1.14.zip)
- [SyncSawSynth 0.1.13 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/SyncSawSynth0.1.13.zip)
- [SyncSawSynth 0.1.12 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/SyncSawSynth0.1.12.zip)
- [SyncSawSynth 0.1.11 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/DrawStringFix/SyncSawSynth0.1.11.zip)
- [SyncSawSynth 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EsPhaser0.1.0/SyncSawSynth0.1.9.zip)
- [SyncSawSynth 0.1.8 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LinuxGUIFix/SyncSawSynth0.1.8.zip)
- [SyncSawSynth 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/IterativeSinCluster0.1.0/SyncSawSynth0.1.7.zip)

## ライセンス
SyncSawSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
