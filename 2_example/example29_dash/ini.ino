/*******************************************************************************
INIファイル解析

                                     Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#define SPIFFS_FILENAME "/esp.mac" 			// TFTP受信ファイル名
#include <FS.h>

byte _parsed_mac[6];

int ini_init(char *data){
	Serial.print("Checking SPIFFS. ");
    while(!SPIFFS.begin()) delay(1000);		// ファイルシステムの開始
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

byte *ini_parse_mac(char *data, char *key){		// MACアドレスを取得する
	int i;
	byte *ret=NULL;
	char keyword[33];
	
	while( data[0] ==';' ){
		data=strchr(data,'\n');
		if( data == 0 ) return 0;
	}
	strncpy(keyword,key,31);
	strcat(keyword,"=");
	data=strstr(data,keyword);
	if( data == 0 ) return 0;
	data += strlen(keyword);
	Serial.print("Parsed ");
	Serial.print(keyword);
	for(i=0;i<6;i++){
		_parsed_mac[i]=(byte)strtol(data,NULL,16);
		if(_parsed_mac[i]<0x10) Serial.print(0);
		Serial.print(_parsed_mac[i],HEX);
		data=strchr(data,':')+1;
		if( data == 0 ) break;
		if( i<5 )Serial.print(':');
	}
	Serial.println();
	if(i<6) return 0;
	return _parsed_mac;
}

int get_parsed_mac(byte *mac, char *data, char *key){
	byte *p;
	int j;
	p=ini_parse_mac(data,key);
	if(p) for(j=0;j<6;j++) mac[j]=p[j];
	else for(j=0;j<6;j++) mac[j]=0x00;
	return (int)(p != 0);
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
