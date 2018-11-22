/*******************************************************************************
Raspberry Pi用 Arduino用 ESP8266用 ESP32用
I2C 温湿度気圧センサ Bosch BME280 (BMP280) raspi_bme280

消費電力が大きい場合は、ESP8266ライブラリのバージョンを2.3.0に変更してください。
Arduino IDEの[ツール]→[ボード]→[ボードマネージャ]で表示されるリストの中から、
「esp8266 by ESP8266 Community」を選択するとバージョンの変更が行えます。

BME280 温度 湿度 気圧
BMP280 温度 気圧

本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続のセンサから測定値を取得する

参考文献：Bosch BME280データシート・データシート上のサンプルソースコード

                                        Copyright (c) 2016-2019 Wataru KUNINO
                                        https://bokunimo.net/bokunimowakaru/
*******************************************************************************/

// usage: raspi_bme280 [address]
//                      0x76    Lowの時
//                      0x77    HIghの時
//
// The last bit is changeable by SDO value and can be changed during operation.
// Connecting SDO to GND results in slave address 1110110 (0x76); 
// connection it to VDDIO results in slave address 1110111 (0x77), 
// which is the same as BMP280’s I²C address.

#ifndef ARDUINO
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "../libs/soft_i2c.h"
#else
	#include <Wire.h>
#endif
typedef uint8_t byte; 
uint8_t I2C_bme280=0x76;

// #define DEBUG

// BST-BME280-DS001-11 | Revision 1.2 | October 2015 Bosch Sensortec

/* singed integer type*/
typedef	int8_t s8;/**< used for signed 8bit */
typedef	int16_t s16;/**< used for signed 16bit */
typedef	int32_t s32;/**< used for signed 32bit */
typedef	int64_t s64;/**< used for signed 64bit */

typedef	uint8_t u8;/**< used for unsigned 8bit */
typedef	uint16_t u16;/**< used for unsigned 16bit */
typedef	uint32_t u32;/**< used for unsigned 32bit */
typedef	uint64_t u64;/**< used for unsigned 64bit */

u16 dig_T1;/**<calibration T1 data*/
s16 dig_T2;/**<calibration T2 data*/
s16 dig_T3;/**<calibration T3 data*/
u16 dig_P1;/**<calibration P1 data*/
s16 dig_P2;/**<calibration P2 data*/
s16 dig_P3;/**<calibration P3 data*/
s16 dig_P4;/**<calibration P4 data*/
s16 dig_P5;/**<calibration P5 data*/
s16 dig_P6;/**<calibration P6 data*/
s16 dig_P7;/**<calibration P7 data*/
s16 dig_P8;/**<calibration P8 data*/
s16 dig_P9;/**<calibration P9 data*/

u8	dig_H1;/**<calibration H1 data*/
s16 dig_H2;/**<calibration H2 data*/
u8	dig_H3;/**<calibration H3 data*/
s16 dig_H4;/**<calibration H4 data*/
s16 dig_H5;/**<calibration H5 data*/
s8	dig_H6;/**<calibration H6 data*/

