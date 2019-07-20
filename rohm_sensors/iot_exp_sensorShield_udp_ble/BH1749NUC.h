/*****************************************************************************
  BH1749NUC.h

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
#ifndef _BH1749NUC_H_
#define _BH1749NUC_H_

#define BH1749NUC_DEVICE_ADDRESS_38             (0x38)    // 7bit Addrss
#define BH1749NUC_DEVICE_ADDRESS_39             (0x39)    // 7bit Addrss
#define BH1749NUC_PART_ID_VAL                   (0x0D)
#define BH1749NUC_MANUFACT_ID_VAL               (0xE0)

#define BH1749NUC_SYSTEM_CONTROL                (0x40)
#define BH1749NUC_MODE_CONTROL1                 (0x41)
#define BH1749NUC_MODE_CONTROL2                 (0x42)
#define BH1749NUC_RED_DATA_LSB                  (0x50)
#define BH1749NUC_MANUFACTURER_ID               (0x92)

#define BH1749NUC_MODE_CONTROL1_MEAS_MODE_120MS (2)
#define BH1749NUC_MODE_CONTROL1_MEAS_MODE_240MS (3)
#define BH1749NUC_MODE_CONTROL1_RGB_GAIN_X1     (1 << 3)
#define BH1749NUC_MODE_CONTROL1_RGB_GAIN_X32    (3 << 3)
#define BH1749NUC_MODE_CONTROL1_IR_GAIN_X1      (1 << 5)
#define BH1749NUC_MODE_CONTROL1_IR_GAIN_X32     (3 << 5)
#define BH1749NUC_MODE_CONTROL2_RGB_EN          (1 << 4)

#define BH1749NUC_MODE_CONTROL1_VAL             (BH1749NUC_MODE_CONTROL1_MEAS_MODE_120MS | BH1749NUC_MODE_CONTROL1_RGB_GAIN_X1 | BH1749NUC_MODE_CONTROL1_IR_GAIN_X1)
#define BH1749NUC_MODE_CONTROL2_VAL             (BH1749NUC_MODE_CONTROL2_RGB_EN)

#define GET_BYTE_RED_TO_GREEN2 (12)
#define WAIT_TMT2_MAX          (340)

class BH1749NUC
{
  public:
    BH1749NUC(int slave_address);
    ~BH1749NUC();
    byte init(void) ;
    byte get_rawval(unsigned char *data);
    byte get_val(unsigned short *data);
    byte write(unsigned char memory_address, unsigned char *data, unsigned char size);
    byte read(unsigned char memory_address, unsigned char *data, int size);
  private:
      int _device_address;
};

#endif // _BH1749NUC_H_
