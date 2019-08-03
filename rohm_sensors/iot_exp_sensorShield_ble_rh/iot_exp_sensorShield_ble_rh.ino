/*******************************************************************************
IoT SensorShield EVK BLE RH 【スマホ用アプリ ROHM RHRawDataMedal2 対応版】

    気圧センサ              ROHM BM1383AGLV
    加速度センサ            ROHM KX224
    地磁気センサ            ROHM BM1422AGMV
    照度・近接一体型センサ  ROHM RPR-0521RS
    カラーセンサ            ROHM BH1749NUC

乾電池などで動作するIoTセンサです
スマホ用アプリRHRawDataMedal2で受信することが出来ます

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/* 
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2 にLEDを接続
#define PIN_BUZZER 12                       // GPIO 12 にスピーカを接続
#define SLEEP_P 5*1000000ul                 // スリープ時間 5秒(uint32_t)
#define BLE_DURATION 300                    // BLEアドバタイズ時間
#define BLE_DEVICE "R"                      // BLE用デバイス名

#include <Wire.h>
#include "BM1383AGLV.h"
#include "KX224.h"
#include "BM1422AGMV.h"
#include "RPR-0521RS.h"
#include "BH1749NUC.h"

BM1383AGLV  bm1383aglv;
KX224       kx224(0x1E);
BM1422AGMV  bm1422agmv(0x0E);
RPR0521RS   rpr0521rs;
BH1749NUC   bh1749nuc(0x39);

BLEAdvertising *pAdvertising;

#define VAL_N 14                            // 送信データ項目数
#define DAT_N 23                            // 送信データ用バイト数

RTC_DATA_ATTR byte SEQ_N = 0;               // 送信番号
int wake;

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // LEDを出力に
    digitalWrite(PIN_EN,1);                 // LEDをON
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT SensorShield EVK"); // 「IoT SensorShield EVK」を表示
    wake = TimerWakeUp_init();
    BLEDevice::init(BLE_DEVICE);            // Create the BLE Device
    Wire.begin();
    if(wake<2){
        ledcWriteNote(0,NOTE_B,7);          // 初期設定中の音
        bm1383aglv.init();                  // 気圧センサの初期化
        kx224.init();                       // 加速度センサの初期化
        ledcWrite(0, 0);
    }
    rpr0521rs.init();                       // 照度・近接一体型センサの初期化
    bm1422agmv.init();                      // 地磁気センサの初期化
    ledcWriteNote(0,NOTE_D,8);              // 送信中の音
}

int d_append(byte *array,int i, byte d){
    if(i >= DAT_N) return i;
    array[i]=d;
    return i+1;
}

int d_append_int16(byte *array,int i, int16_t d){
    if(i+1 >= DAT_N) return i;
    array[i] = (byte)(d & 0xFF); 
    array[i+1] = (byte)(d >> 8);
    return i+2;
}

int d_append_int24(byte *array,int i, int32_t d){
    if(i+2 >= DAT_N) return i;
    array[i] = (byte)(d & 0xFF); 
    array[i+1] = (byte)((d >>8)&0xFF);
    array[i+2] = (byte)((d >>16)&0xFF);
    return i+3;
}

void sensors_log(float *val){
    Serial.print("Temperature    =  ");
    Serial.print(val[0]);
    Serial.println(" [degrees Celsius]");
    Serial.print("Pressure        = ");
    Serial.print(val[1]);
    Serial.println(" [hPa]");
    
    Serial.print("Illuminance     = ");
    Serial.print(val[2]);
    Serial.println(" [lx]");
    Serial.print("Proximity       = ");
    Serial.print(val[3],0);
    Serial.println(" [count]");
    
    Serial.print("Accelerometer X = ");
    Serial.print(val[8]);
    Serial.println(" [m/s^2]");
    Serial.print("Accelerometer Y = ");
    Serial.print(val[9]);
    Serial.println(" [m/s^2]");
    Serial.print("Accelerometer Z = ");
    Serial.print(val[10]);
    Serial.println(" [m/s^2]");
    
    Serial.print("Geomagnetic X   = ");
    Serial.print(val[11], 3);
    Serial.println("[uT]");
    Serial.print("Geomagnetic Y   = ");
    Serial.print(val[12], 3);
    Serial.println("[uT]");
    Serial.print("Geomagnetic Z   = ");
    Serial.print(val[13], 3);
    Serial.println("[uT]");
}

void loop() {
    byte d[DAT_N]; 
    float val[VAL_N];                       // センサ用の浮動小数点数型変数
    long v;
    int l=0;
    
    for(int i=0;i<VAL_N;i++) val[i]=0.;
    
    // 気圧と温度を取得
    if(!bm1383aglv.get_val(&val[1],val)){
        v=(long)round( (val[0] + 45.) * 65536. / 175.);
        l=d_append_int16(d,l,v);
    }else{
        l=d_append_int16(d,l,0);
    }

    // 距離、照度を取得
    unsigned short ps;
    if(!rpr0521rs.get_psalsval(&ps,val+2)){
        val[3] = (float)ps;
        v=(long)round(val[2] * 1.2);    // 照度を送信(本来は湿度)
        l=d_append_int16(d,l,v);
    }else{
        l=d_append_int16(d,l,0);
    }
    
    // 送信番号
    l=d_append(d,l,SEQ_N);
    l=d_append(d,l,0);

    // 加速度を取得
    if(!kx224.get_val(&val[8])){
        for(int i=0;i<3;i++){
            v=(long)round(val[8+i] * 4096);
            l=d_append_int16(d,l,v);
            val[8+i] *= 9.80665;
        }
    }else for(int i=0;i<3;i++) l=d_append_int16(d,l,0);
    
    // 地磁気を取得
    if(!bm1422agmv.get_val(&val[11])){
        for(int i=0;i<3;i++){
            v=(long)round(val[11+i] * 10);
            l=d_append_int16(d,l,v);
        }
    }else for(int i=0;i<3;i++) l=d_append_int16(d,l,0);

    // 気圧を取得
    v=(long)(val[1] * 2048.);
    l=d_append_int24(d,l,v);

    sensors_log(val);
    
    // BLE Advertizing
    pAdvertising = BLEDevice::getAdvertising();
    setBleAdvData(d,l);
    pAdvertising->start();                  // Start advertising
    SEQ_N++;
    sleep();
}

void sleep(){
    delay(150);                             // 送信待ち時間
    if(wake<2) for(int i=0; i<20;i++){
        ledcWrite(0, 0);
        delay(490);
        ledcWriteNote(0,NOTE_D,8);
        delay(10);
    }
    ledcWrite(0, 0);
    digitalWrite(PIN_EN,0);                 // LEDをOFF
    delay(BLE_DURATION);                    // 送信待ち時間
    pAdvertising->stop();
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

void setBleAdvData(byte *data, int data_n){
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
    
    std::string strServiceData = "";
    strServiceData += (char)(data_n+3);     // Len
    strServiceData += (char)0xFF;           // Manufacturer specific data
    strServiceData += (char)0x01;           // Company Identifier(2 Octet)
    strServiceData += (char)0x00;
    for(int i=0;i<data_n;i++) strServiceData += (char)(data[i]);

    oAdvertisementData.addData(strServiceData);
    oAdvertisementData.setFlags(0x06);      // LE General Discoverable Mode | BR_EDR_NOT_SUPPORTED
    oAdvertisementData.setName(BLE_DEVICE); // oAdvertisementDataは逆順に代入する
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanResponseData);

    Serial.print("data            = ");
    int len=strServiceData.size();
    if(len != (int)(strServiceData[0]) + 1 || len < 2) Serial.println("ERROR: BLE length");
    for(int i=2;i<len;i++) Serial.printf("%02x ",(char)(strServiceData[i]));
    Serial.println();
    Serial.print("data length     = 2 + ");
    Serial.printf("%d = %d (%d)\n",data_n,len-2,22-strlen(BLE_DEVICE)-data_n);
}
