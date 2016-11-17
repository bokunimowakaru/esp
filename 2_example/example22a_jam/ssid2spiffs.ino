/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。
HTTPクエリ内のコードを文字に戻す
                                      Copyright (c) 2016 Wataru KUNINO
*********************************************************************/

extern File file;
extern char SSID[17];
extern char PASS[17];

int ssidpass_init(){
	file = SPIFFS.open("_ssid.txt","w");
	if(file==0) return -1;
	file.println("1234ABCD");
	file.println("password");
	file.println("# このファイルにアクセスポイントの情報を保存してください。");
	file.println("# 1行目にSSID、2行目にパスワードを入力してください。");
	file.println("# これらは暗号化せずにESPモジュール内に保持されます。");
	file.close();
	return 0;
}

int ssidpass_read(){
	int i;
	char c;
	
	file = SPIFFS.open("_ssid.txt","r");
	if(file==0) return -1;
	memset(SSID, 0, 17);
	memset(PASS, 0, 17);
	for(i=0;i<16;i++){
		if( !file.available() )break;
		c=file.read();
		if( c=='\n') break;
		if( c!='\r' ) SSID[i]=c;
	}
	for(i=0;i<16;i++){
		if( !file.available() )break;
		c=file.read();
		if( c=='\n') break;
		if( c!='\r' ) PASS[i]=c;
	}
	file.close();
	return 0;
}

int ssidpass_write(char *in){
	int i=0;
	
	while(i<strlen(in)){
		if( in[i]==' ') break;
		SSID[i]==in[i];
		i++;
	}
	i++;
	while(i<strlen(in)){
		if( in[i]=='\r' || in[i]=='\n') break;
		PASS[i]==in[i];
		i++;
	}
	if(i<2) return -1;
	file = SPIFFS.open("_ssid.txt","w");
	if(file==0) return -1;
	file.println(SSID);
	file.println(PASS);
	file.close();
	return 0;
}
