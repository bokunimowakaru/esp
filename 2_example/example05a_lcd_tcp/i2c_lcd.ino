/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

I2C接続の小型液晶に文字を表示する

                               Copyright (c) 2014-2016 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

#include <Wire.h> 
#define I2C_lcd 0x3E			// LCD の I2C アドレス 

void _utf_del_uni(char *s){
	byte i=0;
	byte j=0;
	while(s[i]!='\0'){
		if((byte)s[i]==0xEF){
			if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
			i+=2;
		}
		if(isprint(s[i])){
			s[j]=s[i];
			j++;
		}
		i++;
	}
	s[j]='\0';
}

void lcdOut(byte y,byte *lcd){
	byte data;
	byte i;
	if(y==0) data=0x80;
	else{
		data=0xC0;
		y=1;
	}
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(data);
	Wire.endTransmission();
	for(i=0;i<8;i++){
		if(lcd[i]==0x00) break;
		Wire.beginTransmission(I2C_lcd);
		Wire.write(0x40);Wire.write(lcd[i]);
		Wire.endTransmission();
	}
}

void lcdPrint(char *s){
	byte i,j;
	char str[49];
	byte lcd[9];
	
	strncpy(str,s,48);
	utf_del_uni(str);
	for(j=0;j<2;j++){
		lcd[8]='\0';
		for(i=0;i<8;i++){
			lcd[i]=(byte)str[i+8*j];
			if(lcd[i]==0x00){
				for(;i<8;i++) lcd[i]=' ';
				lcdOut(j,lcd);
				if(j==0){
					for(i=0;i<8;i++) lcd[i]=' ';
					lcdOut(1,lcd);
				}
				return;
			}
		}
		lcdOut(j,lcd);
	}
}

void lcdPrintIp(uint32_t ip){
	byte i,j;
	char lcd[17];
	
	sprintf(lcd,"%i.%i.    ",
		ip & 255,
		ip>>8 & 255
	);
	sprintf(&lcd[8],"%i.%i",
		ip>>16 & 255,
		ip>>24
	);
	lcdPrint(lcd);
}

void lcdSetup() {
    Wire.begin();
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x39);Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x11);Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x70);Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x56);Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x6C);Wire.endTransmission();
	delay(200);
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x38);Wire.endTransmission();
	delay(10);
	Wire.beginTransmission(I2C_lcd);
	Wire.write(0x00);Wire.write(0x0C);Wire.endTransmission();
	lcdPrint("Hello!  I2C LCD ");
}
