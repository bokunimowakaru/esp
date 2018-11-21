/*******************************************************************************
Example 1d: Wi-Fi インジケータ HTTP版＆ソフトウェア無線AP版
デモ用：展示会などでPCやスマホから直接ESPモジュールへ接続する場合のサンプル

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID_AP "ESP_SoftAP"                // 無線LANアクセスポイントのSSID
#define TIMEOUT 20000                       // タイムアウト 20秒
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 01D LED HTTP"); // 「Example 01D」をシリアル出力表示
    WiFi.softAP(SSID_AP);                   // ソフトウェアAPの起動
    WiFi.softAPConfig(
        IPAddress(192,168,1,2),             /* 固定IPアドレス */
        IPAddress(192,168,1,1),             /* ゲートウェイアドレス */
        IPAddress(255,255,255,0)            /* ネットマスク */
    );
    server.begin();                         // サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(SSID_AP);                // 本APのSSIDをシリアル出力
    Serial.println(WiFi.softAPIP());        // 本APのIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[129];                            // 文字列変数を定義 129バイト128文字
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
                    if(len>=128) len=127;       // 文字列変数の上限
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
        IPAddress ip=WiFi.softAPIP();       // 変数ipに本APのIPアドレスを代入
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
