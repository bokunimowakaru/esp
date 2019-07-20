/*****************************************************************************
  RPR-0521RS.h

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
#ifndef _RPR0521RS_H_
#define _RPR0521RS_H_

#define RPR0521RS_DEVICE_ADDRESS                   (0x38)    // 7bit Addrss
#define RPR0521RS_PART_ID_VAL                      (0x0A)
#define RPR0521RS_MANUFACT_ID_VAL                  (0xE0)

#define RPR0521RS_SYSTEM_CONTROL                   (0x40)
#define RPR0521RS_MODE_CONTROL                     (0x41)
#define RPR0521RS_ALS_PS_CONTROL                   (0x42)
#define RPR0521RS_PS_CONTROL                       (0x43)
#define RPR0521RS_PS_DATA_LSB                      (0x44)
#define RPR0521RS_ALS_DATA0_LSB                    (0x46)
#define RPR0521RS_MANUFACT_ID                      (0x92)

#define RPR0521RS_MODE_CONTROL_MEASTIME_100_100MS  (6 << 0)
#define RPR0521RS_MODE_CONTROL_PS_EN               (1 << 6)
#define RPR0521RS_MODE_CONTROL_ALS_EN              (1 << 7)

#define RPR0521RS_ALS_PS_CONTROL_LED_CURRENT_100MA (2 << 0)
#define RPR0521RS_ALS_PS_CONTROL_DATA1_GAIN_X1     (0 << 2)
#define RPR0521RS_ALS_PS_CONTROL_DATA0_GAIN_X1     (0 << 4)

#define RPR0521RS_PS_CONTROL_PS_GAINX1             (0 << 4)

#define RPR0521RS_MODE_CONTROL_VAL                 (RPR0521RS_MODE_CONTROL_MEASTIME_100_100MS | RPR0521RS_MODE_CONTROL_PS_EN | RPR0521RS_MODE_CONTROL_ALS_EN)
#define RPR0521RS_ALS_PS_CONTROL_VAL               (RPR0521RS_ALS_PS_CONTROL_DATA0_GAIN_X1 | RPR0521RS_ALS_PS_CONTROL_DATA1_GAIN_X1 | RPR0521RS_ALS_PS_CONTROL_LED_CURRENT_100MA)
#define RPR0521RS_PS_CONTROL_VAL                   (RPR0521RS_PS_CONTROL_PS_GAINX1)

#define RPR0521RS_NEAR_THRESH                      (1000) // example value
#define RPR0521RS_FAR_VAL                          (0)
#define RPR0521RS_NEAR_VAL                         (1)

#define RPR0521RS_ERROR                            (-1)

class RPR0521RS
{
  public:
      RPR0521RS(void);
    byte init(void) ;
    byte get_rawpsalsval(unsigned char *data);
    byte get_psalsval(unsigned short *ps, float *als);
    byte check_near_far(unsigned short data);
    float convert_lx(unsigned short *data);
    byte write(unsigned char memory_address, unsigned char *data, unsigned char size);
    byte read(unsigned char memory_address, unsigned char *data, int size); 
  private:
    unsigned short _als_data0_gain;
    unsigned short _als_data1_gain;
    unsigned short _als_measure_time;
};

#endif // _RPR0521RS_H_
