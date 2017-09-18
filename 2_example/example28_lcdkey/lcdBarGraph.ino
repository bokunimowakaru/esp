/*********************************************************************
キャラクタＬＣＤへ棒グラフを表示するためのフォントおよびライブラリ
	
本ソースリストおよびソフトウェアは、ライセンスフリーです。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

							   Copyright (c) 2013-2017 Wataru KUNINO
							   http://www.geocities.jp/bokunimowakaru/
*********************************************************************/
/*
画面設計例

	起動時					メニュー表示			レベル表示時
	　0123456789012345		　0123456789012345		　0123456789012345
	┏━━━━━━━━┓	┏━━━━━━━━┓	┏━━━━━━━━┓
	┃M2M Display ----┃	┃LED_SW		00┃	┃----- ----- I=00┃
	┃text message....┃	┃Sent GPIO(11)= 1┃	┃26.5C 3000W 1000┃
	┗━━━━━━━━┛	┗━━━━━━━━┛	┗━━━━━━━━┛
	
*/

// デバイスリスト

#define LIST_DEVICE_MAX 17
// #define DEBUG
int LIST_DEVICE_MAX_var=0;

struct ListOfDevice{
	char dev[6];		// デバイス名 5文字
	byte num_a;			// 測定データ数 0～255
	byte num[2];		// 表示用のデータ番号
	int min[2];			// グラフ最小値
	int max[2];			// グラフ最大値
}listOfDevice[LIST_DEVICE_MAX];

byte lcd_bar_listOfDevice_max(){
	return LIST_DEVICE_MAX_var;
}

