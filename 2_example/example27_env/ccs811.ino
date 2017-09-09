/*******************************************************************************
Arduino 用 I2C 揮発性有機化合物センサ AMS CCS811 ccs811.ino

                                        Copyright (c) 2016-2017 Wataru KUNINO
                                        http://www.geocities.jp/bokunimowakaru/
********************************************************************************

本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続のセンサから測定値を取得する
AMS CCS811 Carbon Monoxide CO VOCs Air Quality Numerical Gas Sensors

I2Cアドレス設定
                        0x5A    When ADDR is low
                        0x5B    When ADDR is high

揮発性有機化合物 VOCガス
二酸化炭素CO2(相当)

参考文献：AMS CCS811 データシート Ver 1.00
*******************************************************************************/

#include <Wire.h> 
#define I2C_ccs811 0x5A						// I2Cアドレス

// #define DEBUG

uint8_t _ccs811_getByte(byte reg){
	/*
	uint8_t tx=reg;
	uint8_t rx;
	i2c_write(I2C_ccs811,&tx,1);	   		// 書込みの実行
	delay(10);
	i2c_read(I2C_ccs811,&rx,1);
	#ifdef DEBUG
	//	printf("rx=%02x\n",rx);
	#endif
	return rx;
	*/
	uint8_t ret=0xFF;
	Wire.beginTransmission(I2C_ccs811);
	Wire.write(reg);
	delay(10);								// 書込み時間
	if( Wire.endTransmission()==0){
		Wire.requestFrom(I2C_ccs811,0x1);
		if(Wire.available()==0) return 0xFE;
		ret =Wire.read();
	}
	return ret;
}

int _ccs811_getReg(byte reg, byte *rx, int len){
	/*
	uint8_t tx=reg;
	int i;
	if(len < 0 || len>8) return -1;
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
	return i;
	*/
	int i=-1;
	if(len < 0 || len>8) return -1;
	Wire.beginTransmission(I2C_ccs811);
	Wire.write(reg);
	delay(10);
	if(Wire.endTransmission() == 0){
		Wire.requestFrom(I2C_ccs811,(byte)len);
		for(i=0;i<len;i++){
			if(Wire.available()==0) break;
			rx[i] =Wire.read();
		}
	}
	return i;
}

int _ccs811_set0Byte(byte reg){			// 書き込み値の無いコマンド(APP_START=0xF4)
	/*
	uint8_t tx=reg;
	return i2c_write(I2C_ccs811,&tx,1); 	  // 書込みの実行
	*/
	Wire.beginTransmission(I2C_ccs811);
	Wire.write(reg);
	return (int)(!Wire.endTransmission());
}

int _ccs811_setByte(byte reg, byte data){
	/*
	uint8_t tx[2];
	tx[0]=reg;
	tx[1]=data;
	return i2c_write(I2C_ccs811,tx,2); 	  // 書込みの実行
	*/
	Wire.beginTransmission(I2C_ccs811);
	Wire.write(reg);
	Wire.write(data);
	return (int)(!Wire.endTransmission());
}

int _ccs811_setReg(byte reg, byte *data, int len){
	/*
	uint8_t tx[9];
	int i;
	
	if(len < 0 || len>8) return -1;
	for(i=0;i<len;i++)tx[i+1]=data[i];
	return i2c_write(I2C_ccs811,tx,len+1);	// 書込みの実行
	*/
	Wire.beginTransmission(I2C_ccs811);
	Wire.write(reg);
	Wire.write(data, len);
	return (int)(!Wire.endTransmission());
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
	int i,len,adc,co2;
	char s[80];
	#ifdef DEBUG
		int current;
	#endif
	
	len=_ccs811_getReg(0x02, rx, 8);		// 0x02 ALG_RESULT_DATA
	if(len != 8){
		Serial.print("ERROR: ccs811_getVals, len=0x");
		Serial.println(len);
		return -1;
	}
	i=((int)rx[6])*256+(int)rx[7];
	adc = i & 0x03FF;
	co2 = ((int)rx[0])*256+(int)rx[1];
	#ifdef DEBUG
		current = i>>10;
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
	return ret;
}

int ccs811_getCO2(){						   // 二酸化炭素濃度（ppm)を取得
	uint8_t rx[6];
	int ret;
	
	ret=_ccs811_getReg(0x02,rx,6);
	if(ret != 6){
		Serial.print("ERROR: getCO2, len=0x");
		Serial.println(ret);
		return -1;
	}
	#ifdef DEBUG
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
	if(rx[5] != 0x00){
		Serial.print("ERROR: getCO2, error code=0x");
		Serial.println(rx[5],HEX);
		return -1;
	}
	return ((int)rx[0])*256+(int)rx[1];
}

int ccs811_setup(float temp, float hum){
	byte ret,id,status,mode;
	
//	Wire.begin();
	id=_ccs811_getByte(0x20);			// HW_ID
	#ifdef DEBUG
		Serial.println();
		Serial.print("ccs811_setup ");
		Serial.print(temp);
		Serial.print(",");
		Serial.println(hum);
		Serial.print("HW_ID       =0x"); Serial.println( id,HEX );
	#endif
	if(id != 0x81){
		Serial.println("ERROR: unknown device ");
		return -1;
	}
	
	ret=ccs811_setEnv(temp, hum);	// 補正用の温度と湿度を設定
	if(ret==0){
		Serial.println("ERROR: failed to set env");
		return -1;
	}
	
	ret=_ccs811_setAppStart();
	#ifdef DEBUG
		Serial.print("AppStart    ="); Serial.println( ret );
	#endif
	if(ret==0){
		Serial.println("ERROR: failed to start app");
		return -1;
	}
	
	status=_ccs811_getByte(0x00);		// STATUS
	#ifdef DEBUG
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
	#endif
	if( (status & 0x80) == 0 || status & 0xFE == 0xFE ){
		Serial.println("ERROR: failed APP starting");
		return -1;
	}
	
	ret= ccs811_start();
	#ifdef DEBUG
		Serial.print("MeasureStart="); Serial.println( ret );
	#endif
	
	delay(1000);
	mode=_ccs811_getByte(0x01); 		// MEAS_MODE
	#ifdef DEBUG
		Serial.println("------------------------");
		Serial.print("MEAS_MODE   =0x"); Serial.println( mode, HEX );
		Serial.println("------------------------");
		Serial.print("DRIVE_MODE  =0x"); Serial.println( (mode & 0x70) >> 4);
		Serial.print("INT_DATARDY =0x"); Serial.println( (mode & 0x08) >> 3);
		Serial.print("INT_THRESH  =0x"); Serial.println( (mode & 0x04) >> 2);
	#endif
	if((mode & 0x70) == 0x00){
		Serial.print("ERROR: failed to set mode");
		return -1;
	}
	return 0;
}

/*
int ccs811_main(int argc,char **argv){
	int co2=0;
	
	if( argc == 2 ) I2C_ccs811=(byte)strtol(argv[1],NULL,16);
	if( I2C_ccs811>=0x80 ) I2C_ccs811>>=1;
	if( argc < 1 || argc > 2 ){
		fprintf(stderr,"usage: %s [I2C_ccs811]\n",argv[0]);
		return -1;
	}
	#ifdef DEBUG
		printf("I2C_ccs811 =0x%02X\n",I2C_ccs811);
	#endif

	ccs811_setup();
	while(co2==0){
		co2=ccs811_getCO2();
		if(co2>0) break;
		#ifdef DEBUG
		if(co2==0) co2=ccs811_getVals();
		#endif
		delay(1000);
	}
	printf("%d\n",co2);
	i2c_close();
	return 0;
}
*/
