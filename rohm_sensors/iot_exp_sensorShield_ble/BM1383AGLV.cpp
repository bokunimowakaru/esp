/*****************************************************************************
  BM1383AGLV.cpp

 Copyright (c) 2018 ROHM Co.,Ltd.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
******************************************************************************/
#include <Wire.h>
#include "Arduino.h"
#include "BM1383AGLV.h"

BM1383AGLV::BM1383AGLV(void)
{

}

byte BM1383AGLV::init(void)
{
  byte rc;
  unsigned char reg;

  rc = read(BM1383AGLV_ID, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't access BM1383AGLV");
    return (rc);
  }
  Serial.print("BM1383GL ID Register Value = 0x");
  Serial.println(reg, HEX);

  if (reg != BM1383AGLV_ID_VAL) {
    Serial.println("Can't find BM1383AGLV");
    return (rc);
  }

  reg = BM1383AGLV_POWER_DOWN_VAL;
  rc = write(BM1383AGLV_POWER_DOWN, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't write BM1383AGLV POWER_DOWN register");
    return (rc);
  }

  delay(WAIT_BETWEEN_POWER_DOWN_AND_RESET);

  reg = BM1383AGLV_RESET_VAL;
  rc = write(BM1383AGLV_RESET, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't write BM1383AGLV SLEEP register");
    return (rc);
  }

  reg = BM1383AGLV_MODE_CONTROL_VAL;
  rc = write(BM1383AGLV_MODE_CONTROL, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't write BM1383AGLV MODE_CONTROL register");
    return (rc);
  }

  delay(WAIT_TMT_MAX);

  return (rc);
  
}

byte BM1383AGLV::get_rawval(unsigned char *data)
{
  byte rc;

  rc = read(BM1383AGLV_PRESSURE_MSB, data, GET_BYTE_PRESS_TEMP);
  if (rc != 0) {
    Serial.println("Can't get BM1383AGLV PRESS value");
  }

  return (rc);
}

byte BM1383AGLV::get_val( float *press, float *temp)
{
  byte rc;
  unsigned char val[GET_BYTE_PRESS_TEMP];
  unsigned long rawpress;
  short rawtemp;

  rc = get_rawval(val);
  if (rc != 0) {
    return (rc);
  }

  rawpress = (((unsigned long)val[0] << 16) | ((unsigned long)val[1] << 8) | val[2]&0xFC) >> 2;

  if (rawpress == 0) {
    return (-1);
  }

  *press = (float)rawpress / HPA_PER_COUNT;

  rawtemp = ((short)val[3] << 8) | val[4];

  if (rawtemp == 0) {
    return (-1);
  }

  *temp = (float)rawtemp / DEGREES_CELSIUS_PER_COUNT;
  
  return (rc);
}

byte BM1383AGLV::write(unsigned char memory_address, unsigned char *data, unsigned char size)
{
  byte rc;
  unsigned int cnt;

  Wire.beginTransmission(BM1383AGLV_DEVICE_ADDRESS);
  Wire.write(memory_address);
  Wire.write(data, size);
  rc = Wire.endTransmission();
  return (rc);
}

byte BM1383AGLV::read(unsigned char memory_address, unsigned char *data, int size)
{
  byte rc;
  unsigned char cnt;

  Wire.beginTransmission(BM1383AGLV_DEVICE_ADDRESS);
  Wire.write(memory_address);
  rc = Wire.endTransmission(false);
  if (rc != 0) {
    return (rc);
  }

  Wire.requestFrom((int)BM1383AGLV_DEVICE_ADDRESS, (int)size, (int)true);
  cnt = 0;
  while(Wire.available()) {
    data[cnt] = Wire.read();
    cnt++;
  }

  return (rc);
}
