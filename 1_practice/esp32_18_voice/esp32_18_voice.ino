/*******************************************************************************
Practice esp32 18 voice 【簡易音声認識子機】

                                           Copyright (c) 2017-2018 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "uspeech.h"                        // 音声認識ライブラリ uspeech
#define PIN_LED 2                           // IO 2へLEDを接続
#define PIN_AIN 33                          // IO 33へマイクロホンを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
#define DEVICE "voice_1,"                   // デバイス名(5文字+"_"+番号+",")

signal voice(PIN_AIN);                      // 音声入力の有効化
char phoneme[17];                           // 音声データ用の変数
int phoneme_i=0,t=0,LED=0;                  // 音声データ数,待ち時間,LED状態保持

void setup(){
    WiFi.mode(WIFI_OFF); btStop();          // 無線LANとBluetoothをOFFに設定する
    pinMode(PIN_LED, OUTPUT);               // LEDを接続したポートを出力に
    digitalWrite(PIN_LED,HIGH);             // LEDを点灯する(起動を知らせる)
    voice.f_enabled = true;                 // f音の検出を行う
    voice.fconstant = 400;                  // f音の閾値設定
    voice.econstant = 1;                    // e音の閾値設定
    voice.aconstant = 2;                    // a音の閾値設定
    voice.vconstant = 3;                    // v音の閾値設定
    voice.shconstant = 4;                   // sh音の閾値設定
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
        t=0; digitalWrite(PIN_LED,!LED);    // 終了待ち時間のリセット
        phoneme[phoneme_i++] = p;           // 認識音を音声データ配列変数へ保存
        if(phoneme_i<16) return;            // 配列16以内ならloopの先頭へ戻る
    }else{                                  // 認識結果が無音だったとき
        if(phoneme_i && t==0) Serial.print(' ');    // 空白をシリアル出力表示
        t++; digitalWrite(PIN_LED,LED);     // 終了待ち時間を1つカウント
        if(t < 16 || phoneme_i == 0)return; // 待ち時間変数tが16未満の時に戻る
    }
    char vr[9]; memset(vr,0,9);             // 認識結果の保存用変数定期と初期化
    int led=-1;                             // 認識結果(未/ON/OFF)の保存用変数
    char *spo=strchr(phoneme,'o');          // 音声データ内に「o」が含まれる
    char *sph=strchr(phoneme,'h');          // 音声データ内に「h」が含まれる
    char *spf=strchr(phoneme,'f');          // 音声データ内に「f」が含まれる
    char *spv=strchr(phoneme,'v');          // 音声データ内に「v」が含まれる
    if(spo>0){                              // 「o」が含まれていたとき
        if(spo<spf || spo<spv) led=0;       // 「f」「v」が後にあればledを0へ
        else led=1;                         // なければONと判定し変数ledを1へ
    }
    if(sph>0){                              // 「h」が含まれていたとき
        if(sph<spf || sph<spv) led=0;       // 「f」「v」が後にあればledを0へ
        else led=1;                         // なければONと判定し変数ledを1へ
    }
    if(led>=0) digitalWrite(PIN_LED,led);   // 判定結果でLEDを制御
      else strcpy(vr,",ERR");               // 結果無し時は文字列変数へERRを代入
    if(led==0) strcpy(vr,",OFF");           // led=0のとき文字列変数へOFFを代入
      else if(led==1) strcpy(vr,",ON");     // led=1のとき文字列変数へONを代入
    Serial.println(vr);                     // 文字列変数をシリアル出力表示
    t=0; phoneme_i=0; memset(phoneme,0,17); // 時間、音声データ用の変数の初期化
    if(led==LED || led<0) return;           // LED状態に変化がないときに戻る
    LED=led;                                // LED状態を保持する変数LEDの更新
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));            // LEDの点滅
        delay(100);                         // 待ち時間
    } Serial.println(WiFi.localIP());       // 本機のIPアドレスをシリアル出力
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(LED);                         // ON=1、OFF=0を送信
    udp.println(vr);                        // 認識結果の文字列vrを送信
    udp.endPacket(); delay(10);             // UDP送信の終了(実際に送信する)
    WiFi.disconnect(true);                  // WiFiアクセスポイントを切断する
    WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定する
}
