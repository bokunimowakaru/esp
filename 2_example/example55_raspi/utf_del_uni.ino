/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。
UTF8の制御コードを除去します
                               Copyright (c) 2014-2016 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

void utf_del_uni(char *s){
	byte i=0;
	byte j=0;
	while(s[i]!='\0'){
		if((byte)s[i]==0xEF){
			if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
			i+=2;
		}
	//	if(isprint(s[i])){
		if(s[j]!='\r'){
			s[j]=s[i];
			j++;
		}
		i++;
	}
	s[j]='\0';
}
