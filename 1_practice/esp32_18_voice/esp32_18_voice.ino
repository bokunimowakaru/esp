/*******************************************************************************
Practice esp32 18 voice 【簡易音声認識子機】

                                           Copyright (c) 2017-2018 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#include "uspeech.h"                        // 音声認識ライブラリ uspeech
#define PIN_LED 2                           // IO 2へLEDを接続
#define PIN_AIN 33                          // IO 33へマイクロホンを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
#define DEVICE "voice_1,"                   // デバイス名(5文字+"_"+番号+",")

int LED=0;                                  // LEDの出力状態
signal voice(PIN_AIN);                      // 音声入力の有効化
char phoneme[17];                           // 音声データ用の変数
int phoneme_i=0;                            // データ位置
int t=0;                                    // 待ち時間カウント用の変数

void setup(){
    pinMode(PIN_LED, OUTPUT);               // LEDを接続したポートを出力に
    digitalWrite(PIN_LED,HIGH);             // LEDを点灯する(起動を知らせる)
    voice.f_enabled = true;                 // f音の検出を行う
    voice.fconstant = 400;                  // f音の閾値設定
    voice.econstant = 1;                    // e音の閾値設定
    voice.aconstant = 2;                    // a音の閾値設定
    voice.vconstant = 3;                    // v音の閾値設定
    voice.shconstant = 4;                   // sh音の閾値設定
    voice.calibrate();                      // マイク入力のDC値の設定
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("esp32 18 voice");       // 「esp32 18 voice」をシリアル出力
    memset(phoneme,0,17);                   // 音声データ用の変数の初期化
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    voice.sample();
    char p=voice.getPhoneme();
    if(p!=' '){
        Serial.print(p);
        if(phoneme_i > 0 && p==phoneme[phoneme_i-1]) return;    // 同音時に戻る
        digitalWrite(PIN_LED,!LED);
        t=0;
        phoneme[phoneme_i] = p;
        phoneme_i++;
        if(phoneme_i<16) return;
    }else{
        if(phoneme_i && t==0) Serial.print(' ');
        t++;
        digitalWrite(PIN_LED,LED);
        if(t < 16 || phoneme_i == 0) return;
    }
    
    /* 認識部 */
    char *spo, *sph, *spf, *spv;
    char vr[9];                             // 認識結果の保存用変数
    int led=-1;                             // 認識結果の保存用変数
    memset(vr,0,9);                         // 認識結果の初期化
    spo=strchr(phoneme,'o');                // 音声データ内に「o」が含まれる
    sph=strchr(phoneme,'h');                // 音声データ内に「h」が含まれる
    spf=strchr(phoneme,'f');                // 音声データ内に「f」が含まれる
    spv=strchr(phoneme,'v');                // 音声データ内に「v」が含まれる
    if(spo>0){
        if(spo<spf || spo<spv) led=0;       // 「オフ」と判定
        else led=1;                         // 「オン」と判定
    }
    if(sph>0){
        if(sph<spf || sph<spv) led=0;       // 「オフ」と判定
        else led=1;                         // 「オン」と判定
    }
    digitalWrite(PIN_LED,led);
    if(led==0) strcpy(vr,",OFF");
    else if(led==1) strcpy(vr,",ON");
    else strcpy(vr,",ERR");
    Serial.println(vr);
    t=0;
    phoneme_i=0;
    memset(phoneme,0,17);                   // 音声データ用の変数の初期化
    if(led==LED || led<0) return;
    LED=led;
    
    /* 送信部 */
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));            // LEDの点滅
        delay(100);                         // 待ち時間
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(LED);                         // ON=1、OFF=0を送信
    udp.println(vr);                        // 認識結果の文字列vrを送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);
    WiFi.disconnect(true);                  // WiFiアクセスポイントを切断する
}
