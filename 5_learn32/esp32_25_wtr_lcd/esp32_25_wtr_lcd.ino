/*******************************************************************************
Practice esp32 25 wtr lcd
                                                Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

#define WTR_PREF 27                         // 県番号：東京=13,福島=7,愛知=23
                                            // 大阪=27,京都=26,兵庫=28,熊本=43
#define WTR_FINE  3                         // 晴れ
#define WTR_CLOWD 2                         // くもり
#define WTR_RAIN  1                         // 雨

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
// LiquidCrystal lcd(12,13,17,16,27,14);    // ESPduino 32 WEMOS D1 32用

void wifi_weather(){                        // Wi-Fi接続とHTTPアクセスの実行関数
    WiFi.mode(WIFI_STA);                    // 無線LANを【子機】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    char s[17];                             // 文字配列変数sの定義
    int t=20;                               // AP接続待ち時間
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(!(--t)) break;                   // tから1を減算し、0ならwhileを終了
        delay(500);                         // 待ち時間処理(LED点滅用)
        lcd.print('.');                     // 接続用プログレス表示
    }
    if(WiFi.status() == WL_CONNECTED){      // Wi-Fiへ接続していた時
        if(httpGetWeather(WTR_PREF,s,16)){  // 天気情報をインターネットから取得
            lcdisp(s,1);                    // 受信文字列を2行目に表示
        }
    }
    WiFi.disconnect(true);                  // WiFiアクセスポイントを切断する
    WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定する
}

void setup(){                               // 起動時に一度だけ実行する関数
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcdisp("IoT ﾃﾝｷ esp32_25");             // タイトルをLCDへ表示
//  Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    wifi_weather();                         // wifi_weatherを実行
}

void loop(){                                // 繰り返し実行する関数
    delay(1000);                            // 待ち時間処理
    unsigned long time=millis();            // ミリ秒の取得
    if(time%10000>=1000) return;            // 10秒に一回だけ以下を実行
    if(time%3600000ul<1000) wifi_weather(); // 1時間に1回だけ天気を取得
    lcd.setCursor(0,0);                     // カーソル位置を液晶の左上へ
    switch(httpGetBufferedWeatherCode()){   // 天気予報に応じた処理
        case WTR_FINE:                      // 晴れの時
            lcdisp("ﾃﾝｷ ﾊ ﾊﾚ   (^o^)");
            break;
        case WTR_CLOWD:                     // 曇りの時
            lcdisp("ｷｮｳ ﾊ ｸﾓﾘ  (^_^;");
            break;
        case WTR_RAIN:                      // 雨の時
            lcdisp("ｱﾒ ｶﾞ ﾌﾘﾏｽ (T_T)");
            break;
        default:                            // その他の時
            lcdisp("ﾌﾒｲ ﾉ ﾃﾝｷ  (@_@)");
            break;
    }
}
