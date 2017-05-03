/*******************************************************************************
Example 33(=32+1): ESP32でLEDを点滅させる
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#define PIN_LED 2                           // IO 2 にLEDを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
WiFiServer server(23);                      // Wi-Fiサーバ(ポート23=TELNET)定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.01 LED");      // 「ESP32 eg.01」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        Serial.print(".");
    }
    server.begin();                         // サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    
    client = server.available();            // 接続されたクライアントを生成
    if(!client) return;                     // 非接続の時にloop()の先頭に戻る
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            Serial.write(c);                // 文字の内容をシリアルに出力(表示)
            if(c=='0'){                     // 文字変数の内容が「0」のとき
                digitalWrite(PIN_LED,LOW);  // LEDを消灯
                
                /* クライアント側からの切断ができない(執筆時点)ためbreakを追加 */
                break;
            }else if(c=='1'){               // 文字変数の内容が「1」のとき
                digitalWrite(PIN_LED,HIGH); // LEDを点灯
                
                /* クライアント側からの切断ができない(執筆時点)ためbreakを追加 */
                break;
            }
        }
    }
    client.stop();                          // クライアントの切断
    Serial.println("\nDisconnected");       // シリアル出力表示
}
