# EnvelopedSine
![](img/envelopedsine.png)

<ruby>EnvelopedSine<rt>エンベロープド サイン</rt></ruby>はノート 1 つあたり 64 のサイン波を計算する加算合成シンセサイザです。各サイン波に AD エンベロープとサチュレータがついているので IterativeSinCluster よりもパーカッシブな音が得意です。

- [EnvelopedSine 0.1.0 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/EnvelopedSine0.1.0/EnvelopedSine0.1.0.zip) <img
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

Linux ビルドは Ubuntu 18.0.4 でビルドしています。また Bitwig 3.0.3 と REAPER 5.983 で動作確認を行っています。どちらも GUI の表示に問題があったので、今のところ Linux ビルドでは GUI を無効にしています。

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

### Linux
Ubuntu 18.0.4 では次のパッケージのインストールが必要です。

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

もし DAW がプラグインを認識しないときは、下のリンクの `Package Requirements` を参考にして VST3 に必要なパッケージがすべてインストールされているか確認してみてください。

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

REAPER 5.983 の Linux 版がプラグインを認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

## 操作
操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

つまみとスライダーでは次の操作ができます。

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

`Octave` 、 `Seed` などで使われている数値スライダーでは、上記に加えて次の操作ができます。

- ホイールクリック : 最小値、最大値の切り替え。

倍音のコントロール `Attack` 、 `Decay` 、 `Gain` 、 `Saturation` ではショートカットが使えます。ショートカットはコントロールを左クリックして、フォーカスしたときだけ有効になります。左下にあるプラグインのタイトルをクリックするとショートカットの一覧を見ることができます。

| 入力                                    | 操作                                       |
| --------------------------------------- | ------------------------------------------ |
| <kbd>Ctrl</kbd> + <kbd>Left Click</kbd> | デフォルト値にリセット                     |
| <kbd>Mouse Middle Drag</kbd>            | 直線の描画                                 |
| <kbd>a</kbd>                            | 昇順にソート                               |
| <kbd>d</kbd>                            | 降順にソート                               |
| <kbd>f</kbd>                            | ローパスフィルタ                           |
| <kbd>F</kbd>                            | ハイパスフィルタ                           |
| <kbd>i</kbd>                            | 値の反転 (最小値を保存)                    |
| <kbd>I</kbd>                            | 値の反転 (最小値を 0 に設定)               |
| <kbd>n</kbd>                            | 最大値を 1 に正規化 (最小値もスケーリング) |
| <kbd>N</kbd>                            | 最大値を 1 に正規化 (最小値を 0 に設定)    |
| <kbd>h</kbd>                            | 高域の強調                                 |
| <kbd>l</kbd>                            | 低域の強調                                 |
| <kbd>p</kbd>                            | ランダムに並べ替え                         |
| <kbd>r</kbd>                            | ランダマイズ                               |
| <kbd>R</kbd>                            | まばらなランダマイズ                       |
| <kbd>s</kbd>                            | 少しだけランダマイズ                       |
| <kbd>,</kbd> (Comma)                    | 左に回転                                   |
| <kbd>.</kbd> (Period)                   | 右に回転                                   |
| <kbd>1</kbd>                            | 奇数インデックスの値を低減                 |
| <kbd>2</kbd>-<kbd>9</kbd>               | インデックスが 2n-9n の値を低減            |

## ブロック線図
![](img/envelopedsine.svg)

## パラメータ
### Overtone
右上にある 4 つの大きなコントロールです。

#### Attack, Decay
各オシレータの音量エンベロープのアタックとディケイです。

#### Gain
各オシレータの音量です。

#### Saturation
各オシレータのサチュレーションの大きさです。

### Gain
#### Boost, Gain
どちらも音量を調整します。出力のピーク値は `Boost * Gain` となります。

### Pitch
#### Add Aliasing
チェックを入れると、ナイキスト周波数より高い周波数が指定されたサイン波もレンダリングします。

