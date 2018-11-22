/*******************************************************************************
Practice esp32 13 lcd 【Wi-Fi LCD親機 UDP版】
・カタカナ対応版
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#define PIN_LED 2                           // GPIO 2にLEDを接続
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
// LiquidCrystal lcd(12,13,17,16,27,14);    // ESPduino 32 WEMOS D1 32用

void utf_del_uni(char *s,int len){
    byte i=0;
    byte j=0;
    while(s[i]!='\0'){
        if((byte)s[i]==0xEF){
            if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
            i+=2;
        }
        if(isprint(s[i]) || ((byte)s[i] >=0xA0 && (byte)s[i] <= 0xDF)){
            s[j]=s[i];
            j++;
        }
        i++;
    }
    s[j]='\0';
    for(j++;j<len;j++) s[j]='\0';
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    char s[]="ﾚﾝｼｭｳ JP \\13 ";
    utf_del_uni(s, 13);                     // UTF-8をLCD表示用キャラクタへ変換
    lcd.print(s);
    lcd.print('\xFB');                      // 漢字の万を表示(LCDによる)
    lcd.print('\xFC');                      // 漢字の円を表示(LCDによる)
    WiFi.mode(WIFI_AP); delay(100);         // 無線LANを【AP】モードに設定
    WiFi.softAP(SSID_AP);                   // ソフトウェアAPの起動
    WiFi.softAPConfig(
        IPAddress(192,168,0,1),             // 固定IPアドレス
        IPAddress(0,0,0,0),                 // 本機のゲートウェイアドレス(なし)
        IPAddress(255,255,255,0)            // ネットマスク
    );
    udp.begin(PORT);                        // UDP待ち受け開始
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    lcd.print(WiFi.softAPIP());             // 本APのIPアドレスをLCDへ表示
}

void lc(char *s,int y){
    char lc[17],i;                          // LCD表示用の文字列変数
    memset(lc,0,17);                        // 文字列変数lcの初期化(17バイト)
    strncpy(lc,s,16);                       // 先頭から16文字をlcdへコピー
    for(i=0;i<16;i++)if(iscntrl(lc[i]))lc[i]=' ';   // 制御文字を空白へ置き換え
    lcd.setCursor(0,y);                     // カーソル位置を左へ
    lcd.print(lc);                          // 液晶へ転送
}

void loop(){                                // 繰り返し実行する関数
    char s[129];                            // 文字列変数を定義
    int len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
    if(len==0)return;                       // TCPとUDPが未受信時にloop()先頭へ
    memset(s, 0, 129);                      // 文字列変数sの初期化
    udp.read(s, 128);                       // UDP受信データを文字列変数sへ代入
    utf_del_uni(s, 128);                    // UTF-8をLCD表示用キャラクタへ変換
    lc(s,0);                                // 受信データ16文字を1行目に表示する
    lc(s+16,1);                             // 17文字目からを2行目に表示する
}
