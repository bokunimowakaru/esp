/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の気圧センサの値を読み取る
STマイクロ社 LPS25H
                               Copyright (c) 2016 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_lps 0x5D			// LPS25H の I2C アドレス 
//#define I2C_lps 0x5C			// LPS25H SDO（Pin4 SA0)がLowの時

int _getReg(int reg){
	Wire.beginTransmission(I2C_lps);
	Wire.write(reg);
	if( Wire.endTransmission() == 0){
		delay(10);
		Wire.requestFrom(I2C_lps,1);
		if(Wire.available()==0) return -2;
		return Wire.read();
	}else return -1;
}

float getTemp(){
	uint16_t ret,in;
	ret = _getReg(0x2C);	// TEMP_OUT_H
	if(ret<0) return -999.;
	ret <<= 8;
	in = _getReg(0x2B);		// TEMP_OUT_L
	if(in<0) return -999.;
	ret += in;
	ret = ~ret + 1;
	return 42.5 - (float)ret / 480.;
}

float getPress(){
	float ret,in;
	in = (float)_getReg(0x2A);	// Press_OUT_H
	if(in<0) return -999.;
	ret = 256. * in;
	in = (float)_getReg(0x29);	// Press_OUT_L
	if(in<0) return -999.;
	ret += in;
	ret *= 256.;
	in = (float)_getReg(0x28);	// Press_OUT_XL
	if(in<0) return -999.;
	ret += in;
	return ret/4096.;
}

int lpsSetup(){
	Wire.begin();				// I2Cインタフェースの使用を開始
	delay(20);
	if(_getReg(0x0F) != 0xBD ) return -1;
	Wire.beginTransmission(I2C_lps);
	Wire.write(0x20);			// CTRL_REG_1
	Wire.write(0x80);			// PD=1 , One Shot Mode
	Wire.endTransmission();
	Wire.beginTransmission(I2C_lps);
	Wire.write(0x21);			// CTRL_REG_2
	Wire.write(0x01);			// One Shot
	Wire.endTransmission();
	return 0;
}

int lpsEnd(){
	Wire.beginTransmission(I2C_lps);
	Wire.write(0x20);			// CTRL_REG_1
	Wire.write(0x00);			// PD=0
	Wire.endTransmission();
	return 0;
}
