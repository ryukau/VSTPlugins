# FDNCymbal
![](img/fdncymbal.png)

<ruby>FDNCymbal<rt>エフディーエヌシンバル</rt></ruby> はシンバルのような音を合成するシンセサイザです。エフェクトとして使うこともできます。名前とは裏腹に金属的な質感は FDN (feedback delay network) ではなく Schroeder allpass section によって得られています。 `FDN.Time` の値を小さくすることで、わりとナイスなばちの衝突音が合成できます。シンバルの揺れをシミュレートするためにトレモロもついています。

- [FDNCymbal 0.1.1 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/MidiNoteTriggerFix/FDNCymbal0.1.1.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit
- macOS 64bit

Mac を持っていないので、 macOS ビルドはテストできていません。もしバグを見つけたときは [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を作るか、 `ryukau@gmail.com` までメールを送っていただければ対応します。

Linux ビルドは Ubuntu 18.0.4 でビルドしています。また Bitwig 3.0.3 と REAPER 5.983 で動作確認を行っています。どちらも GUI の表示に問題があったので、今のところ Linux ビルドでは GUI を無効にしています。

## インストール
`FDNCymbal.vst3` を OS ごとに決められたディレクトリに配置してください。

- Windows では `/Program Files/Common Files/VST3/` に配置します。
- Linux では `$HOME/.vst3/` に配置します。
- macOS では `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/` に配置します。

DAW によっては上記とは別に VST3 をインストールできるディレクトリを提供していることがあります。詳しくは利用している DAW のマニュアルを参照してください。

### Linux
Ubuntu 18.0.4 では次のパッケージのインストールが必要です。

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

もし DAW がプラグインを認識しないときは、下のリンクの `Package Requirements` を参考にして VST3 に必要なパッケージがすべてインストールされているか確認してみてください。

- [VST 3 Interfaces: Setup Linux for building VST 3 Plug-ins](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/linuxSetup.html)

REAPER 5.983 の Linux 版が FDNCymbal を認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

## 操作
つまみとスライダーでは次の操作ができます。

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

## 注意
`FDN.Feedback` の値が 0 でないときは信号が発散することがあります。発散したときは `FDN.Feedback` を一番左まで回してください。

`HP Cutoff` の値を短時間に大きく変更すると直流が乗ることがあります。直流が乗ったときは、いったん `HP Cutoff` を上げてから <kbd>Shift</kbd> + <kbd>左ドラッグ</kbd> でゆっくりと下げなおしてください。

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

#### Decay
ばちオシレータの減衰時間です。

#### ToneMix
ばちオシレータが出力するトーンの音量です。

### Random
#### Seed
乱数のシード値です。

#### Retrigger.Time
チェックを外すと、次の 3 つのパラメータでコントロールされる内部的なディレイ時間がノートオンのたびに変わります。

- `FDN.Time`
- `Allpass.Stage1.Time`
- `Allpass.Stage2.Time`

#### Retrigger.Stick
チェックを外すと、ノートオンのたびにばちオシレータの音が変わります。

#### Retrigger.Tremolo
チェックを外すと、トレモロのパラメータがノートオンのたびに変わります。

### FDN
オンのときはFDN (feedback delay network) セクションを信号が通過します。

#### Time
FDN のディレイ時間を調整します。内部的なディレイ時間はノブで設定した値をもとにランダマイズされます。

#### Feedback
FDN セクションのフィードバックです。この値が 0 でないときは信号が発散することがあるので注意してください。

#### CascadeMix
直列につないだ複数の FDN の出力のミックスを調整します。

### Allpass
#### Mix
Schroeder allpass section のミックスを調整します。

### Stage 1 と Stage 2
2 つの Schroeder allpass section が直列につながっています。 Stage 2 では 4 つの Schroeder allpass section が並列接続されています。

#### Time
Schroeder allpass section のディレイ時間の最大値です。ディレイ時間は内部的にランダマイズされます。

#### Feedback
Schroeder allpass section のフィードバックです。

#### HP Cutoff
Schroeder allpass section の出力にかかるハイパスフィルタのカットオフ周波数です。

#### Tanh
チェックを入れると Stage 1 のフィードバックにサチュレーションがかかります。

### Tremolo
#### Mix
トレモロのミックスを調整します。

#### Depth
`Depth` が大きいほどトレモロによる音量の変化が大きくなります。

#### Frequency
トレモロに使われる LFO の周波数です。

#### DelayTime
LFO によって変化するディレイ時間の最大値です。シンバルの揺れによって生じるドップラー効果を表現しています。

### Random (Tremolo)
#### Depth
ノートオンのたびに `Tremolo.Depth` をランダマイズする度合いです。

#### Freq
ノートオンのたびに `Tremolo.Frequency` をランダマイズする度合いです。

#### Time
ノートオンのたびに `Tremolo.DelayTime` をランダマイズする度合いです。

## チェンジログ
- 0.1.1
  - ノートオン・オフが正確なタイミングでトリガされるように修正。
  - `Allpass.Stage1.Feedback` が無効になっていたバグを修正。
- 0.1.0
  - 初期リリース

## ライセンス
FDNCymbal のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
