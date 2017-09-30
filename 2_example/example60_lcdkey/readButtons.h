/*******************************************************************************
ESP32用 アナログ入力関数 readButtons mvAnalogIn
                                           Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/

#define BUTTON_UP       0x08
#define BUTTON_DOWN     0x04
#define BUTTON_LEFT     0x10
#define BUTTON_RIGHT    0x02
#define BUTTON_SELECT   0x01

uint8_t readButtons(uint8_t PIN);
float mvAnalogIn(uint8_t PIN);
