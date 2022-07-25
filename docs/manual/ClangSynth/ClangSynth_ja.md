---
lang: ja
...

# ClangSynth
![](img/clangsynth.png)

<ruby>ClangSynth<rt>クラング シンセ</rt></ruby> は金属的なパーカッションの音が得意なシンセサイザです。金属的な質感はフィードバック・ディレイ・ネットワーク (FDN) を用いたディレイ間のクロスフィードバックによるものです。 WaveCymbal や FDNCymbal よりはシンバルに似た音が出ます。

- [ClangSynth {{ latest_version["ClangSynth"] }} をダウンロード - VST® 3 (github.com)]({{ latest_download_url["ClangSynth"] }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
{%- if preset_download_url["ClangSynth"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["ClangSynth"] }})
{%- endif %}

パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit
- macOS universal binary

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## 注意
音量の振れ幅が大きいので ClangSynth の後に必ずリミッタを挿入することを推奨します。

FDN のパラメータが短時間に大きく変わると発散することがあります。 LFO やエンベロープを使うときは注意してください。発散はパラメータが時間とともに変わると線形時不変 (linear time invariant, LTI) なシステムではなくなるために起こります。以下は発散の原因となるパラメータの一覧です。

- LFO
  - `Amount > FDN Pitch`
- Envelope
  - `> LP Cut`
  - `> HP Cut`
  - `> FDN Pitch`
  - `> FDN OT +`

`Refresh Wavetable` ボタンを押すとウェーブテーブルが更新されます。更新中は CPU 負荷が上がり、ほぼ確実に音が止まるので注意してください。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/clangsynth.svg)

## パラメータ
角かっこ \[\] で囲まれているのは単位です。以下は単位の一覧です。

- \[dB\] : デシベル (decibel) 。
- \[s\] : 秒 (second) 。
- \[st.\] : 半音 (semitone) 。
- \[Hz\] : 周波数 (Hertz) 。
- \[rad/pi\] : 正規化した位相 (radian / π) 。

### Gain
Output \[dB\]

:   出力のゲインです。

Attack \[s\]

:   出力直前のゲインをスムーシングする秒数です。値が大きくなるとアタック時間とともに、リリース時間も長くなります。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

    `Attack` は、FDN セクションの `Reset to Note On` がチェックされていないときに、ノートオンでポップノイズが起こることを防ぐために導入されました。 FDN セクションのローパスフィルタのカットオフ周波数を下げているときは、値を小さくしすぎるとポップノイズが目立つことがあります。シンバルなどの高い周波数成分が多い音を作るときは 0 まで値を下げても、ほとんど問題になりません。

Release \[s\]

:   ノートオフの後に続くリリース時間です。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

### Tuning
Octave, Semi, Milli

:   全体の音の高さを変更します。

    - `Octave`: オクターブ
    - `Semi`: 半音
    - `Milli`: 半音の 1 / 1000 。 1 / 10 セント。

ET, A4 \[Hz\]

:   音律を変更します。

    `ET` は Equal Temperament (平均律) の略です。 `ET` が 12 のときは 12 平均律となります。 `ET` を 12 よりも小さくすると無音になる範囲が増えるので注意してください。

    `A4 [Hz]` は、音程 A4 の周波数です。

P.Bend Range

:   ピッチベンドの範囲です。単位は半音です。

### Unison/Chord
nUnison

:   ユニゾンにつかうボイスの数です。

Pan

:   パンを振る幅です。

    初回のノートオンでは、右から左へ向かって、ピッチの低い音から高い音へと順番にパンが振られます。以降はノートオンごとにパンの割り当てを左に回転させます。

Pitch *

:   ユニゾンで変更されるピッチに乗算される値です。

    `Pitch *` が 0 のとき、 `Interval` の値は無効になります。

ET

:   `Interval` のみで使われる平均律の値です。

Interval \[st.\], Cycle At