s32 t_fine;/**<calibration T_FINE data*/
	
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t BME280_compensate_T_int32(int32_t adc_T){
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) *
	((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t BME280_compensate_P_int64(int32_t adc_P){
	int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
	if (var1 == 0) return 0; // avoid exception caused by division by zero
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
	return (uint32_t)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
uint32_t bme280_compensate_H_int32(int32_t adc_H){
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
	((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r *
	((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
}

int _bme280_setByte(byte reg, byte data){
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_bme280);
		Wire.write(reg);
		Wire.write(data);
		return (int)(Wire.endTransmission());
	#else
		byte config[2];
		config[0]=reg;
		config[1]=data;
		return !i2c_write(I2C_bme280,config,2);
	#endif
}

uint16_t _bme280_getReg(byte reg){
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_bme280);
		Wire.write(reg);
		if( Wire.endTransmission()==0){
			delay(1);
			Wire.requestFrom((int)I2C_bme280,(int)1);
			if(Wire.available()==0) return -2;
			return Wire.read();
		}
		return -1;
	#else
		byte data;
		i2c_write(I2C_bme280,&reg,1);	   // 書込みの実行
		delay(1);
		i2c_read(I2C_bme280,&data,1);	   // 読み出し
		return (int)data;
	#endif
}

void _bme280_cal(){
	dig_T1 = (u16)(_bme280_getReg(0x88) + (_bme280_getReg(0x89)<<8));
	dig_T2 = (s16)(_bme280_getReg(0x8A) + (_bme280_getReg(0x8B)<<8));
	dig_T3 = (s16)(_bme280_getReg(0x8C) + (_bme280_getReg(0x8D)<<8));
	dig_P1 = (u16)(_bme280_getReg(0x8E) + (_bme280_getReg(0x8F)<<8));
	dig_P2 = (s16)(_bme280_getReg(0x90) + (_bme280_getReg(0x91)<<8));
	dig_P3 = (s16)(_bme280_getReg(0x92) + (_bme280_getReg(0x93)<<8));
	dig_P4 = (s16)(_bme280_getReg(0x94) + (_bme280_getReg(0x95)<<8));
	dig_P5 = (s16)(_bme280_getReg(0x96) + (_bme280_getReg(0x97)<<8));
	dig_P6 = (s16)(_bme280_getReg(0x98) + (_bme280_getReg(0x99)<<8));
	dig_P7 = (s16)(_bme280_getReg(0x9A) + (_bme280_getReg(0x9B)<<8));
	dig_P8 = (s16)(_bme280_getReg(0x9C) + (_bme280_getReg(0x9D)<<8));
	dig_P9 = (s16)(_bme280_getReg(0x9E) + (_bme280_getReg(0x9F)<<8));
	dig_H1 = (u8)(_bme280_getReg(0xA1));
	dig_H2 = (s16)(_bme280_getReg(0xE1) + (_bme280_getReg(0xE2)<<8));
	dig_H3 = (u8)(_bme280_getReg(0xE3));
	dig_H4 = (s16)((_bme280_getReg(0xE4)<<4) + (_bme280_getReg(0xE5)&0x0F));
	dig_H5 = (s16)(((_bme280_getReg(0xE5)&0xF0)>>4) + (_bme280_getReg(0xE6)<<4));
	dig_H6 = (s8)(_bme280_getReg(0xE7));
}

float bme280_getTemp(){
	int32_t in;
	in = _bme280_getReg(0xFA);			  		// temp_msb[7:0]
	in <<= 8;
	in |= _bme280_getReg(0xFB); 				// temp_lsb[7:0]
	in <<= 4;
	in |= _bme280_getReg(0xFC); 				// temp_xlsb[3:0]
//	  printf("getTemp  %08X %d\n",in,in);
	return ((float)BME280_compensate_T_int32(in))/100.;
}


float bme280_getHum(){
	int32_t in;
	in = _bme280_getReg(0xFD);			  		// hum_msb[7:0]
	in <<= 8;
	in |= _bme280_getReg(0xFE); 				// hum_lsb[7:0]
//	printf("getHum   %08X\n",in);
	return ((float)bme280_compensate_H_int32(in))/1024.;
}

float bme280_getPress(){
	int32_t in;
	in = _bme280_getReg(0xF7);					// press_msb[7:0]
	in <<= 8;
	in |= _bme280_getReg(0xF8); 	 			// press_lsb[7:0]
	in <<= 4;
	in |= _bme280_getReg(0xF9);				// press_xlsb[3:0]
//	printf("getPress %08X\n",in);
	return ((float)BME280_compensate_P_int64(in))/25600.;
}

int bme280_init(){
	byte reg,data,in;
	int i;
	
	#ifdef ARDUINO
		Wire.begin();
	#else
		i2c_init();
	#endif
	
	_bme280_cal();
	
	reg= 0xF5;				   	// config
//	data=0b11000000;
	data=0b00000000;
	//	   | || | |___________________ 触るな SCI切換え
	//	   | ||_|_____________________ filter[2:0]
	//	   |_|________________________ t_sb[2:0]
	if(_bme280_setByte(reg,data)){		// 書込みの実行
		#ifdef ARDUINO
			Serial.println("ERROR(11): i2c writing config reg");
		#else
			fprintf(stderr,"ERROR(11): i2c writing config reg\n");
		#endif
		return 11;
	}
	
	reg= 0xF2;				   	// trl_hum
	data=0b00000001;
	//			|_|___________________ osrs_h[2:0]
	if(_bme280_setByte(reg,data)){		// 書込みの実行
		#ifdef ARDUINO
			Serial.println("ERROR(12): i2c writing trl_hum reg");
		#else
			fprintf(stderr,"ERROR(12): i2c writing trl_hum reg\n");
		#endif
		return 12;
	}
	
	reg= 0xF4;				   	// ctrl_meas
	data=0b00100111;
	//	   | || |||___________________ mode[1:0]
	//	   | ||_|_____________________ osrs_p[2:0]
	//	   |_|________________________ osrs_t[2:0]
	if(_bme280_setByte(reg,data)){		// 書込みの実行
		#ifdef ARDUINO
			Serial.println("ERROR(13): i2c writing ctrl_meas reg");
		#else
			fprintf(stderr,"ERROR(13): i2c writing ctrl_meas reg\n");
		#endif
		return 13;
	}	 
	in=_bme280_getReg(0xD0);
	if(in != 0x58 && in != 0x60){
		#ifdef ARDUINO
			Serial.print("ERROR(21):  chip_id = 0x");
			Serial.println(in,HEX);
		#else
			fprintf(stderr,"ERROR(21):  chip_id (%02X)\n",in);
		#endif
		return 21;
	}
	/*
	for(i=0;i<50;i++){
		in=_bme280_getReg(0xF3);
		#ifdef DEBUG
			#ifdef ARDUINO
				Serial.print("getReg 0x");
				Serial.println(in,HEX);
			#else
				printf("getReg   %02X\n",in);
			#endif
		#endif
		if((in&0x04)==0) break;
		delay(20);
	}
	if(i==50){
		#ifdef ARDUINO
			Serial.println("ERROR(31): failed to read results");
		#else
			fprintf(stderr,"ERROR(31): failed to read results\n");
		#endif
		return 31;
	}
	*/
	delay(20);
	return 0;
}

int bme280_stop(){
	byte reg,data;
	int ret;
	reg= 0xF4;					   	// ctrl_meas
	data=0x00;
	ret=_bme280_setByte(reg,data);	// 書込みの実行
	#ifndef ARDUINO
		i2c_close();
	#endif
	return ret;
}

void bme280_print(float temp, float hum, float press){
	#ifdef ARDUINO
		Serial.print("Temp ="); Serial.println(temp,2);
		Serial.print("Humi ="); Serial.println(hum,2);
		Serial.print("Press="); Serial.println(press,2);
	#else
		printf("%3.2f ",temp);
		printf("%3.2f ",hum);
		printf("%4.2f\n",press);
	#endif
}

#ifndef ARDUINO
int main(int argc,char **argv){
	if( argc == 2 ) I2C_bme280=(byte)strtol(argv[1],NULL,16);
	if( I2C_bme280>=0x80 ) I2C_bme280>>=1;
	if( argc < 1 || argc > 2 ){
		fprintf(stderr,"usage: %s [I2C_bme280]\n",argv[0]);
		return -1;
	}
	#ifdef DEBUG
		printf("I2C_bme280 =0x%02X\n",I2C_bme280);
	#endif

	bme280_init();
	bme280_print(bme280_getTemp(),bme280_getHum(),bme280_getPress());
	bme280_stop();
	return 0;
}
#endif
