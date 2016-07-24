/*******************************************************************************
Example 13: LCDへ表示する(HTTP版)
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義

void setup(){                               // 起動時に一度だけ実行する関数
    lcdSetup(16,2);                         // 液晶の初期化(16桁×2行)
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モード設定(将来用)
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    lcdPrint("Example 13 LCD");             // 「Example 13」をLCDに表示する
    lcdPrintIp2(WiFi.localIP());            // IPアドレスを液晶の2行目に表示
    udp.begin(PORT);                        // UDP通信御開始
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数cを定義
    char s[49]="no data";                   // 文字列変数sを定義
    int len=0;                              // 文字列長を示す整数型変数を定義
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0){
	    len = udp.parsePacket();            // 受信パケット長を変数lenに代入
	    if(len==0)return;                   // 未受信のときはloop()の先頭に戻る
	    udp.read(s, 48);                    // 受信データを文字列変数lcdへ代入
	    lcdPrint(s);                        // 液晶に表示する
		return;                   			// 非接続の時にloop()の先頭に戻る
	}
	lcdPrint("Connected");
    lcdPrint2("Waiting for text");          // 接続されたことを表示
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
