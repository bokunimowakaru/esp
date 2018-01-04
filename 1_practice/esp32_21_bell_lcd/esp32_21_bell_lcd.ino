/*******************************************************************************
Practice esp32 21 bell lcd
                                                Copyright (c) 2018 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
// LiquidCrystal lcd(12,13,17,16,27,14);    // ESPduino 32 WEMOS D1 32用

void send(const char *device, int id, const char *message){
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(device);                      // 親機デバイス名を送信
    udp.print('_');                         // アンダースコアを送信
    udp.print(id);                          // 親機デバイス番号を送信
    udp.print(',');                         // カンマを送信
    udp.println(message);                   // 音声用データを他の子機へ送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
}

void setup(){                               // 起動時に一度だけ実行する関数
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcdisp("IoTｼｽﾃﾑ esp32_21");             // タイトルをLCDへ表示
    WiFi.mode(WIFI_AP); delay(100);         // 無線LANを【AP】モードに設定
    WiFi.softAP(SSID_AP);                   // ソフトウェアAPの起動
    WiFi.softAPConfig(
        IPAddress(192,168,0,1),             // 固定IPアドレス
        IPAddress(0,0,0,0),                 // 本機のゲートウェイアドレス(なし)
        IPAddress(255,255,255,0)            // ネットマスク
    );
    udp.begin(PORT);                        // UDP待ち受け開始
    lcd.setCursor(0,1);                     // LCDのカーソル位置を液晶の左下へ
    lcd.print(WiFi.softAPIP());             // 本APのIPアドレスをLCDへ表示
}

void loop(){                                // 繰り返し実行する関数
    char s[129];                            // 文字列変数を定義
    int len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
    if(len==0)return;                       // TCPとUDPが未受信時にloop()先頭へ
    memset(s, 0, 129);                      // 文字列変数sの初期化
    udp.read(s, 128);                       // UDP受信データを文字列変数sへ代入
    lcdisp(s,0);                            // 受信データ16文字を1行目に表示する
    if(!strncmp(s,"Ping",4)){               // 受信データの先頭4文字がPongのとき
        lcdisp("ﾖﾋﾞﾘﾝ ｶﾞ ｵｻﾚﾏｼﾀ",1);        // メッセージをLCD2行目に表示
        delay(2500);                        // 子機での「Ping」処理完了待ち
    }
    if(!strncmp(s,"Pong",4)){               // 受信データの先頭4文字がPongのとき
        send("atalk",0,"raikyakudesu");     // デバイスatalkへ「来客です」を送信
        lcdisp("ﾗｲｷｬｸ ﾃﾞｽ",1);              // メッセージをLCD2行目に表示
    }
    if(len<8 || !isalnum(s[6]) || s[7]!=',' ) return;   // フォーマット外を排除
    if(!strncmp(s,"onoff_",6)) send("atalk",0,"bo'tanga/osarema'_shita.");
    if(!strncmp(s,"pir_s_",6)) send("atalk",0,"jinn'kannse'nnsaga/hannno-.");
    if(!strncmp(s,"rd_sw_",6)) send("atalk",0,"do'aga/hirakima'_shita.");
    if(!strncmp(s,"alarm_",6)) send("atalk",0,"yotei'no/ji'kandesu.");
    if(!strncmp(s,"voice_",6)) send("atalk",0,"o'nse'iwo/jushinshima'_shita.");
}
