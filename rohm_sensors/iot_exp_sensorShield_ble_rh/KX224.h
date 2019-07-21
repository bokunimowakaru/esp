/*****************************************************************************
  KX224.h

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
#ifndef _KX224_H_
#define _KX224_H_

#define KX224_DEVICE_ADDRESS_1E   (0x1E)    // 7bit Addrss
#define KX224_DEVICE_ADDRESS_1F   (0x1F)    // 7bit Address
#define KX224_WAI_VAL             (0x2B)

#define KX224_XOUT_L              (0x06)
#define KX224_WHO_AM_I            (0x0F)
#define KX224_CNTL1               (0x18)
#define KX224_ODCNTL              (0x1B)

#define KX224_CNTL1_TPE           (1 << 0)
#define KX224_CNTL1_WUFE          (1 << 1)
#define KX224_CNTL1_TDTE          (1 << 2)
#define KX224_CNTL1_GSELMASK      (0x18)
#define KX224_CNTL1_GSEL_8G       (0x00)
#define KX224_CNTL1_GSEL_16G      (0x08)
#define KX224_CNTL1_GSEL_32G      (0x10)
#define KX224_CNTL1_DRDYE         (1 << 5)
#define KX224_CNTL1_RES           (1 << 6)
#define KX224_CNTL1_PC1           (1 << 7)

#define KX224_ODCNTL_OSA_50HZ     (2)
#define KX224_ODCNTL_LPRO         (1 << 6)
#define KX224_IIR_BYPASS          (1 << 7)

#define KX224_CNTL1_VAL           (KX224_CNTL1_RES | KX224_CNTL1_GSEL_8G)
#define KX224_ODCNTL_VAL          (KX224_ODCNTL_OSA_50HZ)


class KX224
{
  public:
    KX224(int slave_address);
    ~KX224();
    byte init(void);
    byte get_rawval(unsigned char *data);
    byte get_val(float *data);
    byte write(unsigned char memory_address, unsigned char *data, unsigned char size);
    byte read(unsigned char memory_address, unsigned char *data, int size);
  private:
    int _device_address;
    unsigned short _g_sens;
};

#endif // _KX224_H_
