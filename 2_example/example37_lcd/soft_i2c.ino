/*******************************************************************************
Arduino ESP32 用 ソフトウェアI2C ドライバ soft_i2c

本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

							 			Copyright (c) 2014-2017 Wataru KUNINO
							 			http://www.geocities.jp/bokunimowakaru/
*******************************************************************************/

#define I2C_lcd 0x3E							// LCD の I2C アドレス 
#define PORT_SCL	22							// I2C SCLポート
#define PORT_SDA	21							// I2C SDAポート
#define	I2C_RAMDA	30							// I2C データシンボル長[us]
#define GPIO_RETRY	50							// GPIO 切換え時のリトライ回数
//	#define DEBUG								// デバッグモード

unsigned long micros_prev;
int ERROR_CHECK=1;								// 1:ACKを確認／0:ACKを無視する
static byte _lcd_size_x=8;
static byte _lcd_size_y=2;

int _micros(){
	unsigned long micros_sec=micros();
	if( micros_prev < micros_sec ) return micros_sec - micros_prev;
	return ( UINT_MAX - micros_prev ) + micros_sec;
}

void _micros_0(){
	micros_prev=micros();
}

void _delayMicroseconds(int i){
	delayMicroseconds(i);
}

void i2c_debug(const char *s,byte priority){
	#ifdef DEBUG
	Serial.print(_micros());
	if(priority>3) Serial.print(" ERROR:"); else Serial.print("      :");
	Serial.println(s);
	#endif
}

void i2c_error(const char *s){
	i2c_debug(s,5);
}
void i2c_log(const char *s){
	i2c_debug(s,1);
}

void i2c_SCL(byte level){
	if( level ){
		pinMode(PORT_SCL, INPUT);
	}else{
		pinMode(PORT_SCL, OUTPUT);
		digitalWrite(PORT_SCL, LOW);
	}
	_delayMicroseconds(I2C_RAMDA);
}

void i2c_SDA(byte level){
	if( level ){
		pinMode(PORT_SDA, INPUT);
	}else{
		pinMode(PORT_SDA, OUTPUT);
		digitalWrite(PORT_SDA, LOW);
	}
	_delayMicroseconds(I2C_RAMDA);
}

byte i2c_tx(const byte in){
	int i;
	#ifdef DEBUG
		char s[32];
		sprintf(s,"tx data = [%02X]",in);
		i2c_log(s);
	#endif
	for(i=0;i<8;i++){
		if( (in>>(7-i))&0x01 ){
				i2c_SDA(1);					// (SDA)	H Imp
		}else	i2c_SDA(0);					// (SDA)	L Out
		/*Clock*/
		i2c_SCL(1);							// (SCL)	H Imp
		i2c_SCL(0);							// (SCL)	L Out
	}
	/* ACK処理 */
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp  2016/6/26 先にSDAを終わらせる
	i2c_SCL(1);								// (SCL)	H Imp
	for(i=3;i>0;i--){						// さらにクロックを上げた瞬間には確定しているハズ
		if( digitalRead(PORT_SDA) == 0 ) break;	// 速やかに確認
		_delayMicroseconds(I2C_RAMDA/2);
	}
	if(i==0 && ERROR_CHECK ){
		i2c_SCL(0);							// (SCL)	L Out
		i2c_log("no ACK");
		return(0);
	}
	return(i);
}

byte i2c_init(void){
	int i;

	_micros_0();
	i2c_log("I2C_Init");
	for(i=GPIO_RETRY;i>0;i--){						// リトライ50回まで
		i2c_SDA(1);							// (SDA)	H Imp
		i2c_SCL(1);							// (SCL)	H Imp
		if( digitalRead(PORT_SCL)==1 &&
			digitalRead(PORT_SDA)==1  ) break;
		delay(1);
	}
	if(i==0) i2c_error("I2C_Init / Locked Lines");
	_delayMicroseconds(I2C_RAMDA*8);
	return(i);
}

byte i2c_close(void){
	byte i;
	i2c_log("i2c_close");
	return 0;
}

byte i2c_start(void){
/* 0=エラー	*/
//	if(!i2c_init())return(0);				// SDA,SCL	H Out
	int i;

	for(i=5000;i>0;i--){					// リトライ 5000ms
		i2c_SDA(1);							// (SDA)	H Imp
		i2c_SCL(1);							// (SCL)	H Imp
		if( digitalRead(PORT_SCL)==1 &&
			digitalRead(PORT_SDA)==1  ) break;
		delay(1);
	}
	i2c_log("i2c_start");
	if(i==0 && ERROR_CHECK) i2c_error("i2c_start / Locked Lines");
	_delayMicroseconds(I2C_RAMDA*8);
	i2c_SDA(0);								// (SDA)	L Out
	_delayMicroseconds(I2C_RAMDA);
	i2c_SCL(0);								// (SCL)	L Out
	return(i);
}

