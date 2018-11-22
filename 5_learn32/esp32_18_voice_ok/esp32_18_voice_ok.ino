/*******************************************************************************
Practice esp32 18 voice OK 【簡易音声認識子機】

                                           Copyright (c) 2017-2018 Wataru KUNINO
*******************************************************************************/

/*
※FTPサーバが必要です。
　ラズベリーパイへ、FTPサーバをセットアップするには下記を実行してください。
　~/esp/tools/ftp_setup.sh

※FTPでは、ユーザ名やパスワード、データーが平文で転送されます。
　インターネット上で扱う場合は、セキュリティに対する配慮が必要です。
*/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include "uspeech.h"                        // 音声認識ライブラリ uspeech
#define PIN_LED 2                           // IO 2へLEDを接続
#define PIN_AIN 33                          // IO 33へマイクロホンを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
#define DEVICE "sound_1,"                   // デバイス名(5文字+"_"+番号+",")
#define FILENAME "/sound.wav"               // 音声ファイル名(アップロード用)
#define FTP_TO   "192.168.0.10"             // FTP 送信先のIPアドレス
#define FTP_USER "pi"                       // FTP ユーザ名(Raspberry Pi)
#define FTP_PASS "password"                 // FTP パスワード(Raspberry Pi)
#define FTP_DIR  "~"                        // FTP ディレクトリ(Raspberry Pi)

signal voice(PIN_AIN);                      // 音声入力の有効化
char phoneme[17];                           // 音声データ用の変数
int phoneme_i=0,t=0;                        // 音声データ数,待ち時間

void setup(){
    WiFi.mode(WIFI_OFF); btStop();          // 無線LANとBluetoothをOFFに設定する
    pinMode(PIN_AIN,INPUT);                 // アナログ入力端子の設定
    pinMode(PIN_LED, OUTPUT);               // LEDを接続したポートを出力に
    digitalWrite(PIN_LED,HIGH);             // LEDを点灯する(起動を知らせる)
    voice.micPowerThreshold=25;             // マイク入力閾値を下げる(初期値=50)
    voice.calibrate();                      // マイク入力のDC値の設定
    memset(phoneme,0,17);                   // 音声データ用の変数の初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
}

void loop(){
    voice.sample();                         // 音声の入力
    char p=voice.getPhoneme();              // 入力音声の認識
    if(p!=' '){                             // 認識結果が有音だった場合
        Serial.print(p);                    // 認識した音をシリアル出力表示
        if(phoneme_i > 0 && p==phoneme[phoneme_i-1]) return;    // 同音時に戻る
        t=0; digitalWrite(PIN_LED,HIGH);    // 終了待ち時間のリセット
        phoneme[phoneme_i++] = p;           // 認識音を音声データ配列変数へ保存
        if(phoneme_i<16) return;            // 配列16以内ならloopの先頭へ戻る
    }else{                                  // 認識結果が無音だったとき
        if(phoneme_i && t==0) Serial.print(' ');    // 空白をシリアル出力表示
        t++; digitalWrite(PIN_LED,LOW);     // 終了待ち時間を1つカウント
        if(t < 64 || phoneme_i == 0)return; // 待ち時間変数tが64未満の時に戻る
    }
    
    /**** 単語認識部 ****/
    char vr[9]; memset(vr,0,9);             // 認識結果の保存用変数定期と初期化
    int ok=-1;                              // 認識結果(未/ON/OFF)の保存用変数
    char *spo=strchr(phoneme,'o');          // 音声データ内に「o」が含まれるか？
    char *sph=strchr(phoneme,'h');          // 音声データ内に「o」が含まれるか？
    if(spo<sph) spo=sph;                    // oとhのうち後ろにある方をspoへ代入
    if(spo>0){                              // 「o」または「h」が含まれている
        char *spe=strchr(spo,'e');          // 「o」以降に「e」が含まれるか？
        if(spe>0) ok=1;                     // 「o」が含まれて「e」が後にある
    }
    if(ok>0) strcpy(vr,",OK");              // ok=1のとき文字列変数へOKを代入
      else strcpy(vr,",ERR");               // その他のとき文字列変数へERRを代入
    Serial.println(vr);                     // 文字列変数をシリアル出力表示
    t=0; phoneme_i=0; memset(phoneme,0,17); // 時間、音声データ用の変数の初期化
    if(ok<1) return;                        // ok=1でないときに戻る
    digitalWrite(PIN_LED,HIGH);
    int size=micToWave_rec(PIN_AIN);        // 録音を行う
    digitalWrite(PIN_LED,LOW);
    if(size <= 0) return;                   // サイズが0のときに戻る
    /**** Wi-Fi送信部 ****/
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));            // LEDの点滅
        delay(100);                         // 待ち時間
    } Serial.println(WiFi.localIP());       // 本機のIPアドレスをシリアル出力
    micToWave_FTP(FILENAME);                // FTPによる送信を実行する
    WiFi.disconnect(true);                  // WiFiアクセスポイントを切断する
    WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定する
    Serial.println("done");                 // 終了表示
}
