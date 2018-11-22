/*******************************************************************************
Raspberry Pi用 Arduino用 ESP8266用 ESP32用
I2C 揮発性有機化合物センサ AMS CCS811 raspi_ccs811.c ccs811.ino

										Copyright (c) 2016-2019 Wataru KUNINO
										https://bokunimo.net/bokunimowakaru/
********************************************************************************

本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続のセンサから測定値を取得する
AMS CCS811 Carbon Monoxide CO VOCs Air Quality Numerical Gas Sensors

I2Cアドレス設定
						0x5A	When ADDR is low
						0x5B	When ADDR is high

揮発性有機化合物 VOCガス
二酸化炭素CO2(相当)

参考文献：AMS CCS811 データシート Ver 1.00
*******************************************************************************/

#ifdef ARDUINO
	#include <Wire.h> 
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "../libs/soft_i2c.h"
	typedef unsigned char byte;
#endif
byte I2C_ccs811=0x5A;						// I2Cアドレス

// #define DEBUG

uint8_t _ccs811_getByte(byte reg){
	uint8_t ret=0xFF;
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_ccs811);
		Wire.write(reg);
		delay(10);								// 書込み時間
		if( Wire.endTransmission()==0){
			Wire.requestFrom((int)I2C_ccs811,1);
			if(Wire.available()==0) return 0xFE;
			ret =Wire.read();
		}
	#else
		i2c_write(I2C_ccs811,&reg,1);	   		// 書込みの実行
		delay(10);
		i2c_read(I2C_ccs811,&ret,1);
		#ifdef DEBUG
		//	printf("rx=%02x\n",rx);
		#endif
	#endif
	return ret;
}

int _ccs811_getReg(byte reg, byte *rx, int len){
	uint8_t tx=reg;
	int i=-1;
	if(len < 0 || len>8) return -1;
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_ccs811);
		Wire.write(reg);
		delay(10);
		if(Wire.endTransmission() == 0){
			Wire.requestFrom((int)I2C_ccs811,len);
			for(i=0;i<len;i++){
				if(Wire.available()==0) break;
				rx[i] =Wire.read();
			}
		}
	#else
		i2c_write(I2C_ccs811,&tx,1);	   		// 書込みの実行
		delay(10);
		i=i2c_read(I2C_ccs811,(byte *)rx,len);
		#ifdef DEBUG
			printf("rx[%d]=",i);
			for(i=0;i<len;i++){
				printf("%02x ",rx[i]);
			}
			printf("\n");
		#endif
	#endif
	return i;
}

int _ccs811_set0Byte(byte reg){			// 書き込み値の無いコマンド(APP_START=0xF4)
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_ccs811);
		Wire.write(reg);
		return (int)(!Wire.endTransmission());
	#else
		uint8_t tx=reg;
		return i2c_write(I2C_ccs811,&tx,1); 	  // 書込みの実行
	#endif
}

int _ccs811_setByte(byte reg, byte data){
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_ccs811);
		Wire.write(reg);
		Wire.write(data);
		return (int)(!Wire.endTransmission());
	#else
		uint8_t tx[2];
		tx[0]=reg;
		tx[1]=data;
		return i2c_write(I2C_ccs811,tx,2); 	  // 書込みの実行
	#endif
}

int _ccs811_setReg(byte reg, byte *data, int len){
	#ifdef ARDUINO
		Wire.beginTransmission(I2C_ccs811);
		Wire.write(reg);
		Wire.write(data, len);
		return (int)(!Wire.endTransmission());
	#else
		uint8_t tx[9];
		int i;
		if(len < 0 || len>8) return -1;
		for(i=0;i<len;i++)tx[i+1]=data[i];
		return i2c_write(I2C_ccs811,tx,len+1);	// 書込みの実行
	#endif

}

int _ccs811_setAppStart(){
	return _ccs811_set0Byte(0xF4);			// 0xF4 APP_START W - Application start.
}

