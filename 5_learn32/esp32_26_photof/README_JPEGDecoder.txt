/*
ソースリストJPEGDecoderは2017/11/2に下記からダウンロードしたものを、国野亘が改変したものです。
改変部以外は原作者の権利が継続します。

	https://github.com/MakotoKurauchi/JPEGDecoder

2017/11/2 国野 亘
*/

JPEGDecoder
===========

JPEG Decoder for Arduino

概要
----
Arduino 用 JPEG デコーダです。デコーダ部には [picojpeg](https://code.google.com/p/picojpeg/) を使用しています。

サンプルコード
----
###SerialCsvOut

SD カード上の JPEG ファイルをブロックごとにデコードし、シリアルから CSV を出力します。

変更履歴
----
V0.01 - 最初のリリース