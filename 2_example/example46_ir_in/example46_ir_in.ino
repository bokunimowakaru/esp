/*******************************************************************************
Example 46 (=32+14): ESP32 Wi-Fi リモコン赤外線レシーバ
赤外線リモコン信号を受信し、受信データをWi-Fi送信します。

    GPIO 4(26番ピン)へIRセンサを接続

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define DATA_LEN_MAX 16                     // リモコンコードのデータ長(byte)
#define PIN_IR_IN 4                         // GPIO 4(26番ピン) にIRセンサを接続
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define DEVICE "ir_in_1,"                   // デバイス名(5文字+"_"+番号+",")

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_IR_IN, INPUT);              // IRセンサの入力ポートの設定
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.14 ir_in");    // 「Example 14」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    byte data[DATA_LEN_MAX];                // リモコン信号データ用
    int len,len8;                           // 信号長 len(bits),len8（bytes）
    byte i;

    digitalWrite(PIN_LED,LOW);              // LEDを消灯状態に
    len = ir_read(data, DATA_LEN_MAX, 255); // 赤外線信号を読み取る
    len8 = len / 8;                         // ビット長を8で割った値をlen8へ代入
    if(len%8) len8++;                       // 余りがあった場合に1バイトを加算
    if(len8>=2){                            // 2バイト以上の時に以下を実行
        digitalWrite(PIN_LED,HIGH);         // LEDを点灯状態に
        udp.beginPacket(SENDTO, PORT);      // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(len);                     // 信号長を送信
        Serial.print(len);                  // 信号長をシリアル出力表示
        for(i=0;i<len8;i++){                // 信号長(バイト)の回数の繰り返し
            udp.print(   ",");              // 「,」カンマを送信
            Serial.print(",");              // 「,」カンマを表示
            udp.print(   data[i]>>4,HEX);   // dataを16進で送信(上位4ピット)
            Serial.print(data[i]>>4,HEX);   // dataを16進で表示(上位4ピット)
            udp.print(   data[i]&15,HEX);   // dataを16進で送信(下位4ピット)
            Serial.print(data[i]&15,HEX);   // dataを16進で表示(下位4ピット)
        }
        Serial.println();                   // 改行をシリアル出力表示
        udp.println();                      // 改行をUDP送信
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
    }
}