int _ccs811_setMode(byte mode){
	/*
	000: Mode 0 – Idle (Measurements are disabled in this mode)
	001: Mode 1 – Constant power mode, IAQ measurement every second
	010: Mode 2 – Pulse heating mode IAQ measurement every 10 seconds
	011: Mode 3 – Low power pulse heating mode IAQ measurement every 60 seconds
	*/
	if(mode>3) return -1;
	return _ccs811_setByte(0x01,(byte)(mode<<4));	// MEAS_MODE (Measurement and Conditions) Register (0x01)
}

int ccs811_start(){
	return _ccs811_setMode(0x01);
}

int ccs811_stop(){
	return _ccs811_setMode(0x00);
}

int ccs811_getVals(){
	uint8_t rx[8];
	int len,co2;
	#ifdef DEBUG
		int current,adc,i;
	#endif
	
	len=_ccs811_getReg(0x02, rx, 8);		// 0x02 ALG_RESULT_DATA
	if(len != 8){
		#ifdef ARDUINO
			Serial.print("ERROR: ccs811_getVals, len=0x");
			Serial.println(len);
		#else
			fprintf(stderr,"ERROR: ccs811_getVals, len=0x%02X\n",len);
		#endif
		return -1;
	}
	co2 = ((int)rx[0])*256+(int)rx[1];
	#ifdef DEBUG
		i=((int)rx[6])*256+(int)rx[7];
		adc = i & 0x03FF;
		current = i>>10;
		#ifdef ARDUINO
			if(len==8){
				Serial.println("------------------------");
				Serial.println("RESULT_DATA");
				Serial.println("------------------------");
				Serial.print("eCO2        ="); Serial.println( co2 );
				Serial.print("TVOC        ="); Serial.println( ((int)rx[2])*256+(int)rx[3] );
				Serial.print("STATUS      =0x"); Serial.println( rx[4],HEX );
				Serial.print("ERROR_ID    =0x"); Serial.println( rx[5],HEX );
				Serial.print("Current(uA) ="); Serial.println( current );
				Serial.print("Raw ADC     ="); Serial.println( adc );
			}else{
				for(i=0;i<len;i++){
					Serial.print(" 0x");
					Serial.print(rx[i],HEX);
				}
				Serial.println();
			}
		#else
			if(len==8){
				printf("------------------------\n");
				printf("RESULT_DATA\n");
				printf("------------------------\n");
				printf("eCO2        =0x%02X%02X (%d)\n",rx[0],rx[1],((int)rx[0])*256+(int)rx[1]);
				printf("TVOC        =0x%02X%02X (%d)\n",rx[2],rx[3],((int)rx[2])*256+(int)rx[3]);
				printf("STATUS      =0x%02X\n",rx[4]);
				printf("ERROR_ID    =0x%02X\n",rx[5]);
				printf("RAW_DATA    =0x%02X%02X (%d)\n",rx[6],rx[7],i);
				printf("Current     =0x%02X (%d)\n",current,current);
				printf("Raw ADC     =0x%04X (%d)\n",adc,adc);
			}else{
				printf("rx[%d]=",len);
				for(i=0;i<len;i++){
					printf("%02x ",rx[i]);
				}
				printf("\n");
			}
		#endif
	#endif
	return co2;
}

int ccs811_setEnv(float temp, float hum){
	uint8_t tx[4];
	uint16_t val;
	
	val = (uint16_t)(hum * 512.);
	tx[0]= (uint8_t)(val >> 8);
	tx[1]= (uint8_t)(val && 0xFF);
	val = (uint16_t)((temp + 25.) * 512.);
	tx[2]= (uint8_t)(val >> 8);
	tx[3]= (uint8_t)(val && 0xFF);
	return _ccs811_setReg(0x05,tx,4);		// ENV_DATA (Environment Data) Register (0x05)
}

int ccs811_getRAW(){
	uint8_t rx[2];
	int ret=-1,i;
	
	i=_ccs811_getReg(0x03, rx, 2);
	if(i==2) ret=(((int)(rx[0] & 0x03))<<8) | (int)rx[1];
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.print("raw={0x");
			Serial.print(rx[0],HEX);
			Serial.print(",0x");
			Serial.print(rx[1],HEX);
			Serial.print(",");
			Serial.print((int)(rx[0]>>2));	//	Current Selected (0μA to 63μA).
			Serial.print("uA, ");
			Serial.print(ret);	//
			Serial.print("},");
		#endif
	#endif
	return ret;
}