byte lcd_bar_listOfDevice_init(){
	strncpy(listOfDevice[0].dev,"deeps",6);
	listOfDevice[0].num_a=		2;
	listOfDevice[0].num[0]=		1;
	listOfDevice[0].num[1]=		2;
	listOfDevice[0].min[0]=		0;
	listOfDevice[0].max[0]=	 1023;
	listOfDevice[0].min[1]=		0;
	listOfDevice[0].max[1]=	   10;
	strncpy(listOfDevice[1].dev,"illum",6);
	listOfDevice[1].num_a=		1;
	listOfDevice[1].num[0]=		1;
	listOfDevice[1].num[1]=		0;
	listOfDevice[1].min[0]=		0;
	listOfDevice[1].max[0]=	  200;
	strncpy(listOfDevice[2].dev,"temp.",6);
	listOfDevice[2].num_a=		1;
	listOfDevice[2].num[0]=		1;
	listOfDevice[2].num[1]=		0;
	listOfDevice[2].min[0]=		0;
	listOfDevice[2].max[0]=	   30;
	strncpy(listOfDevice[3].dev,"rd_sw",6);
	listOfDevice[3].num_a=		2;
	listOfDevice[3].num[0]=		1;
	listOfDevice[3].num[1]=		2;
	listOfDevice[3].min[0]=		0;
	listOfDevice[3].max[0]=	    1;
	listOfDevice[3].min[1]=		0;
	listOfDevice[3].max[1]=	    1;
	strncpy(listOfDevice[4].dev,"humid",6);
	listOfDevice[4].num_a=		2;
	listOfDevice[4].num[0]=		1;
	listOfDevice[4].num[1]=		2;
	listOfDevice[4].min[0]=		0;
	listOfDevice[4].max[0]=	   30;
	listOfDevice[4].min[1]=		0;
	listOfDevice[4].max[1]=	  100;
	strncpy(listOfDevice[5].dev,"press",6);
	listOfDevice[5].num_a=		2;
	listOfDevice[5].num[0]=		1;
	listOfDevice[5].num[1]=		2;
	listOfDevice[5].min[0]=		0;
	listOfDevice[5].max[0]=	   30;
	listOfDevice[5].min[1]=	 1000;
	listOfDevice[5].max[1]=	 1026;
	strncpy(listOfDevice[6].dev,"accem",6);
	listOfDevice[6].num_a=		3;
	listOfDevice[6].num[0]=	    1;
	listOfDevice[6].num[1]=     2;
	listOfDevice[6].min[0]=	   -5;
	listOfDevice[6].max[0]=	    5;
	listOfDevice[6].min[1]=	   -5;
	listOfDevice[6].max[1]=	    5;
	strncpy(listOfDevice[7].dev,"tftpc",6);
	listOfDevice[7].num_a=		1;
	listOfDevice[7].num[0]=		1;
	listOfDevice[7].num[1]=		0;
	listOfDevice[7].min[0]=		0;
	listOfDevice[7].max[0]=	 1023;
	strncpy(listOfDevice[8].dev,"e_co2",6);
	listOfDevice[8].num_a=		4;
	listOfDevice[8].num[0]=		1;
	listOfDevice[8].num[1]=		4;
	listOfDevice[8].min[0]=		0;
	listOfDevice[8].max[0]=	   30;
	listOfDevice[8].min[1]=	  400;
	listOfDevice[8].max[1]=	 2400;
	strncpy(listOfDevice[9].dev,"adcnv",6);
	listOfDevice[9].num_a=		1;
	listOfDevice[9].num[0]=		1;
	listOfDevice[9].num[1]=		0;
	listOfDevice[9].min[0]=		0;
	listOfDevice[9].max[0]=	 3300;
	strncpy(listOfDevice[10].dev,"meter",6);
	listOfDevice[10].num_a=		1;
	listOfDevice[10].num[0]=	1;
	listOfDevice[10].num[1]=	0;
	listOfDevice[10].min[0]=	0;
	listOfDevice[10].max[0]= 1000;
	strncpy(listOfDevice[11].dev,"actap",6);
	listOfDevice[11].num_a=		3;
	listOfDevice[11].num[0]=	1;
	listOfDevice[11].num[1]=	0;
	listOfDevice[11].min[0]=	0;
	listOfDevice[11].max[0]=  200;
	strncpy(listOfDevice[12].dev,"xb_ct",6);
	listOfDevice[12].num_a=		1;
	listOfDevice[12].num[0]=	1;
	listOfDevice[12].num[1]=	0;
	listOfDevice[12].min[0]=	0;
	listOfDevice[12].max[0]= 1000;
	strncpy(listOfDevice[13].dev,"xbhum",6);
	listOfDevice[13].num_a=		2;
	listOfDevice[13].num[0]=	1;
	listOfDevice[13].num[1]=	2;
	listOfDevice[13].min[0]=	0;
	listOfDevice[13].max[0]=   30;
	listOfDevice[13].min[1]=	0;
	listOfDevice[13].max[1]=  100;
	strncpy(listOfDevice[14].dev,"xbgas",6);
	listOfDevice[14].num_a=		2;
	listOfDevice[14].num[0]=	1;
	listOfDevice[14].num[1]=	2;
	listOfDevice[14].min[0]=  200;
	listOfDevice[14].max[0]= 2200;
	listOfDevice[14].min[1]=  200;
	listOfDevice[14].max[1]= 2200;
	strncpy(listOfDevice[15].dev,"xbprs",6);
	listOfDevice[15].num_a=		2;
	listOfDevice[15].num[0]=	1;
	listOfDevice[15].num[1]=	2;
	listOfDevice[15].min[0]=	0;
	listOfDevice[15].max[0]=   30;
	listOfDevice[15].min[1]=	0;
	listOfDevice[15].max[1]= 1026;
	// 終端
	strncpy(listOfDevice[16].dev,"",6);
	LIST_DEVICE_MAX_var = sizeof(listOfDevice) / sizeof(listOfDevice[0]);
	#ifdef DEBUG
		Serial.print("LIST_DEVICE_MAX_var=");
		Serial.println(LIST_DEVICE_MAX_var);
	#endif
	return LIST_DEVICE_MAX_var;
}

