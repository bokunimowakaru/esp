/*******************************************************************************
esp32 27 goo lcd3

Google カレンダー(予定表) から予定を取得する

クラウド（Google Apps Script）で実行したスクリプトを本機で受け取ります。
予め本フォルダ内のGoogleCalendar.jsを https://script.google.com/ へ転送し、
ウェブアプリケーションを公開しておく必要があります。

                                           Copyright (c) 2017-2018 Wataru KUNINO
********************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#include "HTTPSRedirect.h"                  // リダイレクト接続用ライブラリ

#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define GScriptId "★ここにGoogle Apps Scriptのトークンを記入してください★"

#define PIN_BUZZER 12                       // GPIO 12にスピーカを接続
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define TIMEOUT 7000                        // タイムアウト 7秒
#define SLEEP_P 10*60*1000000               // スリープ時間 10分(uint32_t)
#define HTTPTO "script.google.com"          // HTTPSアクセス先
#define HTRED "script.googleusercontent.com"// HTTPSリダイレクト先
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
#define DEVICE "alarm_1,"                   // 子機デバイス名(5文字+"_"+ID+",")

#define DEV_AC_ON  "" 						// ACリレーをONにするときのコマンド
#define DEV_AC_OFF ""						// ACリレーをOFFにするときのコマンド

/*	WiFiｺﾝｼｪﾙｼﾞｪ電源担当(ACリレー)を使用する場合は以下のように設定してください。
	（IPアドレスは、WiFiｺﾝｼｪﾙｼﾞｪ電源担当のアドレスに書き換える）
	
#define DEV_AC_ON  "192.168.0.155/?RELAY=1" // ACリレーをONにするときのコマンド
#define DEV_AC_OFF "192.168.0.155/?RELAY=0" // ACリレーをOFFにするときのコマンド
*/

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
String url = String("/macros/s/") + String(GScriptId) + "/exec";
unsigned long TIME=0;                       // 1970年からmillis()＝0までの秒数
unsigned long TIME_WOFF=0;                  // 無線を切断する時刻
char date[20]="2000/01/01,00:00:00";        // 日時保持用
char buf[8][17];                            // データ保持用バッファ8件分
int buf_n=0;                                // データ保持件数
int buf_i=0;                                // 表示中データ番号の保持
int chime=0;                                // チャイム音の鳴音回数

int wifi_on(){                              // 無線LANの接続
    TIME_WOFF = millis() + 70000;           // 70秒後のタイマー値をTIME_WOFFへ
    if(WiFi.status()==WL_CONNECTED)return 1;// 既に接続状態なら何もせずに戻る
    unsigned long time=millis();            // 初期化開始時のタイマー値を保存
    lcdisp("Google ｶﾚﾝﾀﾞ LCD");             // タイトルをLCDに表示する
    WiFi.mode(WIFI_STA);                    // 無線LANを【STA】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(millis()-time > TIMEOUT){        // 7秒経過時
            lcdisp("AP ｾﾂｿﾞｸ ｼｯﾊﾟｲ",1);     // Wi-Fi APへの接続を失敗した
            WiFi.disconnect(true);          // WiFiアクセスポイントを切断する
            WiFi.mode(WIFI_OFF);            // 無線LANをOFFに設定する
            return 0;                       // 終了
        }
        lcd.print(".");                     // 接続進捗を表示
        delay(500);                         // 待ち時間処理
    }
    lcdisp("Connected",1);                  // 接続成功表示
    return 1;
}

void wifi_off(){                            // 無線LANの切断
    if(WiFi.status()!=WL_CONNECTED) return; // 既に切断状態なら何もせずに戻る
    WiFi.disconnect(true);                  // WiFiアクセスポイントを切断する
    WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定する
    delay(10);                              // 処理完了待ち時間
}

void wifi_ntp(){                            // Wi-Fi接続とNTPアクセスの実行関数
    if(!wifi_on()) return;                  // 無線LANの接続
    TIME=getNtp();                          // NTP時刻を取得
    TIME-=millis()/1000;                    // カウント済み内部タイマー値を減算
}

