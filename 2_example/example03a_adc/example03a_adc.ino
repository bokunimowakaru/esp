/*******************************************************************************
Example 3: Wi-Fi レコーダ 多機能版

アナログ入力値、デジタル入力値、シリアル入力データをWi-Fi送信します。
アナログ入力を使用しないときはTOUTをGNDへ接続しておいてください。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_SW 4                            // IO 4(10番ピン) にセンサを接続
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define DEVICE "adcnv_1,"                   // デバイス名(5文字+"_"+番号+",")

#define TRIG_BPS 9600                       // シリアル入力速度
#define TRIG_RISE 15                        // 上昇検出
#define TRIG_FALL 15                        // 下降検出

int adc_prev = 0;                           // 前回値（アナログ）
int din_prev = -1;                          // 前回値（デジタル）
char s[64];                                 // シリアル入力
int s_len = 0;                              // シリアル入力長

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // センサを接続したポートを入力に
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(TRIG_BPS);                 // シリアル出力・入力の開始
    Serial.println("Example 03 ADC");       // 「Example 03」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int adc,din;                            // 整数型変数adcとiを定義
    char c='\0';
    
    adc=system_adc_read();                  // AD変換器から値を取得
    din=digitalRead(PIN_SW);                // デジタル入力値を取得
    if(Serial.available()){
        c = Serial.read();
        if( isprint(c) ){
            s[s_len] = c;
            s_len++;
            if( s_len > 63 ) s_len=63;
            s[s_len] = '\0';
        }
    }
    if(
        ( adc > adc_prev - TRIG_FALL && adc < adc_prev + TRIG_RISE)
        && din == din_prev
        && c != '\n'
    ) return;                               // 変化が無ければloopの先頭に戻る
    digitalWrite(PIN_LED,HIGH);             // LED点灯
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(adc);                         // 変数adcの値を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    udp.print(din);                         // 変数dinの値を送信
    Serial.print(adc);                      // 変数adcの値をシリアル出力表示
    Serial.print(", ");                     // 「,」カンマと「␣」を出力表示
    Serial.print(din);                      // 変数dinの値をシリアル出力表示
    if(s_len > 0 && c == '\n'){
        udp.print(", ");                    // 「,」カンマと「␣」を送信
        udp.println(s);                     // 文字列変数sの内容を送信
        Serial.print(", ");                 // 「,」カンマと「␣」を出力表示
        Serial.println(s);                  // 文字列変数sの内容をシリアル出力
        s_len = 0;
    }else{
        udp.println();
        Serial.println();
    }
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(100);                             // 100msの待ち時間処理
    digitalWrite(PIN_LED,LOW);              // LED消灯
    adc_prev = adc;                         // AD値を前回値として保存
    din_prev = din;                         // デジタル入力値を前回値として保存
}
