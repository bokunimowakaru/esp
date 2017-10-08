/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

UTF8をASCIIカナ文字へ変換する

                               Copyright (c) 2014-2017 Wataru KUNINO
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
		if(isprint(s[i])){
			s[j]=s[i];
			j++;
		}
		i++;
	}
	s[j]='\0';
}
