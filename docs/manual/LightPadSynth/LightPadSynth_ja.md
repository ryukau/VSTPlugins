---
lang: ja
...

# LightPadSynth
![](img/LightPadSynth.png)

<ruby>LightPadSynth<rt>ライト パッド シンセ</rt></ruby> はピッチ変調を無くして補間を線形補完に変えた CubicPadSynth の軽量版です。ピッチ変調ができなくなった代わりに、各ボイスにディレイを追加しています。

{% for target, download_url in latest_download_url["LightPadSynth"].items() %}
- [LightPadSynth {{ latest_version["LightPadSynth"] }} `{{ target }}` - VST 3 をダウンロード (github.com)]({{ download_url }}) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="30px"
  style="display: inline-block; vertical-align: middle;">
{%- endfor %}
{%- if preset_download_url["LightPadSynth"]|length != 0%}
- [プリセットをダウンロード (github.com)]({{ preset_download_url["LightPadSynth"] }})
{%- endif %}

{{ section["package"] }}

{{ section["contact_installation_guiconfig"] }}

## 操作
{{ section["gui_common"] }}

{{ section["gui_knob"] }}

{{ section["gui_barbox"] }}

## 注意
`Refresh LFO` あるいは `Refresh Table` ボタンを押すと音が止まります。発音中のノートも全て停止します。

## ウェーブテーブルの仕様
各 MIDI ノートに対応する 128 の帯域制限されたウェーブテーブルが生成されます。 128 という数字は MIDI ノート番号の値の範囲から来ています。

チューニングがずれているときは、セント値を切り捨てたインデックスのウェーブテーブルが使われます。例えば MIDI ノート番号が 60 、チューニングが -20 セントのときは `floor(60 - 0.20)` よりインデックス 59 のウェーブテーブルが使われます。

言い換えれば、チューニングがずれているときはナイキスト周波数より少し低い周波数で帯域制限されることがあります。

## ブロック線図
図が小さいときはブラウザのショートカット <kbd>Ctrl</kbd> + <kbd>マウスホイール</kbd> や、右クリックから「画像だけを表示」などで拡大できます。

図で示されているのは大まかな信号の流れです。実装と厳密に対応しているわけではないので注意してください。

![](img/LightPadSynth.svg)

## パラメータ
### Main タブ
![](img/LightPadSynth.png)

#### Gain
A, D, S, R

:   音量エンベロープのパラメータです。

    - `A` (Attack) : 鍵盤を押した瞬間から最大音量になるまでのアタック時間。
    - `D` (Decay) : 最大音量からサステイン音量になるまでのディケイ時間。
    - `S` (Sustain) : アタックとディケイが終わった後に鍵盤を押し続けているときのサステイン音量。
    - `R` (Release) : 鍵盤から指を離したあとに音量が 0 になるまでのリリース時間。

Gain

:   音量を変更します。

#### Lowpass
素朴な 3-pole ローパスフィルタです。

Cutoff

:   フィルタのカットオフ周波数です。

Resonance

:   フィルタのレゾナンスです。最大にすると音量が大きくなるので注意してください。

KeyFollow

:   右いっぱいに回すと、カットオフ周波数がノートの音程と同じになります。左いっぱいに回すと音程に関わらず `Cutoff` で設定されたカットオフ周波数を使います。

A, D, S, R, Amount

:   フィルタのカットオフ周波数を変調するエンベロープのパラメータです。 `Amount` は `Cutoff` への変調量です。

#### Tuning
Octave, Semi, Milli

:   全体の音の高さを変更します。

    - `Octave`: オクターブ
    - `Semi`: 半音
    - `Milli`: 半音の 1 / 1000 。 1 / 10 セント。

ET, A4 [Hz]

:   音律を変更します。

    `ET` は Equal Temperament (平均律) の略です。 `ET` が 12 のときは 12 平均律となります。 `ET` を 12 よりも小さくすると無音になる範囲が増えるので注意してください。

    `A4 [Hz]` は、音程 A4 の周波数です。

#### Unison
nUnison

:   ユニゾンに使うボイスの数です。

    `Misc.` の `nVoice` を増やすことで `nUnison` が大きいときに起こるリリースの途切れを減らすことができますが、引き換えに動作が重くなります。

Detune, Random Detune

:   `Detune` はユニゾンに使われているボイスのピッチをずらす量です。

    `Random Detune` のチェックを入れると、ピッチがずれる量がノートオンごとにランダムに変わるようになります。

    ```
    random = RandomDetune ? rand() : 1
    detune = pitch * (1 + random * unisonIndex * Detune)
    ```

GainRnd

:   ユニゾンに使われているボイスの音量をランダムに変更する量です。

Phase

:   ユニゾンに使われているボイスの位相をランダムに変更する量です。

    Phase セクションの `Reset` にチェックが入っていないとき、この値は無視されます。

Spread, Spread Type