byte i2c_read(byte adr, byte *rx, byte len){
/*
入力：byte adr = I2Cアドレス(7ビット)
出力：byte *rx = 受信データ用ポインタ
入力：byte len = 受信長
戻り値：byte 受信結果長、０の時はエラー
*/
	byte ret,i;
	
	if( !i2c_start() && ERROR_CHECK) return(0);
	adr <<= 1;								// 7ビット->8ビット
	adr |= 0x01;							// RW=1 受信モード
	if( i2c_tx(adr)==0 && ERROR_CHECK ){	// アドレス設定
		i2c_error("I2C_RX / no ACK (Address)");
		return(0);		
	}
	
	/* スレーブ待機状態待ち */
	for(i=GPIO_RETRY;i>0;i--){
		_delayMicroseconds(I2C_RAMDA);
		if( digitalRead(PORT_SDA)==0  ) break;
	}
	if(i==0 && ERROR_CHECK){
		i2c_error("I2C_RX / no ACK (Reading)");
		return(0);
	}
	for(i=10;i>0;i--){
		_delayMicroseconds(I2C_RAMDA);
		if( digitalRead(PORT_SCL)==1  ) break;
	}
	if(i==0 && ERROR_CHECK){
		i2c_error("I2C_RX / Clock Line Holded");
		return(0);
	}
	/* 受信データ */
	for(ret=0;ret<len;ret++){
		i2c_SCL(0);							// (SCL)	L Out
		i2c_SDA(1);							// (SDA)	H Imp
		rx[ret]=0x00;
		for(i=0;i<8;i++){
			i2c_SCL(1);						// (SCL)	H Imp
			rx[ret] |= (digitalRead(PORT_SDA))<<(7-i);		//data[22] b4=Port 12(SDA)
			i2c_SCL(0);						// (SCL)	L Out
		}
		if(ret<len-1){
			// ACKを応答する
			i2c_SDA(0);							// (SDA)	L Out
			i2c_SCL(1);							// (SCL)	H Imp
			_delayMicroseconds(I2C_RAMDA);
		}else{
			// NACKを応答する
			i2c_SDA(1);							// (SDA)	H Imp
			i2c_SCL(1);							// (SCL)	H Imp
			_delayMicroseconds(I2C_RAMDA);
		}
	}
	/* STOP */
	i2c_SCL(0);								// (SCL)	L Out
	i2c_SDA(0);								// (SDA)	L Out
	_delayMicroseconds(I2C_RAMDA);
	i2c_SCL(1);								// (SCL)	H Imp
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp
	return(ret);
}

byte i2c_write(byte adr, byte *tx, byte len){
/*
入力：byte adr = I2Cアドレス(7ビット)
入力：byte *tx = 送信データ用ポインタ
入力：byte len = 送信データ長（0のときはアドレスのみを送信する）
*/
	byte ret=0;
	if( !i2c_start() ) return(0);
	adr <<= 1;								// 7ビット->8ビット
	adr &= 0xFE;							// RW=0 送信モード
	if( i2c_tx(adr)>0 ){
		/* データ送信 */
		for(ret=0;ret<len;ret++){
			i2c_SDA(0);						// (SDA)	L Out
			i2c_SCL(0);						// (SCL)	L Out
			if( i2c_tx(tx[ret]) == 0 && ERROR_CHECK){
				i2c_error("i2c_write / no ACK (Writing)");
				return(0);
			}
		}
	}else if( len>0 && ERROR_CHECK){		// len=0の時はエラーにしないAM2320用
		i2c_error("i2c_write / no ACK (Address)");
		return(0);
	}
	/* STOP */
	i2c_SDA(0);								// (SDA)	L Out
	i2c_SCL(0);								// (SCL)	L Out
	_delayMicroseconds(I2C_RAMDA);
	if(len==0)_delayMicroseconds(800);		// AM2320用
	i2c_SCL(1);								// (SCL)	H Imp
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp
	return(ret);
}

void i2c_lcd_out(byte y,byte *lcd){
	byte data[2];
	byte i;
	data[0]=0x00;
	if(y==0) data[1]=0x80;
	else{
		data[1]=0xC0;
		y=1;
	}
	i2c_write(I2C_lcd,data,2);
	for(i=0;i<8;i++){
		if(lcd[i]==0x00) break;
		data[0]=0x40;
		data[1]=lcd[i];
		i2c_write(I2C_lcd,data,2);
	}
}

