/*******************************************************************************
Practice esp32 12 sw 【Wi-Fi スイッチャ子機 UDP版】

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // IO 2にLEDを接続
#define PIN_SW 0                            // IO 0にスイッチを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号

boolean button;                             // スイッチ状態を保存するための変数

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Practice32 12 sw");     // Practice32 12 swをシリアル出力
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    boolean b=button;                       // 前回のスイッチ状態を変数bに保持
    
    while(b==button){
        button = digitalRead(PIN_SW);       // スイッチ状態の変化待ち
    }
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    if(button==0){                          // ボタンが押されたときに
        udp.println("Ping");                // 　「Ping」を送信
        Serial.println("Ping");             // 　シリアル出力表示
    }else{
        udp.println("Pong");                // そうでないときに「Pong」を送信
    }
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 簡易チャタリング防止策
}
