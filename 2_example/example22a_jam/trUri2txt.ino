/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。
HTTPクエリ内のコードを文字に戻す
                                     Copyright (c) 2016-2019 Wataru KUNINO
*********************************************************************/

#ifndef _ahex2i
#define _ahex2i
int ahex2i(char c){
    if(c>='0' && c<='9') return c-'0';
    if(c>='a' && c<='f') return c-'a'+10;
    if(c>='A' && c<='F') return c-'A'+10;
    return -1;
}
#endif

int trShift(char *s,int len){
	int i,j=strlen(s);
	for(i=len;i<=j;i++) s[i-len]=s[i];
	return i-1;	//↑ \0まで
}

char trUri2c(char c){
	switch( c ){
		case ' ':	return '\0';    // スペース文字を区切り文字に置換
		case '&':	return '\0';    // 「&」文字を区切り文字に置換
		case '+':	return ' ';     // 「+」文字をスペースに置換
		default :	return c;
	}
	return 0;
}

char trUri2s(char *s){
    if(s[0]=='%' && strlen(s) > 2 ){
        s[0]=(char)(ahex2i(s[1])*16+ahex2i(s[2]));
        trShift(&s[1],2);
        return s[0];
    }
    return '\0';
}

int trUri2txt(char *s){
    int i,j;
    int len;
    
    for(i=0;i<strlen(s);i++){
        s[i]=trUri2c(s[i]);
        if(s[i]=='%'){
            if(i+2<strlen(s)){
                s[i]=(char)(ahex2i(s[i+1])*16+ahex2i(s[i+2]));
                len=strlen(s);
                for(j=i+3;j<len;j++) s[j-2]=s[j];
                for(j=1;j<=2;j++) s[len-j]='\0';
            }else s[i]='\0';        // オーバーフロー
        }
    }
    return i;
}

/*
動作確認

「*」や「/」の文字に注意する 

              0 1 2 3 4 5 6
        s[t]  A B % x x C D
        
        i=2    -> j=5 s[3]=s[5]
        len=7  -> len=5
        s[5]=null
        
        test
        !"#$%&'()   ok
        |@`[{}];    ok
        :+*<>?_,    ok
        ./~^-=      ok
        
        0!!!!!!78!!!!!!F            OK
        0!!!!!!78!!!!!!F0!!!!       OK
        
        64バイトの場合
        記号だけのとき＝64バイト÷3＝21文字まで
        カタカナの場合＝64バイト÷12＝5文字まで
*/