:   `Spread` はユニゾンの左右の広がりの量です。

    `Spread Type` でボイスのピッチに応じたパンの割り当て方を選択できます。

    - `Alternate L-R`: `Ascend L -> R` と `Ascend R -> L` を交互に変更。
    - `Alternate M-S`: `HighOnMid` と `HighOnSide` を交互に変更。
    - `Ascend L -> R`: 左から右に向かってピッチが上昇。
    - `Ascend R -> L`: 右から左に向かってピッチが上昇。
    - `HighOnMid`: 端から中央に向かってピッチが上昇。
    - `HighOnSide`: 中央から端に向かってピッチが上昇。
    - `Random`: ランダムに生成したパンを割り当て。偏ることがあります。
    - `RotateL`: ノートオンごとに割り当てを左に回転。
    - `RotateR`: ノートオンごとに割り当てを右に回転。
    - `Shuffle`: 均一に並べたパンをランダムに割り当て。

#### Phase
Phase

:   オシレータの初期位相です。

Reset

:   チェックを入れると、ノートオンのたびにオシレータの位相を `Phase` で指定した値にリセットします。

Random

:   チェックを入れると、ノートオンごとに位相をランダマイズします。 `Phase` で指定した値がランダマイズの範囲になります。

#### Misc.
Smooth

:   特定の値を変更したときに、変更前の値から変更後の値に移行する時間（秒）です。

    `Smooth` と関連するパラメータのリストです。 `*` はワイルドカードです。

    - Tuning セクションの全て
    - Gain
      - `Gain`
      - `S`
    - Lowpass
      - `Cutoff`
      - `S`
      - `Amount`
      - `KeyFollow`
    - Delay
      - `Mix`
      - `Feedback`
      - `Attack`
      - `Semi`
      - `Milli`
    - Delay LFO
      - `Tempo`
      - `Multiply`
      - `Amount`
      - `Lowpass`
    - Phase
      - `Phase`

Poly

:   最大同時発音数です。値を小さくすると CPU 消費が減ります。

Seed

:   乱数のシード値です。この値を変えると生成される乱数が変わります。

    `Main` タブと `Wavetable` タブでは異なる乱数列が使われています。

#### Delay
Mix

:   フィルタ出力とディレイ出力を混ぜる比率です。

Feedback

:   ディレイのフィードバックです。右に回すと正のフィードバック、左に回すと負のフィードバックがかかります。

Attack

:   フィルタからの出力をディレイに入力するときに掛け合わされるゲートのアタック時間です。

Semi, Milli

:   ノートの周波数から相対的に決められるディレイ時間です。次の式で計算されます。

    ```
    delayTime = 1 / (noteFreq * pow(2, (semi + 0.001 * milli) / 12))
    ```

#### LFO
Tempo, Multiply

:   LFO の周波数をテンポに応じた形で指定します。 `Tempo` の下の数字が音符の長さ、上の数字が音符の数です。例えば上が 6 、下が 8 なら 8 分音符の 6 個分の長さが LFO の 1 周期になります (6 / 8 拍子)。

    `Multiply` は `Tempo` から計算された周波数に掛け合わされる値です。

    ```
    // (60 秒) * (4 拍子) = 240 。
    lfoFrequency = Multiply * (BPM / 240) / (TempoUpperNumeral / TempoLowerNumeral)
    ```

Amount

:   LFO でピッチを変調する量です。

Lowpass

:   LFO にかけるローパスフィルタのカットオフ周波数を変更します。

Interpolation

:   LFO のウェーブテーブルの補間の種類です。

    ![](img/interpolation_type.png)

Refresh LFO

:   現在の `LFO Wave` のパラメータに基づいて LFO のウェーブテーブルを更新します。

    ウェーブテーブルの更新中は音が止まるので注意してください。発音中のノートも全て停止します。

LFO Wave

:   LFO の波形です。

### Wavetable タブ
![](img/LightPadSynth_wavetable_tab.png)

#### 倍音コントロール
Gain

:   プロファイルの高さです。

Width

:   プロファイルの幅です。

Pitch

:   プロファイルの周波数にかけ合わせる値です。

Phase

:   プロファイルの位相をランダマイズする量です。

#### Pitch
Base Freq.

:   ウェーブテーブルのレンダリング時の基本周波数です。この値が小さいとマスターピッチのチューニングが狂うことがあるので注意してください。

Multiply, Modulo

:   プロファイルの周波数を変更します。

    ```
    profileFrequency = mod(
      BaseFreq * profileIndex * overtonePitch * Multiply,
      440 * pow(2, (Modulo - 69) / 12)
    )
    ```

#### Spectrum
Expand

:   スペクトラムを周波数軸に沿って拡大・縮小するスケーリング係数です。

    ![](img/expand.svg)

Shift

:   スペクトラムを左右にシフトする量です。

    ![](img/shift.svg)

Comb

:   `Comb` の値が 1 以上のとき、プロファイルをくし状に変形します。値はくしのすき間の間隔です。

    ![](img/comb.png)

Shape

:   `Shape` の値を指数として、プロファイルの形をべき乗によって変更します。

    ```
    shapedProfile = powf(profile, shape);
    ```

