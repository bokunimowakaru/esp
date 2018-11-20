# Node-RED 用 サンプル・フロー

Raspberry Pi 上や Bluemix 上で動作する Node-RED を使えば、各種 Wi-Fi対応 IoT センサ用のゲートウェイを簡単に製作することが出来ます。  

## Node-REDのインストール方法

### 実験に必要なもの

この実験に必要なものは、以下の通りです。パソコン、Micro SDカード、電源（ACアダプタ、乾電池）などの各種周辺機器や備品なども必要です。

* Raspberry Pi 3およびその周辺機器
* ESP8266またはESP32用ワイヤレスセンサ
* 無線LANアクセスポイント(インターネット接続済)

Raspberry Piから、以下の手順でインストールします。

### インストール手順

	pi@raspberrypi:~ $ sudo apt-get update
	pi@raspberrypi:~ $ update-nodejs-and-nodered
	
	ここで、エラーが出たとき：
	
	pi@raspberrypi:~ $ bash <(curl -sL https://raw.githubusercontent.com/node-red/raspbian-deb-package/master/resources/update-nodejs-and-nodered)
	
	～更新やインストールに 10分から60分くらいかかります ～

### 実行方法

	pi@raspberrypi:~ $ node-red-start &

メッセージ内に「Server now running」が表示されたら、Node-REDの準備完了です。パソコンのインターネットブラウザから「point a browser at」に続くURLへアクセスしてください。Raspberry Pi上から「http://127.0.0.1:1880」でアクセスることも出来ます。

### サンプルフローの読み込み

以下に、サンプルフロー（本フォルダに収録したjson形式のファイル）の読み込み方を説明します。

1 Node-REDの右上のメニューアイコン（3本の横線）をクリックし、「読込み」→「クリップボード」を選択してください。  
2 インターネットブラウザの別ウィンドウで下記へアクセスし、JSON形式のテキストデータを[Ctrl]＋[A]で全選択し、[Crtl]＋[C]でクリップボードに保持してください。  
3 中央のグレーのエリアに[Ctrl]＋[V]でペーストし、[読込み]をクリックしてください。  
4 フロー図が選択された状態で表示されるので、ウィンドウのフロー1と書かれたシート内の適当な位置をクリックすると、フロー図が固定されます。  
5 動作させるには、画面右上の赤色の「デプロイ」をクリックします。また、右側の子画面で「デバッグ」をクリックすると、受信したUDPデータが表示されます。子画面が表示されていないときは[Ctrl]＋[SPACE]キーを押してください。  


# Node-RED 用 サンプル・フロー

以下にサンプルフローを示します。

## UDP Logger BASIC

各種ESP8266/ESP32用センサが送信したCSV形式のセンサ値をファイルへ保存します。  

	udp_logger.json
	
	保存先：/home/pi/log_udp.csv

## UDP Logger Example

各種ESP8266/ESP32用センサから得られた値に応じた処理を行うサンプルです。

	udp_logger_ex.json

## UDP Logger Ambient

各種ESP8266/ESP32用センサから受信したデータをAmbientへ送信するサンプルです。

	udp_logger_ambient.json

* Ambientノードをダブルクリックし、Channel Id とWrite Keyを入力してください。
* grep_deviceノードの matches regex欄へデバイス名を設定して下さい。

# 関連情報(図入り)

<https://blogs.yahoo.co.jp/bokunimowakaru/56073151.html>

# ライセンス

使用・変更・配布は可能ですが、権利表示を残してください。  
また、提供情報や配布ソフトの使用によって生じた被害については、一切、補償いたしません。  

Copyright (c) 2018 Wataru KUNINO  
<https://bokunimo.net/>
