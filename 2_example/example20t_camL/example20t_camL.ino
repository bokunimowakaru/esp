/*******************************************************************************
Example 20: 監視カメラ for SparkFun SEN-11610 (LynkSprite JPEG Color Camera TTL)
画像一覧表示バージョン [試作品・不具合あり]

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
/*
表示可能な写真の枚数について：
Arduino IDEのタブ「html」をクリックするとコンテンツのスケッチを表示することが
できます。スケッチ先頭の「#define PICT_NUM」を変更することで、写真の表示枚数を
変更することが出来ます。初期値は5枚です。
ブラウザからの応答が無くなったり、写真が表示されない場合は表示枚数を減らしてく
ださい。リソース等に余裕を確保すれば、表示増やすことも可能かもしれません。
*/

#include <SoftwareSerial.h>
#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_CAM 13                          // IO 13(5番ピン)にPch-FETを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

File file;
SoftwareSerial softwareSerial(12,14);       // IO12(4)をRX,IO14(3)をTXに設定
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
int update=60;                              // ブラウザのページ更新間隔(秒)
int photo_n=0;                              // 写真番号
unsigned long shot_time=0;                  // 次回の撮影時間

void setup(){ 
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,0);                // FETをLOW(ON)にする
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 20 cam");       // 「Example 20」をシリアル出力表示
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
    server.begin();                         // サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
}

void loop() {
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    byte data[32];                          // 画像転送用の一時保存変数
    int len=0;                              // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    int i,j;
    char filename[]="/cam***.jpg";          // 画像ファイル名(ダウンロード用)
                                            // filename[4-6]は写真番号100～

    client = server.available();            // 接続されたクライアントを生成
    if(client==0){
        if(shot_time > 0 && millis()/1000 < shot_time) return;
        sprintf(filename,"/cam%03d.jpg",photo_n);
        file = SPIFFS.open(filename,"w");   // 保存のためにファイルを開く
        if(file){                           // ファイルを開けれなければ戻る
            lcdPrint("Cam Capt");           // 「Cam Capt」を液晶に表示
            lcdPrint2(filename);            // ファイル名を液晶に表示
            Serial.print("Capturing ");     // シリアル出力表示
            Serial.println(filename);
            CamSizeCmd2(2);                 // JPEGサイズを最小160x120に
            delay(100);
            CamSendTakePhotoCmd();          // カメラで写真を撮影する
            size=CamReadFileSize();         // ファイルサイズを読み取る
            j=0;                            // 変数j(総受信長)を0に設定
            while(j<size){                  // 終了フラグが0の時に繰り返す
                len = CamRead(data);        // カメラからデータを取得
                j += len;                   // データサイズの合算
                for(i=0;i<len;i++) file.write((byte)data[i]);   // データ保存
            }
            CamStopTakePhotoCmd();          // 撮影の終了(静止画の破棄)の実行
            CamReadADR0();                  // 読み出しアドレスのリセット
            file.close();                   // ファイルを閉じる
            lcdPrint("Done Cap");           // 「Done Cap」を液晶に表示
            Serial.print(size);             // ファイルサイズをシリアル出力表示
            Serial.println(" Bytes");       // シリアル出力表示
            photo_n++; if(photo_n>199) photo_n=0;
        }else Serial.println("File open ERROR");
        shot_time=millis()/1000ul+(unsigned long)update; // 次回の撮影時刻を設定
        return;
    }
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
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    delay(10);
    client.flush();                         // バッファ内のデータを破棄する
    if(!client.connected()||len<6) return;  // 切断された場合はloop()の先頭へ
    Serial.print(s);                        // 受信した命令をシリアル出力表示
    lcdPrint(&s[5]);                        // 受信した命令を液晶に表示
    if(strncmp(s,"GET / ",6)==0){           // コンテンツ取得命令時(撮影指示)
        html(client,size,update,WiFi.localIP()); // コンテンツ表示
        client.stop();                      // クライアントの切断
        Serial.println(" Done");
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /cam",8)==0 && strncmp(&s[11],".jpg",4)==0){  // 画像取得
        strncpy(filename,&s[4],11);
        lcdPrint(filename);                     // ファイル名を液晶へ出力表示
        file = SPIFFS.open(filename,"r");       // 読み込みのためにファイルを開く
        if(file){                               // ファイルを開けることが出来た時、
            client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
            client.print("Content-Length: ");                   // コンテンツ大きさ
            client.println( file.size() );
            client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
            client.println("Connection: close");                // 応答後に閉じる
            client.println();                                   // ヘッダの終了
            t=0; while(file.available()){       // ファイルがあれば繰り返し処理実行
                s[t]=file.read(); t++;          // ファイルの読み込み
                if(t >= 64){                    // 64バイトに達した時に転送処理
                    if(!client.connected()) break;              // 接続状態確認
                    client.write( (byte *)s, 64);               // 64バイト送信
                    t=0; delay(1);                              // 送信完了待ち
                }
            }
            if(t>0 && client.connected()) client.write((byte *)s,t);
            file.close();                       // ファイルを閉じる
        }else{
            client.println("HTTP/1.0 404 Not Found");           // Not Foundを応答
            client.println();                                   // ヘッダの終了
        }
        client.stop();                      // クライアントの切断
        Serial.println(" Done");
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /?SHOT",10)==0){      // 撮影コマンド
        shot_time=0;
    }
    if(strncmp(s,"GET /?INT=",10)==0){      // 更新時間の設定命令を受けた時
        update = atoi(&s[10]);              // 受信値を変数updateに代入
    }
    if(strncmp(s,"GET /?BPS=",10)==0){      // ビットレート設定命令時
        i = atoi(&s[10]);                   // 受信値を変数iに代入
        CamBaudRateCmd(i);                  // ビットレート設定
        delay(100);                         // 処理完了待ち
        softwareSerial.begin(i);            // ビットレート変更
    }
    if(strncmp(s,"GET /?SIZE=",11)==0){     // JPEGサイズ設定命令時
        i = atoi(&s[11]);                   // 受信値を変数iに代入
        CamSizeCmd(i);                      // JPEGサイズ設定
        delay(100);                         // 処理完了待ち
        CamSendResetCmd();                  // リセットコマンド
        softwareSerial.begin(38400);        // シリアル速度を初期値に戻す
    }
    if(strncmp(s,"GET /?RATIO=",12)==0){    // 圧縮率設定命令時
        i = atoi(&s[12]);                   // 受信値を変数iに代入
        CamRatioCmd(i);                     // 圧縮率設定コマンド
    }
    if(strncmp(s,"GET /?RESET=",12)==0){    // リセット命令時
        CamSendResetCmd();                  // リセットコマンド
        softwareSerial.begin(38400);        // シリアル速度を初期値に戻す
        s[11]='\0';                         // RESET=以降に続く文字を捨てる
    }
    if(strncmp(s,"GET /?FORMAT",12)==0){    // ファイルシステム初期化コマンド
        lcdPrint2("FORMAT");                // フォーマットの開始表示
        SPIFFS.format();                    // ファイル全消去
        photo_n=0;
    }
    for(i=6;i<strlen(s);i++) if(s[i]==' '||s[i]=='+') s[i]='\0';
    htmlMesg(client,&s[6],WiFi.localIP());  // メッセージ表示
    Serial.println(" Done");
    client.stop();                          // クライアント切断
}
