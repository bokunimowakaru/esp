/*******************************************************************************
IoT SensorShield EVK BLE

    気圧センサ              ROHM BM1383AGLV
    加速度センサ            ROHM KX224
    地磁気センサ            ROHM BM1422AGMV
    照度・近接一体型センサ  ROHM RPR-0521RS
    カラーセンサ            ROHM BH1749NUC

乾電池などで動作するIoTセンサです

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
//#include <WiFi.h>                         // ESP32用WiFiライブラリ
#include <BLEDevice.h>
#include <BLEServer.h>
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2 にLEDを接続
#define PIN_BUZZER 12                       // GPIO 12 にスピーカを接続
#define SLEEP_P 5*1000000ul                 // スリープ時間 5秒(uint32_t)
#define BLE_DEVICE "espRohm"                // BLE用デバイス名

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
#define DAT_N 18                            // 送信データ用バイト数

RTC_DATA_ATTR byte SEQ_N = 0;               // 送信番号

void setup(){                               // 起動時に一度だけ実行する関数
//  WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT SensorShield EVK"); // 「IoT SensorShield EVK」を表示
    int wake = TimerWakeUp_init();
    BLEDevice::init(BLE_DEVICE);            // Create the BLE Device
    Wire.begin();
    if(wake<2){
        ledcWriteNote(0,NOTE_B,8);          // 送信中の音
        bm1383aglv.init();                  // 気圧センサの初期化
        kx224.init();                       // 加速度センサの初期化
        bh1749nuc.init();                   // カラーセンサの初期化
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
    Serial.println(val[1]);
    
    Serial.print("Illuminance     = ");
    Serial.print(val[2]);
    Serial.println(" [lx]");
    Serial.print("Proximity       = ");
    Serial.print(val[3],0);
    Serial.println(" [count]");
    Serial.print("Color (RED)     = ");
    Serial.println(val[4],1);
    Serial.print("Color (GREEN)   = ");
    Serial.println(val[5],1);
    Serial.print("Color (BLUE)    = ");
    Serial.println(val[6],1);
    Serial.print("Color (IR)      = ");
    Serial.println(val[7],1);
//  Serial.print("Color (GREEN2)  = ");
//  Serial.println(val[8],0);
    
    Serial.print("Accelerometer X = ");
    Serial.print(val[8]);
    Serial.println(" [g]");
    Serial.print("Accelerometer Y = ");
    Serial.print(val[9]);
    Serial.println(" [g]");
    Serial.print("Accelerometer Z = ");
    Serial.print(val[10]);
    Serial.println(" [g]");
    
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

    bm1383aglv.get_val(&val[1],val);        // 気圧と温度を取得
    
    v=(long)((val[0] + 15.) * 4.);          // 温度
    if( v > 255 ) v = 255;
    if( v < 0 ) v = 0;
    l=d_append(d,l,(byte)v);
//  v=(long)((val[0] + 45.) * 374.5);
//  l=d_append_int16(d,l,v);
    v=(long)(val[1] - 1027.);
    if( v > 127 ) v = 127;
    if( v < -128 ) v = -128;
    l=d_append(d,l,(char)((int)v));
//  v=(long)(val[1]);
//  l=d_append_int24(d,l,v);                // 気圧
    
    unsigned short ps;
    rpr0521rs.get_psalsval(&ps,&val[2]);    // 距離、照度を取得
    val[3] = (float)ps;
    if(ps > 255) ps = 255;
    v=(long)(val[2] * 1.2);
    l=d_append_int16(d,l,v);
    l=d_append(d,l,(byte)ps);
    
    unsigned short color[5];
    long color_n = 0;
    bh1749nuc.get_val(color);
    for(int i=0;i<4;i++) color_n += color[i];
    for(int i=0;i<4;i++){
        val[4+i] = (float)color[i] / (float)color_n * 100.;
        if(i<3)l=d_append(d,l,(byte)(val[4+i] * 256. / 100.));
    }
    
    kx224.get_val(&val[8]);                 // 加速度を取得
    for(int i=0;i<3;i++){
        v=(long)(val[8+i] * 64);
        if(v>127) v=127;
        if(v<-128) v=-128;
        l=d_append(d,l,(char)((int)v));
    }
    
    bm1422agmv.get_val(&val[11]);           // 地磁気を取得
    for(int i=0;i<3;i++){
        v=(long)(val[11+i]);
        if(v>127) v=127;
        if(v<-128) v=-128;
        l=d_append(d,l,(char)((int)v));
    }
    
    l=d_append(d,l,SEQ_N);                  // 送信番号

    sensors_log(val);
    
    // BLE Advertizing
    pAdvertising = BLEDevice::getAdvertising();
    setBleAdvData(d,l);
    pAdvertising->start();                  // Start advertising
    SEQ_N++;
    sleep();
}

void sleep(){
    delay(100);                             // 送信待ち時間
    ledcWrite(0, 0);
    pAdvertising->stop();
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

void setBleAdvData(byte *data, int data_n){
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
    oAdvertisementData.setName(BLE_DEVICE);
    oAdvertisementData.setFlags(0x06);      // LE General Discoverable Mode | BR_EDR_NOT_SUPPORTED
    
    std::string strServiceData = "";
    strServiceData += (char)(data_n+3);     // Len
    strServiceData += (char)0xFF;           // Manufacturer specific data
    strServiceData += (char)0x01;           // Company Identifier(2 Octet)
    strServiceData += (char)0x00;
    for(int i=0;i<data_n;i++) strServiceData += (char)(data[i]);

    oAdvertisementData.addData(strServiceData);
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
