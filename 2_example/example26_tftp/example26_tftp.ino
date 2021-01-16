/*******************************************************************************
Example 26: センサデバイス用 TFTPクライアント 設定

                                           Copyright (c) 2016-2019 Wataru KUNINO
********************************************************************************

TFTPサーバ上から設定ファイルをダウンロードし、モジュール内の設定を変更します。
本サンプルではディープスリープ時間を設定することが出来ます。

TFTPとは
　TFTPはネットワーク機器などの設定ファイルやファームウェアを転送するときなどに
　使用されているデータ転送プロトコルです。  
　使い勝手が簡単で、プロトコルも簡単なので、機器のメンテナンスに向いています。
　認証や暗号化は行わないので、転送時のみ有効にする、もしくは侵入・ファイル転送
　されても問題の無い用途で利用します。

Raspberry PiへのTFTPサーバのインストール方法
    $ sudo apt-get install tftpd-hpa
    
    設定ファイル(/etc/default/tftpd-hpa) 例
    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"

TFTPサーバの起動と停止
    $ chmod 755 /srv/tftp
    $ sudo /etc/init.d/tftpd-hpa start
    $ sudo /etc/init.d/tftpd-hpa stop

転送用のファイルを保存
    $ sudo echo "; Hello! This is from RasPi" | sudo tee /srv/tftp/tftpc_1.ini
    $ sudo echo "SLEEP_SEC=50" | sudo tee -a /srv/tftp/tftpc_1.ini
    $ sudo chmod 644 /srv/tftp/tftpc_1.ini
    $ cat /srv/tftp/tftpc_1.ini
    ; Hello! This is from RasPi
    SLEEP_SEC=50

【注意事項】
・TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器が
　セキュリティの脅威にさらされた状態となります。
・また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する
　恐れが高まります。
・インターネットに接続すると外部からの侵入される場合があります。
・TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
・TFTPが不必要なときは、停止させてください。
*******************************************************************************/
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_EN 13                           // IO 13(5番ピン)をセンサ用の電源に
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define TFTP   "192.168.0.255"              // TFTPサーバのIPアドレス
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
uint32_t SLEEP_P=10*1000000;                // スリープ時間 10秒(uint32_t)
#define DEVICE "tftpc_1,"                   // デバイス名(5文字+"_"+番号+",")

void setup(){                               // 起動時に一度だけ実行する関数
    char data[512];                         // TFTPデータ用変数
    int len_tftp;                           // TFTPデータ長
    
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 26 TFTP");      // 「ESP32 eg.26」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    if(ini_init(data)) initialize(data);    // SPIFFSからINIファイルの読み込み
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    tftpStart(TFTP);                        // TFTPの開始
    do{
        len_tftp = tftpGet(data);           // TFTP受信(data=受信データ)
        if(len_tftp>0){
            initialize(data);               // INIファイル内の内容を変数へ代入
            ini_save(data);                 // INIファイルをSPIFFSへ書込み
        }
    }while(len_tftp);
}

void initialize(char *data){
    /* INIファイル中のSLEEP_SECの値をSLEEP_Pへ代入する */
    SLEEP_P=(uint32_t)ini_parse(data,"SLEEP_SEC") * 1000000;
    if(SLEEP_P < 10 * 1000000) SLEEP_P=10 * 1000000;
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int adc;                                // 整数型変数adcを定義
    
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    delay(5);                               // 起動待ち時間
    adc=system_adc_read();                  // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.println(adc);                       // 変数adcの値を送信
    Serial.print("ADC=");                   // シリアル出力表示
    Serial.println(adc);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    sleep();                                // Deep Sleepモードへ移行
}

void sleep(){
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