uint8_t _lcd_bar_font_lv[5][8]={
	{
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10101
	},{
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b11000,
		0b10101
	},{
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b11011,
		0b10101
	},{
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10000,
		0b00000,
		0b10000
	},{
		0b00010,
		0b10110,
		0b00010,
		0b10010,
		0b00010,
		0b10010,
		0b00111,
		0b10000
	}
};
/*
	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●　　　　　●　　　　　●　　　　　●　　　　　●　　　　
	１
	２●　　　　　●　　　　　●　　　　　●　　　　　●　　　　
	３
	４●　　　　　●　　　　　●　　　　　●　　　　　●　　　　
	５
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●
	
	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●●　　　　●　　　　　●　　　　　●　　　　　●　　　　
	１●●
	２●●　　　　●　　　　　●　　　　　●　　　　　●　　　　
	３●●
	４●●　　　　●　　　　　●　　　　　●　　　　　●　　　　
	５●●
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●
	
	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●●　●●　●　　　　　●　　　　　●　　　　　●　　　　
	１●●　●●
	２●●　●●　●　　　　　●　　　　　●　　　　　●　　　　
	３●●　●●
	４●●　●●　●　　　　　●　　　　　●　　　　　●　　　　
	５●●　●●
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●
	

	　０１２３４　０１２３４　０１２３４　０１２３４　０１２３４
	０●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	１●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	２●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	３●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	４●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	５●●　●●　●●　●●　●●　●●　●●　●●　●●　●●
	６●　●　●　●　●　●　●　●　●　●　●　●　●　●　●

*/

void lcd_bar_init(){
	byte i;
	
	/* 初期化処理 */
	for(i=0 ; i < 5 ; i++ ){
		lcd.createChar(i , &(_lcd_bar_font_lv[i][0]));
	}
	lcd_bar_listOfDevice_init();
}

void lcd_cls( const byte line ){						// 指定した行を消去する関数
	lcd.setCursor(0,line);
	for(byte i=0;i<16;i++)lcd.print(" ");
	lcd.setCursor(0,line);
}

void lcd_print_hex(const byte in){						// 16進数2桁(1バイト)の表示関数
	lcd.print( in>>4 , HEX );
	lcd.print( in%16 , HEX );
}

void lcd_print_address(char *str, byte *dev ){			// アドレス表示用の関数
	byte i;
	byte start = 4; 									// アドレスの表示開始バイト
	lcd_cls(1); 										// 2行目を消去
	if( str[0] == '\0' ) start=0;						// strに文字なし時は8バイト表示
	else lcd.print( str );								// 文字あり時は文字+4バイト表示
	for(i=start;i<8;i++) lcd_print_hex(dev[i]); 		// アドレスを液晶に表示する
}

void lcd_bar_print_level2(const byte lev1, const byte lev2, const byte hour12h, const byte min){
	byte i; char s[5]="0:00";

	lcd.setCursor( 0 , 0 );
	for(i=0 ; i < 5 ; i++){
		if( lev1 <= 2 * i )				lcd.write( (byte)0x00 );
		else if( lev1 - 2 * i == 1 )	lcd.write( (byte)0x01 );
		else							lcd.write( (byte)0x02 );
	}
	if(lev1>10) lcd.print("F"); else lcd.write(3);
	for(i=0 ; i < 5 ; i++){
		lcd.setCursor( i+6 , 0 );
		if( lev2 <= 2 * i )				lcd.write( (byte)0x00 );
		else if( lev2 - 2 * i == 1 )	lcd.write( (byte)0x01 );
		else							lcd.write( (byte)0x02 );
	}
	if(lev2>10) lcd.print("F"); else{
		if( hour12h>=10 )lcd.write(4); else lcd.write(3);
	}
	s[0]='0'+(hour12h%10);
	s[2]='0'+(min/10);
	s[3]='0'+(min%10);
	lcd.print(s);
}

