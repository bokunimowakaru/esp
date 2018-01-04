/*******************************************************************************
Practice esp32 24 ntp lcd
                                                Copyright (c) 2018 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
// LiquidCrystal lcd(12,13,17,16,27,14);    // ESPduino 32 WEMOS D1 32用

unsigned long TIME=0;                       // 1970年からmillis()＝0までの秒数
char date[20]="2000/01/01,00:00:00";        // 日時保持用

void wifi_ntp(){                            // Wi-Fi接続とNTPアクセスの実行関数
    WiFi.mode(WIFI_STA);                    // 無線LANを【子機】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    int t=0;                                // AP接続待ち時間
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(t>20){                           // 10秒経過時
            lcdisp(" AP ｾﾂｿﾞｸ ｼｯﾊﾟｲ",1);    // Wi-Fi APへの接続を失敗した
            WiFi.disconnect(true);          // WiFiアクセスポイントを切断する
            WiFi.mode(WIFI_OFF);            // 無線LANをOFFに設定する
            return;                         // 終了
        }
        lcd.print('.');                     // 接続用プログレス表示
        delay(500);                         // 待ち時間処理(LED点滅用)
        t++;                                // tに1を加算
    }
    TIME=getNtp();                          // NTP時刻を取得
    time2txt(date,TIME);                    // 取得した時刻を文字配列変数dateへ
    TIME-=millis()/1000;                    // カウント済み内部タイマー値を減算
    lcd.clear();                            // LCDの表示文字の消去
    lcd.setCursor(0,1);                     // LCDの表示位置を2行目の先頭へ
    lcd.print("ntp/");                      // 「ntp/」を表示
    lcd.print(date+8);                      // dateの9文字目以降をLCDへ表示
    WiFi.disconnect(true);                  // WiFiアクセスポイントを切断する
    WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定する
}

void setup(){                               // 起動時に一度だけ実行する関数
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcdisp("IoT NTP esp32_24");             // タイトルをLCDへ表示
    wifi_ntp();                             // wifi_ntpを実行
}

void loop(){                                // 繰り返し実行する関数
    unsigned long time=millis();            // ミリ秒の取得
    if(time%250<100){                       // 250msに一回
        if(time%21600000ul<100){            // 6時間に1回
            wifi_ntp();                     // wifi_ntpを実行
        }
        time2txt(date,TIME+time/1000);      // 時刻を文字配列変数dateへ代入
        lcd.setCursor(1,0);                 // LCDの表示位置を1行目の2文字目へ
        lcd.print(date+5);                  // dateの6文字目以降をLCDへ表示
    }
    delay(100);                             // 待ち時間処理
}
