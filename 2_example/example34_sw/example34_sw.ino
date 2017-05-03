/*******************************************************************************
Example 34(=32+2): スイッチ状態を送信する
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // IO 2 にLEDを接続
#define PIN_SW 0                            // IO 0 にスイッチを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.02 SW");       // 「ESP32 eg.02」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    
    while(digitalRead(PIN_SW)){             // Hレベル(スイッチ解放)時に繰り返し
        delay(100);                         // 100msの待ち時間処理を実行
    }
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println("Ping");                    // 「Pong」を送信
    Serial.println("Ping");                 // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    
    while(digitalRead(PIN_SW)==0){          // Lレベル(スイッチ押下)時に繰り返し
        delay(100);                         // 100msの待ち時間処理を実行
    }
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println("Pong");                    // 「Pong」を送信
    Serial.println("Pong");                 // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
}
