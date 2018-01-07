/*
本ソースリストは2017/11/2に下記からダウンロードしたものを、国野亘が改変したものです。
原作者の権利内容を継承します。

	https://github.com/MakotoKurauchi/JPEGDecoder

2017/11/2 国野 亘
*/

/*
 JPEGDecoder.cpp
 
 JPEG Decoder for Arduino
 Public domain, Makoto Kurauchi <http://yushakobo.jp>
*/

#include <SD.h>
#include "JPEGDecoder.h"
#include "picojpeg.h"

#define DEBUG_JPEG

JPEGDecoder JpegDec;


JPEGDecoder::JPEGDecoder(){
    mcu_x = 0 ;
    mcu_y = 0 ;
    is_available = 0;
    reduce = 0;
    thisPtr = this;
}


JPEGDecoder::~JPEGDecoder(){
    delete pImage;
}


unsigned char JPEGDecoder::pjpeg_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
    JPEGDecoder *thisPtr = JpegDec.thisPtr ;
    thisPtr->pjpeg_need_bytes_callback(pBuf, buf_size, pBytes_actually_read, pCallback_data);
    return 0;
}


unsigned char JPEGDecoder::pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
    uint n,i=0,t=0;

//  pCallback_data;
    
    n = min(g_nInFileSize - g_nInFileOfs, buf_size);

    while( t<3 ){                       // エラー3回以内で繰り返し処理実行
        if(!g_pInFile.available()){          // ファイルの有無を確認
            t++; delay(100);            // ファイル無し時に100msの待ち時間
            continue;                   // whileループに戻ってリトライ
        }
        i += g_pInFile.read(pBuf,n);          // ファイル読み取り
        if(i>=n) break;
    }
/*
    #ifdef DEBUG_JPEG
        Serial.print("FileSize =");
        Serial.print(g_nInFileSize);
        Serial.print(", FileOfs =");
        Serial.print(g_nInFileOfs);
        Serial.print(", BufSize =");
        Serial.print(buf_size);
        Serial.print(", n= ");
        Serial.print(n);
        Serial.print(", ret = ");
        Serial.println(i);
        for(int j;j<16;j++){
            Serial.print(pBuf[j],HEX);
            Serial.print(' ');
        }
        Serial.println();
    #endif
    
    if(i<n){
        Serial.println("ERROR : file read");
        g_nInFileOfs += i;
        return -1;
    }
*/
    *pBytes_actually_read = (unsigned char)(n);
    g_nInFileOfs += n;
    return 0;
}

int JPEGDecoder::begin(){
    mcu_x = 0 ;
    mcu_y = 0 ;
    is_available = 0;
    reduce = 0;
    
    Serial.print("Initializing SD card...");
    if (!SD.begin()) {
        Serial.println("failed!");
        return -1;
    }
    Serial.println("SD OK!");
    return 0;
}