void wifi_google(){                         // Googleカレンダから予定を取得する
    String data;                            // 受信データ
    String event;                           // イベント名
    char disp[49];                          // 液晶表示データ3バイト×16文字分
    int sp,ep;                              // 文字列位置用の変数
    int hour,min;                           // 時刻
    
    if(!wifi_on()) return;                  // 無線LANの接続
    HTTPSRedirect client(443);              // リダイレクト可能なHTTP接続client
    if(!client.connect(HTTPTO,443)) return; // 接続に失敗したら戻る
    data=client.getData(url,HTTPTO,HTRED);  // データ受信
    Serial.println(data);                   // 受信データをシリアルへ出力
    sp=data.indexOf("|Length,");            // 受信データから文字列Lengthを検索
    if(sp>=0) sp+=8; else return;           // 見つけた場合は、spに8を加算
    buf_n=data.substring(sp).toInt();       // 予定数(Length値)を保存
    if(buf_n>8) buf_n=8;                    // 最大件数8を超過していた時は8に
    lcd.clear();                            // 画面を消去
    memset(buf,0,8*17);                     // バッファメモリの消去
    for(int i=0; i<buf_n; i++){             // 予定回数の繰り返し
        sp=data.indexOf("|",sp+1)+1;        // 次の区切りの文字位置を変数spへ
        hour=data.substring(sp).toInt();    // 文字位置spの予定時間（時）を取得
        sp=data.indexOf(",",sp+1)+1;        // カンマの次の文字位置を変数spへ
        min=data.substring(sp).toInt();     // 文字位置spの予定時間（分）を取得
        sp=data.indexOf(",",sp+1)+1;        // カンマの次の文字位置を変数spへ
        ep=data.indexOf("|",sp+1);          // 次の区切り文字の位置を変数epへ
        event=data.substring(sp,ep);        // sp～epの範囲の文字列を変数eventへ
        memset(disp,0,48);                  // 文字配列変数dispの初期化
        snprintf(disp,7,"%02d:%02d,",hour,min); // dispへ時刻の文字を代入
        event.getBytes((byte*)disp+6,42);   // 時刻の後ろに変数eventの内容を代入
        lcdisp_utf_to_ank(disp);            // UTF8をLCD用文字ANKコードへ変換
        strncpy(buf[i],disp,16);            // LCD用文字データをバッファへコピー
    }
}

void wifi_udp(const char *dv,const char *s){// UDPで送信(dv:デバイス名,s:データ)
    if(!wifi_on()) return;                  // 無線LANの接続
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(dv);                          // デバイス名を送信
    udp.println(s);                         // 文字列を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
    udp.stop();                             // UDP通信の終了
}

void wifi_udp(const char *s){               // メッセージをUDPで送信
    wifi_udp(DEVICE,s);                     // デバイス名を送信
}

void wifi_talk(const char *s){              // 音声データをWi-Fi音声子機へ送信
    wifi_udp("atalk_0,$",s);                // デバイス名とブレーク信号を送信
}

void setup() {
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    wifi_talk("rokuji'kanni'naino/yoteio/osirasesimasu");
    wifi_ntp();                             // 時刻情報の取得
    wifi_google();                          // Googleカレンダから予定を取得する
    char s[33]; snprintf(s,33,"<NUMK VAL=%d COUNTER=ken>desu",buf_n);
    wifi_talk(s);                           // Wi-Fi音声子機「〇件です」を話す
}

void loop() {
    delay(99);                              // 待ち時間処理
    unsigned long time=millis();            // ミリ秒の取得
    if(time%21600000ul<100){                // 6時間に1回の処理
        wifi_ntp();                         // wifi_ntpを実行
    }
    if(time%(SLEEP_P/1000)<100){            // SLEEP_P間隔の処理
        wifi_google();                      // Googleカレンダから予定を取得する
        buf_i=0;                            // 表示用の予定番号をリセット
    }
    if(time%1000 > 100) return;             // 以下は1秒に一回の処理
    time2txt(date,TIME+time/1000);          // 時刻を文字配列変数dateへ代入
    int sec=atoi(date+17);                  // 時刻の中から秒を抽出
    chime=chimeBells(PIN_BUZZER,chime);     // chimeが0以外の時にチャイム鳴音
    if(time > TIME_WOFF) wifi_off();        // 70秒以上経過していたら無線LAN切断
    if(buf_n <= 1){                         // 予定件数が0件または1件の時
        lcdisp(date+5);                     // dateの6文字目以降をLCDへ表示
        if(buf_n==1){                       // 予定件数が1件の時
            lcdisp(buf[0],1);               // LCDの2行目に予定を表示
        }
    }else{
        buf_i++;
        if(buf_i >= buf_n) buf_i=1;         // 表示番号が上限に達したらリセット
        lcdisp(buf[0]);                     // 1行目に1件目の予定を表示
        lcdisp(buf[buf_i],1);               // 2行目に2件目以降の予定を表示
    }
    if(sec%20 != 0) return;                 // 以下は20秒に1回(1分間に3回実行)
    for(int i=0;i<buf_n;i++){               // 予定件数と同じ回数、繰り返す
        if(!strncmp(date+11,buf[i],5)){     // 予定の時刻と一致したとき
            if(!chime) chime = 2;                   // chimeの値が0のとき2に設定
            if(!strncmp(buf[i]+6,"LED=",4)){        // 予定の内容がLED制御の時
                int led = atoi(buf[i]+10) % 2;      // 「LED=」の数値を変数ledへ
                digitalWrite(PIN_LED,led);          // LEDの点灯または消灯
            }
            if(sec==0){								// 現在時刻の秒が0のとき
				wifi_udp(buf[i]);            		// 予定を送信
            	delay(1000);                 		// 音声完了待ち
	            if(!strncmp(buf[i]+6,"AC=",3)){     // 予定の内容がAC制御の時
	                int ac = atoi(buf[i]+9);   		// 「AC=」の数値を変数ledへ
	                if(ac) httpGet(DEV_AC_ON);
	                else httpGet(DEV_AC_OFF);
	            }
            }
        }
    }
}
