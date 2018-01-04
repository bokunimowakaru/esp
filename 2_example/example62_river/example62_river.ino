/*******************************************************************************
Example 62 河川の水位情報をLCDへ表示する River Water Lev

主要機能：
    ・国土交通省「川の防災情報」から水位を取得し、液晶シールド(LCD Keypad)へ表示
    ・水位が急に増加したらチャイムでお知らせ
    ・最高気温と最低気温を棒グラフで表示
    ・現在時刻はNTPで取得
    ・取得した水位情報をSDカード(またはSPIFFS)へ保存
    ・ブラウザでアクセスすると保存した水位情報をダウンロードできる

地域設定：
    ・#define RIVER_SPOT_ID に観測所コードを設定してください。
    ・コードは川の防災情報の河川・観測所検索から検索してください。
            https://www.river.go.jp/kawabou/schObsrv.do

ご注意：
    ・国土交通省・川の防災情報はブラウザで閲覧することを前提に公開されています。
    ・実運用を行う場合は、一般財団法人・河川情報センターが配信する水防災オープン
    　データ提供サービスなどの数値データを利用したシステムへ修正してください。
    ・ブラウザでの閲覧よりもサーバへ負担がかかる使い方は控える必要があります。
            https://www.river.go.jp/kawabou/qa/QA/chu-i3.html
    ・台風の時に近郊の河川の情報を10分毎にアクセスするような使い方については、
      ぎりぎり通常のブラウザでの閲覧の範囲に入ると解釈していますが、可能な限り
      アクセス頻度、取得情報量を控えてください。

必要なハードウェア
    ・トランジスタ技術 IoT Express (CQ出版社)
    ・LCD Keypad Shield (DF Robot製、SainSmart製、D1 ROBOT製など)
    ・ACアダプタ（マイクロUSB、5V 500mA）
    
ボード設定：
    ・CQ出版社 IoT Express用です（初期値）
        - AE-FT234Xの改造もしくは、コンデンサの容量を増量してください。
            https://blogs.yahoo.co.jp/bokunimowakaru/55924799.html
            http://toragi.cqpub.co.jp/tabid/848/Default.aspx#1
    ・ESPduino 32 や WEMOS D1 32で使用する場合、#define CQ_PUB_IOT_EXPRESSを削除

                                                Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/

#define CQ_PUB_IOT_EXPRESS                  // CQ出版 IoT Express 用

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ

#ifdef CQ_PUB_IOT_EXPRESS
    #include <SD.h>                         // CQ出版 IoT Express 用の設定
    #define SD_CARD_EN                      // SDカードを使用する
    #define PIN_BUZZER 12                   // GPIO 12にスピーカを接続
#else
    #include <SPIFFS.h>                     // ESPduino 32 WEMOS D1 32用の設定
    #define PIN_BUZZER 18                   // GPIO 18にスピーカを接続
#endif
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define TIMEOUT 6000                        // タイムアウト 6秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define FILENAME "/river.csv"               // 出力データ用ファイル名

#define RIVER_SPOT_ID "2206800400013"       // 2206800400013 = 淀川
#define RIVER_DEPTH_MAX     5.0             // 警報
#define RIVER_DEPTH_ALERT   4.4             // 注意喚起

WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
#ifdef CQ_PUB_IOT_EXPRESS 
    LiquidCrystal lcd(17,26,13,14,15,16);   // CQ出版 IoT Express 用 LCD開始
#else
    LiquidCrystal lcd(12,13,17,16,27,14);   // ESPduino 32 WEMOS D1 32用 LCD開始
#endif
unsigned long TIME=0;                       // 1970年からmillis()＝0までの秒数
char date[20]="2000/01/01,00:00:00";        // 日時保持用
char lcdisp[17]="";                         // LCD表示用
float depth=-1.0;                           // 水位(負は未取得)
int chime=0;                                // チャイムOFF

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcd_bar_init(); lcd.clear();            // 棒グラフ表示用スケッチの初期化
    lcd.print("River Water Lev.");          // タイトルをLCDに表示する
    Serial.println("Example 62 River");
    unsigned long start_ms=millis();        // 初期化開始時のタイマー値を保存
    unsigned long wait_ms=20000;            // 起動待ち時間(ms)
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    #ifdef SD_CARD_EN
        if(!SD.begin()){                    // ファイルシステムの開始
            lcd.print("ERROR: NO SD Card"); Serial.println("ERROR: NO SD");
    #else
        if(!SPIFFS.begin()){                // ファイルシステムの開始
            lcd.print("ERROR: NO SPIFFS"); Serial.println("ERROR: NO SPIFFS");
    #endif
        delay(3000);                        // エラー表示用の待ち時間
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
            lcd.clear(); lcd.print("To "); lcd.print(SSID); lcd.setCursor(0,1);
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
        i=(int)( depth * 10. / RIVER_DEPTH_ALERT );
        lcd.setCursor(10,0); lcd_bar_print_onlyBar(i); len=strlen(lcdisp);
        lcd.setCursor(0,1);lcd.print(lcdisp);for(i=len;i<16;i++)lcd.print(' ');
        if(time%86400000ul==0){             // 24時間に1回
            TIME=getNtp();                  // NTP時刻を取得
            TIME-=millis()/1000;
        }
        delay(1);                           // 重複作動を回避するための待ち時間

        if(chime){                          // チャイムの有無
            chime=chimeBells(PIN_BUZZER,chime); // チャイム音を鳴らす
        }
        
        // 1分ごと(毎時毎分5秒)の処理
        if(depth >= 0. && strncmp(&date[17],"05",2)) return;
        if(time%1000) return;               // 重複作動を回避するための待ち時間
        httpGetBufferedDepth(lcdisp,16);
        
        // 10分ごと(毎時x3分5秒)の処理 10分間隔
        if(depth >= 0. && date[15] != '3' ) return;
        if(httpGetRiverDepth(RIVER_SPOT_ID) < 0) return;
        depth = httpGetBufferedDepth();
        httpGetBufferedDepth(lcdisp,16);
        if( depth > RIVER_DEPTH_ALERT ) chime = 2;
        if( depth > RIVER_DEPTH_MAX ) chime = 20;
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
        file.println(lcdisp);               // 受信データをファイル出力
        file.close();                       // ファイルを閉じる
        return;                             // loop()の先頭に戻る
    }
    Serial.print(date); Serial.print(", TCP from ");
    Serial.println(client.remoteIP());      // 接続元IPアドレスをシリアル表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(len>12 && strncmp(s,"GET /?FORMAT",12)==0){
                    #ifndef SD_CARD_EN      // SDはフォーマットできない
                        SPIFFS.format();    // ファイル全消去
                    #else
                        SD.remove(FILENAME);// ファイル消去
                    #endif
                    strcpy(lcdisp,"FORMAT ");
                    break;                  // 解析処理の終了
                }else if(len>11 && strncmp(s,"GET /?DEPTH",11)==0){
                    depth=-1;
                    strcpy(lcdisp,"GET DEPTH");
                    break;                  // 解析処理の終了
                }else if (len>6 && strncmp(s,"GET / ",6)==0){
                    len=0;
                    break;                  // 解析処理の終了
                }else if (len>6 && strncmp(s,"GET /",5)==0){
                    for(i=5;i<strlen(s);i++){  // 文字列を検索
                        if(s[i]==' '||s[i]=='&'||s[i]=='+'){        
                            s[i]='\0';      // 区切り文字時に終端する
                        }
                    }
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
                        client.println("HTTP/1.0 404 Not Found");
                        client.println("Connection: close");
                        client.println();
                        client.println("<HTML>404 Not Found</HTML>");
                        break;
                    } // delay(1);
                    client.println("HTTP/1.0 200 OK");
                    client.print("Content-Type: ");
                    if(strstr(&s[5],".jpg")) client.println("image/jpeg");
                    else if(strstr(&s[5],".csv")) client.println("text/csv");
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
        html(client,date,lcdisp,client.localIP()); // HTMLコンテンツを出力する
        time2txt(date,TIME+time/1000);
        Serial.print(date); Serial.println(", Done.");
    }
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
}
