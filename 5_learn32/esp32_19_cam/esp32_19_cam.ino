/*******************************************************************************
Practice esp32 19 cam 【カメラ for SeeedStudio Grove Serial Camera Kit】

    カメラ接続用
    GPIO16(27番ピン) U2RXD カメラ側はTXD端子(黄色)
    GPIO17(28番ピン) U2TXD カメラ側はRXD端子(白色)
    GPIO 2(24番ピン) Pch-FETを接続(※カメラ用の電源制御が必要な場合のみ)

    I2C LCD(AE-AQM0802)接続用
    ※ I2C LCDを使用するときは、#define I2C_LCD_OFF を削除してください
    GPIO 21(33番ピン) I2C SDAポート 
    GPIO 22(36番ピン) I2C SCLポート

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <SPIFFS.h>                         // 内蔵FLASH用ファイルシステム
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_CAM 2                           // GPIO 2(24番ピン)にPch-FETを接続
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 1*55*1000000                // スリープ時間 55秒(uint32_t)
#define DEVICE "cam_a_1,"                   // デバイス名(5文字+"_"+番号+",")
#define FILENAME "/cam.jpg"                 // 画像ファイル名(ダウンロード用)

#define I2C_LCD_OFF                         // I2C接続LCDを使用する場合は削除

File file;
HardwareSerial hardwareSerial2(2);          // カメラ接続用シリアルポートESP32用

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
unsigned long TIME;                         // 写真公開時刻(起動後の経過時間)
int cause;                                  // 起動事由を保持

void send_udp(const char *s){
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println(s);                         // 変数sの内容を送信"
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.println(s);                      // 送信内容をシリアル出力する
    delay(20);                              // 送信完了待ち
}

void setup(){
    char s[65];                             // 64文字65バイトの文字配列変数
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,LOW);              // FETをLOW(ON)にする
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    lcdPrint("ESP32   eg19 Cam");           // 「ESP32 eg19」を液晶に表示
    if(!SPIFFS.begin()){                    // ファイルシステムSPIFFSの開始
        lcdPrint("SPIFFS  Init.ing");       // フォーマット中をLCDへ表示
        SPIFFS.format(); SPIFFS.begin();    // エラー時にSPIFFSを初期化
    }
    file = SPIFFS.open(FILENAME,"w");       // 保存のためにファイルを開く
    if(file==0){                            // ファイルを開けれなければエラー
        lcdPrint("SPIFFS  ERROR");          // SPIFFSエラー表示
        delay(5000); sleep();               // 終了
    }
    delay(100);                             // カメラの起動待ち
    hardwareSerial2.begin(115200);          // カメラとのシリアル通信を開始する
    lcdPrint("Cam Init");                   // 「Cam Init」を液晶に表示
    CamInitialize();                        // カメラの初期化コマンド
    CamSizeCmd(1);                          // 撮影サイズをQVGAに設定(0でVGA)
    lcdPrint("Cam Capt");                   // 「Cam Capt」を液晶に表示
    CamCapture();                           // カメラで写真を撮影する
    size=CamGetData(file);                  // 撮影した画像をファイルに保存
    file.close();                           // ファイルを閉じる
    digitalWrite(PIN_CAM,HIGH);             // FETをOFFにする(WiFiと排他動作)
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        i2c_lcd_print_val("WiFi STA",(TIMEOUT-millis())/1000); // 接続進捗を表示
        if(millis()>TIMEOUT) sleep();       // 20秒を過ぎたらスリープ
        delay(250);                         // 待ち時間処理
    }
    cause=esp_sleep_get_wakeup_cause();     // 起動事由を変数causeへ代入
    if(cause!=3){                           // causeが3(タイマー起動)以外の時
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
    if(cause!=3) send_udp("Pong");          // Pongを送信
    sleep();                                // sleep()へ
}

void sleep(){
    lcdPrint("Sleepingzzz...");             // 「Sleeping」を液晶に表示
    delay(100);                             // 送信完了待ち時間
    WiFi.disconnect();                      // 切断
    Serial.println("Done");                 // 完了表示
    delay(100);                             // 切断完了待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