int JPEGDecoder::decode(File pInFile, unsigned char pReduce){
    
    if(pReduce) reduce = pReduce;
    
//  g_pInFile = SD.open(pFilename, FILE_READ);
    g_pInFile = pInFile;
    if (!g_pInFile){
        #ifdef DEBUG_JPEG
            Serial.println("failed to open jpeg file");
        #endif
        return -1;
    }
    g_nInFileOfs = 0;

    g_nInFileSize = g_pInFile.size();
    #ifdef DEBUG_JPEG
        Serial.print("file size = ");
        Serial.println(g_nInFileSize);
    #endif
        
    status = pjpeg_decode_init(&image_info, pjpeg_callback, NULL, (unsigned char)reduce);


    if (status)
    {
        #ifdef DEBUG_JPEG
        Serial.print("ERROR : pjpeg_decode_init() : ");
        /*
        switch(status & 63){
            case    PJPG_NO_MORE_BLOCKS:                Serial.print("NO_MORE_BLOCKS"); break;
            case    PJPG_BAD_DHT_COUNTS:                Serial.print("BAD_DHT_COUNTS"); break;
            case    PJPG_BAD_DHT_INDEX:                 Serial.print("BAD_DHT_INDEX"); break;
            case    PJPG_BAD_DHT_MARKER:                Serial.print("BAD_DHT_MARKER"); break;
            case    PJPG_BAD_DQT_MARKER:                Serial.print("BAD_DQT_MARKER"); break;
            case    PJPG_BAD_DQT_TABLE:                 Serial.print("BAD_DQT_TABLE"); break;
            case    PJPG_BAD_PRECISION:                 Serial.print("BAD_PRECISION"); break;
            case    PJPG_BAD_HEIGHT:                    Serial.print("BAD_HEIGHT"); break;
            case    PJPG_BAD_WIDTH:                     Serial.print("BAD_WIDTH"); break;
            case    PJPG_TOO_MANY_COMPONENTS:           Serial.print("TOO_MANY_COMPONENTS"); break;
            case    PJPG_BAD_SOF_LENGTH:                Serial.print("BAD_SOF_LENGTH"); break;
            case    PJPG_BAD_VARIABLE_MARKER:           Serial.print("BAD_VARIABLE_MARKER"); break;
            case    PJPG_BAD_DRI_LENGTH:                Serial.print("BAD_DRI_LENGTH"); break;
            case    PJPG_BAD_SOS_LENGTH:                Serial.print("BAD_SOS_LENGTH"); break;
            case    PJPG_BAD_SOS_COMP_ID:               Serial.print("BAD_SOS_COMP_ID"); break;
            case    PJPG_W_EXTRA_BYTES_BEFORE_MARKER:   Serial.print("W_EXTRA_BYTES"); break;
            case    PJPG_NO_ARITHMITIC_SUPPORT:         Serial.print("ARITHMITIC"); break;
            case    PJPG_UNEXPECTED_MARKER:             Serial.print("UNEXPECTED_MARKER"); break;
            case    PJPG_NOT_JPEG:                      Serial.print("NOT_JPEG"); break;
            case    PJPG_UNSUPPORTED_MARKER:            Serial.print("UNSUPPORTED_MARKER"); break;
            case    PJPG_BAD_DQT_LENGTH:                Serial.print("BAD_DQT_LENGTH"); break;
            case    PJPG_TOO_MANY_BLOCKS:               Serial.print("TOO_MANY_BLOCKS"); break;
            case    PJPG_UNDEFINED_QUANT_TABLE:         Serial.print("QUANT_TABLE"); break;
            case    PJPG_UNDEFINED_HUFF_TABLE:          Serial.print("HUFF_TABLE"); break;
            case    PJPG_NOT_SINGLE_SCAN:               Serial.print("SINGLE_SCAN"); break;
            case    PJPG_UNSUPPORTED_COLORSPACE:        Serial.print("COLORSPACE"); break;
            case    PJPG_UNSUPPORTED_SAMP_FACTORS:      Serial.print("SAMP_FACTORS"); break;
            case    PJPG_DECODE_ERROR:                  Serial.print("DECODE_ERROR"); break;
            case    PJPG_BAD_RESTART_MARKER:            Serial.print("RESTART_MARKER"); break;
            case    PJPG_ASSERTION_ERROR:               Serial.print("ASSERTION"); break;
            case    PJPG_BAD_SOS_SPECTRAL:              Serial.print("SOS_SPECTRAL"); break;
            case    PJPG_BAD_SOS_SUCCESSIVE:            Serial.print("SOS_SUCCESSIVE"); break;
            case    PJPG_STREAM_READ_ERROR:             Serial.print("STREAM_READ_ERRO"); break;
            case    PJPG_NOTENOUGHMEM:                  Serial.print("NOTENOUGHMEM"); break;
            case    PJPG_UNSUPPORTED_COMP_IDENT:        Serial.print("COMP_IDENT"); break;
            case    PJPG_UNSUPPORTED_QUANT_TABLE:       Serial.print("QUANT_TABLE"); break;
            case    PJPG_UNSUPPORTED_MODE:              Serial.print("progressive JPEG"); break;
            default:
            Serial.print("Unknown");
                break;
        }
        */
        Serial.print(' ');
        Serial.print(status);
        Serial.print(' ');
        Serial.println(status,BIN);
        #endif
        
        g_pInFile.close();
        return -1;
    }
    #ifdef DEBUG_JPEG
    Serial.print("Width     :");
    Serial.println(image_info.m_width);
    Serial.print("Height    :");
    Serial.println(image_info.m_height);
    Serial.print("Components:");
    Serial.println(image_info.m_comps);
    Serial.print("MCU / row :");
    Serial.println(image_info.m_MCUSPerRow);
    Serial.print("MCU / col :");
    Serial.println(image_info.m_MCUSPerCol);
    Serial.print("Scan type :");
    Serial.println(image_info.m_scanType);
    Serial.print("MCU width :");
    Serial.println(image_info.m_MCUWidth);
    Serial.print("MCU height:");
    Serial.println(image_info.m_MCUHeight);
    Serial.println("");
    #endif
    
    // In reduce mode output 1 pixel per 8x8 block.
    decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
    decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;

    row_pitch = image_info.m_MCUWidth * image_info.m_comps;
    //pImage = (uint8 *)malloc(image_info.m_MCUWidth * image_info.m_MCUHeight * image_info.m_comps);
    pImage = new uint8[image_info.m_MCUWidth * image_info.m_MCUHeight * image_info.m_comps];
    if (!pImage)
    {
        g_pInFile.close();
        #ifdef DEBUG_JPEG
        Serial.println("Memory Allocation Failure");
        #endif
        
        return -1;
    }
    // memset(pImage , 0 , sizeof(pImage));
	memset(pImage , 0 , image_info.m_MCUWidth * image_info.m_MCUHeight * image_info.m_comps);

    row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
    col_blocks_per_mcu = image_info.m_MCUHeight >> 3;
    
    is_available = 1 ;

    width = decoded_width;
    height = decoded_height;
    comps = image_info.m_comps;
    MCUSPerRow = image_info.m_MCUSPerRow;
    MCUSPerCol = image_info.m_MCUSPerCol;
    scanType = image_info.m_scanType;
    MCUWidth = image_info.m_MCUWidth;
    MCUHeight = image_info.m_MCUHeight;
    
    return decode_mcu();
}