:   `Interval` はユニゾンで使われるボイスのピッチを、ノートオンで送られてきたピッチからどれだけずらすかを調整する値です。

    `Cycle At` は `Interval` の値を巻き戻すインデックスです。

    例えば `Interval` が以下の値に設定されているとします。

    ```
    1, 2, 3, 4
    ```

    各  `Interval` には `Pitch *` の値が乗算されます。仮に `Pitch *` の値が 0.1 とすると以下の値へと変換されます。

    ```
    0.1, 0.2, 0.3, 0.4
    ```

    ここで `Cycle At` が 1 に設定されているとすると、以下のようにインデックス 1 で巻き戻しながら繰り返します。

    ```
    0.1, 0.2, 0.1, 0.2, ...
    ```


    さらにノートオンで MIDI ノート番号 60 のピッチが入力されると、以下のようにピッチがボイスに割り当てられます。

    ```
    60,                         // 60
    60 + 0.1,                   // 60.1
    60 + 0.1 + 0.2,             // 60.3
    60 + 0.1 + 0.2 + 0.1,       // 60.4
    60 + 0.1 + 0.2 + 0.1 + 0.2, // 60.6
    ...
    ```

    以下は計算の手続きです。

    ```
    modulo = cycleAt + 1;
    accumulator = 0;
    for index in [0, nUnison) {
      voice[index].pitch = notePitch + pitchMultiplier * accumulator;
      accumulator += interval[index % modulo];
    }
    ```

### Misc.
nVoice

:   最大同時発音数です。

    CPU 負荷を下げるときは `nVoice` の値を下げると効果的です。

Smoothing \[s\]

:   パラメータのスムーシング時間です。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

### Oscillator
Impulse \[dB\]

:   ノートオンと同時に発音されるインパルスの振幅です。

    インパルスはシンバルの音を作るときの味付けに使えます。またインパルスはベロシティに影響されないので、ばちとシンバルの衝突の強弱を演奏時に調整するときにも便利です。

    TODO プリセットの紹介

Gain \[dB\]

:   オシレータ出力のゲインです。

Attack \[s\], Decay \[s\]

:   オシレータの AD エンベロープのアタック時間とディケイ時間です。

    フィルタのカットオフ周波数へと変換されるので、あくまでも目安となる値であって、正確な値ではありません。

    ディケイ時間が短いときは `Attack` を長くしても音はほとんど変わりません。

Octave, Semitone

:   オクターブあるいは半音単位のオシレータのチューニングです。

Denom. Slope, Rot. Slope, Rot. Offset, Interval

:   オシレータのウェーブテーブルの基となるソース・スペクトラムを合成するパラメータです。

    - `Denom. Slope` : 振幅の分子の係数。
    - `Rot. Slope` : 位相の傾き。
    - `Rot. Offset` : 位相の切片。
    - `Interval` : 書き込む間隔。

    `Denom. Slope` の値が 1 より大きいときはローパス、 1 より小さいときはハイパスのように機能します。値が 1 のときは、のこぎり波と同じ振幅特性になります。

    `Interval` を 1 より大きくするとソース・スペクトラムが 0 となる箇所を作り出します。例えば `Interval` の値が 3 なら、インデックス 1, 4, 7, 10, 13, ... の値だけが書き込まれて、それ以外のインデックスでの振幅は 0 となります。 `Denom. Slope` の値が 1 かつ `Interval` の値が 2 なら、矩形波が生成されます。

    以下は `Denom. Slope` 、 `Rot. Slope` 、 `Rot. Offset` 、 `Interval` を使ったソース・スペクトラムの計算方法です。

    ```
    for index in [1, numberOfHarmonics] {
        if (index % interval != 0) continue;
        sourceSpectrum[index].amp = 1 / (denominatorSlope * index);
        sourceSpectrum[index].phase = rotationOffset + rotationSlope * index;
    }
    ```

Harmonic HP

:   ソース・スペクトラムの振幅に適用されるハイパスフィルタです。

    `Harmonic HP` で指定された値よりインデックスが小さい低周波成分の振幅を下げます。値が 0 のときはハイパスフィルタはかかりません。

    以下は `Harmonic HP` の計算方法です。

    ```
    for index in [0, harmonicHighpass) {
      sourceSpectrum[index].amp *= index / harmonicHighpass;
    }
    ```

Blur

:   ソース・スペクトラムの振幅を通過させるローパスフィルタの係数です。

    `Blur` の値を下げると周波数領域で振幅の値が周囲のインデックスへと拡散するので、位相の回転を無視すると、時間領域に戻したときにはインパルスに近い音になります。

    `Blur` が 1 のときはローパスフィルタはかかりません。

    TODO 図

OT Amp., Rot. \[rad/pi\]

