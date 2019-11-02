# Uhhyou Plugins
[VST®](#vst_logo) 3 プラグインを公開しています。

何かあれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を投げるか `ryukau@gmail.com` にメールを送信してもらえれば対応します。お気軽にお問い合わせください。

This page is publishing [VST®](#vst_logo) 3 plugins.

If you found some part of manual is not clear and/or found typo, please file issue at [GitHub repository](https://github.com/ryukau/VSTPlugins) or send email to `ryukau@gmail.com`. Note that I'm not native in English and it's most likely that the manual contains some weird sentences. I'd like to fix it, so feel free to report those issues.

# LV2 Plugins
For Linux, LV2 version is available.

- [GitHub - ryukau/LV2Plugins](https://github.com/ryukau/LV2Plugins)

# パッケージについて | About Packages
パッケージには次のビルドが含まれています。

- Windows 64bit
- Linux 64bit
- macOS 64bit

ダウンロードリンクは各プラグインのマニュアルと [GitHub のリリースページ](https://github.com/ryukau/VSTPlugins/releases)に掲載しています。

Windows ビルドは FL Studio でテストしています。

Mac を持っていないので、 macOS ビルドはテストできていません。もしバグを見つけたときは [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に報告をお願いします。

Linux ビルドは Ubuntu 18.0.4 でビルドしています。また Bitwig 3.0.3 と REAPER 5.983 で動作確認を行っています。どちらも GUI の表示に問題があったので、今のところ Linux ビルドでは GUI を無効にしています。

A package contains following builds:

- Windows 64bit
- Linux 64bit
- macOS 64bit

Download link is available at plugin manual page or [GitHub releases page](https://github.com/ryukau/VSTPlugins/releases).

Windows build is tested on FL Studio.

macOS build isn't tested because I don't have Mac. If you found a bug, please file a issue to [GitHub repository](https://github.com/ryukau/VSTPlugins).

Linux build is built on Ubuntu 18.0.4 and tested on Bitwig 3.0.3 and Reaper 5.983. Both seems to have problem to display GUI, therefore GUI is currently disabled for Linux build.

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