#### Phase
UniformPhase

:   チェックを入れると、プロファイル内での位相を均一にします。

#### Random
Seed

:   乱数のシード値です。この値を変えると生成される乱数が変わります。

    `Main` タブと `Wavetable` タブでは異なる乱数列が使われています。

#### BufferSize
1 つのウェーブテーブルの大きさです。 `2^10` から `2^21` の範囲の 2 のべき乗の値を選択できます。

`()` で表示された値は全てのウェーブテーブルを合わせたバイト数です。バイト数は次の式で計算できます。

```
bytes = 4 * 128 * BufferSize
```

#### Modifier
Gain^

:   倍音コントロールの `Gain` をべき乗する値です。

    ```
    profileGain = pow(Gain, Gain^)
    ```

Width\*

:   倍音コントロールの `Width` にかけ合わせる値です。

    ```
    profileWidth = Width * (Width*)
    ```

#### Refresh Table
現在の Wavetable タブのパラメータに基づいてオシレータのウェーブテーブルを更新します。

ウェーブテーブルの更新中は音が止まるので注意してください。発音中のノートも全て停止します。

## チェンジログ
- 0.1.14
  - x86_64 固有の SIMD 命令への依存を除去。
- 0.1.13
  - Linux でのクラッシュを修正。
  - BarBox が <kbd>Shift</kbd> キーを受け取るように修正。
- 0.1.12
  - プラグインが許可するチャネル数の条件を修正。この修正は REAPER のサイドチェインで意図しないミュートがかかることを防ぐために行った。
- 0.1.11
  - `style.json` でカスタムフォントを設定するオプションを追加。
- 0.1.10
  - VSTGUI を 4.10 から 4.11 にアップデート。
- 0.1.9
  - フォントを Tinos に変更。
- 0.1.8
  - リセットが正しく行われるように修正。
  - `bypass` パラメータの挙動を修正。この修正によって、ホストがミュート中のプラグインにノートを送り続けても、ミュート解除とともにそれまでに送られたノートがすべて再生されなくなった。このバグは VST 3 の `bypass` パラメータを正しく実装しているホストでのみ発生していた。
- 0.1.7
  - Process context requirements を実装。
  - BarBox の機能が LV2 版と同等になるように更新。
    - 1 つのバーを編集を追加。
    - ロックを追加。
    - 内部的なマウスホイールの感度を追加。
    - スナップを追加 (未使用) 。
    - 直線の描画での開始点の値をアンカーポイントに固定するように変更。
- 0.1.6
  - DSP が初期化されているかどうかのチェックを追加。
- 0.1.5
  - いくつかの BarBox にスクロールバーを追加。
  - BarBox に アンドゥ・リドゥの機能を追加。
- 0.1.4
  - カラーコンフィグを追加。
- 0.1.3
  - パラメータの補間を可変サイズのオーディオバッファでも機能する以前の手法に巻き戻した。
- 0.1.2
  - 起動時にパラメータがロードされる前にウェーブテーブルが更新されるバグを修正。
  - LFO ウェーブテーブルの補間が Cubic のときにインデックスが 1 つずれていたバグを修正。
- 0.1.1
  - Refresh LFO ボタンと Refresh Table ボタンをメッセージからパラメータに修正。
- 0.1.0
  - 初期リリース。

## 旧バージョン
- [LightPadSynth 0.1.13 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins0.33.0/LightPadSynth_0.1.13.zip)
- [LightPadSynth 0.1.12 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/UhhyouPlugins_0_31_0/LightPadSynth_0.1.12.zip)
- [LightPadSynth 0.1.11 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CustomFontOptions/LightPadSynth_0.1.11.zip)
- [LightPadSynth 0.1.9 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/LightPadSynth_0.1.9.zip)
- [LightPadSynth 0.1.8 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ResetAndMuteFix/LightPadSynth_0.1.8.zip)
- [LightPadSynth 0.1.7 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/CollidingCombSynth0.1.0/LightPadSynth0.1.7.zip)
- [LightPadSynth 0.1.6 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L3Reverb0.1.0/LightPadSynth0.1.6.zip)
- [LightPadSynth 0.1.5 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/L4Reverb0.1.0/LightPadSynth0.1.5.zip)
- [LightPadSynth 0.1.4 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/ColorConfig/LightPadSynth0.1.4.zip)
- [LightPadSynth 0.1.3 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LatticeReverb0.1.0/LightPadSynth0.1.3.zip)
- [LightPadSynth 0.1.2 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/WavetableInitializationBugFix/LightPadSynth0.1.2.zip)
- [LightPadSynth 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/LightPadSynth0.1.0/LightPadSynth0.1.0.zip)

## ライセンス
LightPadSynth のライセンスは GPLv3 です。 GPLv3 の詳細と、利用したライブラリのライセンスは次のリンクにまとめています。リンクが切れているときは `ryukau@gmail.com` にメールを送ってください。

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

### VST® について
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
