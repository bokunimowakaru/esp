/*******************************************************************************
Example 28: LCDへ表示する(HTTP版・時刻表示機能・NTP受信機能付き・棒グラフ機能)
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
/*

    ボタン操作　　　|内容
    ----------------|----------------------------------------------------
    SELECTボタン　　|IPアドレス表示、※長押しで棒グラフ表示モードへ切り換え
    　　　　　　　　|
    UP/DOWNボタン　 |過去のデータの履歴表示(時刻に「!」が表示される)
    　　　　　　　　|履歴表示を解除するにはSELECTボタン
    LEFT/RIGHTボタン|表示しきれない文字のスクロール表示
    
    保存したデータは、同じ無線LAN内のPCやスマホで本機IPアドレスへアクセスして
    ダウンロードすることが出来る
    
    起動時にアクセスポイントへ接続できなかった場合は、本機が無線アクセスポイント
    となって動作する。
    
    動作モード　　　|内容
    ----------------|----------------------------------------------------
    Wi-Fi STAation　|無線LANアクセスポイントへ接続して動作するモード
    Wi-Fi SoftwareAP|本機が無線アクセスポイントとなって動作するモード
*/
/*
ご注意
    ・リセットもしくは電源を切る前に、ファイル出力をOFFに設定してください
    ・ログが保存されない場合は、SPIFFSを初期化してください
    ・本機のアクセスポイントモード動作時はブロードキャストの受信が出来ません
    　- ESP8266で使用している IPスタックlwIP(lightweight IP)の仕様です
    　- 子機の送信側で本機のアドレス(192.168.0.1)を指定して送信してください
    　- 本機からのブロードキャスト送信は可能です
*/


#include <FS.h>
#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
#include <DNSServer.h>
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "LiquidCrystalDFR.h"

#define TIMEOUT 3000                        // タイムアウト 3秒
#define WIFI_AP_MODE 1                      // Wi-Fi APモード ※「0」でSTAモード
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define PORT 1024                           // 受信ポート番号
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート
#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト
#define HIST_MAX 16                         // 過去データ保持件数(1以上)
#define WAIT_MS 20000                       // 起動待ち時間(最大)

byte packetBuffer[NTP_PACKET_SIZE];         // NTP送受信用バッファ
WiFiUDP udp;                                // NTP通信用のインスタンスを定義
WiFiUDP udpRx;                              // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
LiquidCrystal lcd(14,15,4,5,12,13);         // ESPduino用 LCD開始
// LiquidCrystal lcd(0,2,4,14,12,13);       // WEMOS D1用 LCD開始
unsigned long TIME=0;                       // 1970年からmillis()＝0までの秒数
char lcd0[17]="00:00:00 LCD MON";           // LCD表示用(1行目)文字列変数 16字
char lcd1[65]="";                           // LCD表示用(2行目)文字列変数 64字
int lcd_p=0;                                // LCD表示位置
int hist_p=0;                               // 過去データ位置(インデックス)
int disp_p=-1;                              // 履歴表示位置,負は非表示
int disp_mode=0;                            // 表示モード(0:通常,1:棒グラフ)
uint32_t ip;                                // IPアドレス保持用
char date[20];                              // 日時保持用
int LOG_FILE_OUTPUT=1;                      // ファイル出力の有効(1)／無効(0)

struct HistData{                            // 履歴保持用 50 byte 17+32+1
    char lcd0[17];                          // LCD表示用(1行目)文字列変数 16字
    char lcd1[32];                          // LCD表示用(2行目)文字列変数 31字
    byte len1;                              // LCD表示用(2行目)文字列長
}hist[HIST_MAX];

