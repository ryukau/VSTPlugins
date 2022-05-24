---
lang: ja
...

# BasicLimiter
![](img/basiclimiter.png)

<ruby>BasicLimiter<rt>ベーシック リミッタ</rt></ruby> は名前の通りベーシックなシングルバンドリミッタです。目新しい音は出ませんが、トゥルーピークモードはやや贅沢に設計しています。

- [BasicLimiter 0.1.4 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/BasicLimiter_0.1.4.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/BasicLimiterPresets.zip)

自動メイクアップゲイン、サイドチェイン、左右 (L-R) とミッド-サイド (M-S) の切り替えを追加した BasicLimiterAutoMake もあります。ただし CPU 負荷は 1.5 倍強に上がります。

- [BasicLimiterAutoMake 0.1.4 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/BasicLimiterAutoMake_0.1.4.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">

BasicLimiter と BasicLimiterAutoMake の利用には AVX 以降の SIMD 命令セットをサポートする CPU が必要です。

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
つまみと数値スライダでは次の操作ができます。

- <kbd>Ctrl</kbd> + <kbd>左クリック</kbd> : 値のリセット。
- <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> : 細かい値の変更。

## 注意
アルゴリズムの問題で、入力振幅がとんでもなく大きいと過剰に振幅が制限されることがあります (オーバーリミッティング) 。オーバーリミッティングが起こると、入力振幅が大きくなるほど出力振幅が小さくなります。振幅が `2^53` 、つまり約 319 dB を超えたときに問題が起こることが想定されます。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/basiclimiter.svg)

## パラメータ
Threshold \[dB\]

:   リミッタのしきい値です。

    `True Peak` がオフのとき、出力振幅はしきい値以下に制限されます。

    `True Peak` がオンのとき、出力振幅は完全にしきい値以下に制限されません。これはリアルタイムで使えるアルゴリズムには限界があるからです。

Gate \[dB\]

:   ゲートのしきい値です。 `Attack` で指定した時間を超えて入力振幅が `Gate` の値を下回ると、出力振幅が 0 になります。

Attack \[s\]

