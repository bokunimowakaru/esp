/*******************************************************************************
Example 37(=32+5): ESP32 Wi-Fi LCD UDP版
各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示します。

    ESP32 I2Cポート:
                        I2C SDAポート GPIO 21
                        I2C SCLポート GPIO 22
                        
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    lcdSetup();                             // 液晶の初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.05 LCD");      // 「ESP32 eg.05」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    delay(10);                              // ESP32に必要な待ち時間
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        Serial.print(".");
    }
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    udp.begin(PORT);                        // UDP通信御開始
}

void loop(){                                // 繰り返し実行する関数
    char c;                                 // 文字変数cを定義
    char lcd[49];                           // 表示用変数を定義(49バイト48文字)
    int len;                                // 文字列長を示す整数型変数を定義
    
    memset(lcd, 0, 49);                     // 文字列変数lcdの初期化(49バイト)
    len = udp.parsePacket();                // 受信パケット長を変数lenに代入
    if(len==0)return;                       // 未受信のときはloop()の先頭に戻る
    digitalWrite(PIN_LED,HIGH);             // LEDを点灯する
    udp.read(lcd, 48);                      // 受信データを文字列変数lcdへ代入
    Serial.print(lcd);                      // シリアルへ出力する
    lcdPrint(lcd);                          // 液晶に表示する
    digitalWrite(PIN_LED,LOW);              // LEDを消灯する
}
