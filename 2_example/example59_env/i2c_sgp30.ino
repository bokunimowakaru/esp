/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の温湿度センサの値を読み取る
SENSIRION社 SGP30

intは32bitで動作確認（16bit時の符号ビット未考慮）

                               Copyright (c) 2021 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_sgp 0x58            // sgp30 の I2C アドレス 

uint16_t _i2c_sgp30_tvoc;

int sgp30_getCo2(){
    uint16_t co2;
    _i2c_sgp30_tvoc=0;
    Wire.beginTransmission(I2C_sgp);
    Wire.write(0x20);           // sgp30_measure_iaq 
    Wire.write(0x08);           //            0x2008
    if( Wire.endTransmission() ) return -1;
    delay(14);                  // typ. 10ms, max. 12ms
    if( Wire.requestFrom(I2C_sgp,6) >= 5){
        co2 = Wire.read();
        co2 <<= 8;
        co2 += Wire.read();
        Wire.read();            // CRCの読み捨て
        _i2c_sgp30_tvoc = Wire.read();
        _i2c_sgp30_tvoc <<= 8;
        _i2c_sgp30_tvoc += Wire.read();
        return (int)co2;
    }else return -2;
}

int sgp30_getTvoc(){
    int tvoc = (int)_i2c_sgp30_tvoc;
    if(_i2c_sgp30_tvoc == 65535) return -3;
    _i2c_sgp30_tvoc = 65535;
    return (int)tvoc;
}

float sgp30_CalcAbsHumid(float T, float RH){
/*
Absolute humidity values dV in unit g/m3 can be calculated by the following formula:
dv(T, RH)=216.7∙ [RH/100% ∙ 6.112 ∙ exp(17.62 ∙ T/(243.12 + T))/(273.15+T))]
出典：https://www.sensirion.com/jp/download-center/gas-sensors/sgp30/
*/
	return 216.7 * (RH/100*6.112*exp(17.62*T/(243.12+T)) / (273.15+T));
}

uint8_t gencrc(uint8_t *data, size_t len){
/*
How to calculate crc8 in C? (stack overflow)
出典：https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c
*/
    uint8_t crc = 0xFF;								// SGP30のInitialization 0xFF
    size_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);	// SGP30のPolynomial 0x31
            else
                crc <<= 1;
        }
    }
    return crc;
}


int sgp30_setHumid(float T, float RH){
/*
The 2 data bytes represent humidity values as a fixed-point 8.8bit number
with a minimum value of 0x0001 (=1/256 g/m3) and a maximum value of 0xFFFF
(255 g/m3 + 255/256 g/m3). 
For instance, sending a value of 0x0F80 corresponds to a humidity value of 
15.50 g/m3 (15 g/m3 + 128/256 g/m3).
出典：https://www.sensirion.com/jp/download-center/gas-sensors/sgp30/
*/
    uint16_t ahum_u16;
    byte crc, data[2];
    
    ahum_u16 = (uint16_t)(256. * sgp30_CalcAbsHumid(T, RH));
    data[0] = (byte)(ahum_u16 >> 8);
    data[1] = (byte)(ahum_u16 % 0xFF);
    crc = gencrc(data,2);
    Wire.beginTransmission(I2C_sgp);
    Wire.write(0x20);           // sgp30_set_absolute_humidity
    Wire.write(0x61);           //            0x2061
    Wire.write(data[0]);
    Wire.write(data[1]);
    Wire.write(crc);
    if( Wire.endTransmission() ) return -1;
    delay(12);                  // 10ms(設定待ち)以上
    return (int)ahum_u16;
}

void sgp30_Setup(){
    delay(1);                   // 0.6ms以上
    Wire.begin();               // I2Cインタフェースの使用を開始
    delay(1);                   // 0.6ms以上
    Wire.beginTransmission(I2C_sgp);
    Wire.write(0x20);           // sgp30_iaq_init
    Wire.write(0x03);           //            0x2003
    Wire.endTransmission();
    delay(1012);                // 10ms(設定待ち) + 1000 ms(測定間隔)
}