void utf_del_uni(char *s){
	byte i=0;
	byte j=0;
	while(s[i]!='\0'){
		if((byte)s[i]==0xEF){
			if((byte)s[i+1]==0xBE) s[i+2] += 0x40;
			i+=2;
		}
		// fprintf(stderr,"%02X ",s[i]);
		if(isprint(s[i]) || (s[i]>=0xA1 && s[i] <=0xDF)){
			s[j]=s[i];
			j++;
		}
		i++;
	}
	s[j]='\0';
	// fprintf(stderr,"len=%d\n",j);
}

void i2c_lcd_init(void){
	byte data[2];
	data[0]=0x00; data[1]=0x39; i2c_write(I2C_lcd,data,2);	// IS=1
	data[0]=0x00; data[1]=0x11; i2c_write(I2C_lcd,data,2);	// OSC
	data[0]=0x00; data[1]=0x70; i2c_write(I2C_lcd,data,2);	// コントラスト	0
	data[0]=0x00; data[1]=0x56; i2c_write(I2C_lcd,data,2);	// Power/Cont	6
	data[0]=0x00; data[1]=0x6C; i2c_write(I2C_lcd,data,2);	// FollowerCtrl	C
	delay(200);
	data[0]=0x00; data[1]=0x38; i2c_write(I2C_lcd,data,2);	// IS=0
	data[0]=0x00; data[1]=0x0C; i2c_write(I2C_lcd,data,2);	// DisplayON	C
}

void i2c_lcd_print(char *s){
	byte i,j;
	char str[49];
	byte lcd[9];
	
	strncpy(str,s,48);
	utf_del_uni(str);
	for(j=0;j<2;j++){
		lcd[8]='\0';
		for(i=0;i<8;i++){
			lcd[i]=(byte)str[i+8*j];
			if(lcd[i]==0x00){
				for(;i<8;i++) lcd[i]=' ';
				i2c_lcd_out(j,lcd);
				if(j==0){
					for(i=0;i<8;i++) lcd[i]=' ';
					i2c_lcd_out(1,lcd);
				}
				return;
			}
		}
		i2c_lcd_out(j,lcd);
	}
}

void i2c_lcd_printIp(uint32_t ip){
	char lcd[17];
	
	sprintf(lcd,"%i.%i.    ",
		ip & 255,
		ip>>8 & 255
	);
	sprintf(&lcd[8],"%i.%i",
		ip>>16 & 255,
		ip>>24
	);
	i2c_lcd_print(lcd);
}

/******************************************************************************/
/* トランジスタ技術 2016.6 ESP-WROOM-02特集記事用 I2C LCD ライブラリ 互換 API */
/*																			  */
/*										Copyright (c) 2014-2017 Wataru KUNINO */
/******************************************************************************/

void _utf_del_uni(char *s){
	utf_del_uni(s);
}
void lcdOut(byte y,byte *lcd){
	i2c_lcd_out(y,lcd);
}

void lcdPrint(char *s){
	i2c_lcd_print(s);
}

void lcdPrint2(char *s){
	byte i;
	char str[65];
	byte lcd[21];
	
	strncpy(str,s,64);
	_utf_del_uni(str);
	lcd[_lcd_size_x]='\0';
	for(i=0;i<_lcd_size_x;i++){
		lcd[i]=(byte)str[i];
		if(lcd[i]==0x00){
			for(;i<_lcd_size_x;i++) lcd[i]=' ';
			lcdOut(1,lcd);
			return;
		}
	}
	lcdOut(1,lcd);
}

void lcdPrintIp(uint32_t ip){
	i2c_lcd_printIp(ip);
}

void i2c_lcdPrintIp2(uint32_t ip){
	char lcd[21];
	
	sprintf(lcd,"%i.%i.%i.%i",
		ip & 255,
		ip>>8 & 255,
		ip>>16 & 255,
		ip>>24
	);
	if(_lcd_size_x>=16) lcdPrint2(lcd);
	else lcdPrint(lcd);
}

void i2c_lcdPrintVal(char *s,int in){
	char lcd[21];
	sprintf(lcd,"%d",in);
	lcdPrint(s);
	lcdPrint2(lcd);
}

void lcdSetup() {
	i2c_lcd_init();
	lcdPrint("Hello!  I2C LCD by Wataru Kunino");
}