:   ソース・スペクトラムを、さらに倍音間隔で足し合わせるときの振幅と位相です。

    - `OT Amp.` : 加算する倍音の振幅。
    - `Rot. \[rad/pi\]` : 加算する倍音の位相。

    以下は `OT Amp.` と `Rot.` による最終的なスペクトラムの計算方法です。

    ```
    targetSpectrum.fill(0);

    for i in [0, nOvertone) {
      for k in [0, nFrequency) {
        auto index = (i + 1) * k;
        if (index >= nFrequency) break;
        targetSpectrum[index] += sourceSpectrum[k] * complexFromPolar(otAmp[i], otRot[i]);
      }
    }
    ```

Refresh Wavetable

:   ボタンを押すとウェーブテーブルを更新します。

    更新中は CPU 負荷が上がり、ほぼ確実に音が止まるので注意してください。

    Oscillator セクションの以下のパラメータは `Refresh Wavetable` ボタンを押すまで更新されません。

    - `Denom. Slope`
    - `Rot. Slope`
    - `Rot. Offset`
    - `Interval`
    - `Harmonic HP`
    - `Blur`
    - `OT Amp.`
    - `Rot. [rad/pi]`

### FDN
FDN

:   点灯しているときはオシレータ出力が FDN を通過します。

    オシレータの出力を確認するときには無効にすると便利です。

Identity

:   FDN 内のクロス・フィードバックの量を変更します。

    `Identity` の値が 0 から離れるほど、フィードバック行列が単位行列から離れてクロス・フィードバックが増えます。クロス・フィードバックが増えると非整数次倍音が増えて金属的な質感が出ます。

Feedback

:   FDN のフィードバック量です。

    ディレイ内部の線形補間の影響で、 `Feedback` の値が 1 のときでも少しづつ減衰が起こります。

Interp. Rate

:   ディレイ時間が変更されたときのレート制限の量です。

Interp. LP \[s\]

:   ディレイ時間が変更されたときの補間に使われるローパスフィルタのカットオフ周波数の逆数です。

    ディレイ時間の補間は、ローパスフィルタ、レート制限の順で行われます。

Seed

:   フィードバック行列のランダマイズに使われるシード値です。

Randomize

:   ノートオンごとにフィードバック行列をランダマイズする割合です。

    内部的には行列をランダマイズする基となる値の組を 2 つ持っています。 1 つの組は再生開始の時点で固定されます。もう 1 つの組はノートオンごとに生成されます。この 2 つの組を混ぜる割合が `Randomize` です。

    `Randomize` が 0 のときは再生開始の時点で固定される組だけを使うので、ノートオンによって倍音が変わりません。 `Randomize` が 1 のときはノートオンのたびにフィードバック行列が変わるので、倍音も変わります。

OT +, OT *, OT Offset, OT Modulo, OT Random

:   FDN のディレイ時間を、ノートオンで提供されるピッチの倍音として設定するパラメータです。

    - `OT +` : 倍音のインデックスが進むたびに加算される値。
    - `OT *` : 倍音のインデックスが進むたびに 1 つ前の値に乗算される値。
    - `OT Offset` : 倍音を高いほうに向かってシフトする量。
    - `OT Modulo` : `OT +` と `OT *` から計算される値を、巻き戻す点。
    - `OT Random` : ノートオンごとに倍音をランダマイズする量。

    `OT +` と `OT Offset` を整数でない値に設定することで打楽器のような音に近づきます。

    `OT *` と `OT Modulo` はやや不自然な倍音になります。

    以下は倍音の計算方法です。

    ```
    // At note on.
    for idnex in [0, fdnMatrixSize) {
        overtoneRandomness[idx] = randomUniform(-1, 1) * otRandom;
    }

    // For each sample.
    overtone = 1;
    for idnex in [0, fdnMatrixSize) {
      ot = otOffset + (1 + overtoneRandomness[index]) * overtone;
      fdn.delayTime[index] = sampleRate / (ot * noteFrequency);
      overtone = overtone * otMul + otAdd;
      overtone = fmod(overtone, 1 + otModulo);
    }
    ```

Reset at Note ON

:   チェックを入れるとノートオンのたびに FDN の状態をリセットします。

    リセットによる CPU 負荷のスパイクに注意してください。音が止まるなどの問題が出るときは `nVoice` の値を小さくすることで CPU 負荷を軽減できます。

Cutoff \[st.\]

:   ディレイの出力が通るローパスフィルタとハイパスフィルタのカットオフ周波数です。

Q

:   ディレイの出力が通るローパスフィルタとハイパスフィルタの Q です。

Key Follow

:   チェックを入れるとフィルタのカットオフ周波数をノートのピッチに応じて変更します。

### LFO
LFO Wave

