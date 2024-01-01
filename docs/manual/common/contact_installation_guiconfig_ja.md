## 連絡先
何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか `ryukau@gmail.com` までお気軽にどうぞ。

[paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau) から開発資金を投げ銭することもできます。

## インストール
### プラグイン
名前が `.vst3` で終わるディレクトリを OS ごとに決められた位置に配置してください。

- Windows では `/Program Files/Common Files/VST3/` に配置します。
- Linux では `$HOME/.vst3/` に配置します。
- macOS では `/Library/Audio/Plug-ins/VST3/` あるいは `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/` に配置します。

DAW によっては上記とは別に VST3 をインストールできるディレクトリを提供していることがあります。詳しくは利用している DAW のマニュアルを参照してください。

### プリセット
**注意**: プリセットの無いプラグインもあります。

プリセットはページの最上部のリンクからダウンロードできるプラグインパッケージに含まれています。以下はプリセットのインストール手順です。

1. ページの最上部のリンクからプラグインをダウンロード。
2. ダウンロードした zip ファイルを解凍。
3. `presets` ディレクトリ内の `Uhhyou` ディレクトリを OS ごとに決められたディレクトリに配置。

以下は OS ごとのプリセットの配置先の一覧です。

- Windows : `/Users/$USERNAME/Documents/VST3 Presets`
- Linux : `$HOME/.vst3/presets`
- macOS : `/Users/$USERNAME/Library/Audio/Presets`

プリセットディレクトリの名前はプラグインと同じである必要があります。配置先のディレクトリが無いときは作成してください。

- [Preset Locations - VST 3 Developer Portal](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Locations+Format/Preset+Locations.html)

### Windows
プラグインが DAW に認識されないときは C++ redistributable をインストールしてみてください。インストーラは次のリンクからダウンロードできます。ファイル名は `vc_redist.x64.exe` です。

- [The latest supported Visual C++ downloads](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

### Linux
Ubuntu 22.04 では次のパッケージのインストールが必要です。

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

もし DAW がプラグインを認識しないときは、下のリンクの `Package Requirements` を参考にして VST3 に必要なパッケージがすべてインストールされているか確認してください。

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

REAPER の Linux 版がプラグインを認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

### macOS
**重要**: `full` パッケージは動作未確認です。以下のファイルは `codesign` されていないので削除する必要があるかもしれません。

- `Contents/Resources/Documentation`
- `Contents/x86_64-linux`
- `Contents/x86_64-win`

`macOS` パッケージでは上記のファイルの削除は不要です。また、すべてのパッケージは `codesign` コマンドの ad-hoc signing の状態になっています。

#### 隔離の解除
プラグインの初回起動時に「破損している」という趣旨のメッセージが表示されることがあります。この場合は、ターミナルを開いて、解凍した `.vst3` ディレクトリに次のコマンドのどちらか、あるいは両方を適用してみてください。 `/path/to/PluginName.vst3` の部分はインストールしたプラグインのパスに置き換えてください。

```sh
xattr -rd com.apple.quarantine /path/to/PluginName.vst3
xattr -rc /path/to/PluginName.vst3
```

#### Gatekeeper の迂回
プラグインは署名されていない (unsigned) 、または公証されていない (un-notarized) アプリケーションとして認識されることがあります。この場合は、警告メッセージが表示された後に、システム環境設定を開いて「セキュリティとプライバシー」 → 「一般」の「このまま開く」ボタンを押してください。以下にリンクした Apple 公式のヘルプページにスクリーンショット付きで同じ手順が掲載されています。「ノータリゼーションを受けていない App や未確認の開発元の App を開きたい場合」を参照してください。

- [Mac で App を安全に開く - Apple サポート (日本)](https://support.apple.com/ja-jp/HT202491)

ここまでの手順を経てまだ動かないときはインストール先を `/Library/Audio/Plug-ins/VST3/` あるいは `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/` のいずれか使っていなかったほうに変えてみてください。

上記の方法がすべて上手くいかなかったときは以下の手順を試してみてください。

1. ターミナルを開いて `sudo spctl --master-disable` を実行。
2. システム環境設定を開いて「セキュリティとプライバシー」 → 「一般」 → 「ダウンロードしたアプリケーションの実行許可」と辿り、「全てのアプリケーションを許可」を選択。

上記の手順を実行するとシステムのセキュリティが弱くなるので注意してください。元に戻すには以下の手順を実行してください。

1. システム環境設定を開いて「セキュリティとプライバシー」 → 「一般」 → 「ダウンロードしたアプリケーションの実行許可」と辿り、「App Store と認証済みの開発元からのアプリケーションを許可」を選択。
2. ターミナルを開いて `sudo spctl --master-enable` を実行。

#### `codesign` の適用
**注意** この節の内容は `macOS` パッケージには関連しないと考えられますが未検証です。 ([GitHub の issue](https://github.com/ryukau/VSTPlugins/issues/27))

`full` パッケージを利用する場合は以下のコマンドを適用することで利用できるかもしれません。

```sh
sudo codesign --force --deep -s - /path/to/PluginName.vst3
```

`codesign` の利用には [Xcode](https://developer.apple.com/xcode/) のインストールが必要となるかもしれません。

#### 参考リンク
- [Safely open apps on your Mac - Apple Support](https://support.apple.com/en-us/HT202491)
- [java - “libprism_sw.dylib” cannot be opened because the developer cannot be verified. on mac JAVAFX - Stack Overflow](https://stackoverflow.com/questions/66891065/libprism-sw-dylib-cannot-be-opened-because-the-developer-cannot-be-verified-o)
- [How to Fix App “is damaged and can’t be opened. You should move it to the Trash” Error on Mac](https://osxdaily.com/2019/02/13/fix-app-damaged-cant-be-opened-trash-error-mac/)
- [Allowing unsigned/un-notarized applications/plugins in Mac OS | Venn Audio](https://web.archive.org/web/20221223004334/https://www.vennaudio.com/allowing-unsigned-un-notarized-applications-plugins-in-mac-os/)
- [codesign Man Page - macOS - SS64.com](https://ss64.com/osx/codesign.html)

## GUI の見た目の設定
初回設定時は手動で次のファイルを作成してください。

- Windows では `/Users/ユーザ名/AppData/Roaming/UhhyouPlugins/style/style.json` 。
- Linux では `$XDG_CONFIG_HOME/UhhyouPlugins/style/style.json` 。
  - `$XDG_CONFIG_HOME` が空のときは `$HOME/.config/UhhyouPlugins/style/style.json` 。
- macOS では `/Users/$USERNAME/Library/Preferences/UhhyouPlugins/style/style.json` 。

プラグインのウィンドウを開くたびに `style.json` が読み込まれて更新されます。

既存の色のテーマを次のリンクに掲載しています。 `style.json` にコピペして使ってください。

- [VSTPlugins/package/style/themes at master · ryukau/VSTPlugins](https://github.com/ryukau/VSTPlugins/tree/master/package/style/themes)

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
