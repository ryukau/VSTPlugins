---
lang: ja
...

# BasicLimiter
![](img/basiclimiter.png)

<ruby>BasicLimiter<rt>ベーシック リミッタ</rt></ruby> は名前の通りベーシックなシングルバンドリミッタです。目新しい音は出ませんが、トゥルーピークモードはやや贅沢に設計しています。

- [BasicLimiter 0.1.2 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/BasicLimiter_0.1.2.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [プリセットをダウンロード (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/BasicLimiterPresets.zip)

BasicLimiter の利用には AVX 以降の SIMD 命令セットをサポートする CPU が必要です。

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit

2022-05-14 の時点ではユニバーサルバイナリのビルドに失敗するので、 macOS ビルドはパッケージに含まれていません。将来的に対応したいですが、お財布の問題で M1 mac 入手のめどが立たないので時期は未定です。

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

## 連絡先
何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか `ryukau@gmail.com` までお気軽にどうぞ。

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
つまみと数値スライダでは次の操作ができます。

- <kbd>Ctrl</kbd> + <kbd>左クリック</kbd> : 値のリセット。
- <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> : 細かい値の変更。

## 注意
アルゴリズムの問題で、入力信号の振幅がとんでもなく大きいときは過剰に振幅が制限されることがあります (オーバーリミッティング) 。オーバーリミッティングが起こると、入力の振幅が大きくなるほど出力の振幅が小さくなります。振幅が `2^53` 、つまり約 319 dB を超えたときに問題が起こると想定しています。

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

    ドラムなどの振幅の変化が急峻な音にかけるときは 0.02 秒以下に設定することを推奨します。 0.02 秒という値は[継時マスキング](https://ccrma.stanford.edu/~bosse/proj/node21.html)という人間の耳の聞こえ方の性質に基づいています。

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

## チェンジログ
- 0.1.2
  - 初期リリース。

### 旧バージョン
旧バージョンはありません。

## ライセンス
BasicLimiter のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
