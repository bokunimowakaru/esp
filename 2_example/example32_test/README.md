Example 32
# 最新IoTモジュール ESP32-WROOM-32用 スイッチ早押し

ストップウォッチのSTART/STOPボタンを2度連続押しして、どれだけ早くボタンが押せるかを競った経験はあるでしょうか？  
純正のESP32用 DevKit C 開発ボードや DOIT製ESP32用 DEV KIT V1 開発ボード上のBOOT（SW2）ボタンを連続押下したときの速度を、IoTセン
サ用クラウド・サービスAmbientへ送信します。  
速度値（Hz）は、インターネット上に公開されます。

## IoTセンサ用クラウドサービスAmbient

    https://ambidata.io/ch/channel.html?id=725

## 関連ブログ（筆者）

    http://blogs.yahoo.co.jp/bokunimowakaru/55622707.html

## 製作方法

純正のESP32用 DevKitC 開発ボード、またはDOIT製ESP32用 DEV KIT V1 開発ボードなどへスケッチを書き込んでください。  
予め、本スケッチ内の#defineのSSIDとPASSに、お手持ちの無線LANアクセスポイントの設定が必要です。

## 操作方法

各開発ボードのBOOTボタンで早押しが行えます。

Copyright (c) 2016-2018 Wataru KUNINO  
<https://bokunimo.net/>
