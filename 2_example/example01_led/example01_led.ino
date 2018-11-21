/*******************************************************************************
Example 1: Wi-Fi インジケータ
LEDをWi-Fi経由で制御するワイヤレスLチカ実験を行います。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
WiFiServer server(23);                      // Wi-Fiサーバ(ポート23=TELNET)定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 01 LED");       // 「Example 01」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0) return;                   // 非接続の時にloop()の先頭に戻る
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            Serial.write(c);                // 文字の内容をシリアルに出力(表示)
            if(c=='0'){                     // 文字変数の内容が「0」のとき
                digitalWrite(PIN_LED,LOW);  // LEDを消灯
            }else if(c=='1'){               // 文字変数の内容が「1」のとき
                digitalWrite(PIN_LED,HIGH); // LEDを点灯
            }
        }
    }
    client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
