/*******************************************************************************
Example 1A: LEDを点滅させる TELNET版
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define TIMEOUT 20000                       // タイムアウト 20秒
WiFiServer server(23);                      // Wi-Fiサーバ(ポート23=TELNET)定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 01A LED TELNET");   // 「Example 01A」を出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    int i=0;                                // 待ち受け時間のカウント用の変数
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0) return;                   // 非接続の時にloop()の先頭に戻る
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            i=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            switch(c){                      // 文字cに応じて
                case '0':                       // 文字変数cの内容が「0」のとき
                    digitalWrite(PIN_LED,LOW);  // LEDを消灯
                    Serial.println("0");        // 「0」をシリアルに出力
                    break;
                case '1':                       // 文字変数cの内容が「1」のとき
                    digitalWrite(PIN_LED,HIGH); // LEDを点灯
                    Serial.println("1");        // 「1」をシリアルに出力
                    break;
                case 0xF6:                      // AYT信号
                    Serial.println("AYT");      // 文字の内容をシリアルに出力
                    client.println("Yes");      // 「Yes」を応答
                    break;
                case 0xF3:                      // BRK信号
                    Serial.println("BRK");      // 文字の内容をシリアルに出力
                    i=TIMEOUT;                  // 切断するためにiを最大値に
                    break;
            }
        }
        i++;                                // 変数iの値を1だけ増加させる
        if(i>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
