/*******************************************************************************
Practice32 01 led 【Wi-Fi インジケータ親機 UDP版】

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // GPIO 2にLEDを接続
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Practice32 01 led");    // Practice32 01 ledをシリアル出力
    WiFi.mode(WIFI_AP); delay(100);         // 無線LANを【AP】モードに設定
    WiFi.softAP(SSID_AP);                   // ソフトウェアAPの起動
    WiFi.softAPConfig(
        IPAddress(192,168,0,1),             // 固定IPアドレス
        IPAddress(0,0,0,0),                 // 本機のゲートウェイアドレス(なし)
        IPAddress(255,255,255,0)            // ネットマスク
    );
    udp.begin(PORT);                        // UDP待ち受け開始
    Serial.println(SSID_AP);                // 本APのSSIDをシリアル出力
    Serial.println(WiFi.softAPIP());        // 本APのIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
    if(len==0)return;                       // TCPとUDPが未受信時にloop()先頭へ
    memset(s, 0, 65);                       // 文字列変数sの初期化(65バイト)
    udp.read(s, 64);                        // UDP受信データを文字列変数sへ代入
    Serial.println(s);                      // 受信データを表示する
    
    if(!strncmp(s,"Ping",4)){               // 受信データの先頭4文字がPingのとき
        digitalWrite(PIN_LED,HIGH);         // LEDの点灯
    }
    if(!strncmp(s,"Pong",4)){               // 受信データの先頭4文字がPongのとき
        digitalWrite(PIN_LED,LOW);          // LEDの消灯
    }
    if(!strncmp(s,"Test",4)){               // 受信データの先頭4文字がTestのとき
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));        // LEDの状態を反転
    }
    /* 以下は本文中の演習の正解例の一つです。 */
    if(!strncmp(s,"onoff_",6) && isalnum(s[6]) && s[7]==','){
        digitalWrite(PIN_LED,atoi(s+8) & 1);
    }
    /* 解説 ********************************************************************
    
        onoff_n,0を受信するとLEDをOFFし、
        onoff_n,1を受信するとLEDをONする
        nは数字またはアルファベットとした。
        
        !strncmp(s,"onoff_",6)  : 先頭6文字がonoff_のときに真となる
        isalnum(s[6])           : 先頭から7文字目が数字かアルファベットの時に真
        s[7]==','               : 先頭から8文字目がカンマの時に真
        
        以上の条件を満たしたときに、LEDを制御する
        
        atoi(s+8)               : 先頭から9文字目(以降)の文字列型の数字を数値へ
        & 1                     : 論理積。上記が0なら0、0以外なら1となる
        
        ※本文中には「nが数字またはアルファベット」であることを明記していない。
        したがって、本来の正解は、!strncmp(s,"onoff_n,",8)）のようにnで判定。
        ここでは、本書中の他のサンプルとの整合性からこのようにした。
    */
}
