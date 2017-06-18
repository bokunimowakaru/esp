/*******************************************************************************
Example 7: 温度センサ MCP9700 or LM61CIZ
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
#define SLEEP_P 290*1000000                 // スリープ時間 290秒(約5分)
#define DEVICE "temp._1,"                   // デバイス名(5文字+"_"+番号+",")
#define TEMP_OFFSET -50.0                   // LM61CIZの場合は-60.0に変更する
void sleep();

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 07 TEMP");      // 「Example 07」をシリアル出力表示
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
    float temp;                             // 温度値用の変数
    
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    delay(100);                             // 起動待ち時間
    temp=(float)system_adc_read();          // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    temp *= 1000. / 1023. / 10.;            // 温度(相対値)へ変換
    temp += TEMP_OFFSET;                    // オフセットにより絶対値へ変換
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.println(temp,1);                    // 温度値を送信
    Serial.println(temp,1);                 // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
