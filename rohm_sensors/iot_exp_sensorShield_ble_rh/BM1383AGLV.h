/*****************************************************************************
  BM1383AGLV.h

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
#ifndef _BM1383AGLV_H_
#define _BM1383AGLV_H_

#define BM1383AGLV_DEVICE_ADDRESS           (0x5D)    // 7bit Addrss
#define BM1383AGLV_ID_VAL                   (0x32)

#define BM1383AGLV_ID                       (0x10)
#define BM1383AGLV_POWER_DOWN               (0x12)
#define BM1383AGLV_RESET                    (0x13)
#define BM1383AGLV_MODE_CONTROL             (0x14)
#define BM1383AGLV_PRESSURE_MSB             (0x1A)

#define BM1383AGLV_POWER_DOWN_PWR_DOWN          (1 << 0)
#define BM1383AGLV_RESET_RSTB                   (1 << 0)
#define BM1383AGLV_MODE_CONTROL_AVE_NUM64       (6 << 5)
#define BM1383AGLV_MODE_CONTROL_RESERVED_3BIT   (1 << 3)
#define BM1383AGLV_MODE_CONTROL_MODE_CONTINUOUS (4 << 0)

#define BM1383AGLV_POWER_DOWN_VAL      (BM1383AGLV_POWER_DOWN_PWR_DOWN)
#define BM1383AGLV_RESET_VAL           (BM1383AGLV_RESET_RSTB)
#define BM1383AGLV_MODE_CONTROL_VAL    (BM1383AGLV_MODE_CONTROL_AVE_NUM64 | BM1383AGLV_MODE_CONTROL_RESERVED_3BIT | BM1383AGLV_MODE_CONTROL_MODE_CONTINUOUS)

#define HPA_PER_COUNT                           (2048)
#define DEGREES_CELSIUS_PER_COUNT               (32)
#define GET_BYTE_PRESS_TEMP                     (5)
#define WAIT_TMT_MAX                            (240)
#define WAIT_BETWEEN_POWER_DOWN_AND_RESET       (2)

class BM1383AGLV
{
  public:
      BM1383AGLV(void);
    byte init(void) ;
    byte get_rawval(unsigned char *data);
    byte get_val(float *press, float *temp);
    byte write(unsigned char memory_address, unsigned char *data, unsigned char size);
    byte read(unsigned char memory_address, unsigned char *data, int size);
};

#endif // _BM1383AGLV_H_
