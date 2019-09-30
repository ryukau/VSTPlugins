# Uhhyou Plugins
[VST®](#vst_logo) 3 プラグインを公開しています。

マニュアルの分かりにくい箇所や誤字脱字があれば [GitHub のリポジトリ](https://github.com/ryukau/VSTPlugins)に issue を投げるか `ryukau@gmail.com` にメールを送信してもらえれば対応します。お気軽にお問い合わせください。

Linux ビルドは Ubuntu 18.0.4 でビルドしています。また Bitwig 3.0.3 と REAPER 5.983 で動作確認を行っています。どちらも GUI の表示に問題があったので、今のところ Linux ビルドでは GUI を無効にしています。

This page is publishing [VST®](#vst_logo) 3 plugins.

If you found some part of manual is not clear and/or found typo, please file issue at [GitHub repository](https://github.com/ryukau/VSTPlugins) or send email to `ryukau@gmail.com`. Note that I'm not native in English and it's most likely that the manual contains some weird sentences. I'd like to fix it, so feel free to report those issues.

Linux build is built on Ubuntu 18.0.4 and tested on Bitwig 3.0.3 and Reaper 5.983. Both seems to have problem to display GUI, therefore GUI is currently disabled for Linux build.

## SevenDelay
![](manual/SevenDelay/img/sevendelay.png)

- [Download SevenDelay 0.1.0 - VST 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/SevenDelay0.1.0/SevenDelay.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">

パッケージには次のビルドが含まれています。

- Windows 64bit
- Windows 32bit
- Linux 64bit

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
