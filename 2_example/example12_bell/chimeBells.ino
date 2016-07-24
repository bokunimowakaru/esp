/*******************************************************************************
チャイム

int chimeBells(int output, int count);

int output  LEDを接続したIOピン番号
int count   チャイム音が終わるまでのカウントダウン値（0で終了）
int 戻り値  count-1（0以上）

処理時間：約1秒

                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

int chimeBells(int output, int count) {
    int t;
    if(count<=0) return 0;
    if(!(count%2)){
        tone(output,NOTE_CS6,800);
        for(t=0;t<8;t++) delay(100);
        noTone(output);
        for(t=0;t<2;t++) delay(100);
    }else{
        tone(output,NOTE_A5,800);
        for(t=0;t<8;t++) delay(100);
        noTone(output);
        for(t=0;t<2;t++) delay(100);
    }
    count--;
    if(count<0) count=0;
    return(count);
}