int ccs811_getCO2(){						   // 二酸化炭素濃度（ppm)を取得
	uint8_t rx[6];
	int ret;
	
	ret=_ccs811_getReg(0x02,rx,6);
	if(ret != 6){
		#ifdef ARDUINO
			Serial.print("ERROR: getCO2, len=0x");
			Serial.println(ret);
		#else
	  		fprintf(stderr,"ERROR: getCO2, len=0x%02X\n",ret);
		#endif
		return -1;
	}
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.print("co2={0x");
			Serial.print(rx[0],HEX);	// eCO2 High Byte
			Serial.print(",0x");
			Serial.print(rx[1],HEX);	// eCO2 Low Byte
			Serial.print("},tv={0x");
			Serial.print(rx[2],HEX);	// TVOC High Byte
			Serial.print(",0x");
			Serial.print(rx[3],HEX);	// TVOC Low Byte
			Serial.print("},sta={0x");
			Serial.print(rx[4],HEX);	// STATUS
			Serial.print(",0x");
			Serial.print(rx[5],HEX);	// ERROR_ID
			Serial.print("},");
			Serial.print(ret);
			Serial.print(", ");
		#endif
	#endif
	if(rx[5] != 0x00){
		#ifdef ARDUINO
			Serial.print("ERROR: getCO2, error code=0x");
			Serial.println(rx[5],HEX);
		#else
	  		fprintf(stderr,"ERROR: getCO2, error code=0x%02X\n",rx[5]);
		#endif
		return -1;
	}
	return ((int)rx[0])*256+(int)rx[1];
}