void setup(){                               // 起動時に一度だけ実行する関数
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcd_bar_init(); lcd.clear();            // 棒グラフ表示用スケッチの初期化
    lcd.printKana("ｻﾝﾌﾟﾙ 28 LCD");          // 「ｻﾝﾌﾟﾙ 28」をLCDに表示する
    Serial.println("Example 28 LCD");
    unsigned long start_ms=millis();        // 初期化開始時のタイマー値を保存
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    if(!SPIFFS.begin()){                    // ファイルシステムの開始
        lcd.print("ERROR: NO SPIFFS"); Serial.println("ERROR: NO SPIFFS");
        delay(3000);
    }
    Dir dir = SPIFFS.openDir("/");          // ファイルシステムの確認
    if(dir.next()==0){
        lcd.print("ERROR: NO FILES"); Serial.println("ERROR: NO FILES");
        delay(3000);
    }
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モード設定
    WiFi.mode(WIFI_STA);                    // 無線LANを【STA】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        lcd.print(".");                     // 接続進捗を表示
        if(millis()-start_ms>WAIT_MS){      // 待ち時間後の処理
            WiFi.disconnect();              // WiFiアクセスポイントを切断する
            lcd.clear();
            lcd.print("No Internet AP");    // 接続が出来なかったときの表示
            break;
        }
    }
    Serial.print("Wi-Fi STA :");            // STAモード
    if(WiFi.status() == WL_CONNECTED){      // 接続に成功した時
        lcd.clear();
        lcd.print("Wi-Fi STAation  ");      // STAモードであることを表示
        lcd.setCursor(0,1);                 // カーソル位置を液晶の左下へ
        lcd.print(WiFi.localIP());          // IPアドレスを液晶の2行目に表示
        ip=WiFi.localIP();                  // IPアドレスを保持
        Serial.println(SSID);               // SSIDを表示
        Serial.println(WiFi.localIP());     // IPアドレスをシリアル表示
        Serial.println(WiFi.subnetMask());  // ネットマスクをシリアル表示
        Serial.println(WiFi.gatewayIP());   // ゲートウェイをシリアル表示
        udp.begin(NTP_PORT);                // NTP待ち受け開始(STA側)
        TIME=getNtp();                      // NTP時刻を取得
        TIME-=millis()/1000;                // カウント済み内部タイマー事前考慮
    }else{
        WiFi.mode(WIFI_AP);                 // 無線LANを【AP】モードに設定
        delay(1000);                        // 切換え・設定待ち時間
        lcd.clear();                        // APモード
        lcd.print("Wi-Fi SoftwareAP");      // APモードであることを表示
        Serial.println("OFF");              // STA接続に失敗したことを出力
        WiFi.softAPConfig(
            IPAddress(192,168,0,1),         // AP側の固定IPアドレス
            IPAddress(0,0,0,0),             // 本機のゲートウェイアドレス
            IPAddress(255,255,255,0)        // ネットマスク
        );
        WiFi.softAP(SSID_AP,PASS_AP);       // ソフトウェアAPの起動
        lcd.setCursor(0,1);                 // カーソル位置を液晶の左下へ
        lcd.print(WiFi.softAPIP());         // AP側IPアドレスを液晶の1行目に表示
        ip=WiFi.softAPIP();                 // IPアドレスを保持
        Serial.print("Wi-Fi Soft AP :");    // Soft APモードの情報表示
        Serial.println(SSID_AP);
        Serial.println(WiFi.softAPIP());    // AP側IPアドレスをシリアル表示
        Serial.println(WiFi.softAPmacAddress());// AP側MACアドレスをシリアル表示
    }
    delay(1000);                            // 表示内容の確認待ち時間
    server.begin();                         // Wi-Fiサーバを起動する
    udpRx.begin(PORT);                      // UDP待ち受け開始(STA側+AP側)
    memset(hist,0,HIST_MAX*50);             // 過去データの初期化
}

