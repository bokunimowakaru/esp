/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の温湿度センサの値を読み取る
SILICON LABS社 Si7021
                               Copyright (c) 2017-2019 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_si7021 0x40            // Si7021 の I2C アドレス 

float _i2c_si7021_hum;

float getTemp(){
    int temp,hum;
    _i2c_si7021_hum=-999.;
    Wire.beginTransmission(I2C_si7021);
    Wire.write(0xF5);
    if(Wire.endTransmission()) return -999.;
    
    delay(30);                  // 15ms以上
    Wire.requestFrom(I2C_si7021,2);
    if(Wire.available()!=2) return -999.;
    hum = Wire.read();
    hum <<= 8;
    hum += Wire.read();
    
    delay(18);                  // 15ms以上
    Wire.beginTransmission(I2C_si7021);
    Wire.write(0xE0);
    if(Wire.endTransmission()) return -989.;
    
    delay(30);                  // 15ms以上
    Wire.requestFrom(I2C_si7021,2);
    if(Wire.available()!=2) return -999.;
    temp = Wire.read();
    temp <<= 8;
    temp += Wire.read();

    _i2c_si7021_hum = (float)hum / 65536. * 125. - 6.;
    return (float)temp / 65535. * 175.72 - 46.85;
}

float getHum(){
    return _i2c_si7021_hum;
}

void si7021Setup(){
    delay(2);                   // 1ms以上
    Wire.begin();               // I2Cインタフェースの使用を開始
    delay(18);                  // 15ms以上
    Wire.beginTransmission(I2C_si7021);
    Wire.write(0xE6);
    Wire.write(0x3A);
    Wire.endTransmission();
    delay(18);                  // 15ms以上
}
