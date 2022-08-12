---
lang: ja
...

# Uhhyou Plugins
[VST®](#vst_logo) 3 プラグインを公開しています。

何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を投げるか `ryukau@gmail.com` にメールを送信してもらえれば対応します。お気軽にお問い合わせください。

[paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau) から開発資金を投げ銭することもできます。

This page is publishing [VST®](#vst_logo) 3 plugins.

If you found some part of manual is not clear and/or found typo, please file issue at [GitHub repository](https://github.com/ryukau/VSTPlugins) or send email to `ryukau@gmail.com`. Note that I'm not native in English and it's most likely that the manual contains some weird sentences. I'd like to fix it, so feel free to report those issues.

You can fund the development via [paypal.me/ryukau](https://www.paypal.com/paypalme/ryukau).

# プラグイン一覧 | Plugin List

<style>
div.pluginlist {
  margin: auto 0.25em;
  padding: 0.25em;
  display: inline-flex;
}
</style>

以下のリリースページからプラグインをまとめてダウンロードできます。

- [Releases · ryukau/VSTPlugins · GitHub](https://github.com/ryukau/VSTPlugins/releases)

All the plugin download link can be found at release page on following link.

- [Releases · ryukau/VSTPlugins · GitHub](https://github.com/ryukau/VSTPlugins/releases)

#### Synthesizers
<div class="pluginlist"><a href="#clangsynth">
<ruby>ClangSynthh<rt>クラング シンセ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#collidingcombsynth">
<ruby>CollidingCombSynth<rt>コライディング コム シンセ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#cubicpadsynth">
<ruby>CubicPadSynth<rt>キュービック パッドシンセ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#envelopedsine">
<ruby>EnvelopedSine<rt>エンベロープド サイン</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#fdncymbal">
<ruby>FDNCymbal<rt>エフディーエヌ シンバル</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#iterativesincluster">
<ruby>IterativeSinCluster<rt>イテレイティブ サイン クラスタ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#lightpadsynth">
<ruby>LightPadSynth<rt>ライト パッド シンセ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#syncsawsynth">
<ruby>SyncSawSynth<rt>シンク ソウ シンセ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#trapezoidsynth">
<ruby>TrapezoidSynth<rt>トラピゾイド シンセ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#wavecymbal">
<ruby>WaveCymbal<rt>ウェイブ シンバル</rt></ruby>
</a></div>

#### Effects
<div class="pluginlist"><a href="#basiclimiter">
<ruby>BasicLimiter<rt>ベーシック リミッタ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#esphaser">
<ruby>EsPhaser<rt>エス フェイザ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#fdn64reverb">
<ruby>FDN64Reverb<rt>エフディーエヌ 64 リバーブ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#l4reverb">
<ruby>L4Reverb<rt>エル フォー リバーブ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#latticereverb">
<ruby>LatticeReverb<rt>ラティス リバーブ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#minicliffeq">
<ruby>MiniCliffEQ<rt>ミニ クリフ イーキュー</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#parallelcomb">
<ruby>ParallelComb<rt>パラレル コム</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#pitchshiftdelay">
<ruby>PitchShiftDelay<rt>ピッチ シフト ディレイ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#sevendelay">
<ruby>SevenDelay<rt>セブン ディレイ</rt></ruby>
</a></div>
<div class="pluginlist"><a href="#ultrasonicringmod">
<ruby>UltrasonicRingMod<rt>ウルトラソニック リング モッド</rt></ruby>
</a></div>

## UltrasonicRingMod
<figure>
<img src="manual/UltrasonicRingMod/img/UltrasonicRingMod.png" alt="Image of UltrasonicRingMod graphical user interface." style="padding-bottom: 12px;"/>
</figure>

UltrasonicRingMod は 64 倍のオーバーサンプリングを行うリングモジュレータで、最高 1 MHz のサイン波による変調をかけることができます。とは言っても単に変調すると可聴域外なので何も聞こえなくなってしまいますが、フィードバックによる位相変調やハードクリッピングができるので、いくらかノイズが出せます。

- [UltrasonicRingMod のマニュアル (日本語)](manual/UltrasonicRingMod/UltrasonicRingMod_ja.html)

UltrasonicRingMod is a ring modulator with 64-fold oversampling. Maximum modulator frequency is 1 MHz. That said, just modulating with ultrasonic frequency doesn't produce anything audible. Therefore, phase modulation and hard clipping are added to make some noise in audible frequency.

- [UltrasonicRingMod Manual (English)](manual/UltrasonicRingMod/UltrasonicRingMod_en.html)

## ClangSynth
<figure>
<img src="manual/ClangSynth/img/clangsynth.png" alt="Image of ClangSynth graphical user interface." style="padding-bottom: 12px;"/>
</figure>

ClangSynth は金属的なパーカッションの音が得意なシンセサイザです。金属的な質感はフィードバック・ディレイ・ネットワーク (FDN) を用いたディレイ間のクロスフィードバックによるものです。 WaveCymbal や FDNCymbal よりはシンバルに似た音が出ます。 FDN をより大きくしてシンバルの合成に特化した ClangCymbal もあります。

- [ClangSynth のマニュアル (日本語)](manual/ClangSynth/ClangSynth_ja.html)

ClangSynth is a synthesizer suitable for percussion sounds. Each voice equips an 8 * 8 feedback delay network (FDN) as a resonator. Oscillator is static wavetalble with controls specialized to make pulse train. For better cymbal sound, ClangCymbal is also available.

- [ClangSynth Manual (English)](manual/ClangSynth/ClangSynth_en.html)

## MiniCliffEQ
<figure>
<img src="manual/MiniCliffEQ/img/minicliffeq.png" alt="Image of MiniCliffEQ graphical user interface." style="padding-bottom: 12px;"/>
</figure>

MiniCliffEQ はタップ数 2^15 = 32768 の FIR フィルタです。初期バージョンからは改善しましたが、それでもタップ数が多いのでレイテンシがサンプリング周波数 48000 Hz のときに 0.34 秒を超えます。主な用途は直流信号 (DC) の抑制ですが、とても急峻なローパス、ハイパス、ローシェルフ、ハイシェルフフィルタとしても使えます。

- [MiniCliffEQ のマニュアル (日本語)](manual/MiniCliffEQ/MiniCliffEQ_ja.html)

MiniCliffEQ is a linear phase FIR filter with 2^15 = 32768 taps. Latency is improved from initial version, however it still exceeds 0.34 seconds in 48000 Hz sampling rate. The primary purpose is to suppress direct current. It can also be used as very sharp low-pass, high-pass, low-shelf, and high-shelf filter.

- [MiniCliffEQ Manual (English)](manual/MiniCliffEQ/MiniCliffEQ_en.html)

## PitchShiftDelay
<figure>
<img src="manual/PitchShiftDelay/img/pitchshiftdelay.png" alt="Image of PitchShiftDelay graphical user interface." style="padding-bottom: 12px;"/>
</figure>

PitchShiftDelay はディレイを使った時間領域ピッチシフタです。 16 倍のオーバーサンプリングによってピッチシフタ特有の癖を多少抑えています。また内部のバッファ長をリアルタイムで変更できます。フォルマント補正がないので、声に使うとヘリウムを吸ったときのような音になります。

- [PitchShiftDelay のマニュアル (日本語)](manual/PitchShiftDelay/PitchShiftDelay_ja.html)

PitchShiftDelay is a time domain pitch shifter based on usual delay. The quirk of pitch shifter is somewhat reduced by 16x oversampling. Also the internal buffer length can be changed in real time. PitchShiftDelay doesn't equipped with formant correction. So applying to voice may sounds strange, like Helium is inhaled.

- [PitchShiftDelay Manual (English)](manual/PitchShiftDelay/PitchShiftDelay_en.html)

## ParallelComb
<figure>
<img src="manual/ParallelComb/img/parallelcomb.png" alt="Image of ParallelComb graphical user interface." style="padding-bottom: 12px;"/>
</figure>

ParallelComb は 1 つバッファの異なる 4 つの時点をフィードバックするコムフィルタです。フィードバック経路にリミッタが挟んであるので、それなりにクリーンな音が出ます。フィードバックの振幅でディレイ時間を変調することで、変な歪みを出すこともできます。

- [ParallelComb のマニュアル (日本語)](manual/ParallelComb/ParallelComb_ja.html)

ParallelComb is a comb filter which has 4 different delay time and 1 shared buffer. A limiter is inserted on feedback path to make the output somewhat clean. Weird distorsion can be added by modulating delay time using feedback signal amplitude.

- [ParallelComb Manual (English)](manual/ParallelComb/ParallelComb_en.html)

## FDN64Reverb
<figure>
<img src="manual/FDN64Reverb/img/fdn64reverb.png" alt="Image of FDN64Reverb graphical user interface." style="padding-bottom: 12px;"/>
</figure>

FDN64Reverb はフィードバック・ディレイ・ネットワーク (FDN) を 1 つだけ搭載したリバーブです。しかしながら、フィードバック行列の大きさは 64 です。

- [FDN64Reverb のマニュアル (日本語)](manual/FDN64Reverb/FDN64Reverb_ja.html)

FDN64Reverb is a reverb equipped with a feedback delay network (FDN). However, the size of feedback matrix is 64.

- [FDN64Reverb Manual (English)](manual/FDN64Reverb/FDN64Reverb_en.html)

## BasicLimiter
<figure>
<img src="manual/BasicLimiter/img/basiclimiter.png" alt="Image of BasicLimiter graphical user interface." style="padding-bottom: 12px;"/>
</figure>

BasicLimiter は名前の通りベーシックなシングルバンドリミッタです。目新しい音は出ませんが、トゥルーピークモードはやや贅沢に設計しています。

- [BasicLimiter のマニュアル (日本語)](manual/BasicLimiter/BasicLimiter_ja.html)

BasicLimiter is a basic single band limiter. The sound is nothing new, but the design of true peak mode is a bit luxurious.

- [BasicLimiter Manual (English)](manual/BasicLimiter/BasicLimiter_en.html)

## CollidingCombSynth
<figure>
<img src="manual/CollidingCombSynth/img/collidingcombsynth.png" alt="Image of CollidingCombSynth graphical user interface." style="padding-bottom: 12px;"/>
</figure>

CollidingCombSynth は Karplus-Strong アルゴリズムによる弦の物理モデルをぶつけることで、弦を擦ったときのような音が出る実験的なシンセサイザです。音程はでますがチューニングが難しいので、どちらかと言うと効果音に向いています。

- [CollidingCombSynth のマニュアル (日本語)](manual/CollidingCombSynth/CollidingCombSynth_ja.html)

CollidingCombSynth is an experimental synthesizer which sounds like bowed string. Equipped with 24 Karplus-Strong string models which collide to each other. Tuning of pitch is possible, but difficult. So mostly suitable for sound effects.

- [CollidingCombSynth Manual (English)](manual/CollidingCombSynth/CollidingCombSynth_en.html)

## L4Reverb
<figure>
<img src="manual/L4Reverb/img/l4reverb.png" alt="Image of L4Reverb graphical user interface." style="padding-bottom: 12px;"/>
</figure>

L4Reverb は LatticeReverb の拡張版です。今回の格子構造はチャンネル毎に 4 * 4 * 4 * 4 = 256 のセクションが設けてあります。

- [L4Reverb のマニュアル (日本語)](manual/L4Reverb/L4Reverb_ja.html)

L4Reverb is an extended version of LatticeReverb. This time, lattice structure has 4 * 4 * 4 * 4 = 256 sections per channel.

- [L4Reverb Manual (English)](manual/L4Reverb/L4Reverb_en.html)

## LatticeReverb
<figure>
<img src="manual/LatticeReverb/img/latticereverb.png" alt="Image of LatticeReverb graphical user interface." style="padding-bottom: 12px;"/>
</figure>

LatticeReverb はディレイを使った高次のオールパスフィルタを格子状につないで入れ子にしたリバーブです。 1 チャンネルあたり 16 のオールパスフィルタを備えています。

- [LatticeReverb のマニュアル (日本語)](manual/LatticeReverb/LatticeReverb_ja.html)

LatticeReverb is a reverb using lattice structure. Equipped with 16 delays per channel.

- [LatticeReverb Manual (English)](manual/LatticeReverb/LatticeReverb_en.html)

## LightPadSynth
<figure>
<img src="manual/LightPadSynth/img/lightpadsynth.png" alt="Image of LightPadSynth graphical user interface." style="padding-bottom: 12px;"/>
</figure>

LightPadSynth はピッチ変調を無くして補間を線形補完に変えた CubicPadSynth の軽量版です。ピッチ変調ができなくなった代わりに、各ボイスにディレイを追加しています。

- [LightPadSynth のマニュアル (日本語)](manual/LightPadSynth/LightPadSynth_ja.html)

LightPadSynth is a lightweight version of CubicPadSynth. For efficiency, interpolation is changed to linear. Also pitch modulation is omitted. Instead, a delay is added for each voice.

- [LightPadSynth Manual (English)](manual/LightPadSynth/LightPadSynth_en.html)

## CubicPadSynth
<figure>
<img src="manual/CubicPadSynth/img/cubicpadsynth.png" alt="Image of CubicPadSynth graphical user interface." style="padding-bottom: 12px;"/>
</figure>

CubicPadSynth は PADsynth アルゴリズムを使ってオシレータのウェーブテーブルを生成するシンセサイザです。キュービック補間を使っているので、可聴域以下の低い周波数でもわりと滑らかな音が出ます。波形を直接描画できる LFO もついています。

- [CubicPadSynth のマニュアル (日本語)](manual/CubicPadSynth/CubicPadSynth_ja.html)

CubicPadSynth is a wavetable synthesizer which uses PADsynth algorithm to generate oscillator tables. Cubic interpolation is used to get smooth sound even at inaudible low frequency range. LFO waveform can be directly drawn.

- [CubicPadSynth Manual (English)](manual/CubicPadSynth/CubicPadSynth_en.html)

## EsPhaser
<figure>
<img src="manual/EsPhaser/img/esphaser.png" alt="Image of EsPhaser graphical user interface." style="padding-bottom: 12px;"/>
</figure>

EsPhaser は最大 4096 の 2 次 Thiran オールパスフィルタを直列につなぐことができるフェイザです。 EnvelopedSine のフェイザと同じアルゴリズムを使っています。

- [EsPhaser のマニュアル (日本語)](manual/EsPhaser/EsPhaser_ja.html)

EsPhaser is a phaser with up to 4096 stages of order 2 Thiran all-pass filters. This is the same phaser used in EnvelopedSine.

- [EsPhaser Manual (English)](manual/EsPhaser/EsPhaser_en.html)

## EnvelopedSine
<figure>
<img src="manual/EnvelopedSine/img/envelopedsine.png" alt="Image of EnvelopedSine graphical user interface." style="padding-bottom: 12px;"/>
</figure>

EnvelopedSineはノート 1 つあたり 64 のサイン波を計算する加算合成シンセサイザです。各サイン波に AD エンベロープとサチュレータがついているので IterativeSinCluster よりもパーカッシブな音が得意です。

- [EnvelopedSine のマニュアル (日本語)](manual/EnvelopedSine/EnvelopedSine_ja.html)

EnvelopedSine is an additive synthesizer that computes 64 sine waves for each note. Difference to IterativeSinCluster is that this synth has AD envelope and saturator for each oscillator. EnvelopedSine is better suited for percussive sounds.

- [EnvelopedSine Manual (English)](manual/EnvelopedSine/EnvelopedSine_en.html)

## IterativeSinCluster
<figure>
<img src="manual/IterativeSinCluster/img/iterativesincluster.png" alt="Image of IterativeSinCluster graphical user interface." style="padding-bottom: 12px;"/>
</figure>

IterativeSinCluster はノート 1 つあたりで 512 のサイン波を計算する加算合成シンセサイザです。このページで配布しているバイナリを利用するには AVX2 をサポートしている CPU が必要です。

- [IterativeSinCluster のマニュアル (日本語)](manual/IterativeSinCluster/IterativeSinCluster_ja.html)

IterativeSinCluster is an additive synthesizer. This synth computes 512 sine waves for each note to make tone cluster. Requires CPU that support AVX2 instructions.

- [IterativeSinCluster Manual (English)](manual/IterativeSinCluster/IterativeSinCluster_en.html)

## TrapezoidSynth
<figure>
<img src="manual/TrapezoidSynth/img/trapezoidsynth.png" alt="Image of TrapezoidSynth graphical user interface." style="padding-bottom: 12px;"/>
</figure>

TrapezoidSynth は 4 つの PTR ランプ関数を継ぎ接ぎした台形オシレータを使ったモノフォニックシンセサイザです。継ぎ接ぎしたのが仇をなしてピッチが高くなるとノイズが乗る欠点があったので8倍にオーバーサンプリングしました。結果としてやたら重たくなっています。

- [TrapezoidSynth のマニュアル (日本語)](manual/TrapezoidSynth/TrapezoidSynth_ja.html)

TrapezoidSynth is a monophonic synthesizer equipped with trapezoid oscillator which is a piecewise function of 4 PTR ramp function. For being piecewise, it has a drawback that increase noise when playing higher tone. Therefore 8 times oversampled and hogging up more CPU.

- [TrapezoidSynth Manual (English)](manual/TrapezoidSynth/TrapezoidSynth_en.html)

## FDNCymbal
<figure>
<img src="manual/FDNCymbal/img/fdncymbal.png" alt="Image of FDNCymbal graphical user interface." style="padding-bottom: 12px;"/>
</figure>

FDNCymbal は WaveCymbal に続いてシンバルのような音を作ろうとしたプラグインです。今回は FDN (Feedback Delay Network) と Schroeder allpass を使っています。名前とは裏腹に、金属的な質感はほとんど Schroeder allpass によって加えられています。シンバルの揺れをシミュレートするためにトレモロもついています。

- [FDNCymbal のマニュアル (日本語)](manual/FDNCymbal/FDNCymbal_ja.html)

FDNCymbal is another attempt to make cymbal sound. This time FDN (Feedback Delay Network) and Schroeder allpass are used. Most of metallic tone comes from Schroeder allpass rather than FDN. Tremolo is added to simulate wobbling of cymbal.

- [FDNCymbal Manual (English)](manual/FDNCymbal/FDNCymbal_en.html)

## WaveCymbal
<figure>
<img src="manual/WaveCymbal/img/wavecymbal.png" alt="Image of WaveCymbal graphical user interface." style="padding-bottom: 12px;"/>
</figure>

WaveCymbal は banded wave-guide という物理モデリングの手法でシンバルの音を作ろうとしたプラグインです。シンバルというよりも、薄い金属板をアスファルトの上で引きずったときのような音が出ます。シンセサイザ、エフェクトのどちらとしても使えます。

- [WaveCymbal のマニュアル (日本語)](manual/WaveCymbal/WaveCymbal_ja.html)

WaveCymbal is an attempt to make cymbal sound with banded wave-guide which is a method for physical modeling. It sounds more like dragging thin metal plate on asphalt rather than cymbal. WaveCymbal can be used as both synthesizer/FX.

- [WaveCymbal Manual (English)](manual/WaveCymbal/WaveCymbal_en.html)

## SyncSawSynth
<figure>
<img src="manual/SyncSawSynth/img/syncsawsynth.png" alt="Image of SyncSawSynth graphical user interface." style="padding-bottom: 12px;"/>
</figure>

SyncSawSynthは10次までのPTR鋸歯波オシレータが使える32ボイスのポリフォニックシンセサイザです。主にハードシンクと周波数変調で電池が切れかけのおもちゃのような音を作る用途に向いています。もちろん普通の音も出ます。

- [SyncSawSynth のマニュアル (日本語)](manual/SyncSawSynth/SyncSawSynth_ja.html)

SyncSawSynth is a 32 voice polyphonic synthesizer using up to 10th order PTR sawtooth oscillator. Just a basic synthesizer but very easy to make a noise similar to low battery sound of some toys.

- [SyncSawSynth Manual (English)](manual/SyncSawSynth/SyncSawSynth_en.html)

## SevenDelay
<figure>
<img src="manual/SevenDelay/img/sevendelay.png" alt="Image of SevenDelay graphical user interface." style="padding-bottom: 12px;"/>
</figure>

SevenDelayはステレオディレイです。分数ディレイに7次のラグランジュ補間を使って7倍のオーバーサンプリングをしています。オートメーションなどでディレイ時間を積極的に動かして音を作る用途に向いています。

- [SevenDelay のマニュアル (日本語)](manual/SevenDelay/SevenDelay_ja.html)

SevenDelay is a stereo delay using 7th order lagurange interpolated fractional delay. Also 7 times oversampled. Suitable for making sound with actively changing delay time by hand or DAW automation.

- [SevenDelay Manual (English)](manual/SevenDelay/SevenDelay_en.html)

# 開発ノート
VST3 の開発中に分かりにくかったことをまとめました。あまり整理されていません。何かあれば issue を作ってもらえれば対応します。 DAW ごとの問題やワークアラウンドについては追加情報を歓迎します。

- [VST3 の開発](dev_note/vst3_dev.html)

こちらはアルゴリズムなどについてまとめたノートです。もともとは波のシミュレーションについて調べていました。

- [波ノート](https://ryukau.github.io/filter_notes/index.html)

These links are notes I took while development. All are written in Japanese.

# ブログ
- [Uhhyou (ブログ)](https://ryukau.blogspot.com/)

# VST について
<a name="vst_logo"></a>
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
