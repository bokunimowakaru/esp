/*******************************************************************************
Example 47 (=32+15): 監視カメラ for SeeedStudio Grove Serial Camera Kit 

    カメラ接続用
    GPIO16(27番ピン) U2RXD カメラ側はTXD端子(黄色)
    GPIO17(28番ピン) U2TXD カメラ側はRXD端子(白色)

    GPIO 2(24番ピン)にPch-FETを接続

※本サンプル作成の段階では、(暫定的に)下記ライブラリを使用しました。
    https://github.com/copercini/arduino-esp32-SPIFFS

※今後、esp-idfや上記ライブラリを基にしたものが公式サポートされると思います。

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <SPIFFS.h>
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_CAM 2                           // GPIO 2(24番ピン)にPch-FETを接続
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 1*60*1000000                // スリープ時間 1分(uint32_t)
#define DEVICE "cam_a_1,"                   // デバイス名(5文字+"_"+番号+",")
#define FILENAME "/cam.jpg"                 // 画像ファイル名(ダウンロード用)

File file;
HardwareSerial hardwareSerial2(2);          // カメラ接続用シリアルポートESP32用

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
unsigned long TIME;                         // 写真公開時刻(起動後の経過時間)

void setup(){ 
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,LOW);              // FETをLOW(ON)にする
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.15 Cam");      // 「Example 15」をシリアル出力表示
    lcdPrint("ESP32 eg15 Cam");             // 「Example 15」を液晶に表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    if(!SPIFFS.begin()){                    // ファイルシステムSPIFFSの開始
        Serial.println("Formating SPIFFS.");
        SPIFFS.format(); SPIFFS.begin();    // エラー時にSPIFFSを初期化
    }
    delay(100);                             // カメラの起動待ち
    hardwareSerial2.begin(115200);          // カメラとのシリアル通信を開始する
    lcdPrint("Cam Init");                   // 「Cam Init」を液晶に表示
    CamInitialize();                        // カメラの初期化コマンド
    CamSizeCmd(1);                          // 撮影サイズをQVGAに設定(0でVGA)
    delay(4000);                            // 完了待ち(開始直後の撮影防止対策)
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    file = SPIFFS.open(FILENAME,"w");       // 保存のためにファイルを開く
    if(file==0) sleep();                    // ファイルを開けれなければ戻る
    lcdPrint("Cam Capt");                   // 「Cam Capt」を液晶に表示
    CamCapture();                           // カメラで写真を撮影する
    size=CamGetData(file);                  // 撮影した画像をファイルに保存
    file.close();                           // ファイルを閉じる
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(size);                        // ファイルサイズを送信
    udp.print(", http://");                 // デバイス名を送信
    udp.print(WiFi.localIP());              // 本機のIPアドレスを送信
    udp.println(FILENAME);                  // ファイル名を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.print("http://");                // デバイス名を送信
    Serial.print(WiFi.localIP());           // 本機のIPアドレスを送信
    Serial.println(FILENAME);               // ファイル名を送信
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
    TIME=millis()+TIMEOUT;                  // 終了時刻を保存(現時刻＋TIMEOUT)
    server.begin();                         // HTTPサーバを起動する
}

void loop(){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int i,len=0;                            // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    
    if(millis() > TIME) sleep();            // 終了時刻になったらsleep()を実行
    client = server.available();            // 接続されたクライアントを生成
    if(!client)return;                      // loop()の先頭に戻る
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
    delay(1);                               // クライアントの準備待ち時間
    if(!client.connected()) return;         // 切断されていた場合はloopの先頭へ
    Serial.println(s);                      // 受信した命令をシリアル出力表示
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
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
    Serial.print(size);                     // ファイルサイズをシリアル出力表示
    Serial.println(" Bytes");               // シリアル出力表示
    sleep();                                // sleep()へ
}

void sleep(){
    lcdPrint("Sleepingzzz...");             // 「Sleeping」を液晶に表示
    Serial.println("Done");                 // 終了表示
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
