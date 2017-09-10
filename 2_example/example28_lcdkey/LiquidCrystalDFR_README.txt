/*********************************************************************
DF ROBOT LCD Keypad Shield 用 液晶＆キーパッドドライバ

Arduino 標準ライブラリにLiquidCrystalにDF ROBOT社Keypad用関数を
追加しました。

関連情報
	http://www.geocities.jp/bokunimowakaru/diy/arduino/lcds.html

本ソースリストおよびソフトウェアはGPL V2に基づくライセンスを有します。
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

                               Copyright (c) 2012-2017 Wataru KUNINO
                               http://www.geocities.jp/bokunimowakaru/
*********************************************************************/

/*
	ファイル：
		LiquidCrystal.h
		LiquidCrystal.cpp

	場所(例)：
		C:\arduino\libraries\LiquidCrystal

*/

/*
LCD部
	Arduino UNO   13 12 11 10  9  8 | 7  6  5  4  3  2  1  0
	DOIT ESPduino NC  1  3 16 15 14 |13 12  5  4  2  0 TX RX

	Arduino用	LiquidCrystal lcd( 8, 9, 4, 5, 6, 7);  
	ESPduino用	LiquidCrystal lcd(14,15, 4, 5,12,13);  

	lcd.begin(16, 2);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Hello!");

KEY部

	Arduino		int adc_key_in = analogRead(0);
	ESPduino	int adc_key_in = system_adc_read();

	ESPduino基板上の分割抵抗、100k / (100k + 400k)で、
	0～5Vの入力を0～1Vへ変換しているので、そのまま接続可能

	uint8_t buttons = lcd.readButtons();
	
*/

/******************************************************************************/

/*******************************************************************************
表示・キーパッド入力用サンプル
*******************************************************************************/

#include "LiquidCrystalDFR.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
  int time = millis();
  lcd.print("Hello, world!");
  time = millis() - time;
  Serial.print("Took "); Serial.print(time); Serial.println(" ms");
}

uint8_t i=0;
void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);

  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
      lcd.print("UP ");
    }
    if (buttons & BUTTON_DOWN) {
      lcd.print("DOWN ");
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("LEFT ");
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.print("RIGHT ");
    }
    if (buttons & BUTTON_SELECT) {
      lcd.print("SELECT ");
    }
  }
}


/*******************************************************************************
Arduino 用 変更点
*******************************************************************************/

/***************************************
LiquidCrystal.h		60行目
****************************************/

// for button by Wataru
#define BUTTON_UP 0x08
#define BUTTON_DOWN 0x04
#define BUTTON_LEFT 0x10
#define BUTTON_RIGHT 0x02
#define BUTTON_SELECT 0x01

/***************************************
LiquidCrystal.h		106行目
****************************************/

  uint8_t readButtons();	// Wataru

/***************************************
LiquidCrystal.cpp	327行目から
****************************************/

	// Wataru
	/******************************************************

	This program will test the LCD panel and the buttons
	Mark Bramwell, July 2010 

	********************************************************/

	uint8_t LiquidCrystal::readButtons(void) {
	  int adc_key_in = analogRead(0);      // read the value from the sensor
	  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741 
	  // we add approx 50 to those values and check to see if we are close 
	  if (adc_key_in > 1000) return 0x00; // We make this the 1st option for speed reasons since it will be the most likely result 
	  if (adc_key_in < 50)   return BUTTON_RIGHT;
	  if (adc_key_in < 195)  return BUTTON_UP;
	  if (adc_key_in < 380)  return BUTTON_DOWN;
	  if (adc_key_in < 555)  return BUTTON_LEFT;
	  if (adc_key_in < 790)  return BUTTON_SELECT;
	  return 0x1F;  // when all others fail, return this...
	}

/******************************************************************************/
