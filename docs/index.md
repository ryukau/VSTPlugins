---
lang: ja
...

# Uhhyou Plugins
[VST®](#vst_logo) 3 プラグインを公開しています。

何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を投げるか `ryukau@gmail.com` にメールを送信してもらえれば対応します。お気軽にお問い合わせください。

This page is publishing [VST®](#vst_logo) 3 plugins.

If you found some part of manual is not clear and/or found typo, please file issue at [GitHub repository](https://github.com/ryukau/VSTPlugins) or send email to `ryukau@gmail.com`. Note that I'm not native in English and it's most likely that the manual contains some weird sentences. I'd like to fix it, so feel free to report those issues.

# LV2 Plugins
Linux 向けに LV2 版のプラグインもあります。

- [GitHub - ryukau/LV2Plugins](https://github.com/ryukau/LV2Plugins)

For Linux, LV2 version is available.

- [GitHub - ryukau/LV2Plugins](https://github.com/ryukau/LV2Plugins)

# プラグイン一覧 | Plugin List

- [CollidingCombSynth](#collidingcombsynth)

- [CubicPadSynth](#cubicpadsynth)

- [EnvelopedSine](#envelopedsine)

- [EsPhaser](#esphaser)

- [FDNCymbal](#fdncymbal)

- [IterativeSinCluster](#iterativesincluster)

- [L4Reverb](#l4reverb)

- [LatticeReverb](#latticereverb)

- [LightPadSynth](#lightpadsynth)

- [SevenDelay](#sevendelay)

- [SyncSawSynth](#syncsawsynth)

- [TrapezoidSynth](#trapezoidsynth)

- [WaveCymbal](#wavecymbal)

## CollidingCombSynth
![](manual/CollidingCombSynth/img/collidingcombsynth.png)

CollidingCombSynth は Karplus-Strong アルゴリズムによる弦の物理モデルをぶつけることで、弦を擦ったときのような音が出る実験的なシンセサイザです。音程はでますがチューニングが難しいので、どちらかと言うと効果音に向いています。

- [CollidingCombSynth のマニュアル (日本語)](manual/CollidingCombSynth/CollidingCombSynth_ja.html)

CollidingCombSynth is an experimental synthesizer which sounds like bowed string. Equipped with 24 Karplus-Strong string models which collide to each other. Tuning of pitch is possible, but difficult. So mostly suitable for sound effects.

- [CollidingCombSynth Manual (English)](manual/CollidingCombSynth/CollidingCombSynth_en.html)

## L4Reverb
![](manual/L4Reverb/img/l4reverb.png)

L4Reverb は LatticeReverb の拡張版です。今回の格子構造はチャンネル毎に 4 * 4 * 4 * 4 = 256 のセクションが設けてあります。

- [L4Reverb のマニュアル (日本語)](manual/L4Reverb/L4Reverb_ja.html)

L4Reverb is an extended version of LatticeReverb. This time, lattice structure has 4 * 4 * 4 * 4 = 256 sections per channel.

- [L4Reverb Manual (English)](manual/L4Reverb/L4Reverb_en.html)

## LatticeReverb
![](manual/LatticeReverb/img/latticereverb.png)

LatticeReverb はディレイを使った高次のオールパスフィルタを格子状につないで入れ子にしたリバーブです。 1 チャンネルあたり 16 のオールパスフィルタを備えています。

- [LatticeReverb のマニュアル (日本語)](manual/LatticeReverb/LatticeReverb_ja.html)

LatticeReverb is a reverb using lattice structure. Equipped with 16 delays per channel.

- [LatticeReverb Manual (English)](manual/LatticeReverb/LatticeReverb_en.html)

## LightPadSynth
![](manual/LightPadSynth/img/lightpadsynth.png)

LightPadSynth はピッチ変調を無くして補間を線形補完に変えた CubicPadSynth の軽量版です。ピッチ変調ができなくなった代わりに、各ボイスにディレイを追加しています。

- [LightPadSynth のマニュアル (日本語)](manual/LightPadSynth/LightPadSynth_ja.html)

LightPadSynth is a lightweight version of CubicPadSynth. For efficiency, interpolation is changed to linear. Also pitch modulation is omitted. Instead, a delay is added for each voice.

- [LightPadSynth Manual (English)](manual/LightPadSynth/LightPadSynth_en.html)

## CubicPadSynth
![](manual/CubicPadSynth/img/cubicpadsynth.png)

CubicPadSynth は PADsynth アルゴリズムを使ってオシレータのウェーブテーブルを生成するシンセサイザです。キュービック補間を使っているので、可聴域以下の低い周波数でもわりと滑らかな音が出ます。波形を直接描画できる LFO もついています。

- [CubicPadSynth のマニュアル (日本語)](manual/CubicPadSynth/CubicPadSynth_ja.html)

CubicPadSynth is a wavetable synthesizer which uses PADsynth algorithm to generate oscillator tables. Cubic interpolation is used to get smooth sound even at inaudible low frequency range. LFO waveform can be directly drawn.

- [CubicPadSynth Manual (English)](manual/CubicPadSynth/CubicPadSynth_en.html)

## EsPhaser
![](manual/EsPhaser/img/esphaser.png)

EsPhaser は最大 4096 の 2 次 Thiran オールパスフィルタを直列につなぐことができるフェイザです。 EnvelopedSine のフェイザと同じアルゴリズムを使っています。

- [EsPhaser のマニュアル (日本語)](manual/EsPhaser/EsPhaser_ja.html)

EsPhaser is a phaser with up to 4096 stages of order 2 Thiran all-pass filters. This is the same phaser used in EnvelopedSine.

- [EsPhaser Manual (English)](manual/EsPhaser/EsPhaser_en.html)

## EnvelopedSine
![](manual/EnvelopedSine/img/envelopedsine.png)

EnvelopedSineはノート 1 つあたり 64 のサイン波を計算する加算合成シンセサイザです。各サイン波に AD エンベロープとサチュレータがついているので IterativeSinCluster よりもパーカッシブな音が得意です。

- [EnvelopedSine のマニュアル (日本語)](manual/EnvelopedSine/EnvelopedSine_ja.html)

EnvelopedSine is an additive synthesizer that computes 64 sine waves for each note. Difference to IterativeSinCluster is that this synth has AD envelope and saturator for each oscillator. EnvelopedSine is better suited for percussive sounds.

- [EnvelopedSine Manual (English)](manual/EnvelopedSine/EnvelopedSine_en.html)

## IterativeSinCluster
![](manual/IterativeSinCluster/img/iterativesincluster.png)

IterativeSinCluster はノート 1 つあたりで 512 のサイン波を計算する加算合成シンセサイザです。このページで配布しているバイナリを利用するには AVX2 をサポートしている CPU が必要です。

- [IterativeSinCluster のマニュアル (日本語)](manual/IterativeSinCluster/IterativeSinCluster_ja.html)

IterativeSinCluster is an additive synthesizer. This synth computes 512 sine waves for each note to make tone cluster. Requires CPU that support AVX2 instructions.

- [IterativeSinCluster Manual (English)](manual/IterativeSinCluster/IterativeSinCluster_en.html)

## TrapezoidSynth
![](manual/TrapezoidSynth/img/trapezoidsynth.png)

TrapezoidSynth は 4 つの PTR ランプ関数を継ぎ接ぎした台形オシレータを使ったモノフォニックシンセサイザです。継ぎ接ぎしたのが仇をなしてピッチが高くなるとノイズが乗る欠点があったので8倍にオーバーサンプリングしました。結果としてやたら重たくなっています。

- [TrapezoidSynth のマニュアル (日本語)](manual/TrapezoidSynth/TrapezoidSynth_ja.html)

TrapezoidSynth is a monophonic synthesizer equipped with trapezoid oscillator which is a piecewise function of 4 PTR ramp function. For being piecewise, it has a drawback that increase noise when playing higher tone. Therefore 8 times oversampled and hogging up more CPU.

- [TrapezoidSynth Manual (English)](manual/TrapezoidSynth/TrapezoidSynth_en.html)

## FDNCymbal
![](manual/FDNCymbal/img/fdncymbal.png)

FDNCymbal は WaveCymbal に続いてシンバルのような音を作ろうとしたプラグインです。今回は FDN (Feedback Delay Network) と Schroeder allpass を使っています。名前とは裏腹に、金属的な質感はほとんど Schroeder allpass によって加えられています。シンバルの揺れをシミュレートするためにトレモロもついています。

- [FDNCymbal のマニュアル (日本語)](manual/FDNCymbal/FDNCymbal_ja.html)

FDNCymbal is another attempt to make cymbal sound. This time FDN (Feedback Delay Network) and Schroeder allpass are used. Most of metallic tone comes from Schroeder allpass rather than FDN. Tremolo is added to simulate wobbling of cymbal.

- [FDNCymbal Manual (English)](manual/FDNCymbal/FDNCymbal_en.html)

## WaveCymbal
![](manual/WaveCymbal/img/wavecymbal.png)

WaveCymbal は banded wave-guide という物理モデリングの手法でシンバルの音を作ろうとしたプラグインです。シンバルというよりも、薄い金属板をアスファルトの上で引きずったときのような音が出ます。シンセサイザ、エフェクトのどちらとしても使えます。

- [WaveCymbal のマニュアル (日本語)](manual/WaveCymbal/WaveCymbal_ja.html)

WaveCymbal is an attempt to make cymbal sound with banded wave-guide which is a method for physical modeling. It sounds more like dragging thin metal plate on asphalt rather than cymbal. WaveCymbal can be used as both synthesizer/FX.

- [WaveCymbal Manual (English)](manual/WaveCymbal/WaveCymbal_en.html)

## SyncSawSynth
![](manual/SyncSawSynth/img/syncsawsynth.png)

SyncSawSynthは10次までのPTR鋸歯波オシレータが使える32ボイスのポリフォニックシンセサイザです。主にハードシンクと周波数変調で電池が切れかけのおもちゃのような音を作る用途に向いています。もちろん普通の音も出ます。

- [SyncSawSynth のマニュアル (日本語)](manual/SyncSawSynth/SyncSawSynth_ja.html)

SyncSawSynth is a 32 voice polyphonic synthesizer using up to 10th order PTR sawtooth oscillator. Just a basic synthesizer but very easy to make a noise similar to low battery sound of some toys.

- [SyncSawSynth Manual (English)](manual/SyncSawSynth/SyncSawSynth_en.html)

## SevenDelay
![](manual/SevenDelay/img/sevendelay.png)

SevenDelayはステレオディレイです。分数ディレイに7次のラグランジュ補間を使って7倍のオーバーサンプリングをしています。オートメーションなどでディレイ時間を積極的に動かして音を作る用途に向いています。

- [SevenDelay のマニュアル (日本語)](manual/SevenDelay/SevenDelay_ja.html)

SevenDelay is a stereo delay using 7th order lagurange interpolated fractional delay. Also 7 times oversampled. Suitable for making sound with actively changing delay time by hand or DAW automation.

- [SevenDelay Manual (English)](manual/SevenDelay/SevenDelay_en.html)

# 開発ノート
VST3 の開発中に分かりにくかったことをまとめました。あまり整理されていません。何かあれば issue を作ってもらえれば対応します。 DAW ごとの問題やワークアラウンドについては追加情報を歓迎します。

- [VST3 の開発](dev_note/vst3_dev.html)

こちらはアルゴリズムなどについてまとめたノートです。もともとは波のシミュレーションについて調べていました。

- [波ノート](https://ryukau.github.io/filter_notes/index.html)

If you are interested in those articles and would like read it in English, contact me via GitHub issue or email. I'll translate it on demand. Contact link/information is at the top of this page.

# ブログ
- [Uhhyou (ブログ)](https://ryukau.blogspot.com/)

# VST について
<a name="vst_logo"></a>
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
