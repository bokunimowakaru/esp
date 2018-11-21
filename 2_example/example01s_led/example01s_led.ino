/*******************************************************************************
Example 1s: Wi-Fi インジケータ HTTP版＆IPアドレス固定版

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define TIMEOUT 20000                       // タイムアウト 20秒
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 01S LED STAT"); // 「Example 01S」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.config(
        IPAddress(192,168,0,143),           /* 固定IPアドレス */
        IPAddress(192,168,0,1),             /* ゲートウェイアドレス */
        IPAddress(255,255,255,0),           /* ネットマスク */
        IPAddress(8,8,8,8)                  /* DNSサーバ(省略可能) */
    );
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    Serial.println(WiFi.gatewayIP());       // ゲートウェイのIPアドレスを出力
    Serial.println(WiFi.subnetMask());      // ネットマスクのIPアドレスを出力
    Serial.println(WiFi.dnsIP());           // DNSのIPアドレスを出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列の長さカウント用の変数
    int led=-1;                             // LEDの設定(-1は未設定)
    int i=0;                                // 待ち受け時間のカウント用の変数
    int headF=0;                            // HTTPヘッダ用フラグ(0:初期状態)
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0) return;                   // 非接続の時にloop()の先頭に戻る
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            i=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            switch(c){                      // 文字cに応じて
                case '\0':                      // 文字変数cの内容が空のとき
                case '\r':                      // 文字変数cの内容がCRのとき
                    break;                      // 何もしない
                case '\n':                      // 文字変数cの内容がLFのとき
                    if(strncmp(s,"GET /L",6)==0 && len>6){
                        led=1;                  // 変数ledに1を代入
                        if(s[6]=='0')led=0;     // 7番目の文字が0のとき0を代入
                    }else if( len==0 ) headF=1; // ヘッダの終了
                    len=0;                      // 文字列長を0に
                    break;
                default:                        // その他の場合
                    s[len]=c;                   // 文字列変数に文字cを追加
                    len++;                      // 変数lenに1を加算
                    s[len]='\0';                // 文字列を終端
                    if(len>=64) len=63;         // 文字列変数の上限
                    break;
            }
        }
        i++;                                // 変数iの値を1だけ増加させる
        if(headF) break;                    // HTTPヘッダが終わればwhileを抜ける
        if(i>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        client.println("HTTP/1.1 200 OK");  // HTTP OKを応答
        if(led>=0)digitalWrite(PIN_LED,led);// LEDの制御
        IPAddress ip=WiFi.localIP();        // 変数ipに本機のIPアドレスを代入
        sprintf(s,
            "<p>LED=%01d<p><a href=\"/L1\">LED ON </a>,http://%d.%d.%d.%d/L1",
            digitalRead(PIN_LED),ip[0],ip[1],ip[2],ip[3]);
        client.print("Content-Length: ");   // HTTPヘッダ情報を出力
        client.println(17+strlen(s)*2-8+4+44); // コンテンツ長さを出力(改行2バイト)
        client.println();                   // HTTPヘッダの終了を出力
        client.println("<html>");           // HTML開始タグを出力(IE以外で必要)
        client.println("<meta name=\"viewport\" content=\"width=240\">");
        client.println(s);                  // HTTPコンテンツ(LED ON)を出力
        Serial.println(s);                  // シリアルへコンテンツを出力
        sprintf(s,
            "<p><a href=\"/L0\">LED OFF</a>,http://%d.%d.%d.%d/L0",
            ip[0],ip[1],ip[2],ip[3]);
        client.println(s);                  // HTTPコンテンツ(LED OFF)を出力
        Serial.println(s);                  // シリアルへコンテンツを出力
        client.println("</html>");          // HTML終了タグを出力(IE以外で必要)
    }
//  client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
