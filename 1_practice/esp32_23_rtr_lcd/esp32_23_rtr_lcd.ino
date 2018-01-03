/*******************************************************************************
Practice esp32 23 rtr lcd
                                                Copyright (c) 2018 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#include "Ambient.h"                        // Ambient接続用 ライブラリ
#define AmbientChannelId 0000               // チャネル名(整数) 0=無効
#define AmbientWriteKey "0123456789abcdef"  // ライトキー(16桁の16進数)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
#define DEV1 "pir_s_1,"                     // DEV1に人感センサ1を設定
#define DEV2 "illum_1,"                     // DEV2に照度センサ1を設定
#define DEV3 "humid_1,"                     // DEV3に温湿度センサ1を設定

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
Ambient ambient;                            // クラウドサーバ Ambient用
WiFiClient ambClient;                       // Ambient接続用のTCPクライアント
LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
// LiquidCrystal lcd(12,13,17,16,27,14);    // ESPduino 32 WEMOS D1 32用

void setup(){                               // 起動時に一度だけ実行する関数
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcdisp("IoTｼｽﾃﾑ esp32_23");             // タイトルをLCDへ表示
    WiFi.mode(WIFI_STA);                    // 無線LANを【子機】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(millis()>10000){                 // 10秒経過orボタン押下
            lcdisp("AP ｾﾂｿﾞｸ ｼｯﾊﾟｲ",1);     // Wi-Fi APへの接続を失敗した
            WiFi.disconnect(true);          // WiFiアクセスポイントを切断する
            WiFi.mode(WIFI_OFF);            // 無線LANをOFFに設定する
            while(1);                       // 永久に停止
        }
        lcd.print('.');                     // 接続用プログレス表示
        delay(500);                         // 待ち時間処理(LED点滅用)
    }
    udp.begin(PORT);                        // UDP待ち受け開始
    ambient.begin(AmbientChannelId, AmbientWriteKey, &ambClient);   // Ambient
    lcd.clear();                            // LCD消去　　　　　　　　　　開始
    lcd.print(DEV1); lcd.print(DEV3);       // 初期表示1行目
    lcd.setCursor(0,1); lcd.print(DEV2);    // 初期表示2行目
}

void loop(){                                // 繰り返し実行する関数
    char s[129];                            // 文字列変数を定義
    int num=3;                              // 表示番号用変数numを定義
    int len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
    if(len==0)return;                       // TCPとUDPが未受信時にloop()先頭へ
    memset(s, 0, 129);                      // 文字列変数sの初期化
    udp.read(s, 128);                       // UDP受信データを文字列変数sへ代入
    if(len<8 || !isalnum(s[6]) || s[7]!=',' ) return;   // フォーマット外を排除
    if(!strncmp(s,DEV1,8)) num=0;           // DEV0と一致したらnumへ0を代入
    if(!strncmp(s,DEV2,8)) num=1;           // DEV0と一致したらnumへ1を代入
    if(!strncmp(s,DEV3,8)) num=2;           // DEV0と一致したらnumへ2を代入
    lcdisp4(s,num); delay(500);             // センサ名を0.5秒間、表示する

    /* デジタル入力センサ値を受信したとき */
    if(!strncmp(s,"onoff_",6)||!strncmp(s,"pir_s_",6)||!strncmp(s,"rd_sw_",6)){
        if(atoi(s+8)) lcdisp4("ON",num);
        else lcdisp4("OFF",num);
    }
    /* 照度センサ値を受信したとき */
    if(!strncmp(s,"illum_",6)) lcdisp4(atoi(s+8),num);
    /* 温湿度センサ値を受信したとき */
    if(!strncmp(s,"humid_",6)) lcdisp4(atoi(s+8),atoi(strchr(s+9,',')+1),num);
    /* その他のIoTセンサ値を受信したとき */
    if(!strncmp(s,"alarm_",6)) lcdisp4(s+8,num);
    if(!strncmp(s,"voice_",6)) lcdisp4(s+8,num);
    if(!strncmp(s,"temp._",6)) lcdisp4(atoi(s+8),num);
    if(!strncmp(s,"press_",6)) lcdisp4(atoi(s+8),atoi(strchr(s+9,',')+1),num);
    if(!strncmp(s,"accem_",6)) lcdisp4(atoi(s+8),atoi(strchr(s+9,',')+1),num);
    if(!strncmp(s,"adcnv_",6)) lcdisp4(s+8,num);
    if(!strncmp(s,"ocean_",6)) lcdisp4(atoi(s+8),num);
    if(!strncmp(s,"meter_",6)) lcdisp4(atoi(s+8),num);

    /* 【ルータ機能】温湿度センサ1のときだけ Ambientへ送信 */
    if(strncmp(s,"humid_1,",8)) return;     // Wi-Fi温湿度センサ1以外を排除
    char data[16];                          // Ambient送信用文字列
    dtostrf(atof(s+8),-15,3,data);          // 温度を数値に変換後、文字列へ
    ambient.set(1,data);                    // Ambient(データ1)へ送信
    dtostrf(atof(strchr(s+9,',')+1),-15,3,data);    // 湿度を文字列に変換
    ambient.set(2,data);                    // Ambient(データ2)へ送信
    ambient.send();                         // Ambient送信の終了(実際に送信する)
}
