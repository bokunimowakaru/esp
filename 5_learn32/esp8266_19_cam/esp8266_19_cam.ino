/*******************************************************************************
Practice esp8266 19 cam 【カメラ for SeeedStudio Grove Serial Camera Kit】

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/*
ご注意
    本サンプルではSPIFFSを使用します。Arduino IDE のFlash Sizeの設定で
    2MB(1MB SPIFFS)を選択してください。
    本スケッチを実行するとSPIFFS内のファイルは消去されます。
*/

#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_SW 12                           // IO 12(4番ピン)にSWを接続する
#define PIN_CAM 13                          // IO 13(5番ピン)にPch-FETを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#include <WiFiUdp.h>                        // udp通信を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 59*60*1000000ul             // スリープ時間 59分(uint32_t)
#define DEVICE "cam_a_1,"                   // デバイス名(5文字+"_"+番号+",")
#define FILENAME "/cam.jpg"                 // 画像ファイル名(ダウンロード用)
void sleep();

File file;
WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
unsigned long TIME;                         // 写真公開時刻(起動後の経過時間)
int cause;                                  // 起動事由を保持

void send_udp(const char *s){
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println(s);                         // 変数sの内容を送信"
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(20);                              // 送信完了待ち
}

void setup(){ 
    char s[65];                             // 64文字65バイトの文字配列変数
    pinMode(PIN_SW,INPUT);                  // SWを接続したポートを入力に
    cause=digitalRead(PIN_SW);              // 状態を確認
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,LOW);              // FETをLOW(ON)にする
    lcdPrint("ESP8266 eg19 Cam");           // 「ESP32 eg19」を液晶に表示
    while(!SPIFFS.begin()) delay(100);      // ファイルシステムの開始
    Dir dir = SPIFFS.openDir("/");          // ファイルシステムの確認
    if(dir.next()==0) SPIFFS.format();      // ディレクトリが無い時に初期化
    file = SPIFFS.open(FILENAME,"w");       // 保存のためにファイルを開く
    if(file==0) sleep();                    // ファイルを開けれなければ戻る
    delay(100);                             // カメラの起動待ち
    Serial.begin(115200);                   // カメラとのシリアル通信を開始する
    lcdPrint("Cam Init");                   // 「Cam Init」を液晶に表示
    CamInitialize();                        // カメラの初期化コマンド
    CamSizeCmd(1);                          // 撮影サイズをQVGAに設定(0でVGA)
    lcdPrint("Cam Capt");                   // 「Cam Capt」を液晶に表示
    CamCapture();                           // カメラで写真を撮影する
    size=CamGetData(file);                  // 撮影した画像をファイルに保存
    file.close();                           // ファイルを閉じる
    pinMode(PIN_CAM,INPUT);                 // FETをOFFにする(WiFiと排他動作)
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        i2c_lcd_print_val("WiFi STA",(TIMEOUT-millis())/1000); // 接続進捗を表示
        if(millis()>TIMEOUT) sleep();       // 20秒を過ぎたらスリープ
        delay(250);                         // 待ち時間処理
    }
    if(!cause){                             // ボタンが押下状態の時
        i2c_lcd_print_val("WakeIvnt",cause);// causeの内容をLCDへ表示
        send_udp("Ping");                   // Pingを送信
    }
    uint32_t ip=WiFi.localIP();             // 本機のIPアドレスを変数ipへ代入
    snprintf(s,64,"%s%d, http://%d.%d.%d.%d%s",
        DEVICE,size,ip&255,ip>>8&255,ip>>16&255,ip>>24,FILENAME);
    send_udp(s);                            // UDPメッセージを送信する
    TIME=millis()+TIMEOUT;                  // 終了時刻を保存(現時刻＋TIMEOUT)
    server.begin();                         // HTTPサーバを起動する
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
    lcdPrint(&s[5]);                        // 受信した命令を液晶に表示
    file = SPIFFS.open(FILENAME,"r");       // 読み込みのためにファイルを開く
    if(file){                               // ファイルを開けることが出来た時、
        client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
        client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
        client.println("Connection: close");                // 応答後に閉じる
        client.println();                                   // ヘッダの終了
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
    client.stop();                          // クライアントの切断
    sleep();                                // sleep()へ
}

void sleep(){
    lcdPrint("Sleepingzzz...");             // 「Sleeping」を液晶に表示
    pinMode(PIN_CAM,INPUT);                 // FETを接続したポートをオープンに
    Serial.end();
    pinMode(3,INPUT);                       // RXDをオープンに
    pinMode(1,INPUT);                       // TXDをオープンに(PUP抵抗の節電)

    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
