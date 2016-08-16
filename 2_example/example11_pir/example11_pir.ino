/*******************************************************************************
Example 11: ワイヤレス人感センサ
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_SW 4                            // IO 4(10番ピン) にスイッチを接続
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 3550*1000000                // スリープ時間 3550秒(約60分)
#define DEVICE "pir_s_1,"                   // デバイス名(5文字+"_"+番号+",")

int pir=0;                                  // 人感センサの反応回数
int prev=0;                                 // 人感センサの前回の状態値

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    prev=digitalRead(PIN_SW);               // スイッチの状態を取得
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 11 PIR SW");    // 「Example 11」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        digitalWrite(PIN_LED,waiting%2);    // LED(EN信号)の点滅
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int i,in;
    
    for(i=0;i<30;i++){                      // 繰り返し処理
        in = digitalRead(PIN_SW);           // 現在の状態を取得
        if(prev != in) break;               // 前回と異なれば繰り返しを終了
        delay(100);                         // 待ち時間処理
    }
    if(i >= 30){                            // 3秒間、変化が無かった時
        udp.beginPacket(SENDTO, PORT);      // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.println(pir);                   // 起動直後のスイッチ状態を送信
        Serial.println(pir);                // 起動直後のスイッチ状態を出力表示
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
        sleep();                            // sleepを実行
    }
    pir++;                                  // 人感センサの反応回数をカウント
    prev=in;                                // 現在の状態を変数pirに保存
}

void sleep(){
    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
