/*
本ソースリストは2017/11/2に下記からダウンロードしたものを、国野亘が改変したものです。

	https://learn.adafruit.com/096-mini-color-oled/wiring

ダウンロード時点ではESP32に対応していませんでしたので、ESP32で動作するように修正しました。
将来的には元の権利者であるAdafruitによってESP32対応が図られると思われるため、最小限度の
修正に止めています。動作に問題が生じる可能性もあります。

Adafruitによる最新の情報は、下記に掲載されると思います。

	https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino/issues/10

2017/11/2 国野 亘
*/
--------------------------------------------------------------------------------

This is a library for the 0.96" 16-bit Color OLED with SSD1331 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/684

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above must be included in any redistribution

To download. click the DOWNLOADS button in the top right corner, rename the uncompressed folder Adafruit_SSD1131. Check that the Adafruit_SSD1331 folder contains Adafruit_SSD1331.cpp and Adafruit_SSD1331.h

Place the Adafruit_SSD1331 library folder your <arduinosketchfolder>/libraries/ folder. You may need to create the libraries subfolder if its your first library. Restart the IDE.

You will also have to download the Adafruit GFX Graphics core which does all the circles, text, rectangles, etc. You can get it from
https://github.com/adafruit/Adafruit-GFX-Library
and download/install that library as well 
