/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の温湿度センサの値を読み取る
SENSIRION社 SHT31
                               Copyright (c) 2017-2019 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_sht 0x45            // SHT31 の I2C アドレス 
// #define I2C_sht 0x44         // M5用SHT30 の I2C アドレス 

float _i2c_sht31_hum;

float sht31_getTemp(){
    int temp,hum;
    _i2c_sht31_hum=-999.;
    Wire.beginTransmission(I2C_sht);
    Wire.write(0x2C);
    Wire.write(0x06);
    delay(18);                  // 15ms以上
    if( Wire.endTransmission() ) return -999.;
    if( Wire.requestFrom(I2C_sht,6) >= 5){
        temp = Wire.read();
        temp <<= 8;
        temp += Wire.read();
        Wire.read();
        hum = Wire.read();
        hum <<= 8;
        hum += Wire.read();
        Wire.read();
        _i2c_sht31_hum = (float)hum / 65535. * 100.;
        return (float)temp / 65535. * 175. - 45.;
    }else return -999.;
}

float sht31_getHum(){
    return _i2c_sht31_hum;
}

boolean sht31_Setup(){
    delay(2);                   // 1ms以上
    Wire.begin();               // I2Cインタフェースの使用を開始
    delay(18);                  // 15ms以上
    float temp = sht31_getTemp();
    if(temp >= -45) return true; else return false;
}