int ccs811_setup(float temp, float hum){
	byte ret,id,status,mode;
	
	#ifdef ARDUINO
	//	Wire.begin();
	#else
		i2c_init();
	#endif
	id=_ccs811_getByte(0x20);			// HW_ID
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.println();
			Serial.print("ccs811_setup ");
			Serial.print(temp);
			Serial.print(",");
			Serial.println(hum);
			Serial.print("HW_ID       =0x"); Serial.println( id,HEX );
		#else
			printf("HW_ID       =0x%02X\n",id);
		#endif
	#endif
	if(id != 0x81){
		#ifdef ARDUINO
			Serial.println("ERROR: unknown device ");
		#else
			fprintf(stderr,"ERROR: unknown device (0x%02X)\n",id);
			printf("-1\n");
			i2c_close();
			exit(-1);
		#endif
		return -1;
	}
	if(temp < -100 || temp > 100) temp=27;
	if(hum <= 0 || hum > 100) hum=50;
	ret=ccs811_setEnv(temp, hum);	// 補正用の温度と湿度を設定
	if(ret==0){
		#ifdef ARDUINO
			Serial.println("ERROR: failed to set env");
		#else
			fprintf(stderr,"ERROR: failed to set env\n");
		#endif
		return -1;
	}
	
	ret=_ccs811_setAppStart();
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.print("AppStart    ="); Serial.println( ret );
		#else
			printf("AppStart    =%1X\n",ret);
		#endif
	#endif
	if(ret==0){
		#ifdef ARDUINO
			Serial.println("ERROR: failed to start app");
		#else
			fprintf(stderr,"ERROR: failed to start app\n");
			printf("-1\n");
			i2c_close();
			exit(-1);
		#endif
		return -1;
	}
	
	status=_ccs811_getByte(0x00);		// STATUS
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.println("------------------------");
			Serial.print("STATUS      =0x"); Serial.println( status, HEX );
			Serial.println("------------------------");
			Serial.print("FW_MODE     =0x"); Serial.println( (status & 0x80) >> 7,HEX);
			Serial.print("APP_VALID   =0x"); Serial.println( (status & 0x10) >> 4,HEX);
			Serial.print("DATA_READY  =0x"); Serial.println( (status & 0x08) >> 3,HEX);
			Serial.print("ERROR_FLAG  =0x"); Serial.println( (status & 0x01),HEX);
			if(status & 0x01){
				Serial.print(" 0x");
				Serial.println(_ccs811_getByte(0xE0),HEX);
			}Serial.println();
		#else
			printf("------------------------\n");
			printf("STATUS      =0x%02X\n",status);
			printf("------------------------\n");
			printf("FW_MODE     =%1X\n",(status & 0x80) >> 7);
			printf("APP_VALID   =%1X\n",(status & 0x10) >> 4);
			printf("DATA_READY  =%1X\n",(status & 0x08) >> 3);
			printf("ERROR       =%1X",(status & 0x01));
			if(status & 0x01){
				printf(" (0x%02X)\n",_ccs811_getByte(0xE0));
			}else printf("\n");
		#endif
	#endif
	if( (status & 0x80) == 0 || (status & 0xFE) == 0xFE ){
		#ifdef ARDUINO
			Serial.println("ERROR: failed APP starting");
		#else
			fprintf(stderr,"ERROR: failed APP starting\n");
			printf("-1\n");
			i2c_close();
			exit(-1);
		#endif
		return -1;
	}
	
	ret= ccs811_start();
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.print("MeasureStart="); Serial.println( ret );
		#else
			printf("MeasureStart=%1X\n",(ret>0));
		#endif
	#endif
	
	delay(1000);
	mode=_ccs811_getByte(0x01); 		// MEAS_MODE
	#ifdef DEBUG
		#ifdef ARDUINO
			Serial.println("------------------------");
			Serial.print("MEAS_MODE   =0x"); Serial.println( mode, HEX );
			Serial.println("------------------------");
			Serial.print("DRIVE_MODE  =0x"); Serial.println( (mode & 0x70) >> 4);
			Serial.print("INT_DATARDY =0x"); Serial.println( (mode & 0x08) >> 3);
			Serial.print("INT_THRESH  =0x"); Serial.println( (mode & 0x04) >> 2);
		#else
			printf("------------------------\n");
			printf("MEAS_MODE   =0x%02X\n",mode);
			printf("------------------------\n");
			printf("DRIVE_MODE  =%1X\n",(mode & 0x70) >> 4);
			printf("INT_DATARDY =%1X\n",(mode & 0x08) >> 3);
			printf("INT_THRESH  =%1X\n",(mode & 0x04) >> 2);
		#endif
	#endif
	if((mode & 0x70) == 0x00){
		#ifdef ARDUINO
			Serial.print("ERROR: failed to set mode");
		#else
			fprintf(stderr,"ERROR: failed to set mode (%d)\n",(mode & 0x70)>>4);
			printf("-1\n");
			i2c_close();
			exit(-1);
		#endif
		return -1;
	}
	return 0;
}

#ifndef ARDUINO
int main(int argc,char **argv){
	int co2=0,prev= 9999,time=0;
	
	if( argc == 2 ) I2C_ccs811=(byte)strtol(argv[1],NULL,16);
	if( I2C_ccs811>=0x80 ) I2C_ccs811>>=1;
	if( argc < 1 || argc > 2 ){
		fprintf(stderr,"usage: %s [I2C_ccs811]\n",argv[0]);
		return -1;
	}
	#ifdef DEBUG
		printf("I2C_ccs811 =0x%02X\n",I2C_ccs811);
	#endif

	ccs811_setup(27,50);	// 温度27℃、湿度50％
	do{
		if(co2<0){							// CO2センサのI2Cの通信異常時
			fprintf(stderr,"I2C hardware is locked. Please Reset RST pin.\n");
			i2c_close();
			ccs811_setup(27,50);
		}
		if(time > 10) break;
		time++;
		delay(1010);						// CO2測定間隔が1秒なので1.01待ち
		if(co2>400)prev=co2;				// 前回値を保存
		co2=ccs811_getCO2();
		fprintf(stderr,"CO2 = %d\n",co2);
		#ifdef DEBUG
		if(co2==0) co2=ccs811_getVals();
		#endif
	}while(co2<=400 || (abs(co2-prev)+1)*100/co2 > 5);
	printf("%d\n",co2);
	i2c_close();
	return 0;
}
#endif