void loop(){                                // 繰り返し実行する関数
    File file;
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,t=0;                              // 整数型変数i,tを定義
    char c;                                 // 文字変数cを定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    char filename[13];                      // ファイル名格納用
    uint8_t buttons;                        // キーパッド用
    int len=0;                              // 文字列長を示す整数型変数を定義
    int postF=0;                            // POSTフラグ(0:未 1:POST 2:BODY)
    int postL=64;                           // POSTデータ長
    unsigned long time=millis();            // ミリ秒の取得

    client = server.available();            // 接続されたTCPクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
        if(time%250 < 1){                   // 250msに一回
		    if(time%86400000ul==0){         // 24時間に1回
		        TIME=getNtp();              // NTP時刻を取得
		        TIME-=millis()/1000;
		    }
		    time2txt(date,TIME+time/1000);  // 以下、表示用コンテンツ作成
            strcpy(lcd0,&date[11]); lcd0[8]=' '; lcd0[16]='\0';
            buttons=lcd.readButtons(); switch( buttons ){
                case BUTTON_SELECT: if(disp_p>0) disp_p=-1;
                    if(disp_p==-1){lcd_p=0; strcpy(&lcd0[9],"IP_ADDR"); sprintf(
                        lcd1,"%d.%d.%d.%d",ip&255,ip>>8&255,ip>>16&255,ip>>24);}
                    if(disp_p<=-9){ disp_mode=!disp_mode; disp_p=-4;
                        snprintf(&lcd0[9],8,"MODE=%d ",disp_mode);
                        if(disp_mode==0) strcpy(lcd1,"Raw Data ");
                        else strcpy(lcd1,"Bar Grph"); } disp_p--; break;
                case BUTTON_UP: lcd_p=0; if(disp_p<0) disp_p=hist_p;
                    disp_p--; if(disp_p<0) disp_p=HIST_MAX-1; break;
                case BUTTON_DOWN: lcd_p=0; if(disp_p<0) disp_p=hist_p;
                    disp_p++;if(disp_p>=HIST_MAX) disp_p=0; break;
                case BUTTON_LEFT: lcd_p-=1; if(lcd_p<0) lcd_p=0; break;
                case BUTTON_RIGHT: if(hist[disp_p].len1>lcd_p+7)lcd_p+=1;
                    break;
                default: if(disp_p<0) disp_p=-1; break;
            }
            if(disp_p >=0 && !hist[disp_p].lcd1[0]){ disp_p=-1; lcd_p=0; }
            if(disp_p >=0){                 // 履歴表示中
                lcd.setCursor(0,0); lcd.print(hist[disp_p].lcd0);
                lcd.setCursor(0,1); lcd.print(hist[disp_p].lcd1+lcd_p);
                for(int i=hist[disp_p].len1;i<16;i++) lcd.print(' ');
            }else if(disp_mode==0){         // 現在値表示中(通常モード)
                lcd.setCursor(0,0); lcd.print(lcd0);
                strncpy(s,lcd1+lcd_p,16); lcd.setCursor(0,1); lcd.print(s);
                len=strlen(lcd1); if(len>16) lcd_p++; if(len<=lcd_p)lcd_p=0;
                for(int i=len;i<16;i++) lcd.print(' ');
            }else if(disp_mode==1){         // 現在値表示中(棒グラフモード)
                if(TIME) lcd_bar_print(&lcd0[9],lcd1,lcd0);
                else lcd_bar_print(&lcd0[9],lcd1,"00:00");
                lcd.setCursor(0,1); lcd.print(&lcd0[9]); lcd.print(' ');
                strncpy(s,lcd1+lcd_p,8); lcd.setCursor(8,1); lcd.print(s);
                len=strlen(lcd1); for(int i=len-lcd_p;i<8;i++) lcd.print(' ');
                if(len>8) lcd_p++; if(len<=lcd_p)lcd_p=0;
            }
            delay(1);
        }
        len = udpRx.parsePacket();          // UDP受信パケット長を変数lenに代入
        if(len <= 8)return;                 // UDPが未受信時にloop()先頭へ
        memset(s, 0, 65);                   // 文字列変数sの初期化(65バイト)
        udpRx.read(s, 64);                  // UDP受信データを文字列変数sへ代入
        if(len>64){len=64; udpRx.flush();}  // 受信データが残っている場合に破棄
        for(i=0;i<len;i++) if( !isgraph(s[i]) ) s[i]=' ';   // 特殊文字除去
        strncpy(&lcd0[9],s,8); lcd0[16]=0;  // LCD表示用(1行目)に機器名を代入
        strcpy(hist[hist_p].lcd0,lcd0);     // 履歴保持
        hist[hist_p].lcd0[8]='!'; hist[hist_p].lcd0[16]='\0';
        strncpy(lcd1,&s[8],56); strncpy(hist[hist_p].lcd1,lcd1,32); lcd_p=0;
        hist[hist_p].len1=strlen(hist[hist_p].lcd1);
        hist_p++; if(hist_p>=HIST_MAX) hist_p=0;
        if(strncmp(s,"timer_",6)==0 && strlen(s)>=27){
            TIME=atoi(&s[16]); TIME*=24;    TIME+=atoi(&s[19]); TIME*=60;
            TIME+=atoi(&s[22]); TIME*=60;   TIME+=atoi(&s[25]); 
            TIME-=millis()/1000;            // 012345678901234567890123456
        }                                   // timer_1,2016,10,11,18,46,36
        if(!LOG_FILE_OUTPUT) return;
        if(s[5]!='_' || s[7]!=',')return;   // 6文字目「_」8文字目「,」を確認
        for(i=0;i<7;i++) if(!isalnum(s[i])) s[i]='_';
        s[7]='\0';s[len]='\0';              // 8番目の文字を文字列の終端に設定
        sprintf(filename,"/%s.txt",s);
        file = SPIFFS.open(filename,"a");   // 追記保存のためにファイルを開く
        if(file==0){
            Serial.println("ERROR: FALIED TO OPEN. Please format SPIFFS.");
            return;                  // ファイルを開けれなければ戻る
        }
        file.print(date);                   // 日時を出力する
        file.print(',');                    // 「,」カンマをファイル出力
        file.println(&s[8]);                // 受信データをファイル出力
        file.close();                       // ファイルを閉じる
        return;                             // loop()の先頭に戻る
    }
    lcd.clear();lcd.print("TCP from ");     // 接続されたことを表示
    Serial.print(date); Serial.print(", TCP from ");
    if(client.localIP()==ip) lcd.print("STAtion");
    else lcd.print("Soft AP");
    lcd_cls(1);lcd.print(client.remoteIP());// 接続元IPアドレスをLCD表示
    Serial.println(client.remoteIP());      // 接続元IPアドレスをシリアル表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(postF==0){               // ヘッダ処理
                    if(len>11 && strncmp(s,"GET /?TEXT=",11)==0){
                        strncpy(lcd1,&s[11],64);    // 受信文字列をlcd1へコピー
                        trUri2txt(lcd1);            // URLエンコードの変換処理
                        utf_del_uni(lcd1);          // 半角カタカナ文字処理
                        if(strlen(lcd1)>16) lcd1[16]='\0';
                        strcpy(&lcd0[9],"Message");
                        len=strlen(lcd1);
                        break;                      // 解析処理の終了
                    }else if(len>11 && strncmp(s,"GET /?START",11)==0){
                        strcpy(&lcd0[9],"LogSTART");
                        strcpy(lcd1,"START Logging");
                        LOG_FILE_OUTPUT=1;
                        len=strlen(lcd1);
                        break;                      // 解析処理の終了
                    }else if(len>10 && strncmp(s,"GET /?STOP",10)==0){
                        strcpy(&lcd0[9],"LogSTOP");
                        strcpy(lcd1,"STOP Logging");
                        LOG_FILE_OUTPUT=0;
                        len=strlen(lcd1);
                        break;                      // 解析処理の終了
                    }else if(len>12 && strncmp(s,"GET /?FORMAT",12)==0){
                        SPIFFS.format();            // ファイル全消去
                        strcpy(&lcd0[9],"FORMAT ");
                        strcpy(lcd1,"FORMAT SD Card or SPIFFS");
                        len=strlen(lcd1);
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
                        strcpy(&lcd0[9],"GetFile"); strcpy(lcd1,&s[5]);
                        Serial.print(date); Serial.print(", GetFile: ");
                        Serial.println(&s[5]);
                        delay(1);
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/plain");
                        client.println("Connection: close");
                        client.println();
                        file = SPIFFS.open(&s[4],"r");      // 読取ファイル開く
                        i=0;                                // 変数iに0を代入
                        if(file==0){                        // 開けなかった時
                            Serial.print(date); Serial.print(", no data: ");
                            Serial.println(&s[4]);
                            client.println("no data");      // ファイル無し表示
                        }else{  // 以下でt,sを再使用        // ファイル開けた時
                            t=0; while(file.available()){   // データ残確認
                                s[t]=file.read(); t++;      // ファイルの読込み
                                if(t >= 64){                // 転送処理
                                    if(!client.connected()) break;
                                    client.write((byte*)s,64);
                                    t=0; delay(1);
                                }
                            }
                            if(t>0&&client.connected())client.write((byte*)s,t);
                            file.close();                   // ファイルを閉じる
                        }
                        client.stop();                      // クライアント切断
                        return;
                    }else if(len>6 && strncmp(s,"POST /",6)==0){
                        postF=1;            // POSTのBODY待ち状態へ
                    }
                }else if(postF==1){
                    if(len>16 && strncmp(s,"Content-Length: ",16)==0){
                        postL=atoi(&s[16]); // 変数postLにデータ値を代入
                    }
                }
                if( len==0 ) postF++;       // ヘッダの終了
                len=0;                      // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                   // 文字列変数に文字cを追加
                len++;                      // 変数lenに1を加算
                s[len]='\0';                // 文字列を終端
                if(len>=64) len=63;         // 文字列変数の上限
            }
            if(postF>=2){                   // POSTのBODY処理
                if(postL<=0){               // 受信完了時
                    if(len>5 && strncmp(s,"TEXT=",5)==0){
                        strncpy(lcd1,&s[5],64);     // 受信文字列をlcd1へコピー
                        trUri2txt(lcd1);            // URLエンコードの変換処理
                        utf_del_uni(lcd1);          // 半角カタカナ文字処理
                        if(strlen(lcd1)>16) lcd1[16]='\0';
                        strcpy(&lcd0[9],"HT_POST");
                    }else len=0;
                    break;                  // 解析処理の終了
                }
                postL--;                    // 受信済POSTデータ長の減算
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT){                      // TIMEOUTに到達したらwhileを抜ける
            len=0; break;
        }else delay(1);
    }
    if(len){
        strcpy(hist[hist_p].lcd0,lcd0); 
        strncpy(hist[hist_p].lcd1,lcd1,32);
        hist[hist_p].len1=strlen(hist[hist_p].lcd1);
        hist_p++; if(hist_p>=HIST_MAX) hist_p=0;
    }
    delay(10);                              // クライアント側の応答待ち時間
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        html(client,date,lcd1,client.localIP()); // HTMLコンテンツを出力する
        time2txt(date,TIME+time/1000);
        Serial.print(date); Serial.println(", Done.");
    }
    client.stop();                          // クライアントの切断
}

