/*******************************************************************************
Practice esp32 22 sens lcd
                                                Copyright (c) 2018 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define PORT 1024                           // 受信ポート番号
#define DEV1 "pir_s_1,"                     // DEV1に人感センサ1を設定
#define DEV2 "illum_1,"                     // DEV2に照度センサ1を設定
#define DEV3 "humid_1,"                     // DEV3に温湿度センサ1を設定

WiFiUDP udp;                                // UDP通信用のインスタンスを定義

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
// LiquidCrystal lcd(12,13,17,16,27,14);    // ESPduino 32 WEMOS D1 32用

void lcdisp4(const char *s,int num){        // LCDへの4分割表示用関数の定義
    char lc[7];                             // LCD表示用の6文字配列変数lcの定義
    strncpy(lc,s,6);                        // 文字列を6文字まで変数lcへコピー
    if(lc[5]=='_') lc[5]=s[6];              // 6文字目が_のときは7文字目をコピー
    for(int i=strlen(lc);i<6;i++) lc[i]=' ';// 6文字に満たない場合に空白を代入
    lc[6]='\0';                             // 文字列の終端(文字列の最後は'\0')
    lcd.setCursor(8*((num/2)%2),(num%2));   // num=0:左上,1:左下,2:右上,3:右下
    lcd.print(num+1);                       // num+1の値を表示
    lcd.print(':');                         // 「:」を表示
    lcd.print(lc);                          // 文字列を表示
}

void lcdisp4(int val,int num){              // 整数1値入力時の表示用関数の定義
    char lc[7];                             // LCD表示用の6文字配列変数lcの定義
    itoa(val,lc,10);                        // 数値valを文字列変数lcへ代入
    lcdisp4(lc,num);                        // 前記lcdisp4を呼び出す
}

void lcdisp4(int val1,int val2,int num){    // 整数2値入力時の表示用関数の定義
    char lc[7];                             // LCD表示用の6文字配列変数lcの定義
    snprintf(lc,7,"%d,%d",val1,val2);       // 数値val1と2を文字列変数lcへ代入
    lcdisp4(lc,num);                        // 前記lcdisp4を呼び出す
}

void setup(){                               // 起動時に一度だけ実行する関数
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcdisp("IoTｼｽﾃﾑ esp32_22");             // タイトルをLCDへ表示
    WiFi.mode(WIFI_AP); delay(100);         // 無線LANを【AP】モードに設定
    WiFi.softAP(SSID_AP);                   // ソフトウェアAPの起動
    WiFi.softAPConfig(
        IPAddress(192,168,0,1),             // 固定IPアドレス
        IPAddress(0,0,0,0),                 // 本機のゲートウェイアドレス(なし)
        IPAddress(255,255,255,0)            // ネットマスク
    );
    udp.begin(PORT);                        // UDP待ち受け開始
    lcd.clear();                            // LCD消去
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
}
