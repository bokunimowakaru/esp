/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

UTF8の制御コードを除去して(UTF8をASCIIカナ文字へ変換するときに使用)
LCD(16文字2桁)へ表示します

                               Copyright (c) 2014-2018 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

void _utf_del_uni(char *s){
    unsigned int i=0;
    unsigned int j=0;
    while(s[i]!='\0'){
        if((byte)s[i]==0xEF){
            if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
            i+=2;
        }
        if(isprint(s[i]) || ((byte)s[i] >=0xA0 && (byte)s[i] <= 0xDF)){
            s[j]=s[i];
            j++;
        }
        i++;
    }
    s[j]='\0';
}

void lcdisp(const char *s){
    lcdisp(s,0);
}

void lcdisp(const char *s,int y){
    char lc[17];                            // LCD表示用の文字列変数
    char utf[129];
    int i;
    memset(lc,0,17);                        // 文字列変数lcの初期化(17バイト)
    memset(utf,0,129);
    strncpy(utf,s,128);
    
    if(y<0 || y>1)return;
    _utf_del_uni(utf);
    strncpy(lc,utf,16);
    lcd.setCursor(0,y);                     // カーソル位置を左へ
    lcd.print(lc);                          // 液晶へ転送
    for(i=strlen(utf);i<16;i++)lcd.print(' ');
    if( strlen(utf)<=16) return;
    strncpy(lc,utf+16,16);
    lcd.setCursor(0,1);
    lcd.print(lc);                          // 液晶へ転送
    for(i=strlen(utf+16);i<16;i++)lcd.print(' ');
}
