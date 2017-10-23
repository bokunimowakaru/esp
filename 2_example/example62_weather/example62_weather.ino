/*******************************************************************************
Example 62 天気情報をLCDへ表示する

主要機能：
・Yahoo!天気・災害から天気情報を取得し、液晶シールド(LCD Keypad Shield)へ表示
・予報が雨や曇りに変わったらチャイムでお知らせ（雨＝ピンポン、曇り＝ブーン）
・最高気温と最低気温を棒グラフで表示
・現在時刻はNTPで取得
・取得した天気予報情報をSDカード(またはSPIFFS)へ保存
・ブラウザでアクセスすると保存した天気予報情報をダウンロードできる

地域設定：
・「#define WEATHER_PREF_ID」 にYahoo!天気・災害の地域コードを設定してください。

ボード設定：
・CQ出版社 IoT Express用です（初期値）
・ESPduino 32 や WEMOS D1 32で使用する場合は、#define CQ_PUB_IOT_EXPRESSを削除

                                                Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/

#define CQ_PUB_IOT_EXPRESS                  // CQ出版 IoT Express 用

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#include "pitches.h"

#ifdef CQ_PUB_IOT_EXPRESS           // CQ出版 IoT Express 用
    #include <SD.h>
    #define SD_CARD_EN                      // SDカードを使用する
    #define PIN_BUZZER 12                   // GPIO 12にスピーカを接続
#else                               // ESPduino 32 WEMOS D1 32用
    #include <SPIFFS.h>
    #define PIN_BUZZER 18                   // GPIO 18にスピーカを接続
#endif
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define TIMEOUT 6000                        // タイムアウト 6秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート
#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト
#define FILENAME "/weather.csv"             // 出力データ用ファイル名

#define WEATHER_PREF_ID 27                  // 県番号：東京=13,福島=7,愛知=23
                                            // 大阪=27,京都=26,兵庫=28,熊本=43
#define WEATHER_FINE  3                     // 晴れ
#define WEATHER_CLOWD 2                     // くもり
#define WEATHER_RAIN  1                     // 雨
#define WEATHER_ALL   0                     // 全受信データをファイルへ出力

byte packetBuffer[NTP_PACKET_SIZE];         // NTP送受信用バッファ
WiFiUDP udp;                                // NTP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
#ifdef CQ_PUB_IOT_EXPRESS 
    LiquidCrystal lcd(17,26,13,14,15,16);   // CQ出版 IoT Express 用 LCD開始
#else
    LiquidCrystal lcd(12,13,17,16,27,14);   // ESPduino 32 WEMOS D1 32用 LCD開始
#endif
unsigned long TIME=0;                       // 1970年からmillis()＝0までの秒数
char date[20]="2000/01/01,00:00:00";        // 日時保持用
char lcdisp[17]="";                         // LCD表示用
int weather=0;                              // 天気コード
int chime=0;                                // チャイムOFF

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcd_bar_init(); lcd.clear();            // 棒グラフ表示用スケッチの初期化
    lcd.print("eg. 62 Weather");            // 「ｻﾝﾌﾟﾙ 62」をLCDに表示する
    Serial.println("Example 62 Weather");
    unsigned long start_ms=millis();        // 初期化開始時のタイマー値を保存
    unsigned long wait_ms=20000;            // 起動待ち時間(ms)
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
#ifdef SD_CARD_EN
    if(!SD.begin()){                        // ファイルシステムの開始
        lcd.print("ERROR: NO SD Card"); Serial.println("ERROR: NO SD");
#else
    if(!SPIFFS.begin()){                    // ファイルシステムの開始
        lcd.print("ERROR: NO SPIFFS"); Serial.println("ERROR: NO SPIFFS");
#endif
        delay(3000);
    }
    WiFi.mode(WIFI_STA);                    // 無線LANを【STA】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(millis() - start_ms < wait_ms){  // 待ち時間後の処理
            lcd.print(".");                 // 接続進捗を表示
            digitalWrite(PIN_LED,!digitalRead(PIN_LED));        // LEDの点滅
            ledcWriteNote(0,NOTE_B,7);delay(50);ledcWrite(0,0); // ブザー音
            delay(450);                     // 待ち時間処理
        }else{
            lcd.clear(); lcd.print("No "); lcd.print(SSID); lcd.setCursor(0,1);
            WiFi.disconnect(); delay(3000); // WiFiアクセスポイントを切断する
            WiFi.begin(SSID,PASS);          // 無線LANアクセスポイントへ再接続
            wait_ms += 20000;               // 待ち時間に20秒追加
        }
    }
    lcd.clear();
    lcd.print("Wi-Fi STAation  ");          // STAモードであることを表示
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    lcd.print(WiFi.localIP());              // IPアドレスを液晶の2行目に表示
    Serial.println(WiFi.localIP());         // IPアドレスをシリアル出力表示
    udp.begin(NTP_PORT);                    // NTP待ち受け開始(STA側)
    TIME=getNtp();                          // NTP時刻を取得
    TIME-=millis()/1000;                    // カウント済み内部タイマー事前考慮
    delay(1000);                            // 表示内容の確認待ち時間
    server.begin();                         // Wi-Fiサーバを起動する
    lcd.clear();
}

void loop(){                                // 繰り返し実行する関数
    File file;
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,t=0;                              // 整数型変数i,tを定義
    char c;                                 // 文字変数cを定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列長を示す整数型変数を定義
    unsigned long time=millis();            // ミリ秒の取得

    client = server.available();            // 接続されたTCPクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
    
        // 250msに一回、液晶表示を更新する
        if(time%250) return;                // 250msで割り切れないときはretrun
        time2txt(date,TIME+time/1000); lcd.setCursor(0,0); lcd.print(&date[11]);
        if(time%500) i=httpGetBufferedTempL(); else i=httpGetBufferedTempH();
        i/=3; lcd.setCursor(10,0); lcd_bar_print_onlyBar(i); len=strlen(lcdisp);
        if(len>0){
            lcd.setCursor(0,1); lcd.print(lcdisp);
            for(i=len;i<16;i++) lcd.print(' ');
        }
        if(time%86400000ul==0){             // 24時間に1回
            TIME=getNtp();                  // NTP時刻を取得
            TIME-=millis()/1000;
        }
        delay(1);                           // 重複防止

        if(chime){                          // チャイムの有無
            chime=chimeBells(PIN_BUZZER,chime); // チャイム音を鳴らす
        }
        
        // 1分ごと(毎時毎分5秒)の処理
        if(len>0 && strncmp(&date[17],"05",2)) return;
        len=httpGetBufferedWeather(lcdisp,16);
        
        // 1時間ごと(毎時5分5秒)の処理
        if(len>0 && strncmp(&date[14],"05",2)) return;
        if(!httpGetWeather(WEATHER_PREF_ID,s,64,WEATHER_ALL)) return;
        if(weather != httpGetBufferedWeatherCode()){
            weather = httpGetBufferedWeatherCode();
            chime = 3 - weather;
        }
        #ifdef SD_CARD_EN
            file=SD.open(FILENAME,"a");     // 追記保存のためにファイルを開く
        #else
            file=SPIFFS.open(FILENAME,"a"); // 追記保存のためにファイルを開く
        #endif
        if(file==0){
            Serial.println("ERROR: FALIED TO OPEN. Please format storage.");
            return;                         // ファイルを開けれなければ戻る
        }
        file.print(date);                   // 日時を出力する
        file.print(',');                    // 「,」カンマをファイル出力
        file.println(s);                    // 受信データをファイル出力
        file.close();                       // ファイルを閉じる
        return;                             // loop()の先頭に戻る
    }
    lcd.clear();lcd.print("TCP from ");     // 接続されたことを表示
    Serial.print(date); Serial.print(", TCP from ");
    lcd_cls(1);lcd.print(client.remoteIP());// 接続元IPアドレスをLCD表示
    Serial.println(client.remoteIP());      // 接続元IPアドレスをシリアル表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(len>12 && strncmp(s,"GET /?FORMAT",12)==0){
                    #ifndef SD_CARD_EN          // SDはフォーマットできない
                        SPIFFS.format();        // ファイル全消去
                        strcpy(lcdisp,"FORMAT ");
                    #endif
                    break;                      // 解析処理の終了
                }else if (len>6 && strncmp(s,"GET / ",6)==0){
                    len=0;
                    break;                      // 解析処理の終了
                }else if (len>6 && strncmp(s,"GET /",5)==0){
                    for(i=5;i<strlen(s);i++){  // 文字列を検索
                        if(s[i]==' '||s[i]=='&'||s[i]=='+'){        
                            s[i]='\0';         // 区切り文字時に終端する
                        }
                    }
                    strncpy(lcdisp,&s[5],16);
                    Serial.print(date); Serial.print(", GetFile: ");
                    Serial.println(&s[5]);
                    #ifdef SD_CARD_EN
                        file = SD.open(&s[4],"r");      // 読取ファイル開く
                    #else
                        file = SPIFFS.open(&s[4],"r");  // 読取ファイル開く
                    #endif
                    if(file==0){                        // 開けなかった時
                        Serial.print(date); Serial.print(", no data: ");
                        Serial.println(&s[4]);          // ファイル無し表示
                        client.println("HTTP/1.1 404 Not Found");
                        client.println("Connection: close");
                        client.println();
                        client.println("<HTML>404 Not Found</HTML>");
                        break;
                    } // delay(1);
                    client.println("HTTP/1.1 200 OK");
                    client.print("Content-Type: ");
                    if(strstr(&s[5],".jpg")) client.println("image/jpeg");
                    else client.println("text/plain");
                    client.println("Connection: close");
                    client.println();
                    t=0; while(file.available()){   // データ残確認
                        s[t]=file.read(); t++;      // ファイルの読込み
                        if(t >= 64){                // 転送処理
                            if(!client.connected()) break;
                            client.write((byte*)s,64);
                            t=0; delay(1);
                        }
                    }
                    if(t>0&&client.connected())client.write((byte*)s,t);
                    client.flush();     // ESP32用 ERR_CONNECTION_RESET対策
                    file.close();       // ファイルを閉じる
                    client.stop();      // クライアント切断
                    return;
                }
                len=0;                      // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                   // 文字列変数に文字cを追加
                len++;                      // 変数lenに1を加算
                s[len]='\0';                // 文字列を終端
                if(len>=64) len=63;         // 文字列変数の上限
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT){                      // TIMEOUTに到達したらwhileを抜ける
            len=0; break;
        }else delay(1);
    }
    delay(10);                              // クライアント側の応答待ち時間
    if(client.connected()){                 // 当該クライアントの接続状態を確認
    	httpGetBufferedWeather(s,64,0);     // 取得した天気データを変数sへ読込む
        html(client,date,s,client.localIP()); // HTMLコンテンツを出力する
        time2txt(date,TIME+time/1000);
        Serial.print(date); Serial.println(", Done.");
    }
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
}
