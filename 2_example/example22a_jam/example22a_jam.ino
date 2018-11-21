/*******************************************************************************
Example 22a: Wi-Fi コンシェルジェ マイコン担当 IchigoJam情報表示 i.My.espJam
IoTセンサ機器が送信するセンサ値情報を、テレビへ表示することが可能なIoT機器です。

Windows エクスプローラ（ファイル表示機能）へ以下を入力すると、本機内のファイルの
一覧を表示し、ファイルをデスクトップなどにコピーすることも可能です。

	ftp://cqpub:bokunimowakaru@192.168.0.XXX/
	（192.168.0.XXXの部分はLCDへ表示される本機のIPアドレス）

Arduino IDE Version 1.6.8以上を推奨（Version 1.6.5ではコンパイルできない）
esp8266 by esp8266 Community Version 2.3.0以上を推奨
IchigoJam Firmware 1.2以上を推奨

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                        // Wi-Fi機能を利用するために必要
#include <WiFiClient.h>
#include "ESP8266FtpServer.h"
extern "C" {
#include "user_interface.h"                     // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define TIMEOUT 20000                           // タイムアウト 20秒
char SSID[33]="1234ABCD";                       // 無線LANアクセスポイントのSSID
char PASS[33]="password";                       // パスワード
#define PORT 1024                               // 受信ポート番号
#define BUF_N 4096                              // バッファサイズ
#define NTP_SERVER "ntp.nict.jp"                // NTPサーバのURL
#define NTP_PORT 8888                           // NTP待ち受けポート
#define FTP_USER "cqpub"                        // FTPユーザ名
#define FTP_PASS "bokunimowakaru"               // FTPパスワード
WiFiUDP udp;                                    // UDP通信用のインスタンスを定義
WiFiServer server(80);                          // Wi-Fiサーバ(ポート80)定義
IPAddress ip(192,168,0,1);                      // APモード時のIPアドレス
int WiFiStat=0;                                 // ESPサーバ状態
char tx[BUF_N+1]="\0";                          // 送信バッファ
char mj[65]="\0";                               // MJコマンドの格納用
File file;
unsigned long TIME=0;                           // 1970年からmillis()＝0までの秒数
                                                // ※現在時刻は TIME+millis()/1000
FtpServer ftpSrv;

int connect(){
    int t=0;
    ssidpass_read();
    if(strcmp(SSID,"1234ABCD")==0){
        strcpy(tx,"'ERROR SSID\n");
        return -2;
    }
//  wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
//  ↑設定解除中 ∵SPIFFSとの組み合わせ動作不安定 esp8266 Ver.2.4.2 + LwIP v1.4
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS); delay(1000);         // 無線LANアクセスポイントへ接続
    Serial.print("'Connecting to ");            // 「Connecting」を出力する
    Serial.println(SSID);                       // SSIDを出力する
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        Serial.print("'");                      // 無線APへの接続プログレス表示
        t++;
        if(t>60){
            WiFi.disconnect();
            Serial.println(); delay(100);
            Serial.println("'ERROR AP"); delay(100);
            return -1;                          // タイムアウト時 -1
        }
    }
    ip=WiFi.localIP();
    Serial.println();                           // 改行を出力
    Serial.print("'");                          // コメント命令を送信
    Serial.println(ip); delay(100);             // IPアドレスを出力する
    WiFiStat=1;
    t=0;
    while(TIME==0){
        delay(500);                             // 待ち時間処理
        TIME=getNTP(NTP_SERVER,NTP_PORT);       // NTPを用いて時刻を取得
        tx[0]='\'';
        time2txt(&tx[1],TIME);                  // 日時をテキストに変換する
        strcat(tx,"\n");
        t++;
        if(t>30){
            Serial.println("'ERROR NTP"); delay(100);
            return -2;                          // タイムアウト時 -1
        }
    }
    TIME-=millis()/1000;                        // 起動後の経過時間を減算
    Serial.println("'NTP OK"); delay(100);      // NTP OK表示
    WiFiStat=2;
    server.begin();                             // TCPサーバを起動する
    Serial.println("'HTTP Serv."); delay(100);  // 表示
    WiFiStat=3;
    udp.begin(PORT);                            // UDP通信御開始
    Serial.println("'UDP listen"); delay(100);  // 表示
    WiFiStat=4;
    ftpSrv.begin(FTP_USER,FTP_PASS);            // username, password for ftp.
    Serial.println("'FTP Serv."); delay(100);   // 表示
    WiFiStat=5;
    return 0;
}

void disconnect(){
    if(WiFiStat==5){
        // ftpSrv.stop();
        WiFiStat--;
    }
    if(WiFiStat==4){
        udp.stop();
        WiFiStat--;
    }
    if(WiFiStat==3){
        server.close();
        WiFiStat--;
    }
    if(WiFiStat==2){
        WiFiStat--;
    }
    if(WiFiStat==1){
        WiFi.disconnect();
        WiFiStat--;
    }
}

void setup(){                                   // 起動時に一度だけ実行する関数
    Serial.begin(115200);                       // IchigoJamとの通信ポート
    delay(5000);                                // IchigoJamの起動・通信処理待ち
    Serial.write(25); Serial.write(16);         // 停止コマンドとDLEコードの送信
    Serial.println("cls:?\"ESP Wi-Fi");         // シリアル出力表示
    delay(500);                                 // シリアル出力の完了待ち
    while(!SPIFFS.begin())delay(1000);          // ファイルシステムの開始
    connect();                                  // SSIDが初期状態以外の時に接続
}

void loop(){                                    // 繰り返し実行する関数
    WiFiClient client;                          // Wi-Fiクライアントの定義
    char c;                                     // 文字変数cを定義
    char s[65];                                 // 文字列変数を定義
    char filename[13];                          // ファイル名格納用
    char date[20];                              // 日付データ格納用
    char com[65]="\0";                          // IchigoJamへの送信コマンド
    char rx[65]="起動しました";                 // IchigoJamからの応答(表示用)
    int i,j;
    int len=0;                                  // 文字列長を示す変数を定義
    int rxi=0;                                  // IchigoJamからの応答文字長
    int t=0;                                    // 待ち受け時間カウント用の変数
    int postF=0;                                // POSTフラグ(0:- 1:POST 2:BODY)
    int postL=0;                                // POSTデータ長
    unsigned long time=millis();                // ミリ秒の取得

    if(time<100){
        TIME=getNTP(NTP_SERVER,NTP_PORT);       // NTPを用いて時刻を取得
        if(TIME)TIME-=millis()/1000;            // 経過時間を減算
        while(millis()<100)delay(1);            // 100ms超過待ち
    }
    time2txt(date,TIME+time/1000);              // 日時をテキストに変換する
    if(Serial.available()){
        c=Serial.read();
        if(c=='\n'||c=='\r'){
            // MJコマンド
            if(strncmp(mj,"MJ APC ",7)==0){
                disconnect();
                ssidpass_write(&mj[7]);
                connect();
            }else if(strcmp(mj,"MJ IP")==0){
                Serial.print("'");              // コメント命令を送信
                Serial.println(ip); delay(100); // IPアドレスを出力する
            }else if(strcmp(mj,"MJ APC")==0){
                connect();
            }else if(strcmp(mj,"MJ APD")==0){
                disconnect();
            }else if(strcmp(mj,"MJ APS")==0){
                if(WiFiStat<1)strcpy(tx,"'0\n");
                else{
                    strcpy(tx,"'1 (-)\n");
                    tx[4]='0'+WiFiStat;
                }
            }else if(strcmp(mj,"MJ FORMAT")==0){
                SPIFFS.format();                // ファイル全消去
                ssidpass_init();                // 設定ファイル保存
                strcpy(tx,"'DONE FORMAT\n");    // シリアル出力表示
            }else if(strcmp(mj,"MJ DATE")==0){
                strcpy(tx,"'");
                strcat(tx,date);
                strcat(tx,"\n");
            }else if(strcmp(mj,"MJ FILES")==0){
                Dir dir = SPIFFS.openDir("/");
                tx[0]='\0';
                while(dir.next()){
                    strcat(tx,"'?\"MJ LOAD ");
                    String string=dir.fileName();
                    string.toCharArray(s,32);
                    if(strlen(s)>1) strcat(tx,&s[1]);
                    strcat(tx,"\n");
                    if( strlen(tx) > BUF_N - 64 ) break;
                }
                if(!strlen(tx))strcpy(tx,"'no files\n");
            }else if(strncmp(mj,"MJ LOAD ",8)==0){
                mj[7]='/';
                file = SPIFFS.open(&mj[7],"r");
                if( strcmp(&mj[strlen(mj)-4],".txt")==0 ||
                    strcmp(&mj[strlen(mj)-4],".TXT")==0 ||
                    strcmp(&mj[strlen(mj)-4],".bas")==0 ||
                    strcmp(&mj[strlen(mj)-4],".BAS")==0){
                    tx[0]='\0'; i=0; j=0;
                }else{
                    tx[0]='\''; i=1; j=1;
                }
                if(file){
                    while(file.available()){
                        c=file.read();
                        if(c=='\r') continue;
                        tx[i]=c;
                        i++;
                        tx[i]='\0';
                        if(i>=BUF_N) break;
                        if(c=='\n' && j ){
                            tx[i]='\'';
                            i++;
                            tx[i]='\0';
                            if(i>=BUF_N) break;
                        }
                    }
                    file.close();
                    if(i>0)if(tx[i-1]=='\'')tx[i-1]='\0';
                }else{
                    strcpy(tx,"'no File\n");
                }
            }else if(strncmp(mj,"MJ ",3)==0){   // HELP用：最後に記述する
                if(isupper((int)mj[3])){
                    tx[0]=0; //0123456789012345678901234567890
                    strcat(tx,"'ｺﾏﾝﾄﾞ ﾉ ﾂｶｲｶﾀ for i.My.espJam\n");
                    strcat(tx,"' ?\"MJ FORMAT : SPIFFS ｼｮｷｶ\n");
                    strcat(tx,"' ?\"MJ APC ssid pass : ｾﾂｿﾞｸ\n");
                    strcat(tx,"' ?\"MJ FILES : ﾌｧｲﾙ ﾉ ｶｸﾆﾝ\n");
                    utf_del_uni(tx); if(strlen(tx) > BUF_N-128) return;
                    strcat(tx,"' ?\"MJ LOAD file : ﾌｧｲﾙ ﾛｰﾄﾞ\n");
                    strcat(tx,"' ?\"MJ DATE : ｼﾞｺｸ ｦ ﾋｮｳｼﾞ\n");
                    strcat(tx,"' ?\"MJ IP : IP ｱﾄﾞﾚｽ ﾉ ｶｸﾆﾝ\n");
                    strcat(tx,"' ?\"MJ APD : ｾﾂﾀﾞﾝ\n");
                    utf_del_uni(tx); if(strlen(tx) > BUF_N-128) return;
                    strcat(tx,"' ?\"MJ APC : ｻｲ ｾﾂｿﾞｸ\n");
                    strcat(tx,"' ?\"MJ APS : ｾﾂｿﾞｸ ｼﾞｮｳﾀｲ\n");
                    utf_del_uni(tx);
                }
            }
            mj[0]='\0';
        }else if(c==' '){
            if( mj[strlen(mj)-1] !=' ' ) strcat(mj," ");
        }else if(isgraph(c)){
            s[0]=c; s[1]='\0';
            strcat(mj,s);
        }
        if( strlen(mj)>=63 ) mj[63]='\0';
    }
    ftpSrv.handleFTP();                         // make sure in loop you call
    client = server.available();                // TCPクライアントを生成
    if(client==0){                              // TCPクライアントが無かった場合
        if(tx[0]){                              // 変数txに代入されていた場合
            Serial.write(tx[0]); delay(18);     // IchigoJamへ出力
            if(tx[0]=='\n') delay(100);         // IchigoJamの処理待ち
            trShift(tx,1);                      // FIFOバッファのシフト処理
        }else{
            delay(11);
            len = udp.parsePacket();            // UDP受信パケット長をlenに代入
            if(len==0)return;                   // TCPとUDPが未受信時にloop()先頭へ
            memset(s, 0, 65);                   // 文字列変数sの初期化(65バイト)
            udp.read(s, 64);                    // UDP受信データを文字列変数sへ代入
            utf_del_uni(s);                     // UTF8の制御コードの除去
            file = SPIFFS.open("/log.csv","a"); // 追記保存のためにファイルを開く
            if(file==0)return;                  // ファイルを開けれなければ戻る
            for(i=0;i<64;i++) if(s[i]=='\r'||s[i]=='\n') s[i]='\0';
            strcpy(tx,"'");
            file.print(date);                   // 日時を出力する
            strcat(tx,date);
            file.print(',');                    // 「,」カンマをファイル出力
            strcat(tx,",");
            file.println(s);                    // 受信データをファイル出力
            strcat(tx,s);
            strcat(tx,"\n");
            file.close();                       // ファイルを閉じる
        }
        return;                                 // loop()の先頭に戻る
    }
    while(client.connected()){                  // クライアントの接続状態を確認
        if(client.available()){                 // クライアントからのデータ確認
            t=0;                                // 待ち時間変数をリセット
            c=client.read();                    // データを文字変数cに代入
            if(c=='\n'){                        // 改行を検出した時
                if(postF==0){                   // ヘッダ処理
                    if(len>10 && strncmp(s,"GET /?COM=",10)==0){
                        strncpy(com,&s[10],64); // 受信文字列をcomへコピー
                        break;                  // 解析処理の終了
                    }else if(len>12 && strncmp(s,"GET /?FORMAT",12)==0){
                        Serial.println("'FORMAT SPIFFS");   // シリアル出力表示
                        SPIFFS.format();                    // ファイル全消去
                        ssidpass_init();                    // 設定ファイル保存
                        s[5]='\0';                          // 取得ファイル名なし
                        break;
                    }else if (len>5 && strncmp(s,"GET /",5)==0){
                        break;                  // 解析処理の終了
                    }else if(len>6 && strncmp(s,"POST /",6)==0){
                        postF=1;                // POSTのBODY待ち状態へ
                    }
                }else if(postF==1){             // POSTのHEAD処理中のとき、
                    if(len>16 && strncmp(s,"Content-Length: ",16)==0){
                        postL=atoi(&s[16]);     // 変数postLにデータ値を代入
                    }
                }
                if( len==0 ) postF++;           // ヘッダの終了
                if( postF>=2) break;            // 解析処理の終了
                len=0;                          // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                       // 文字列変数に文字cを追加
                len++;                          // 変数lenに1を加算
                s[len]='\0';                    // 文字列を終端
                if(len>=64) len=63;             // 文字列変数の上限
            }
        }
        t++;                                    // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1);     // TIMEOUTしたらwhileを抜ける
    }
    delay(1);                                   // クライアント側の応答待ち時間
    while(Serial.available())Serial.read();     // シリアル受信バッファのクリア
    if(com[0]){                                 // コマンドあり
        trUri2txt(com);                         // URLエンコードの変換処理
        utf_del_uni(com);                       // UTF8の制御コードの除去
        Serial.write(16);                       // DLEコードの送信
        Serial.println(com);                    // 文字データを出力
        delay(200);
        rx[0]='\0';
        while(Serial.available() && rxi<64){
            c=Serial.read();
            if(c!='\r'){
                if(c=='\n'){
                    if(
                        (rxi>=2 && strncmp(&rx[rxi-2],"OK",2)==0) ||
                        (rxi>=5 && strncmp(&rx[rxi-5],"error",5)==0)
                    ) break;
                    if(rxi<=60){
                        strcpy(&rx[rxi],"<BR>");
                        rxi+=4;
                    }
                }else{
                    rx[rxi]=c; rxi++; rx[rxi]='\0';
                }
            }
        }
        if(!client.connected())return;
        html(client,"",rx,ip,date);             // HTMLコンテンツを出力する
    //  client.stop();                          // クライアントの切断
        return;
    }else if(postF>=2 && postL>4){
        strcpy(rx,"HTTP POST データ転送");
        com[4]='\0'; tx[0]='\0';                // 文字列変数の初期化
        for(i=0;i<4;i++) com[i]=client.read();
        if(strncmp(com,"PRG=",4)==0){           // プログラムモード時
            memset(tx, 0, BUF_N+1);             // 文字列変数txの初期化
            i=0; postL-=4; rxi=0;               // PCからの受信準備
            if(postL > BUF_N)postL=BUF_N;       // 送信バッファの上限を設定
            for(i=0;i<postL;i++){
                if(client.available()){
                    tx[i]=client.read();
                }else break;
            }
            trUri2txt(tx);
        }
        if(!client.connected())return;
        html(client,"",rx,ip,date);
    //  client.stop();                          // クライアントの切断
        Serial.write(16);                       // IchigoJamへDLEコードを送信
        return;
    }else if(strncmp(s,"GET / ",6)==0){         // コンテンツ要求があった時
        if(!client.connected())return;          // 切断された場合はloop()の先頭へ
        html(client,"",rx,ip,date);             // HTMLコンテンツを出力する
    //  client.stop();                          // クライアントの切断
        return;
    }else if(strncmp(s,"GET /",5)==0){          // コンテンツ要求時
        if(!client.connected())return;          // 切断された場合はloop()の先頭へ
        for(i=5;i<strlen(s);i++){                       // 文字列を検索
            if(s[i]==' '||s[i]=='&'||s[i]=='+'){        // 区切り文字のとき
                s[i]='\0';                              // 文字列を終端する
            }
        }
        client.println("HTTP/1.1 200 OK");              // HTTP OKを応答
        client.println("Content-Type: text/plain");     // テキスト・コンテンツ
        client.println("Connection: close");            // 応答終了後に切断
        client.println();
        if(s[5]=='_') client.println("protected file"); // 保護ファイル表示
        else{
            file = SPIFFS.open(&s[4],"r");      // データを読取ファイルを開く
            i=0;                                // 変数iに0を代入
            if(file==0){                        // ファイルが開けなかった時
                client.println("no data");      // ファイル無し表示
            }else{                              // ファイルが開けた時
                while(file.available()){        // データが残っているとき
                    client.write(file.read());  // ファイルを読み取って転送
                    i++;                        // 変数iに1を加算
                }
            }
            file.close();                       // ファイルを閉じる
        }
        Serial.print("'HTTP Sends ");           // ファイルサイズの表示
        Serial.print(i);                        // ファイルサイズの表示
        Serial.println(" Bytes");               // シリアル出力表示
    }
//  client.stop();                              // クライアントの切断
}
