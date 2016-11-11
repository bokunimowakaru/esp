/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の温湿度センサの値を読み取る
TI社 HDC1000
                               Copyright (c) 2015-2016 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_hdc 0x40			// HDC1000 の I2C アドレス 

float getTemp(){
	int ret;
	Wire.beginTransmission(I2C_hdc);
	Wire.write(0x00);			// 温度レジスタ 00
	if( Wire.endTransmission() == 0){
		delay(9);					// 6.5ms以上
		Wire.requestFrom(I2C_hdc,2);
		if(Wire.available()==0) return -999.;
		ret = Wire.read();
		ret <<= 8;
		if(Wire.available()==0) return -999.;
		ret += Wire.read();
		return (float)ret / 65536. * 165. - 40.;
	}else return -999.;
}

float getHum(){
	int ret;
	Wire.beginTransmission(I2C_hdc);
	Wire.write(0x01);			// 湿度レジスタ 01
	if( Wire.endTransmission() == 0){
		delay(9);					// 6.5ms以上
		Wire.requestFrom(I2C_hdc,2);
		if(Wire.available()==0) return -999.;
		ret = Wire.read();
		ret <<= 8;
		if(Wire.available()==0) return -999.;
		ret += Wire.read();
		return (float)ret / 65536. * 100.;
	}else return -999.;
}

void hdcSetup(){
    Wire.begin();				// I2Cインタフェースの使用を開始
	delay(18);					// 15ms以上
	Wire.beginTransmission(I2C_hdc);
	Wire.write(0x02);			// 設定レジスタ 02
	Wire.write(0x00);
	Wire.write(0x00);
	Wire.endTransmission();
}
