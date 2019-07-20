/*****************************************************************************
  KX224.cpp

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
//#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Wire.h>
#include "KX224.h"

KX224::KX224(int slave_address)
{
  _device_address = slave_address;
  _g_sens = 4096;
}

KX224::~KX224()
{
  _device_address = 0;
  _g_sens = 0;
}

byte KX224::init(void)
{
  byte rc;
  unsigned char reg;
  unsigned char gsel;

  rc = read(KX224_WHO_AM_I, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't access KX224");
    return (rc);
  } 
  Serial.print("KX224_WHO_AMI Register Value = 0x");
  Serial.println(reg, HEX);
  
  if (reg != KX224_WAI_VAL) {
    Serial.println("Can't find KX224");
    return (rc);
  }

  reg = KX224_CNTL1_VAL;
  rc = write(KX224_CNTL1, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't write KX224 CNTL1 register at first");
    return (rc);
  }

  reg = KX224_ODCNTL_VAL;
  rc = write(KX224_ODCNTL, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't write KX224 ODCNTL register");
    return (rc);
  }

  rc = read(KX224_CNTL1, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't read KX224 CNTL1 register");
    return (rc);
  }
  gsel = reg & KX224_CNTL1_GSELMASK;

  reg |= KX224_CNTL1_PC1;
  rc = write(KX224_CNTL1, &reg, sizeof(reg));
  if (rc != 0) {
    Serial.println("Can't write KX224 CNTL1 register at second");
    return (rc);
  }
  
  switch(gsel) {
    case KX224_CNTL1_GSEL_8G :
      // (Equivalent counts) / (Range) = (32768 / 8)
      _g_sens = 4096;
    break;

    case KX224_CNTL1_GSEL_16G :
      // (Equivalent counts) / (Range) = (32768 / 16)
      _g_sens = 2048;
    break;

    case KX224_CNTL1_GSEL_32G :
      // (Equivalent counts) / (Range) = (32768 / 32)
      _g_sens = 1024;
    break;

    default:
    break;
  }
  return (rc);
}

byte KX224::get_rawval(unsigned char *data)
{
  byte rc;

  rc = read(KX224_XOUT_L, data, 6);
  if (rc != 0) {
    Serial.println("Can't get KX224 accel value");
  }

  return (rc);
}

byte KX224::get_val(float *data)
{
  byte rc;
  unsigned char val[6];
  signed short acc[3];

  rc = get_rawval(val);
  if (rc != 0) {
    return (rc);
  }

  acc[0] = ((signed short)val[1] << 8) | (val[0]);
  acc[1] = ((signed short)val[3] << 8) | (val[2]);
  acc[2] = ((signed short)val[5] << 8) | (val[4]);

  // Convert LSB to g
  data[0] = (float)acc[0] / _g_sens;
  data[1] = (float)acc[1] / _g_sens;
  data[2] = (float)acc[2] / _g_sens;

  return (rc);  
}

byte KX224::write(unsigned char memory_address, unsigned char *data, unsigned char size)
{
  byte rc;

  Wire.beginTransmission(_device_address);
  Wire.write(memory_address);
  Wire.write(data, size);
  rc = Wire.endTransmission(true);
  return (rc);
}

byte KX224::read(unsigned char memory_address, unsigned char *data, int size)
{
  byte rc;
  unsigned char cnt;

  Wire.beginTransmission(_device_address);
  Wire.write(memory_address);
  rc = Wire.endTransmission(false);
  if (rc != 0) {
    return (rc);
  }

  Wire.requestFrom(_device_address, size, true);
  cnt = 0;
  while(Wire.available()) {
    data[cnt] = Wire.read();
    cnt++;
  }

  return (0);
}