void lcd_bar_print_level1(const byte lev1, const int val, const byte hour12h, const byte min){
	byte i; char s[5]="0:00";

	lcd.setCursor(0,0);
	for(i=0 ; i < 5 ; i++){
		if( lev1 <= 2 * i )				lcd.write( (byte)0x00 );
		else if( lev1 - 2 * i == 1 )	lcd.write( (byte)0x01 );
		else							lcd.write( (byte)0x02 );
	}
	if(lev1>10) lcd.print("F"); else lcd.write(3);
	lcd.print(val);
	lcd.print("    ");
	
	lcd.setCursor(11,0);
	if( hour12h>=10 )lcd.write(4); else lcd.write(3);
	s[0]='0'+(hour12h%10);
	s[2]='0'+(min/10);
	s[3]='0'+(min%10);
	lcd.print(s);
}

void lcd_bar_print_level0(const char *csv, const byte hour12h, const byte min){
	byte i; char s[5]="0:00";
	char s2[12];
	
	lcd_cls(0);
	strncpy(s2,csv,11); lcd.print(s2);
	lcd.setCursor(11,0);
	if( hour12h>=10 )lcd.write(4); else lcd.write(3);
	s[0]='0'+(hour12h%10);
	s[2]='0'+(min/10);
	s[3]='0'+(min%10);
	lcd.print(s);
}

byte lcd_bar_val2lev(int val,int min,int max){
	if( min == max ) max = min + 1;		// 0除算対策
	if(min < max) return ((val - min ) * 10 ) / (max - min);
	else return ((val - max ) * 10 ) / (min - max);
}

void lcd_bar_print(char *dev, char *csv, char *time){
	// dev デバイス名 5文字
	// csv カンマ区切りの数値データ
	// time 時刻5文字 00:00 形式
	byte lev1=0xFF,lev2=0xFF;
	byte hour,min;
	byte index,index_n,j;
	int i,val,val1;
	char *p,*p2;

	byte num_a;			// 測定データ数 0～255
	byte num[2];		// 表示用のデータ番号

	#ifdef DEBUG
		Serial.print("lcd_bar_print dev=");
		Serial.print(dev);
		Serial.print(" time=");
		Serial.println(time);
	#endif
	
	hour=(byte)atoi(time);
	min=(byte)atoi(&time[3]);
	if(hour>12) hour %= 12;
	if(min>=60) min %= 60;
	
	if(dev[5] == '_' ){
		index_n=lcd_bar_listOfDevice_max();
		for(i=0;i<index_n;i++){
			if(strncmp(listOfDevice[i].dev,dev,5)==0) break;
			if(listOfDevice[i].dev[0]==0){ i=-1; break; }
		}
	}else i=-1;
	if(i==index_n || i<0){
		lcd_bar_print_level0(csv,hour,min);
		#ifdef DEBUG
			Serial.println("lcd_bar_print exit");
		#endif
		return;
	}
	index=i;
	#ifdef DEBUG
		Serial.print("index=");
		Serial.print(index);
		Serial.print(" dev=");
		Serial.println(listOfDevice[index].dev);
	#endif
	j=1;			// jは常に1以上
	for(i=0;i<strlen(csv);){
		p=&csv[i];
		val=atoi(p);
		#ifdef DEBUG
			Serial.print(val);
			Serial.print(", ");
		#endif
		if(listOfDevice[index].num[0] == j){
			lev1 = lcd_bar_val2lev(val,listOfDevice[index].min[0],listOfDevice[index].max[0]);
			val1 = val;
		}
		if(listOfDevice[index].num[1] == j){
			lev2 = lcd_bar_val2lev(val,listOfDevice[index].min[1],listOfDevice[index].max[1]);
		}
		p2=strchr(p,',');  j++;
		if( !p2 || *(p2+1)==0 || j>10) break;
		i += (int)(p2-p)+1;
	}
	#ifdef DEBUG
		Serial.println();
		Serial.print("lev1=");
		Serial.print(lev1);
		Serial.print(" (");
		Serial.print(val1);
		Serial.print(") lev2=");
		Serial.print(lev2);
		Serial.println();
	#endif
	if(lev1<=10 && lev2<=10) lcd_bar_print_level2(lev1,lev2,hour,min);
	else if(lev1<=10) lcd_bar_print_level1(lev1,val1,hour,min);
	else lcd_bar_print_level0(csv,hour,min);
}
