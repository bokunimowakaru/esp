/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の温湿度センサの値を読み取る
SENSIRION社 SGP30
                               Copyright (c) 2021 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_sgp 0x58            // sgp30 の I2C アドレス 

uint16_t _i2c_sgp30_tvoc;

uint16_t sgp30_getCo2(){
    uint16_t co2;
    _i2c_sgp30_tvoc=0;
    Wire.beginTransmission(I2C_sgp);
    Wire.write(0x20);           // sgp30_measure_iaq 
    Wire.write(0x08);           //            0x2008
    if( Wire.endTransmission() ) return 0;
    delay(18);                  // 規定なし
    if( Wire.requestFrom(I2C_sgp,6) >= 5){
        co2 = Wire.read();
        co2 <<= 8;
        co2 += Wire.read();
        Wire.read();            // CRCの読み捨て
        _i2c_sgp30_tvoc = Wire.read();
        _i2c_sgp30_tvoc <<= 8;
        _i2c_sgp30_tvoc += Wire.read();
        return co2;
    }else return 0;
}

uint16_t sgp30_getTvoc(){
    return _i2c_sgp30_tvoc;
}

void sgp30_Setup(){
    delay(1);                   // 0.6ms以上
    Wire.begin();               // I2Cインタフェースの使用を開始
    delay(1);                   // 0.6ms以上
}
