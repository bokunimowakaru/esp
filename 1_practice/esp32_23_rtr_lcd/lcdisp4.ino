/*********************************************************************
本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

LCDへの4分割表示用関数

                               Copyright (c) 2018 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

void lcdisp4(const char *s,int num){        // LCDへの4分割表示用関数の定義
    char lc[7];                             // LCD表示用の6文字配列変数lcの定義
    strncpy(lc,s,6);                        // 文字列を6文字まで変数lcへコピー
    if(lc[5]=='_') lc[5]=s[6];              // 6文字目が_のときは7文字目をコピー
    for(int i=strlen(lc);i<6;i++) lc[i]=' ';// 6文字に満たない場合に空白を代入
    lc[6]='\0';                             // 文字列の終端(文字列の最後は'\0')
    lcd.setCursor(8*((num/2)%2),(num%2));   // num=0:左上,1:左下,2:右上,3:右下
    lcd.print(num+1);                       // num+1の値を表示
    lcd.print(':');                         // 「:」を表示
    lcd.print(lc);                          // 文字列を表示
}

void lcdisp4(int val,int num){              // 整数1値入力時の表示用関数の定義
    char lc[7];                             // LCD表示用の6文字配列変数lcの定義
    itoa(val,lc,10);                        // 数値valを文字列変数lcへ代入
    lcdisp4(lc,num);                        // 前記lcdisp4を呼び出す
}

void lcdisp4(int val1,int val2,int num){    // 整数2値入力時の表示用関数の定義
    char lc[7];                             // LCD表示用の6文字配列変数lcの定義
    snprintf(lc,7,"%d,%d",val1,val2);       // 数値val1と2を文字列変数lcへ代入
    lcdisp4(lc,num);                        // 前記lcdisp4を呼び出す
}
