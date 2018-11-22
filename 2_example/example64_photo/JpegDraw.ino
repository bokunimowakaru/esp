/*******************************************************************************
Example 64 JPEG・BMP 描画 Wrapper

                                      Copyright (c) 2016-2019 Wataru KUNINO

本ソースリストは国野亘がJPEG描画用に改変したものです。
下記からダウンロードしたものを基にしました。

    https://learn.adafruit.com/096-mini-color-oled/drawing-bitmaps

構造図

    JpegDraw.ino
        |
        +------ JPEGDecoder.h, JPEGDecoder.cpp
        |       JPEG Decoder for Arduino / Public domain, Makoto Kurauchi
        |           |
        |           +------ picojpeg.h, picojpeg.c
        |                   picojpeg.c / Public domain, Rich Geldreich
        |
        +------ Adafruit_SPITFT / Adafruit_GFX

*******************************************************************************/

// Color definitions
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF

#include "JPEGDecoder.h"
#include <inttypes.h>

int _jpeg_div(int in , int max){
    if( max >= 32){
        if( in >= 16 ) return 16;
        if( in >= 10 ) return 10;
    }
    if( in >= max ) return max;
    if( in >= 16 ) return 16;
    if( in >= 8 ) return 8;
    if( in >= 5 ) return 5;
    if( in >= 1 ) return in;
    return 1;
}

void _jpeg_avrage(uint8 *color, uint8 *newdata, int n){
    int i;
    uint16_t val;
    
    for(i=0;i<3;i++){
        val=(uint16_t)(color[i]);
        val*=n;
        val+=newdata[i];
        val/=(n+1);
        color[i]=(uint8)(val);
    }
}

void jpegDraw(File file){
    uint8 *pImg;
    unsigned int index;
    unsigned int pImg_len;
    int x,y,axis=0;
    int bx,by;
    int div_x,div_y;
    uint8 color[16][16][3];
    uint8 color_n[16][16];
    int color_x,color_y;
    uint16_t color_out;
    
    JpegDec.begin();
    JpegDec.decode(file,0);
    
    if(JpegDec.width >= JpegDec.height){
        div_x = _jpeg_div(JpegDec.width / oled.width(), JpegDec.MCUWidth);
    }else{
        axis=1;
        div_x = _jpeg_div(JpegDec.height / oled.width(), JpegDec.MCUHeight);
    }
    div_y=div_x;
    while(JpegDec.read()){
        index=0;
        for(by=0;by<16;by++)for(bx=0;bx<16;bx++)for(int i=0;i<3;i++){
            color[by][bx][i]=0;
            color_n[by][bx]=0;
        }
        pImg = JpegDec.pImage ;
        pImg_len = (unsigned int)(JpegDec.MCUWidth * JpegDec.MCUHeight * JpegDec.comps);
        for(by=0; by<JpegDec.MCUHeight; by++){
            for(bx=0; bx<JpegDec.MCUWidth; bx++){
                color_y=by/div_y; if( color_y > 15) color_y = 15;
                color_x=bx/div_x; if( color_x > 15) color_x = 15;
                _jpeg_avrage(&color[color_y][color_x][0], &pImg[index], color_n[color_y][color_x]);
                color_n[color_y][color_x]++;
                index += JpegDec.comps ;
                if(index >= pImg_len) index -= JpegDec.comps ;
            }
        }
        for(by=0; by<JpegDec.MCUHeight; by+= div_y){
            for(bx=0; bx<JpegDec.MCUWidth; bx+= div_x){
                y = JpegDec.MCUy * JpegDec.MCUHeight + by;
                y /= div_y;
                x = JpegDec.MCUx * JpegDec.MCUWidth + bx;
                x /= div_x;
                color_y=by/div_y; if( color_y > 15) color_y = 15;
                color_x=bx/div_x; if( color_x > 15) color_x = 15;
                if(color_n[color_y][color_x]) color_out= oled.Color565(color[color_y][color_x][0],color[color_y][color_x][1],color[color_y][color_x][2]);
                else color_out = 0;
                if(axis) oled.goTo(y, x); else oled.goTo(x, y);
                oled.pushColor(color_out);
            }
        }
    }
}

#define BUFFPIXEL 20

void bmpDraw(File bmpFile) {
    bmpDraw(bmpFile,0,0);
    bmpFile.close();
}

