/*******************************************************************************
INIファイル解析

・開発時に下記ライブラリを使用しました(現在はESP32ライブラリに含まれています。)
　https://github.com/copercini/arduino-esp32-SPIFFS

                                     Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#define SPIFFS_FILENAME "/esp.ini" 			// TFTP受信ファイル名
#include <SPIFFS.h>
//#include <FS.h>

int ini_init(char *data){
	Serial.print("Checking SPIFFS. ");
    if(!SPIFFS.begin()){					// ファイルシステムの開始
		Serial.println("ERROR: Please format SPIFFS.");
		return 0;
	}
	Serial.println("Done ini_init.");
	return ini_load(data);
}

int ini_parse(char *data, char *key){		// data=対象データ、key=検索文字(32文字まで)
	char *p;
	int i,ret=-1;
	char keyword[33];
	
	strncpy(keyword,key,31);
	i=strlen(keyword);
	strcat(keyword,"=");
	p=strstr(data,keyword);
	if(p){
		p+=strlen(keyword);
		ret=atoi(p);
	}
	Serial.print("Parsed ");
	Serial.print(keyword);
	Serial.println(ret);
	return ret;
}

int ini_load(char *data){
	File file;
	int len=0;

	Serial.print("Loading ini file. ");
    file=SPIFFS.open(SPIFFS_FILENAME,"r");	// 読み取りのためにファイルを開く
    if(file){
		memset(data,0,512);
		file.read( (uint8_t *)data, 511);
		len=strlen(data);
		Serial.print(len);
		Serial.print("Bytes. ");
		file.close();
	}
	Serial.println("Done ini_init.");
	return len;
}

int ini_save(char *data){
	File file;
	char old[512];
	int len,len_old;
	
	Serial.print("Comparing ini file. ");
	file=SPIFFS.open(SPIFFS_FILENAME,"r");	// 読み取りのためにファイルを開く
    if(file){
		memset(old,0,512);
		file.read( (uint8_t *)old, 511);
		len_old=strlen(old);
		file.close();
	}else old[0]='\0';
	if(strcmp(data,old)){
		Serial.println();
		Serial.print("Saving ini file. ");
	    file=SPIFFS.open(SPIFFS_FILENAME,"w");	// 保存のためにファイルを開く
	    if(file){
			len=strlen(data);
			file.print(data);
			Serial.print(len);
			Serial.print("Bytes. ");
			file.close();
		}else return 0;
	}else Serial.print("No change. ");
	Serial.println("Done ini_save.");
	return len;
}
