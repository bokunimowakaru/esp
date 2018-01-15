/*********************************************************************
UTF8の制御コードを除去して半角カタカナをLCD(16文字2桁)へ表示します
全角ひらがな、全角カタカナからの変換に対応しました(2018/1/12)

                               Copyright (c) 2014-2018 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
**********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。
*********************************************************************/

void lcdisp_utf_del_uni(char *s){
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

const char lcdisp_utf_table[85][2]={
    {0x00,0x00},{0xA7,0x00},{0xB1,0x00},{0xA8,0x00},
    {0xB2,0x00},{0xA9,0x00},{0xB3,0x00},{0xAA,0x00},
    {0xB4,0x00},{0xAB,0x00},{0xB5,0x00},{0xB6,0x00},
    {0xB6,0xDE},{0xB7,0x00},{0xB7,0xDE},{0xB8,0x00},
    
    {0xB8,0xDE},{0xB9,0x00},{0xB9,0xDE},{0xBA,0x00},
    {0xBA,0xDE},{0xBB,0x00},{0xBB,0xDE},{0xBC,0x00},
    {0xBC,0xDE},{0xBD,0x00},{0xBD,0xDE},{0xBE,0x00},
    {0xBE,0xDE},{0xBF,0x00},{0xBF,0xDE},{0xC0,0x00},
    
    {0xC0,0xDE},{0xC1,0x00},{0xC1,0xDE},{0xAF,0x00},
    {0xC2,0x00},{0xC2,0xDE},{0xC3,0x00},{0xC3,0xDE},
    {0xC4,0x00},{0xC4,0xDE},{0xC5,0x00},{0xC6,0x00},
    {0xC7,0x00},{0xC8,0x00},{0xC9,0x00},{0xCA,0x00},
    
    {0xCA,0xDE},{0xCA,0xDF},{0xCB,0x00},{0xCB,0xDE},
    {0xCB,0xDF},{0xCC,0x00},{0xCC,0xDE},{0xCC,0xDF},
    {0xCD,0x00},{0xCD,0xDE},{0xCD,0xDF},{0xCE,0x00},
    {0xCE,0xDE},{0xCE,0xDF},{0xCF,0x00},{0xD0,0x00},
    
    {0xD1,0x00},{0xD2,0x00},{0xD3,0x00},{0xAC,0x00},
    {0xD4,0x00},{0xAD,0x00},{0xD5,0x00},{0xAE,0x00},
    {0xD6,0x00},{0xD7,0x00},{0xD8,0x00},{0xD9,0x00},
    {0xDA,0x00},{0xDB,0x00},{0xDC,0x00},{0xDC,0x00},
    
    {0xB2,0x00},{0xB4,0x00},{0xA6,0x00},{0xDD,0x00},{0xB3,0xDE}
};

void lcdisp_utf_to_ank(char *s){
    unsigned int i=0;
    unsigned int j=0;
    unsigned int max=strlen(s);
    while(s[i]!='\0' && i < max){
        if((byte)s[i]==0xEF){
            if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
            i+=2;
        }else if((byte)s[i]==0xE3){
            if((byte)s[i+1]==0x81 && (byte)s[i+2] >= 0x80 && (byte)s[i+2] <= 0xBF){
                s[i+1] = lcdisp_utf_table[((byte)s[i+2])-0x80][0];
                s[i+2] = lcdisp_utf_table[((byte)s[i+2])-0x80][1];
                if((byte)s[i+2]==0){
                    s[i+2]=s[i+1];
                    i++;
                }
                i++;
            }else if((byte)s[i+1]==0x82 && (byte)s[i+2] >= 0x80 && (byte)s[i+2] <= 0x93){
                s[i+1] = lcdisp_utf_table[((byte)s[i+2])-0x40][0];
                s[i+2] = lcdisp_utf_table[((byte)s[i+2])-0x40][1];
                if((byte)s[i+2]==0){
                    s[i+2]=s[i+1];
                    i++;
                }
                i++;
            }else if((byte)s[i+1]==0x82 && (byte)s[i+2] >= 0xA0 && (byte)s[i+2] <= 0xBF){
                s[i+1] = lcdisp_utf_table[((byte)s[i+2])-0xA0][0];
                s[i+2] = lcdisp_utf_table[((byte)s[i+2])-0xA0][1];
                if((byte)s[i+2]==0){
                    s[i+2]=s[i+1];
                    i++;
                }
                i++;
            }else if((byte)s[i+1]==0x83 && (byte)s[i+2] >= 0x80 && (byte)s[i+2] <= 0xB4){
                s[i+1] = lcdisp_utf_table[((byte)s[i+2])-0x60][0];
                s[i+2] = lcdisp_utf_table[((byte)s[i+2])-0x60][1];
                if((byte)s[i+2]==0){
                    s[i+2]=s[i+1];
                    i++;
                }
                i++;
            }else if((byte)s[i+1]==0x80 && (byte)s[i+2]==0x82){
                s[i+2] = (char)0xA1;		// 。
                i+=2;
            }else if((byte)s[i+1]==0x80 && (byte)s[i+2]==0x8C){
                s[i+2] = (char)0xA2;		// 「
                i+=2;
            }else if((byte)s[i+1]==0x80 && (byte)s[i+2]==0x8D){
                s[i+2] = (char)0xA3;		// 」
                i+=2;
            }else if((byte)s[i+1]==0x80 && (byte)s[i+2]==0x81){
                s[i+2] = (char)0xA4;		// 、
                i+=2;
            }else if((byte)s[i+1]==0x83 && (byte)s[i+2]==0xBB){
                s[i+2] = (char)0xA5;		// ・
                i+=2;
            }else if((byte)s[i+1]==0x83 && (byte)s[i+2]==0xBC){
                s[i+2] = (char)0xB0;		// ー
                i+=2;
            }else{
                s[i+2]=' ';
                i+=2;
            }
        }else if( (byte)s[i] >= 0xFC ){
            i+=5;
        }else if( (byte)s[i] >= 0xF8 ){
            i+=4;
        }else if( (byte)s[i] >= 0xF0 ){
            i+=3;
        }else if( (byte)s[i] >= 0xE0 ){
            i+=2;
        }
        
        if(i<max){
            if(isprint(s[i]) || ((byte)s[i] >=0xA0 && (byte)s[i] <= 0xDF)){
                s[j]=s[i];
                j++;
            }
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
    char utf[97];
    int i;

    if(y<0 || y>1)return;
    memset(lc,0,17);                        // 文字列変数lcの初期化(17バイト)
    memset(utf,0,97);
    strncpy(utf,s,96);
    lcdisp_utf_to_ank(utf);
    
    strncpy(lc,utf,16);
    lcd.setCursor(0,y);                     // カーソル位置を左へ
    lcd.print(lc);                          // 液晶へ転送
    for(i=strlen(utf);i<16;i++)lcd.print(' ');
    if( strlen(utf)<=16 || y>0 ) return;
    strncpy(lc,utf+16,16);
    lcd.setCursor(0,1);
    lcd.print(lc);                          // 液晶へ転送
    for(i=strlen(utf+16);i<16;i++)lcd.print(' ');
}


/*
#include <stdio.h>
#include <ctype.h>
#include <string.h>
typedef unsigned char byte;

const char _utf_table[85][7]={
"",  "ｧ", "ｱ", "ｨ", "ｲ", "ｩ", "ｳ", "ｪ", "ｴ", "ｫ", "ｵ", "ｶ", "ｶﾞ","ｷ", "ｷﾞ","ｸ",
"ｸﾞ","ｹ", "ｹﾞ","ｺ", "ｺﾞ","ｻ", "ｻﾞ","ｼ", "ｼﾞ","ｽ", "ｽﾞ","ｾ", "ｾﾞ","ｿ", "ｿﾞ","ﾀ",
"ﾀﾞ","ﾁ", "ﾁﾞ","ｯ", "ﾂ", "ﾂﾞ","ﾃ", "ﾃﾞ","ﾄ", "ﾄﾞ","ﾅ", "ﾆ", "ﾇ", "ﾈ", "ﾉ", "ﾊ",
"ﾊﾞ","ﾊﾟ","ﾋ", "ﾋﾞ","ﾋﾟ","ﾌ", "ﾌﾞ","ﾌﾟ","ﾍ", "ﾍﾞ","ﾍﾟ","ﾎ", "ﾎﾞ","ﾎﾟ","ﾏ", "ﾐ",
"ﾑ", "ﾒ", "ﾓ", "ｬ", "ﾔ", "ｭ", "ﾕ", "ｮ", "ﾖ", "ﾗ", "ﾘ", "ﾙ", "ﾚ", "ﾛ", "ﾜ", "ﾜ",
"ｲ", "ｴ", "ｦ", "ﾝ", "ｳﾞ"};

void _make_utf_table(){
    char s[16];
    for(int i;i<85;i++){
        memset(s,0,16);
        strncpy(s,&_utf_table[i][0],15);
        
        _utf_del_uni(s);
        printf("{");
        for(int j=0;j<2;j++){
            printf("0x");
            printf("%02X",(byte)s[j]);
            if(j<1)printf(",");
        }
        printf("},");
        if(i%16==15) printf("\n");
    }
}

void main(){
    _make_utf_table();
}
*/
