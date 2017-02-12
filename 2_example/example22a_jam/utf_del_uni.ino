/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。
UTF8の制御コードを除去します.
文字数制限をintまで拡大しました(2016/11/27)

                               Copyright (c) 2014-2016 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

void utf_del_uni(char *s){
	unsigned int i=0;
	unsigned int j=0;
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
