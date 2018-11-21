/*******************************************************************************
LEDの輝度制御

int ledCtrl(int start,int end,int speed);

int pin		LEDを接続したポート番号
int start	現在の輝度　（0～1023）
int end		制御後の輝度（0～1023）
int speed	制御速度	(1～100程度・1で約1秒、10で約0.1秒)
int 戻り値	制御後の輝度

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

int ledCtrl(int pin,int start,int end,int speed){   // ledのアナログ制御用の関数
    int i;                                  // (startからendへ輝度を推移する)
    if(speed<1)speed=1;
    if(start<=end){
        if(start<1) start=1;
        if(end>1023) end=1023;
        for(i=start;i<end;i<<=1){
            analogWrite(pin,i);
            delay(100/speed);
        }
    }else{
        if(start>1023) start=1023;
        if(end<0) end=0;
        for(i=start;i>end;i>>=1){
            analogWrite(pin,i);
            delay(100/speed);
        }
    }
    analogWrite(pin,end);
    return(end);
}
