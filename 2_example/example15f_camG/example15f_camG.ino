/*******************************************************************************
Example 15f:(IoTセンサ) Wi-Fi カメラ SeeedStudio Grove Serial Camera Kit用 FTP版
定期的にカメラ撮影を行い、撮影後に通知を送信する監視カメラです。
撮影した写真をFTPで転送します。

※本スケッチ内define部のFTP_TO、FTP_USER、FTP_PASS、FTP_DIRを設定して下さい。

※FTPサーバが必要です。
　ラズベリーパイへ、FTPサーバをセットアップするには下記を実行してください。
　~/esp/tools/ftp_setup.sh

　実験を終えたら Raspberry Pi の電源を切っておく、もしくはftp_uninstall.shを
　実行してvsftpdを削除してください。（FTPによる不正アクセスを防止するため）

※FTPでは、ユーザ名やパスワード、データーが平文で転送されます。
　インターネット上で扱う場合は、セキュリティに対する配慮が必要です。

※FTPサーバ側の処理速度などに応じてftp.inoを修正する必要が生じる可能性があります
　一例として、example25a_fsのFTPサーバ機能への送信は行えません。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <SoftwareSerial.h>
#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_CAM 13                          // IO 13(5番ピン)にPch-FETを接続する
#define TIMEOUT 2000                        // タイムアウト 2秒
#include <WiFiUdp.h>                        // udp通信を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // UDP送信先IPアドレス
#define PORT 1024                           // UDP送信ポート番号

#define FTP_TO   "192.168.0.10"             // FTP 送信先のIPアドレス
#define FTP_USER "pi"                       // FTP ユーザ名(Raspberry Pi)
#define FTP_PASS "password"                 // FTP パスワード(Raspberry Pi)
#define FTP_DIR  "~"                        // FTP ディレクトリ(Raspberry Pi)

#define SLEEP_P 59*60*1000000ul             // スリープ時間 59分(uint32_t)
#define DEVICE "cam_a_1,"                   // デバイス名(5文字+"_"+番号+",")
#define FILENAME "/cam.jpg"                 // 画像ファイル名(ダウンロード用)
void sleep();

File file;
SoftwareSerial softwareSerial(12,14);       // IO12(4)をRX,IO14(3)をTXに設定
WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
unsigned long TIME;                         // 写真公開時刻(起動後の経過時間)

void setup(){ 
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,LOW);              // FETをLOW(ON)にする
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 15 Cam");       // 「Example 15」をシリアル出力表示
    lcdPrint("Example 15 Cam");             // 「Example 15」を液晶に表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(!SPIFFS.begin()) delay(100);      // ファイルシステムの開始
    Dir dir = SPIFFS.openDir("/");          // ファイルシステムの確認
    if(dir.next()==0) SPIFFS.format();      // ディレクトリが無い時に初期化
    delay(100);                             // カメラの起動待ち
    softwareSerial.begin(115200);           // カメラとのシリアル通信を開始する
    lcdPrint("Cam Init");                   // 「Cam Init」を液晶に表示
    CamInitialize();                        // カメラの初期化コマンド
    CamSizeCmd(1);                          // 撮影サイズをQVGAに設定(0でVGA)
    delay(4000);                            // 完了待ち(開始直後の撮影防止対策)
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    file = SPIFFS.open(FILENAME,"w");       // 保存のためにファイルを開く
    if(file==0) sleep();                    // ファイルを開けれなければ戻る
    lcdPrint("Cam Capt");                   // 「Cam Capt」を液晶に表示
    CamCapture();                           // カメラで写真を撮影する
    size=CamGetData(file);                  // 撮影した画像をファイルに保存
    file.close();                           // ファイルを閉じる
    lcdPrint("FTP send");                   // 「FTP send」を液晶に表示
    byte ret=doFTP(FILENAME);               // FTPで送信する＜追加＞
    if(ret){
        lcdPrintVal("FTP Err",ret);         // 失敗時にLCD表示
        Serial.print("FTP Err :");
        Serial.println(ret);
        delay(3000);
    }
    Serial.print("http://");                // デバイス名を送信
    Serial.print(WiFi.localIP());           // 本機のIPアドレスを送信
    Serial.println(FILENAME);               // ファイル名を送信
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(size);                        // ファイルサイズを送信
    udp.print(", http://");                 // デバイス名を送信
    udp.print(WiFi.localIP());              // 本機のIPアドレスを送信
    udp.println(FILENAME);                  // ファイル名を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    TIME=millis()+TIMEOUT;                  // 終了時刻を保存(現時刻＋TIMEOUT)
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
}

void loop(){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int i,len=0;                            // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    
    if(millis() > TIME) sleep();            // 終了時刻になったらsleep()を実行
    client = server.available();            // 接続されたクライアントを生成
    if(client==0)return;                    // loop()の先頭に戻る
    Serial.println("Connected");            // シリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            t=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(len>5 && strncmp(s,"GET /",5)==0) break;
                len=0;                      // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                   // 文字列変数に文字cを追加
                len++;                      // 変数lenに1を加算
                s[len]='\0';                // 文字列を終端
                if(len>=64) len=63;         // 文字列変数の上限
            }
        }
        if(t>TIMEOUT){                      // TIMEOUTに到達したら終了
            client.stop();                  // セッションを閉じる
            sleep();                        // sleep()へ
        }
        delay(1); t++;                      // 変数tの値を1だけ増加させる
    }
    if(!client.connected()) return;         // 切断されていた場合はloopの先頭へ
    Serial.println(s);                      // 受信した命令をシリアル出力表示
    lcdPrint(&s[5]);                        // 受信した命令を液晶に表示
    file = SPIFFS.open(FILENAME,"r");       // 読み込みのためにファイルを開く
    if(file){                               // ファイルを開けることが出来た時、
        client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
        client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
        client.println("Connection: close");                // 応答後に閉じる
        client.println();                                   // ヘッダの終了
        /*  以下(計4行)、処理速度が遅かった
        for(t=0;t<size;t++){                // ファイルサイズ分の繰り返し処理
            if(!file.available()) break;    // ファイルが無ければ転送終了
            if(!client.connected()) break;  // 切断されていた場合は転送終了
            client.write((byte)file.read());// ファイルの転送
        }
        以下のように修正し、高速化を行った */
        len=0; t=0;                         // 変数lenとtを再利用
        while( t<3 ){                       // エラー3回以内で繰り返し処理実行
            if(!file.available()){          // ファイルの有無を確認
                t++; delay(100);            // ファイル無し時に100msの待ち時間
                continue;                   // whileループに戻ってリトライ
            }
            i=file.read((byte *)s,64);      // ファイル64バイトを読み取り
            if(i>0){
                client.write((byte *)s,i);  // ファイルの書き込み
                len+=i; t=0;                // ファイル長lenを加算
                if(len>=size) break;        // ファイルサイズに達したら終了
            }
        }
        file.close();                       // ファイルを閉じる
    }
//  client.stop();                          // クライアントの切断
    Serial.print(len);                      // ファイルサイズをシリアル出力表示
    Serial.println(" Bytes");               // シリアル出力表示
    sleep();                                // sleep()へ
}

void sleep(){
    lcdPrint("Sleepingzzz...");             // 「Sleeping」を液晶に表示
    Serial.println("Done");                 // 終了表示
    pinMode(PIN_CAM,INPUT);                 // FETを接続したポートをオープンに
    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