int JPEGDecoder::decode_mcu(void){

    status = pjpeg_decode_mcu();
    if (status)
    {
        is_available = 0 ;

        g_pInFile.close();

        if (status != PJPG_NO_MORE_BLOCKS)
        {
            #ifdef DEBUG_JPEG
            Serial.print("pjpeg_decode_mcu() failed with status ");
            Serial.println(status);
            #endif
            if(status){
                delete pImage;
                return -1;
            }
        }
    }
    return 1;
}


int JPEGDecoder::read(void)
{
    int y, x;
    uint8 *pDst_row;
    
    if(is_available == 0) return 0;

    if (mcu_y >= image_info.m_MCUSPerCol)
    {
        delete pImage;
        g_pInFile.close();
        return 0;
    }

    if (reduce)
    {
        // In reduce mode, only the first pixel of each 8x8 block is valid.
        pDst_row = pImage;
        if (image_info.m_scanType == PJPG_GRAYSCALE)
        {
            *pDst_row = image_info.m_pMCUBufR[0];
        }
        else
        {
            uint y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
                uint src_ofs = (y * 128U);
                for (x = 0; x < row_blocks_per_mcu; x++)
                {
                    pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
                    pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
                    pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
                    pDst_row += 3;
                    src_ofs += 64;
                }

                pDst_row += row_pitch - 3 * row_blocks_per_mcu;
            }
        }
    }
    else
    {
        // Copy MCU's pixel blocks into the destination bitmap.
        pDst_row = pImage;
        for (y = 0; y < image_info.m_MCUHeight; y += 8)
        {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
                uint8 *pDst_block = pDst_row + x * image_info.m_comps;

                // Compute source byte offset of the block in the decoder's MCU buffer.
                uint src_ofs = (x * 8U) + (y * 16U);
                const uint8 *pSrcR = image_info.m_pMCUBufR + src_ofs;
                const uint8 *pSrcG = image_info.m_pMCUBufG + src_ofs;
                const uint8 *pSrcB = image_info.m_pMCUBufB + src_ofs;

                const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

                if (image_info.m_scanType == PJPG_GRAYSCALE)
                {
                    int bx, by;
                    for (by = 0; by < by_limit; by++)
                    {
                        uint8 *pDst = pDst_block;

                        for (bx = 0; bx < bx_limit; bx++)
                            *pDst++ = *pSrcR++;

                        pSrcR += (8 - bx_limit);

                        pDst_block += row_pitch;
                    }
                }
                else
                {
                    int bx, by;
                    for (by = 0; by < by_limit; by++)
                    {
                        uint8 *pDst = pDst_block;

                        for (bx = 0; bx < bx_limit; bx++)
                        {
                            pDst[0] = *pSrcR++;
                            pDst[1] = *pSrcG++;
                            pDst[2] = *pSrcB++;

                            pDst += 3;
                        }

                        pSrcR += (8 - bx_limit);
                        pSrcG += (8 - bx_limit);
                        pSrcB += (8 - bx_limit);

                        pDst_block += row_pitch;
                    }
                }
            }
            pDst_row += (row_pitch * 8);
        }
    }

    MCUx = mcu_x;
    MCUy = mcu_y;
    
    mcu_x++;
    if (mcu_x == image_info.m_MCUSPerRow)
    {
        mcu_x = 0;
        mcu_y++;
    }

    if(decode_mcu()==-1) is_available = 0 ;

    return 1;
}


