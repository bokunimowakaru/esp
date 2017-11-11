/*******************************************************************************
Example 65 アナログ入力ポートから録音した音声を送信する

                                                Copyright (c) 2017 Wataru KUNINO
********************************************************************************
　マイク入力ポート：GPIO 34 ADC1_CH6
　　　　　　　　　　IoT Express     A2ピン(P3 3番ピン)
　　　　　　　　　　WeMos D1 R32        Analog 3ピン
　　　　　　　　　　ESP-WROOM-32        6番ピン

　録音開始入力ポート：GPIO 0 を Lレベルへ移行する
　　　　　　　　　　IoT Express     BOOTスイッチを押す

　受信用ソフト：toolsフォルダ内のget_sound.sh(Raspberry Pi用)などを使用する
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_AIN 34                          // GPIO 34 ADC1_CH6(6番ピン)をADCに
#define PIN_EN 2                            // GPIO 2(24番ピン)をセンサの電源に
#define PIN_WAKE GPIO_NUM_0                 // GPIO 0をスリープ解除信号へ設定
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 1*60*1000000                // スリープ時間 1分(uint32_t)
#define DEVICE "sound_1,"                   // デバイス名(5文字+"_"+番号+",")
#define FILENAME "/sound.wav"               // 音声ファイル名(ダウンロード用)
#define SOUND_LEN 3*8000                    // 音声長 8000=約1秒

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
unsigned long TIME;                         // 写真公開時刻(起動後の経過時間)

byte snd[SOUND_LEN];                        // 音声録音用変数
int snd_size=0;                             // 音声データの大きさ(バイト)
const byte Wave_Header0[2]={0,0};
const byte Wave_Header1[8]={16,0,0,0,1,0,1,0};
const byte Wave_Header2[4]={0x40,0x1F,0,0}; // 8000Hz -> 00 00 1F 40
const byte Wave_Header3[4]={1,0,8,0};

int snd_rec(byte *snd, int len, int port){
    unsigned long time,time_trig=0;
    int i=0, wait_us=125;                   // 8kHz時 サンプリング間隔 125us
    
    while(i<len){
        time=micros();
        if(time < time_trig) continue;      // 未達時
        snd[i]=(byte)(analogRead(PIN_AIN)>>4);  // アナログ入力 12ビット→8ビット
        time_trig = time + wait_us;
        i++;
        if(time_trig < wait_us) break;      // 時間カウンタのオーバフロー
    }
    return i;
}

void setup(){ 
    pinMode(PIN_AIN,INPUT);                 // アナログ入力端子の設定
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("eg.65 Microphone");     // タイトルをシリアル出力表示
    TIME=millis();
    digitalWrite(PIN_EN,HIGH);
    Serial.print("Recording... ");
    snd_size=snd_rec(snd,SOUND_LEN,PIN_AIN);// 音声入力
    Serial.print(millis()-TIME);
    Serial.println("msec. Done");
    if(snd_size<82) sleep();
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    digitalWrite(PIN_EN,HIGH);
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(snd_size+44);                 // ファイルサイズを送信
    udp.print(", http://");                 // デバイス名を送信
    udp.print(WiFi.localIP());              // 本機のIPアドレスを送信
    udp.println(FILENAME);                  // ファイル名を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    Serial.print("http://");                // デバイス名を送信
    Serial.print(WiFi.localIP());           // 本機のIPアドレスを送信
    Serial.println(FILENAME);               // ファイル名を送信
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
    client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
    client.println("Content-Type: audio/wav");          // WAVコンテンツ
    client.println("Connection: close");                // 応答後に閉じる
    client.println();                                   // ヘッダの終了
    client.write("RIFF",4);                 // RIFF                     ->[4]
    i = snd_size + 44 - 8;                  // ファイルサイズ-8
    client.write(i & 0xFF);                 // サイズ・最下位バイト     ->[5]
    client.write((i>>8) & 0xFF);            // サイズ・第2位バイト      ->[6]
    client.write(Wave_Header0,2);           // サイズ・第3-4位バイト    ->[8]
    client.write("WAVEfmt ",8);             // "Wave","fmt "            ->[16]
    client.write(Wave_Header1,8);           // 16,0,0,0,1,0,1,0         ->[24]
    client.write(Wave_Header2,4);           // 68,172,0,0               ->[28]
    client.write(Wave_Header2,4);           // 68,172,0,0               ->[32]
    client.write(Wave_Header3,4);           // 1,0,8,0                  ->[36]
    client.write("data",4);                 //  "data"                  ->[40]
    i = snd_size + 44 - 126;                // ファイルサイズ-126
    client.write(i & 0xFF);                 // サイズ・最下位バイト     ->[41]
    client.write((i>>8) & 0xFF);            // サイズ・最下位バイト     ->[42]
    client.write("\0\0",2);                 // サイズ3-4
    len=0; t=0;                         // 変数lenとtを再利用
    for(i=0;i<snd_size;i++) client.write(snd[i]);
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
    Serial.print(snd_size+44);              // ファイルサイズをシリアル出力表示
    Serial.println(" Bytes");               // シリアル出力表示
    sleep();                                // sleep()へ
}

void sleep(){
    Serial.println("Bye");                  // 終了表示
    delay(200);                             // 送信待ち時間
    esp_sleep_enable_ext0_wakeup(PIN_WAKE,0);  // 1=High, 0=Low
                                            // センサの状態が変化すると
                                            // スリープを解除するように設定
    esp_deep_sleep_start();                 // Deep Sleepモードへ移行
}