:   LFO のウェーブテーブルの波形です。

    ウェーブテーブルの波形はすべてのボイスで共有されます。

Retrigger

:   チェックを入れるとノートオンされたボイスの LFO の位相をリセットします。

    チェックを外すとすべてのボイスが同じ位相を使います。

    1 つのボイスが 1 つの物体を表すのであれば、チェックを入れたほうがそれらしく聞こえます。ノートオンによって同じ物体を叩く、例えば 8 人が両手にばちをもって同じ中華鍋を叩いているとするなら、チェックを外してください。

Wave Interp.

:   LFO の波形の補間方法です。

    - `Step`: ホールド。
    - `Linear`: 線形補間。
    - `PCHIP`: 単調な 3 次補間。

    `Step` を選ぶと LFO をシーケンサのように使えます。 `Linear` は `PCHIP` と似たような音になりますが、計算がすこし速いです。デフォルトの `PCHIP` はサンプル間をだいたい滑らかに補間します。

Sync.

:   チェックを入れるとテンポ同期を有効にします。また同期間隔が変わったときに再生開始時点から導かれる位相へと同期します。

    チェックが外れているときは 120 BPM に同期した状態と同じになります。ただし、同期間隔が変わったときに位相を調整しなくなります。

Tempo Upper

:   テンポ同期が有効な時の同期間隔を表す分数の分子です。

    `1/1` のときに 1 小節、 4/4拍子であれば `1/4` のときに 1 拍で LFO が 1 周します。 `Rate` が乗算されて周期が変わる点に注意してください。

    以下は同期間隔の計算方法です。

    ```
    syncInterval = (Rate) * (Tempo Upper) / (Tempo Lower);
    ```

Tempo Lower

:   テンポ同期が有効な時の同期間隔を表す分数の分母です。

    `Rate` が乗算されて周期が変わる点に注意してください。

Rate

:   同期間隔に乗算される係数です。

    `Tempo Upper` と `Tempo Lower` を変えずに LFO の同期間隔を変えたいときに使えます。

Pitch Amount

:   LFO によるピッチの変調量です。単位は半音です。

    `> Osc. Pitch` の行のパラメータはオシレータ、 `> FDN Pitch` の行のパラメータは FDN への変調です。

Pitch Alignment

:   LFO のよるピッチの変調量は `Alignment` で指定された値の倍数だけを使います。例えば現在の変調量が 12.345 で `Alignment` が 6 とすると、実際の変調量は 12 になります。

    `Alignment` が 0 のときは `Amount` の値を直接使います。

    以下は変調量の計算方法です。

    ```
    if (alignment == 0)
      modulation = amount * lfo;
    else
      modulation = alignment * floor(lfo * amount / alignment + 0.5);
    ```

### Envelope
Envelope Wave

:   エンベロープのウェーブテーブルの波形です。

    ウェーブテーブルの波形はすべてのボイスで共有されます。

Time \[s\]

:   ノートオンからエンベロープの終了までにかかる時間です。

    エンベロープが終了すると変調量はすべて 0 になります。

Wave Interp.

:   エンベロープのウェーブテーブルの補間方法です。

    LFO と同じく `Step` 、 `Linear` 、 `PCHIP` の 3 つの補間方法から選択できます。補間方法の詳細は LFO セクションの `Wave Interp.` を参照してください。

> LP Cut

:   FDN のローパスフィルタのカットオフ周波数への変調量です。単位は半音です。

> HP Cut

:   FDN のハイパスフィルタのカットオフ周波数への変調量です。単位は半音です。

> Osc. Pitch

:   オシレータのピッチの変調量です。単位は半音です。

> FDN Pitch

:   FDN のピッチの変調量です。単位は半音です。

> FDN OT +

:   FDN セクションの `OT +` への変調量です。

    `> FDN Pitch` とは異なり、倍音に基づいたピッチの変調を行います。

## チェンジログ
{%- for version, logs in changelog["ClangSynth"].items() %}
- {{version}}
  {%- for log in logs["ja"] %}
  - {{ log }}
  {%- endfor %}
{%- endfor %}

## 旧バージョン
### ClangSynth
{%- if old_download_link["ClangSynth"]|length == 0 %}
旧バージョンはありません。
{%- else %}
  {%- for x in old_download_link["ClangSynth"] %}
- [ClangSynth {{ x["version"] }} - VST 3 (github.com)]({{ x["url"] }})
  {%- endfor %}
{%- endif %}

## ライセンス
ClangSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
