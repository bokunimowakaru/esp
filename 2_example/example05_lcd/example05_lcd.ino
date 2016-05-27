/*******************************************************************************
Example 5: LCDへ表示する
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

void setup(){                               // 起動時に一度だけ実行する関数
    lcdSetup();                             // 液晶の初期化
    lcdPrint("Example 05 LCD");             // 「Example 05」をLCDに表示する
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モード設定
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
    udp.begin(PORT);                        // UDP通信御開始
}

void loop(){                                // 繰り返し実行する関数
    char c;                                 // 文字変数cを定義
    char data[49];                          // 受信用変数を定義(49バイト48文字)
    char lcd[49];                           // 表示用変数を定義(49バイト48文字)
    int len;                                // 文字列長を示す整数型変数を定義
    int i,j=0;                              // ループ用の整数型変数i,jを定義
    
    memset(lcd, 0, 49);                     // 文字列変数lcdの初期化(49バイト)
    len = udp.parsePacket();                // 受信パケット長を変数lenに代入
    if(len==0)return;                       // 未受信のときはloop()の先頭に戻る
    udp.read(data, 48);                     // 受信データを文字列変数dataへ代入
    if(len>48)len = 48;                     // 受信データは48文字(カナ16字)以内
    for(i=0;i<len;i++){                     // 受信文字数len回の繰り返し処理
        if(isprint(data[i])){               // 文字data[i]が表示可能のとき
            lcd[j]=data[i];                 // 当該文字を変数lcdにコピーする
            j++;                            // 変数jの値を1だけ増やす
        }else if(data[i]==0) break;         // 終端コードの時にforループを抜ける
    }
    lcdPrint(lcd);                          // 液晶に表示する
}