ナイキスト周波数とは、ざっくりいえば録音したデータが再現可能な最高周波数のことです。音を生成するときはナイキスト周波数以上の値を指定することができますが、折り返し（エイリアシング）と呼ばれる現象によって計算結果は予期しない周波数になってしまいます。 `Add Aliasing` は、こうしたエイリアシングノイズを足し合わせるオプションです。

#### Octave
ノートのオクターブです。

#### Multiply, Modulo
サイン波の周波数を変更します。

計算式は `ノートの周波数 * (1 + fmod(Multiply * pitch, Modulo))` のようになっています。 `pitch` はノートの音程と倍音のインデックスから計算される係数です。 `fmod(a, b)` は `a` を `b` で割った余りを計算する関数です。

#### Expand
倍音コントロールのインデックスを拡大・縮小するスケーリング係数です。

![](img/envelopedsine_expand.svg)

#### Shift
倍音コントロールのインデックスを右シフトする量です。

![](img/envelopedsine_shift.svg)

### Random
ランダマイズに関するパラメータです。 `Unison` にチェックを入れた状態で使うと効果的です。

#### Retrigger
チェックを入れると、ノートオンごとに乱数列をリセットします。

#### Seed
乱数のシード値です。この値を変えると生成される乱数が変わります。

#### To Gain, To Attack, To Decay, To Sat.
倍音コントロールの値をランダマイズします。ランダマイズの式は `value * random` です。 `random` の値の範囲は `[0.0, 1.0)` です。

#### To Pitch
ピッチをランダマイズします。

#### To Phase
位相をランダマイズします。

### Misc.
#### Smooth
特定の値を変更したときに、変更前の値から変更後の値に移行する時間（秒）です。

`Smooth` と関連するパラメータのリストです。 `*` はワイルドカードです。

- `Gain` セクションの全て
- `Phaser` セクションの `nStages` 以外

他のパラメータはノートオンのタイミングで指定されていた値を、ノートの発音が終わるまで使い続けます。

#### nVoices
最大同時発音数です。

#### Unison
チェックを入れるとユニゾンを有効にします。

ユニゾンが有効になると 1 つのノートについて左右のチャンネルに 1 つずつボイスを配置します。 `Random.To Phase` や `Random.To Pitch` と組み合わせることで広がりのある音を作ることができます。

### Modifier
#### Attack*, Decay*
倍音コントロールの `Attack` と `Decay` にかけ合わせる値です。まとめて長さを変えたいときに使えます。

#### Declick
チェックを入れると、倍音コントロールの `Attack` あるいは `Decay` の値が 0 のときに生じるプチノイズを低減します。各オシレータのピッチに応じた値を足し合わせます。

#### Gain^
倍音コントロールの `Gain` を累乗する値です。 1 つのオシレータの最終的なゲインは次の式で計算されます。

```
Gain.Boost * Gain.Gain * pow(Overtone.Gain, Modifier.Gain^)
```

#### Sat. Mix
サチュレーションの Dry : Wet の信号比です。右いっぱいにすると Dry : Wet = 0 : 1 になります。

### Phaser
#### nStages
オールパスフィルタの数です。

#### Mix
フェイザの Dry : Wet の信号比です。右いっぱいにすると Dry : Wet = 0 : 1 になります。

#### Freq
LFO の周波数です。

#### Feedback
フィードバックの大きさです。 12 時にするとフィードバックなし、左に回すと負のフィードバック、右に回すと正のフィードバックとなります。

#### Range
LFO によって変更するオールパスフィルタの特性の幅です。

#### Min
LFO によって変調するオールパスフィルタの特性の最小値です。

#### Offset
左右の LFO の位相差です。

#### Phase
LFO の位相です。オートメーションで音を作りたいときに使えます。 `Freq` を左いっぱいに回すことで LFO の周波数を 0 にできます。

## チェンジログ
- 0.1.0
  - 初期リリース

### 旧バージョン
旧バージョンはありません。

## ライセンス
IterativeSinCluster のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