:   リミッタのエンベロープを滑らかにするフィルタの遷移時間です。 `Attack` の値はそのままレイテンシに加算されます。

    ドラムなどの振幅の変化が急峻な音にかけるときは 0.02 秒以下に設定することを推奨します。 0.02 秒という値は[継時マスキング](https://web.archive.org/web/20210624083625/http://ccrma.stanford.edu:80/~bosse/proj/node21.html)という人間の耳の聞こえ方の性質に基づいています。

Release \[s\]

:   リミッタのエンベロープが振幅を制限しない状態に戻るときの滑らかさです。

    内部では `Release` で設定された時間の逆数をカットオフ周波数として使っているので、表示されている値はあくまでも目安です。普段は `Sustain` と足し合わせた値が 0.2 秒以下になるよう設定することを推奨します。この値は継時マスキングに基づいています。

Sustain \[s\]

:   リミッタのエンベロープが `Attack` で設定した時間に加えて振幅のピークをホールドする時間です。

    ドラムなどにかけると `Release` よりもダッキングが目立つ傾向があります。歪ませたギターやサステイン中のシンセサイザなど、振幅が一定な音に対してはオートゲインと同じように働くので `Release` よりクリーンな出力が得られます。普段は `Sustain` と足し合わせた値が 0.2 秒以下になるよう設定することを推奨します。サステインは継時マスキングの曲線とは一致しない点に注意してください。

Stereo Link

:   `Stereo Link` が 0.0 のとき、左右のリミッタは独立して動作します。 1.0 のとき、すべてのチャンネルの振幅の最大値がすべてのリミッタに入力されます。以下は `Stereo Link` の計算式です。

    ```
    absL = fabs(leftInput).
    absR = fabs(rightInput).

    absMax = max(absL, absR).

    amplitudeL = absL + stereoLink * (absMax - absL).
    amplitudeR = absR + stereoLink * (absMax - absR).
    ```

    細かく設定するときは、まず `Stereo Link` を 0.0 に設定します。すると左右のどちらかに偏った信号が入力されたときに、中央に位置する信号が押し出されて、パンがふらつくように聞こえることがあります。このようなパンのふらつきが聞き取れなくなるまで `Stereo Link` の値を少しづつ増やしていけば、そのうち適切な値にたどり着きます。

True Peak

:   チェックを入れるとトゥルーピークモードがオンになります。トゥルーピークモードがオンのときは 94 サンプルのレイテンシが加わります。

    トゥルーピークモードがオンのときはナイキスト周波数に近い成分を落とすためのローパスフィルタがかかります。また、サンプルピークが `Threshold` で指定した値を超えることがあります。特にサンプルピークが 0 dB を超えたときは `Overshoot` の値が 0 でなくなります。このときは `Threshold` の値を下げてください。

    リアルタイム処理ではトゥルーピークをしきい値以下に抑えることは困難です。ナイキスト周波数に近い成分が含まれているとトゥルーピークの値が正確に計算できなくなるのでローパスフィルタをかけるのですが、しきい値以下への振幅の制限を徹底すると可聴域内の周波数成分が大きく低減されて音が変わってしまいます。 BasicLimiter は、およそ 18000 Hz 以下の成分についてはローパスフィルタの影響が及ばないように設計しています。

Reset Overshoot

:   クリックすると `Overshoot` の値を 0 にリセットします。

    出力のサンプルピークが 0 dB を超えると `Overshoot` の値が 0 でなくなり、 `Reset Overshoot` が点灯します。

### BasicLimiterAutoMake の固有パラメータ
Auto Make Up

:   チェックを入れると自動メイクアップゲインが有効になります。

    自動メイクアップゲインが有効のときに `Threshold` の値が `Auto Make Up Target Gain` を超えると出力振幅が小さくなります。

    自動メイクアップゲインが有効のときに、手やオートメーションで `Threshold` を上げると、オーバーシュートすることがあります。そのため、オーバーシュートでの歪みを防ぐために `Auto Make Up Target Gain` を -0.1 dB 以下に設定することを推奨します。また、入力信号があるときに `Threshold` を動かすのであれば BasicLimiterAutoMake の後にお守りとしてさらにもう一つリミッタを挿入することを推奨します。

Auto Make Up Target Gain

:   自動メイクアップゲインが適用された後の最大振幅です。 `Auto Make Up` の右側に配置されているコントロールで、単位はデシベルです。

    `Channel Type` が `M-S` のときは設定値の +6 dB が最大振幅となります。

Sidechain

:   チェックを入れるとサイドチェインが有効になります。サイドチェインが有効のとき、ソース信号の振幅は `Threshold` に影響されないので `Auto Make Up` は無効になります。

    BasicLimiterAutoMake は 2 つのステレオ入力を備えています。 1 番がソース信号、 2 番がサイドチェイン信号の入力です。ルーティングについてはお使いの DAW のマニュアルを参照してください。

Channel Type

:   ステレオチャンネルの種類を左右 (`L-R`) とミッド-サイド (`M-S`) で切り替えます。

    種類を `M-S` に設定すると、サンプルピークが `Threshold` を最大で +6 dB 上回ることがあります。そこで `Auto Make Up` を有効にして `M-S` を使うときは、 `Auto Make Up Target Gain` の値を -6.1 dB 以下に設定することを推奨します。この仕様は `L-R` と `M-S` を切り替えて比較するときに聴感上の音の大きさが同じになることを狙っています。

## チェンジログ
### BasicLimiter
- 0.1.4
  - `style.json` でカスタムフォントを設定するオプションを追加。
  - `Gate` が -inf dB のときにゲートがかからないように変更。
- 0.1.3
  - VSTGUI を 4.10 から 4.11 にアップデート。
- 0.1.2
  - 初期リリース。

#### BasicLimiterAutoMake
- 0.1.4
  - 初期リリース。

### 旧バージョン
### BasicLimiter
- [BasicLimiter 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/BasicLimiter_0.1.2.zip)

#### BasicLimiterAutoMake
現在、旧バージョンはありません。

## ライセンス
BasicLimiter のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
