/*******************************************************************************
最新IoTモジュール ESP32-WROOM-32用

Example 31 ESP32 DEMO (HTTP版)
デモ用：展示会などでPCやスマホから直接ESPモジュールへ接続する場合のサンプル

　　・内蔵ホール素子
　　・内蔵タッチセンサ

                                           Copyright (c) 2016-2018 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define PIN_SW 0                            // GPIO 0(25番ピン)にスイッチを接続
#define LCD_EN 0                            // 液晶の接続有=1,無=0
#define WIFI_AP_MODE 1                      // Wi-Fi APモード ※「0」でSTAモード
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SSID_AP "ESP_SoftAP"                // 無線LANアクセスポイントのSSID
#define TIMEOUT 10000                       // タイムアウト 20秒
#define PORT 1024                           // 受信ポート番号
#define HALL_THRESH 20                      // ホール素子の変化通知しきい値
#define HALL_COUNT  50                      // ホール素子の積算回数
#define TOUCH_THRESH 50                     // タッチセンサのしきい値
#define DEVICE "esp32_1,"                   // デバイス名(5文字+"_"+番号+",")
WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int hall_prev=0;                            // ホール素子の前回送信時の測定値
int hall_sum=0;                             // ホール素子の測定積算値
int hall_count=0;                           // ホール素子の積算値のサンプル数
int touch_prev[10];                         // タッチセンサの前回値
int touch_sum[10];                          // タッチセンサの積算値
boolean touchB[10];                         // タッチセンサ判定値
uint8_t touch_pin[10]={T0,T1,T2,T3,T4,T5,T6,T7,T8,T9};  // タッチセンサのpin番号
IPAddress IP;                               // IPアドレス
unsigned long LCD_TIME;                     // 次回の液晶表示更新時刻

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    if(LCD_EN)lcdSetup();                   // 液晶の初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 DEMO");           // 「ESP32 DEMO」をシリアル出力表示
    delay(10);                              // ESP32に必要な待ち時間
    #if WIFI_AP_MODE==0
        WiFi.mode(WIFI_STA);                // 無線LANを[STA]モードに設定
        WiFi.begin(SSID,PASS);              // 無線LANアクセスポイントへ接続
        while(WiFi.status()!=WL_CONNECTED){ // 接続に成功するまで待つ
            delay(500);                     // 待ち時間処理
            digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
            Serial.print(".");
        }
        Serial.println();                   // 改行をシリアル出力
        IP = WiFi.localIP();
    #else // WIFI_AP_MODE==1
        WiFi.mode(WIFI_AP);                 // 無線LANを[AP]モードに設定
        delay(1000);                        // 切換え・設定待ち時間
        WiFi.softAPConfig(
            IPAddress(192,168,0,1),         // AP側の固定IPアドレス
            IPAddress(0,0,0,0),             // 本機のゲートウェイアドレス
            IPAddress(255,255,255,0)        // ネットマスク
        );
        WiFi.softAP(SSID_AP);               // ソフトウェアAPの起動
        Serial.println(SSID_AP);            // SSIDを表示
        IP = WiFi.softAPIP();
    #endif
    Serial.println(IP);                     // IPアドレスを表示
    server.begin();                         // サーバを起動する
    udp.begin(PORT);                        // UDP通信御開始
    for(int i=0;i<10;i++){ touchB[i]=0; touch_prev[i]=0; touch_sum[i]=0; }
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[129];                            // 文字列変数を定義 129バイト128文字
    int len=0;                              // 文字列の長さカウント用の変数
    int i=0;                                // 待ち受け時間のカウント用の変数
    int headF=0;                            // HTTPヘッダ用フラグ(0:初期状態)
    int hall;                               // ホール素子(磁気センサ)用の変数
    int led=digitalRead(PIN_LED);           // LEDの読み取り
    int udpF=0;                             // UDP送信用フラグ
    int val;                                // タッチセンサ値
    
    hall_sum += hallRead();                 // ホール効果素子から測定値を取得
    hall_count++;                           // カウント
    hall = hall_sum / hall_count;           // 平均値を算出
    for(i=3;i<10;i++) touch_sum[i] += touchRead(touch_pin[i]);
    touch_sum[1] += digitalRead(0)*100;     // BOOTスイッチの入力
    
    if( hall_count >= HALL_COUNT ){
        hall_count=0; hall_sum=0;
        if( hall > hall_prev + HALL_THRESH || hall < hall_prev - HALL_THRESH ){
            hall_prev=hall;                 // 前回値を更新
            udpF=1;                         // UDP送信フラグを立てる
        }
        for(i=0;i<10;i++){
            val=touch_sum[i] / HALL_COUNT;
            if( val >  touch_prev[i] + TOUCH_THRESH){
                udpF=1;
                touch_prev[i]=val;          // センサ値を更新
                touchB[i]=0;                // センサ値はOFF
            }else
            if( val < touch_prev[i] - TOUCH_THRESH){
                udpF=1;
                touch_prev[i]=val;          // センサ値を更新
                touchB[i]=1;                // センサ値はON
            }
            touch_sum[i]=0;
        }
    }
    
    if(udpF){
        udp.beginPacket((IPAddress)(0xFF000000|(uint32_t)IP),PORT); // UDP送信先
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(hall_prev);               // 温度値を送信
        Serial.print(hall);                 // シリアル出力表示
        if(LCD_EN)lcdPrintVal("Hall Eff",hall_prev);    // 液晶へ表示
        LCD_TIME=millis()+TIMEOUT;          // 次回の液晶更新時刻
        for(i=0;i<10;i++){
            udp.print(',');         Serial.print(",");
            udp.print(touchB[i]);   Serial.print(touchB[i]);
        }
        udp.println(); Serial.println();    // 改行を送信
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
    }
    client = server.available();            // 接続されたクライアントを生成
    if(!client){
        if( millis() > LCD_TIME ){
            if(LCD_EN) lcdPrintIp(IP);
            LCD_TIME=millis()+TIMEOUT;
        }
        return;                             // 非接続の時にloop()の先頭に戻る
    }
    Serial.print("Connected from ");        // 接続されたことをシリアル出力表示
    Serial.println(client.remoteIP());
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            i=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            switch(c){                      // 文字cに応じて
                case '\0':                  // 文字変数cの内容が空のとき
                case '\r':                  // 文字変数cの内容がCRのとき
                    break;                  // 何もしない
                case '\n':                  // 文字変数cの内容がLFのとき
                    if(strncmp(s,"GET /?L=",8)==0 && len>8){
                        led=1;              // 変数ledに1を代入
                        if(s[8]=='0')led=0; // 9番目の文字が0のとき0を代入
                    }else if(len==0)headF=1;// ヘッダの終了
                    len=0;                  // 文字列長を0に
                    break;
                default:                    // その他の場合
                    s[len]=c;               // 文字列変数に文字cを追加
                    len++;                  // 変数lenに1を加算
                    s[len]='\0';            // 文字列を終端
                    if(len>=128) len=127;   // 文字列変数の上限
                    break;
            }
        }
        i++;                                // 変数iの値を1だけ増加させる
        if(headF) break;                    // HTTPヘッダが終わればwhileを抜ける
        if(i>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        client.println("HTTP/1.1 200 OK");  // HTTP OKを応答
        digitalWrite(PIN_LED,led);          // LEDの制御
        html(client,led,hall,IP);           // HTMLコンテンツを出力
    }
    client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