unsigned long getNtp(){
    unsigned long highWord;                 // 時刻情報の上位2バイト用
    unsigned long lowWord;                  // 時刻情報の下位2バイト用
    unsigned long time;                     // 1970年1月1日からの経過秒数
    int waiting=0;                          // 待ち時間カウント用
    char s[20];                             // 表示用
    
    sendNTPpacket(NTP_SERVER);              // NTP取得パケットをサーバへ送信する
    while(udp.parsePacket()<44){
        delay(100);                         // 受信待ち
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 100) return 0ul;       // 100回(10秒)を過ぎたら戻る
    }
    udp.read(packetBuffer,NTP_PACKET_SIZE); // 受信パケットを変数packetBufferへ
    highWord=word(packetBuffer[40],packetBuffer[41]);   // 時刻情報の上位2バイト
    lowWord =word(packetBuffer[42],packetBuffer[43]);   // 時刻情報の下位2バイト
    time = highWord<<16 | lowWord;          // 時刻(1900年1月からの秒数)を代入
    time -= 2208988800UL;                   // 1970年と1900年の差分を減算
    time2txt(s,time);                       // 時刻をテキスト文字に変換
    time += 32400UL;                        // +9時間を加算
    time2txt(s,time);                       // 時刻をテキスト文字に変換
    return time;
}
