/*******************************************************************************
Example 20t: Wi-Fi コンシェルジェ カメラ担当 画像一覧表示機能付き版
Adafruit 1386・ SparkFun SEN-11610 LynkSprite JPEG Color Camera TTL 用

Webサーバ機能を使って、カメラのシャッターを制御し、撮影した写真を表示します。
[ESP8266WebServerライブラリ使用]

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/*
表示可能な写真の枚数について：
Arduino IDEのタブ「html」をクリックするとコンテンツのスケッチを表示することが
できます。スケッチ先頭の「#define PICT_NUM」を変更することで、写真の表示枚数を
変更することが出来ます。初期値は6枚です。
*/
/*
★★★ 写真データの転送に失敗するときは ★★★
Arduino IDEの[ツール]メニューの[CPU Frequency]で[160MHz]を設定してください
*/

#include <SoftwareSerial.h>
#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <ESP8266WebServer.h>
#define PIN_CAM 13                          // IO 13(5番ピン)にPch-FETを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

File file;
SoftwareSerial softwareSerial(12,14);       // IO12(4)をRX,IO14(3)をTXに設定
ESP8266WebServer server(80);                // Wi-Fiサーバ(ポート80=HTTP)定義
int photo_size=0;                                 // 画像データの大きさ(バイト)
int update=60;                              // ブラウザのページ更新間隔(秒)
int photo_n=0;                              // 写真番号
unsigned long shot_time=0;                  // 次回の撮影時間

void handleRoot() {
    char html[2048];
    Serial.print("HTTP");
    
    if(server.hasArg("SHOT")){
        shot_time=1;
        Serial.print(" SHOT");
    }
    if(server.hasArg("INT")){
        String val=server.arg("INT");
        update=val.toInt();
        if(update < 10) update = 10;
        Serial.print(" INT=");
        Serial.print(update);
    }
    if(server.hasArg("SIZE")){
        String val=server.arg("SIZE");
        CamSizeCmd(val.toInt());
        Serial.print(" SIZE=");
        Serial.print(val.toInt());
    }
    if(server.hasArg("BPS")){
        String val=server.arg("BPS");
        CamBaudRateCmd(val.toInt());
        Serial.print(" BPS=");
        Serial.print(val.toInt());
        delay(100);                         // 処理完了待ち
        softwareSerial.begin(val.toInt());  // ビットレート変更
    }
    if(server.hasArg("RATIO")){
        String val=server.arg("RATIO");
        CamRatioCmd(val.toInt());
        Serial.print(" RATIO=");
        Serial.print(val.toInt());
    }
    if(server.hasArg("RESET")){
        CamSendResetCmd();                  // リセットコマンド
        softwareSerial.begin(38400);        // シリアル速度を初期値に戻す
        Serial.print(" RESET");
    }
    if(server.hasArg("FORMAT")){
        photo_n=0;
        lcdPrint2("FORMAT");                // フォーマットの開始表示
        SPIFFS.format();                    // ファイル全消去
        Serial.print(" FORMAT");
    }
    getHtml(html,2047);
    server.send ( 200, "text/html", html );
    Serial.println(" Done");
}

void handleNotFound() {
    sendError404();
}

void setup(){ 
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,0);                // FETをLOW(ON)にする
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 20t cam");      // 「Example 20t」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(!SPIFFS.begin()) delay(100);      // ファイルシステムの開始
    Dir dir = SPIFFS.openDir("/");          // ファイルシステムの確認
    if(dir.next()==0) SPIFFS.format();      // ディレクトリが無い時に初期化
    lcdPrint("Cam Init");                   // 「Cam Init」を液晶に表示
    delay(100);                             // カメラの起動待ち
    softwareSerial.begin(38400);            // カメラとのシリアル通信を開始する
    CamSendResetCmd();                      // カメラへリセット命令を送信する
    delay(4000);
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.on ( "/", handleRoot );
    server.serveStatic("/", SPIFFS, "/", "max-age=60000");
    server.onNotFound ( handleNotFound );
    server.begin();                         // サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
}

void loop() {
    char filename[]="/cam***.jpg";          // 画像ファイル名(ダウンロード用)
    int size;
    byte data[32];                          // 画像転送用の一時保存変数
    int len=0;                              // 文字列等の長さカウント用の変数

    server.handleClient();                  // HTTPサーバ処理
    if(shot_time > 0){                      // 初回動作時以外のとき
        if(millis()/1000<shot_time) return; // 撮影時刻では無いのでloopへ戻る
        if(millis()/1000<update) return;    // 桁あふれ時の対策
    }
    sprintf(filename,"/cam%03d.jpg",photo_n);
    file = SPIFFS.open(filename,"w");       // 保存のためにファイルを開く
    if(file){                               // ファイルを開けれなければ戻る
        lcdPrint("Cam Capt");               // 「Cam Capt」を液晶に表示
        lcdPrint2(filename);                // ファイル名を液晶に表示
        Serial.print("Capturing ");         // シリアル出力表示
        Serial.println(&filename[1]);
        CamSizeCmd(2);                      // JPEGサイズを最小160x120に
        delay(100);
        CamSendTakePhotoCmd();              // カメラで写真を撮影する
        size=CamReadFileSize();             // ファイルサイズを読み取る
        int j=0;                            // 変数j(総受信長)を0に設定
        while(j<size){                      // 終了フラグが0の時に繰り返す
            len = CamRead(data);            // カメラからデータを取得
            j += len;                       // データサイズの合算
            for(int i=0;i<len;i++) file.write((byte)data[i]);   // データ保存
        }
        CamStopTakePhotoCmd();              // 撮影の終了(静止画の破棄)の実行
        CamReadADR0();                      // 読み出しアドレスのリセット
        file.close();                       // ファイルを閉じる
        Serial.print(size);                 // ファイルサイズをシリアル出力表示
        Serial.print(" Bytes ");            // シリアル出力表示
        photo_n++; if(photo_n>199) photo_n=0;
        photo_size=size;
    }else Serial.println("File open ERROR");
    shot_time=millis()/1000ul+(unsigned long)(update-5); // 次回の撮影時刻
    Serial.println(", Done Capturing");
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
}
