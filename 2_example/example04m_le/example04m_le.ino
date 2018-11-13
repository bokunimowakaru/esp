/*******************************************************************************
Example 4m: 乾電池駆動に向けた低消費電力動作のサンプル
[取得値が近いときは送信しない]
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_EN 13                           // IO 13(5番ピン)をセンサ用の電源に
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 59*1000000                  // スリープ時間 59秒(uint32_t)
#define SLEEP_N 30                          // 最長スリープ時間 SLEEP_P×SLEEP_N
void sleep();

int adc;                                    // 整数型変数adcを定義
extern int WAKE_COUNT;

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    int mem;                                // RTCメモリからの数値データ保存用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 04 LE");        // 「Example 04」をシリアル出力表示
    adc = system_adc_read();                // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    Serial.print("ADC = ");
    Serial.println(adc);                    // 取得値をシリアル出力表示
    mem = readRtcInt();                     // RTCメモリからの読みとる
    if(WAKE_COUNT%SLEEP_N){                 // SLEEP_Nが0以外の時に以下を実行
        if( mem ) mem = adc * 100 / mem;    // メモリ値に対するAD変換値を算出
        if( mem>95 && mem<105 ) sleep();    // メモリ値と近い場合にスリープ
    }
    writeRtcInt(adc);                       // AD変換値をRTCメモリへ保存
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        digitalWrite(PIN_EN,waiting%2);     // LED(EN信号)の点滅
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println(adc);                       // 変数adcの値を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    sleep();
}

void sleep(){
    digitalWrite(PIN_EN,LOW);               // LED(EN信号)の消灯
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