void bmpDraw(File bmpFile, uint8_t x, uint8_t y) {
    int      bmpWidth, bmpHeight;   // W+H in pixels
    uint8_t  bmpDepth;              // Bit depth (currently must be 24)
    uint32_t bmpImageoffset;        // Start of image data in file
    uint32_t rowSize;               // Not always = bmpWidth; may have padding
    uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
    uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
    boolean  goodBmp = false;       // Set to true on valid header parse
    boolean  flip    = true;        // BMP is stored bottom-to-top
    int      w, h, row, col;
    uint8_t  r, g, b;
    uint32_t pos = 0, startTime = millis();

    if((x >= oled.width()) || (y >= oled.height())) return;

    Serial.println();
    Serial.print("Loading image '");
    Serial.print(bmpFile.name());
    Serial.println('\'');

    // Parse BMP header
    if(read16(bmpFile) == 0x4D42) { // BMP signature
        Serial.print("File size: "); Serial.println(read32(bmpFile));
        (void)read32(bmpFile); // Read & ignore creator bytes
        bmpImageoffset = read32(bmpFile); // Start of image data
        Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
        // Read DIB header
        Serial.print("Header size: "); Serial.println(read32(bmpFile));
        bmpWidth    = read32(bmpFile);
        bmpHeight = read32(bmpFile);
        if(read16(bmpFile) == 1) { // # planes -- must be '1'
            bmpDepth = read16(bmpFile); // bits per pixel
            Serial.print("Bit Depth: "); Serial.println(bmpDepth);
            if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

                goodBmp = true; // Supported BMP format -- proceed!
                Serial.print("Image size: ");
                Serial.print(bmpWidth);
                Serial.print('x');
                Serial.println(bmpHeight);

                // BMP rows are padded (if needed) to 4-byte boundary
                rowSize = (bmpWidth * 3 + 3) & ~3;

                // If bmpHeight is negative, image is in top-down order.
                // This is not canon but has been observed in the wild.
                if(bmpHeight < 0) {
                    bmpHeight = -bmpHeight;
                    flip            = false;
                }

                // Crop area to be loaded
                w = bmpWidth;
                h = bmpHeight;
                if((x+w-1) >= oled.width())    w = oled.width()    - x;
                if((y+h-1) >= oled.height()) h = oled.height() - y;

                for (row=0; row<h; row++) { // For each scanline...
                    oled.goTo(x, y+row);

                    // Seek to start of scan line.    It might seem labor-
                    // intensive to be doing this on every line, but this
                    // method covers a lot of gritty details like cropping
                    // and scanline padding.    Also, the seek only takes
                    // place if the file position actually needs to change
                    // (avoids a lot of cluster math in SD library).
                    if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
                        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
                    else         // Bitmap is stored top-to-bottom
                        pos = bmpImageoffset + row * rowSize;
                    if(bmpFile.position() != pos) { // Need seek?
                        bmpFile.seek(pos);
                        buffidx = sizeof(sdbuffer); // Force buffer reload
                    }

                    // optimize by setting pins now
                    for (col=0; col<w; col++) { // For each pixel...
                        // Time to read more pixel data?
                        if (buffidx >= sizeof(sdbuffer)) { // Indeed
                            bmpFile.read(sdbuffer, sizeof(sdbuffer));
                            buffidx = 0; // Set index to beginning
                        }

                        // Convert pixel from BMP to TFT format, push to display
                        b = sdbuffer[buffidx++];
                        g = sdbuffer[buffidx++];
                        r = sdbuffer[buffidx++];

                        //oled.drawPixel(x+col, y+row, oled.Color565(r,g,b));
                        // optimized!
                        oled.pushColor(oled.Color565(r,g,b));
                    } // end pixel
                } // end scanline
                Serial.print("Loaded in ");
                Serial.print(millis() - startTime);
                Serial.println(" ms");
            } // end goodBmp
        }
    }
    bmpFile.close();
    if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
}

uint32_t read32(File f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
}

int jpegDrawSlide(File file){
    if(!file.isDirectory()){
        String filename = file.name();
        Serial.println(filename);
        if( filename.endsWith(".jpg") || filename.endsWith(".JPG") ){
            Serial.print("found jpeg file : ");
            Serial.println(filename);
            jpegDraw(file);
            oled.setTextColor(BLACK);
            oled.setTextSize(0);
            oled.setCursor(1, 1);
            oled.println(filename.substring(1));
            oled.setCursor(96-6*4, 1);
            oled.println("JPEG");
            return 1;
        }
        if( filename.endsWith(".bmp") || filename.endsWith(".BMP") ){
            Serial.print("found bmp file  : ");
            Serial.println(filename);
            bmpDraw(file);
            oled.setTextColor(BLACK);
            oled.setTextSize(0);
            oled.setCursor(1, 1);
            oled.println(filename.substring(1));
            oled.setCursor(96-6*3, 1);
            oled.println("BMP");
            return 1;
        }
    }
    return 0;
}

File _SlideShowRoot;

void jpegDrawSlideShowNext(fs::FS &fs){
    File file = _SlideShowRoot.openNextFile();
    if(!file){
		_SlideShowRoot.close();
		_SlideShowRoot = fs.open("/");
		file = _SlideShowRoot.openNextFile();
	}
    while(file){
        Serial.println(file.name());
        if( jpegDrawSlide(file) ) break;
        file = _SlideShowRoot.openNextFile();
    }
}

int jpegDrawSlideShowBegin(fs::FS &fs){
    _SlideShowRoot = fs.open("/");
    if (!_SlideShowRoot) {
        Serial.println("Failed to open directory");
        return -1;
    }
    jpegDrawSlideShowNext(fs);
    return 0;
}

int jpegDrawSlideShowMain(fs::FS &fs,int wait_s){
    int count=0;
    File root = fs.open("/");
    if (!root) {
        Serial.println("Failed to open directory");
        return -1;
    }
    File file = root.openNextFile();
    while(file){
        Serial.println(file.name());
        if( jpegDrawSlide(file) ){
            count++;
            delay(wait_s * 1000);
        }
        file = root.openNextFile();
    }
    root.close();
    return count;
}



