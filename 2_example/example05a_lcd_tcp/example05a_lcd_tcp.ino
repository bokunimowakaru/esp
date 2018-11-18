/*******************************************************************************
Example 5a: LCDへ表示する(TCP版)
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
WiFiServer server(23);                      // Wi-Fiサーバ(ポート23=TELNET)定義

void setup(){                               // 起動時に一度だけ実行する関数
    lcdSetup();                             // 液晶の初期化
    lcdPrint("Example 05a LCD");            // 「Example 05a」をLCDに表示する
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数cを定義
    char s[49]="no data";                   // 文字列変数sを定義
    int len=0;                              // 文字列長を示す整数型変数を定義
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0) return;                   // 非接続の時にloop()の先頭に戻る
    lcdPrint("Waiting for text");           // 接続されたことを表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            if(isprint(c)){                 // 表示可能な文字だった場合
                s[len]=c;                   // 文字列変数のlen文字目に代入
                len++;                      // 変数lenに1を加算
            }else if(c=='\n'){              // 改行コードが入力された場合
                s[len]=0;                   // 文字列を終端する
                lcdPrint(s);                // 液晶に表示する
                len=0;                      // 文字列長をリセットする
            }
        }
    }
    client.stop();                          // クライアントの切断
    if(len) s[len]=0;                       // 文字列変数sを終端する
    lcdPrint(s);                            // 文字列変数sの内容を液晶に表示する
}
