# TrapezoidSynth
![](img/trapezoidsynth.png)

<ruby>TrapezoidSynth<rt>トラピゾイドシンセ</rt></ruby>は台形オシレータを使ったモノフォニックシンセサイザです。台形オシレータは PTR (Polynomial Transition Regions) という手法に基づいて作ったのですが、ピッチが高くなると逆にノイズが増える欠点があるので 8 倍のオーバーサンプリングをしています。コードを作るために 2 つの AM ピッチシフタを搭載しています。

- [TrapezoidSynth 0.1.1 をダウンロード - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/TrapezoidSynth0.1.1/TrapezoidSynth0.1.1.zip) <img
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
`TrapezoidSynth.vst3` を OS ごとに決められたディレクトリに配置してください。

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

REAPER 5.983 の Linux 版が TrapezoidSynth を認識しないときは `~/.config/REAPER/reaper-vstplugins64.ini` を削除して REAPER を再起動してみてください。

## 操作
つまみとスライダーでは次の操作ができます。

- Ctrl + 左クリック : 値のリセット。
- Shift + 左ドラッグ : 細かい値の変更。

操作できる箇所を右クリックすると DAW によって提供されているコンテキストメニューを開くことができます。

## 注意
`SlideType` を `Reset to 0` にすると極端に低い周波数が出るので、ハイパスフィルタをかけることを推奨します。

## パラメータ
### Oscillator 共通
#### Semi
ピッチを半音単位で変更します。

#### Cent
ピッチをセント単位で変更します。 100 セント = 1 半音です。

#### Slope
台形の両端の傾きです。

#### PW
台形の上辺の長さです。 Pulse Width の略です。

### Oscillator 1
#### Drift
ホワイトノイズでピッチを変調します。

#### Feedback
Oscillator 1 の位相をフィードバックで変調します。

```
osc1Phase += Feedback * (osc1 + OscMix * (osc2 - osc1))
```

### Oscillator 2
#### Overtone
周波数を `Overtone` 倍にします。

#### PM
Oscillator 1 の位相を Oscillator 2 の出力で変調します。

### Envelope 共通
#### Retrigger
チェックを入れると鍵盤が押されるたびにエンベロープをリセットします。チェックが外れているときは、同時に押している鍵盤の数が 0 から 1 以上になったときだけエンベロープをリセットします。

#### A
アタック時間の秒数です。アタックは音が最大音量に到達するまでのエンベロープの区間です。

#### D
ディケイ時間の秒数です。ディケイはアタックが終わった後にエンベロープが減衰する区間です。

#### S
サステインの大きさです。サステインはディケイが終わったのエンベロープの出力の大きさです。

#### R
リリース時間の秒数です。エンベロープは、鍵盤から指を離した時点（ノートオフ）からリリース状態に移行して、出力が 0 になるまで減衰します。

#### Curve
エンベロープの特性を変更します。

### Gain Envelope
#### Gain
出力音量です。

### Filter
#### Order
フィルタの次数です。 TrapezoidSynth では 1 次フィルタを 8 個直列につないでいます。 `Order` の値によってフィルタから信号を取り出す位置を変更しています。

#### Cut
カットオフ周波数です。

#### Res
レゾナンスの強さです。

#### Sat
フィルタのサチュレーションの強さです。

```
filterInput = tanh(Sat * (input + Res * filterOutput))
```

#### Env>Cut
Filter Envelope による `Cut` の変調量です。

#### Key>Cut
鍵盤の高さによる `Cut` の変調量です。

#### +OscMix
Oscillator 1 と Oscillator 2 をミックスした信号による `Cut` の変調量です。

### Filter Envelope
#### >Octave
Filter Envelope の出力を使ってピッチをオクターブ単位で変調する量です。

### Misc
#### OscMix
Osillator 1 と Oscillator 2 の比率です。左いっぱいに回すと Oscillator 1 の音だけになります。

#### Octave
ピッチをオクターブ単位で変更します。

#### Smooth
パラメータを変更したときに変更前の値から変更後の値に移行する秒数です。次のパラメータに影響します。

- `Drift`
- `Slope`
- `PW` （同名のパラメータすべて）
- `Feedback` （同名のパラメータすべて）
- `PM`
- `Gain` （同名のパラメータすべて）
- `Cut`
- `Res`
- `Sat`
- `Env>Cut`
- `Key>Cut`
- `+OscMix`
- `OscMix`
- `>PM`
- `>Feedback`
- `>LFO`
- `>Slope2`
- `>Shifter1`
- `Shifter1.Semi`
- `Shifter1.Cent`
- `Shifter2.Semi`
- `Shifter2.Cent`
- `Freq`
- `Shape`
- `>Pitch1`
- `>Slope1`
- `>PW1`
- `>Cut`

### Mod 共通
#### Retrigger
チェックを入れると鍵盤が押されるたびにエンベロープをリセットします。チェックが外れているときは、同時に押している鍵盤の数が 0 から 1 以上になったときだけエンベロープをリセットします。

#### Attack
モジュレーションエンベロープのアタック時間の秒数です。

#### Curve
モジュレーションエンベロープの特性です。

### Mod 1
#### >PM
`Oscillator2.PM` をモジュレーションエンベロープ 1 で変調します。

### Mod 2
#### >Feedback
`Oscillator1.Feedback` をモジュレーションエンベロープ 2 で変調します。

#### >LFO
`LFO.Freq` をモジュレーションエンベロープ 2 で変調します。

#### >Slope2
`Oscillator2.Slope` をモジュレーションエンベロープ 2 で変調します。

#### >Shifter1
Shifter 1 のピッチをモジュレーションエンベロープ 2 で変調します。

### Shifter 共通
#### Semi
半音単位でピッチシフトの量を変更します。

#### Cent
セント単位でピッチシフトの量を変更します。

#### Gain
ピッチシフトした信号を足し合わせる大きさです。

### LFO
#### LFOType
LFO の波形の種類を次の 4 つから選択できます。

- `Sin` : サイン波です。 出力は `Shape` 乗されます。
- `Saw` : 鋸歯波です。 `Shape` によって三角波にもできます。
- `Pulse` : 矩形波です。 `Shape` によってデューティ比を変更できます。
- `Noise` : ブラウンノイズです。 `Shape` によってステップあたりの移動量を変更できます。

#### Tempo
LFO の周波数をテンポシンクします。

#### Freq
LFO の周波数です。

#### Shape
LFO の波形を変更します。

#### >Pitch
LFO による Oscillator 1 のピッチの変調量です。

#### >Slope1
LFO による `Oscillator1.Slope` の変調量です。

#### >PW1
LFO による `Oscillator1.PW` の変調量です。

#### >PW1
LFO による `Filter.Cut` の変調量です。

### Slide
#### SlideType
ピッチスライドの種類を変更できます。

- `Always` : 常に最後に演奏されたノートのピッチからスライドします。
- `Sustain` : 2 つ以上の鍵盤が同時に押されたときだけスライドします。
- `Reset to 0` : 同時に押している鍵盤の数が 0 から 1 、あるいは 1 から 0 になるときは 0 Hz にスライドします。それ以外の場合は他の 2 つと同じようにスライドします。

#### Time
ピッチスライドにかかる時間です。

#### Offset
Oscillator 1 のスライド時間に対する Oscillator 2 のスライド時間の比率です。

## チェンジログ
- 0.1.1
  - オシレータの実装の誤りを修正。
- 0.1.0
  - 初期リリース。

## ライセンス
TrapezoidSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
