/*******************************************************************************#
#           gvideo              http://gvideo.berlios.de                        #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/

/*******************************************************************************#
#                                                                               #
#  GVJpeg class : gvideo M/Jpeg compressor class                                #
#                                                                               #
********************************************************************************/

#include <string.h>
#include "GVJpeg_encoder.h"

START_LIBGVENCODER_NAMESPACE

#define PUTBITS \
{   \
    bits_in_next_word = (INT16) (bitindex + numbits - 32); \
    if (bits_in_next_word < 0) \
    { \
            lcode = (lcode << numbits) | data; \
            bitindex += numbits; \
    } \
    else \
    { \
        lcode = (lcode << (32 - bitindex)) | (data >> bits_in_next_word); \
        if ((*output_ptr++ = (UINT8)(lcode >> 24)) == 0xff) \
            *output_ptr++ = 0; \
        if ((*output_ptr++ = (UINT8)(lcode >> 16)) == 0xff) \
            *output_ptr++ = 0; \
        if ((*output_ptr++ = (UINT8)(lcode >> 8)) == 0xff) \
            *output_ptr++ = 0; \
        if ((*output_ptr++ = (UINT8) lcode) == 0xff) \
            *output_ptr++ = 0; \
        lcode = data; \
        bitindex = bits_in_next_word; \
    } \
}

/* This function implements 16 Step division for Q.15 format data */
UINT16 GVJpeg::DSP_Division (UINT32 numer, UINT32 denom)
{
    UINT16 i;
    denom <<= 15;

    for (i=16; i>0; i--)
    {
        if (numer > denom)
        {
            numer -= denom;
            numer <<= 1;
            numer++;
        }
        else numer <<= 1;
    }
    return (UINT16) numer;
}

GVJpeg::GVJpeg(int image_width, int image_height)
{
    UINT16 i = 0;
    UINT16 index = 0;
    UINT32 value = 0;
    
    width = image_width;
    height = image_height;
    
    mcu_width = 16;
    horizontal_mcus = (UINT16) (image_width >> 4);/* width/16 */

    mcu_height = 8;
    vertical_mcus = (UINT16) (image_height >> 3); /* height/8 */ 

    UINT16 bytes_per_pixel = 2;

    length_minus_mcu_width = (UINT16) ((image_width - mcu_width) * bytes_per_pixel);
    length_minus_width = (UINT16) (image_width * bytes_per_pixel);
    mcu_width_size = (UINT16) (mcu_width * bytes_per_pixel);

    rows = mcu_height;
    cols = mcu_width;
    incr = length_minus_mcu_width;
    offset = (UINT16) ((image_width * mcu_height) * bytes_per_pixel);
    
    //initialize quantization tables
    for (i=0; i<64; i++)
    {
        index = zigzag_table [i];

        value= luminance_quant_table [i];

        Lqt [index] = (UINT8) value;
        ILqt [i] = DSP_Division (0x8000, value);


        value = chrominance_quant_table [i];

        Cqt[index] = (UINT8) value;
        ICqt [i] = DSP_Division (0x8000, value);
    }

    clean();
}

void GVJpeg::clean ()
{
    ldc1 = 0;
    ldc2 = 0;
    ldc3 = 0;
    lcode = 0;
    bitindex = 0;
}

UINT8* GVJpeg::huffman (UINT16 component, UINT8 *output_ptr)
{
    UINT16 i;
    const UINT16 *DcCodeTable, *DcSizeTable, *AcCodeTable, *AcSizeTable;

    INT16 *Temp_Ptr, Coeff, LastDc;
    UINT16 AbsCoeff, HuffCode, HuffSize, RunLength=0, DataSize=0, index;

    INT16 bits_in_next_word;
    UINT16 numbits;
    UINT32 data;

    Temp_Ptr = Temp;
    Coeff = *Temp_Ptr++;/* Coeff = DC */

    /* code DC - Temp[0] */
    if (component == 1)/* luminance - Y */
    {
        DcCodeTable = luminance_dc_code_table;
        DcSizeTable = luminance_dc_size_table;
        AcCodeTable = luminance_ac_code_table;
        AcSizeTable = luminance_ac_size_table;

        LastDc = ldc1;
        ldc1 = Coeff;
    }
    else /* Chrominance - U V */
    {
        DcCodeTable = chrominance_dc_code_table;
        DcSizeTable = chrominance_dc_size_table;
        AcCodeTable = chrominance_ac_code_table;
        AcSizeTable = chrominance_ac_size_table;

        if (component == 2) /* Chrominance - U */
        {
            LastDc = ldc2;
            ldc2 = Coeff;
        }
        else/* Chrominance - V */
        {
            LastDc = ldc3;
            ldc3 = Coeff;
        }
    }

    Coeff = Coeff - LastDc; /* DC - LastDC */

    AbsCoeff = (Coeff < 0) ? -(Coeff--) : Coeff;

    /*calculate data size*/
    while (AbsCoeff != 0)
    {
        AbsCoeff >>= 1;
        DataSize++;
    }

    HuffCode = DcCodeTable [DataSize];
    HuffSize = DcSizeTable [DataSize];

    Coeff &= (1 << DataSize) - 1;
    data = (HuffCode << DataSize) | Coeff;
    numbits = HuffSize + DataSize;

    PUTBITS

    /* code AC */
    for (i=63; i>0; i--)
    {
        if ((Coeff = *Temp_Ptr++) != 0)
        {
            while (RunLength > 15)
            {
                RunLength -= 16;
                data = AcCodeTable [161];   /* ZRL 0xF0 ( 16 - 0) */
                numbits = AcSizeTable [161];/* ZRL                */
                PUTBITS
            }

            AbsCoeff = (Coeff < 0) ? -(Coeff--) : Coeff;

            if (AbsCoeff >> 8 == 0) /* Size <= 8 bits */
                DataSize = bitsize [AbsCoeff];
            else /* 16 => Size => 8 */
                DataSize = bitsize [AbsCoeff >> 8] + 8;

            index = RunLength * 10 + DataSize;

            HuffCode = AcCodeTable [index];
            HuffSize = AcSizeTable [index];

            Coeff &= (1 << DataSize) - 1;
            data = (HuffCode << DataSize) | Coeff;
            numbits = HuffSize + DataSize;

            PUTBITS
            RunLength = 0;
        }
        else
            RunLength++;/* Add while Zero */
    }

    if (RunLength != 0)
    {
        data = AcCodeTable [0];   /* EOB - 0x00 end of block */
        numbits = AcSizeTable [0];/* EOB                     */ 
        PUTBITS
    }
    return output_ptr;
}

/* For bit Stuffing and EOI marker */
UINT8* GVJpeg::close_bitstream (UINT8 *output_ptr)
{
    UINT16 i, count;
    UINT8 *ptr;

    if (bitindex > 0)
    {
        lcode <<= (32 - bitindex);
        count = (bitindex + 7) >> 3;

        ptr = (UINT8 *) &lcode + 3;

        for (i=count; i>0; i--)
        {
            if ((*output_ptr++ = *ptr--) == 0xff)
                *output_ptr++ = 0;
        }
    }

    /* End of image marker (EOI) */
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xD9;
    return output_ptr;
}


/* Level shifting to get 8 bit SIGNED values for the data  */
void GVJpeg::levelshift (INT16* const data)
{
    INT16 i;

    for (i=63; i>=0; i--)
        data [i] -= 128;
}

/* DCT for One block(8x8) */
void GVJpeg::DCT (INT16 *data)
{
    UINT16 i;
    INT32 x0, x1, x2, x3, x4, x5, x6, x7, x8;
    INT16 *tmp_ptr;
    tmp_ptr=data;
    /*  
     *  All values are shifted left by 10
     *  and rounded off to nearest integer
     */

    /* row pass */
    for (i=8; i>0; i--)
    {
        x8 = data [0] + data [7];
        x0 = data [0] - data [7];
        x7 = data [1] + data [6];
        x1 = data [1] - data [6];
        x6 = data [2] + data [5];
        x2 = data [2] - data [5];
        x5 = data [3] + data [4];
        x3 = data [3] - data [4];
        x4 = x8 + x5;
        x8 -= x5;
        x5 = x7 + x6;
        x7 -= x6;

        data [0] = (INT16) (x4 + x5);
        data [4] = (INT16) (x4 - x5);
        data [2] = (INT16) ((x8*c2 + x7*c6) >> s2);
        data [6] = (INT16) ((x8*c6 - x7*c2) >> s2);
        data [7] = (INT16) ((x0*c7 - x1*c5 + x2*c3 - x3*c1) >> s2);
        data [5] = (INT16) ((x0*c5 - x1*c1 + x2*c7 + x3*c3) >> s2);
        data [3] = (INT16) ((x0*c3 - x1*c7 - x2*c1 - x3*c5) >> s2);
        data [1] = (INT16) ((x0*c1 + x1*c3 + x2*c5 + x3*c7) >> s2);

        data += 8;
    }

    data = tmp_ptr;/* return to start of mcu */

    /* column pass */
    for (i=8; i>0; i--)
    {
        x8 = data [0] + data [56];
        x0 = data [0] - data [56];
        x7 = data [8] + data [48];
        x1 = data [8] - data [48];
        x6 = data [16] + data [40];
        x2 = data [16] - data [40];
        x5 = data [24] + data [32];
        x3 = data [24] - data [32];
        x4 = x8 + x5;
        x8 -= x5;
        x5 = x7 + x6;
        x7 -= x6;

        data [0] = (INT16) ((x4 + x5) >> s1);
        data [32] = (INT16) ((x4 - x5) >> s1);
        data [16] = (INT16) ((x8*c2 + x7*c6) >> s3);
        data [48] = (INT16) ((x8*c6 - x7*c2) >> s3);
        data [56] = (INT16) ((x0*c7 - x1*c5 + x2*c3 - x3*c1) >> s3);
        data [40] = (INT16) ((x0*c5 - x1*c1 + x2*c7 + x3*c3) >> s3);
        data [24] = (INT16) ((x0*c3 - x1*c7 - x2*c1 - x3*c5) >> s3);
        data [8] = (INT16) ((x0*c1 + x1*c3 + x2*c5 + x3*c7) >> s3);

        data++;
    }
    data=tmp_ptr; /* return to start of mcu */
}

/* multiply DCT Coefficients with Quantization table and store in ZigZag location */
void GVJpeg::quantization (INT16* const data, UINT16* const quant_table_ptr)
{
    INT16 i;
    INT32 value;

    for (i=63; i>=0; i--)
    {
        value = data [i] * quant_table_ptr [i];
        value = (value + 0x4000) >> 15;

        Temp [zigzag_table [i]] = (INT16) value;
    }
}

UINT8* GVJpeg::encodeMCU (UINT8 *output_ptr)
{
    levelshift(Y1);
    DCT(Y1);
    quantization(Y1, ILqt);

    output_ptr = huffman(1, output_ptr);

    levelshift(Y2);
    DCT(Y2);
    quantization(Y2, ILqt);

    output_ptr = huffman(1, output_ptr);
 
    levelshift(CB);
    DCT(CB);
    quantization (CB, ICqt);

    output_ptr = huffman( 2, output_ptr);

    levelshift(CR);
    DCT(CR);
    quantization (CR, ICqt);

    output_ptr = huffman ( 3, output_ptr);

    return output_ptr;
}

UINT8* GVJpeg::write_markers (UINT8 *output_ptr, bool huff)
{
    UINT16 i, header_length;
    UINT8 number_of_components;

    // Start of image marker
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xD8;
    //added from here 
    // Start of APP0 marker
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xE0;
    //header length
    *output_ptr++= 0x00;
    *output_ptr++= 0x10;//16 bytes

    //type
    if(huff) 
    {
        //JFIF0 0x4A46494600
        *output_ptr++= 'J';
        *output_ptr++= 'F';
        *output_ptr++= 'I';
        *output_ptr++= 'F';
        *output_ptr++= 0x00;
    } 
    else
    {
        // AVI10 0x4156493100
        *output_ptr++= 'A';
        *output_ptr++= 'V';
        *output_ptr++= 'I';
        *output_ptr++= '1';
        *output_ptr++= 0x00;
    }
    // version
    *output_ptr++= 0x01;
    *output_ptr++= 0x02;
    // density 0- no units 1- pix per inch 2- pix per mm
    *output_ptr++= 0x01;
    // xdensity - 120
    *output_ptr++= 0x00;
    *output_ptr++= 0x78;
    // ydensity - 120
    *output_ptr++= 0x00;
    *output_ptr++= 0x78;

    //thumb x y
    *output_ptr++= 0x00;
    *output_ptr++= 0x00;
    //to here

    // Quantization table marker
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xDB;

    // Quantization table length
    *output_ptr++ = 0x00;
    *output_ptr++ = 0x43;

    // Pq, Tq
    *output_ptr++ = 0x00;

    // Lqt table
    for (i=0; i<64; i++)
        *output_ptr++ = Lqt [i];

    // Quantization table marker
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xDB;

    // Quantization table length
    *output_ptr++ = 0x00;
    *output_ptr++ = 0x43;

    // Pq, Tq
    *output_ptr++ = 0x01;

    // Cqt table
    for (i=0; i<64; i++)
        *output_ptr++ = Cqt [i];

    if (huff) 
    {
        // huffman table(DHT)
        *output_ptr++=0xff;
        *output_ptr++=0xc4;
        *output_ptr++=0x01;
        *output_ptr++=0xa2;
        memmove(output_ptr, &(gvcommon::JPEGHuffmanTable), JPG_HUFFMAN_TABLE_LENGTH);/*0x01a0*/
        output_ptr += JPG_HUFFMAN_TABLE_LENGTH;
    }

    number_of_components = 3;

    // Frame header(SOF)

    // Start of frame marker
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xC0;

    header_length = (UINT16) (8 + 3 * number_of_components);

    // Frame header length	
    *output_ptr++ = (UINT8) (header_length >> 8);
    *output_ptr++ = (UINT8) header_length;

    // Precision (P)
    *output_ptr++ = 0x08;/*8 bits*/

    // image height
    *output_ptr++ = (UINT8) (height >> 8);
    *output_ptr++ = (UINT8) height;

    // image width
    *output_ptr++ = (UINT8) (width >> 8);
    *output_ptr++ = (UINT8) width;

    // Nf
    *output_ptr++ = number_of_components;

    /* type 422 */
    *output_ptr++ = 0x01; /*id (y)*/
    *output_ptr++ = 0x21; /*horiz|vertical */
    *output_ptr++ = 0x00; /*quantization table used*/

    *output_ptr++ = 0x02; /*id (u)*/
    *output_ptr++ = 0x11; /*horiz|vertical*/
    *output_ptr++ = 0x01; /*quantization table used*/

    *output_ptr++ = 0x03; /*id (v)*/
    *output_ptr++ = 0x11; /*horiz|vertical*/
    *output_ptr++ = 0x01; /*quantization table used*/


    // Scan header(SOF)

    // Start of scan marker
    *output_ptr++ = 0xFF;
    *output_ptr++ = 0xDA;

    header_length = (UINT16) (6 + (number_of_components << 1));

    // Scan header length
    *output_ptr++ = (UINT8) (header_length >> 8);
    *output_ptr++ = (UINT8) header_length;

    // Ns = number of scans
    *output_ptr++ = number_of_components;

    /* type 422*/
    *output_ptr++ = 0x01; /*component id (y)*/
    *output_ptr++ = 0x00; /*dc|ac tables*/

    *output_ptr++ = 0x02; /*component id (u)*/
    *output_ptr++ = 0x11; /*dc|ac tables*/

    *output_ptr++ = 0x03; /*component id (v)*/
    *output_ptr++ = 0x11; /*dc|ac tables*/

    *output_ptr++ = 0x00; /*0 */
    *output_ptr++ = 0x3F; /*63*/
    *output_ptr++ = 0x00; /*0 */

    return output_ptr;
}

/*YUYV*/
UINT8* GVJpeg::read_422_format (UINT8 *input_ptr)
{
    INT32 i, j;

    INT16 *Y1_Ptr = Y1; /*64 int16 block*/ 
    INT16 *Y2_Ptr = Y2;
    INT16 *CB_Ptr = CB;
    INT16 *CR_Ptr = CR;

    UINT8 *tmp_ptr=NULL;
    tmp_ptr=input_ptr;

    for (i=8; i>0; i--) /*8 rows*/
    {
        for (j=4; j>0; j--) /* 8 cols*/
        {
            *Y1_Ptr++ = *tmp_ptr++;
            *CB_Ptr++ = *tmp_ptr++;
            *Y1_Ptr++ = *tmp_ptr++;
            *CR_Ptr++ = *tmp_ptr++;
        }

        for (j=4; j>0; j--) /* 8 cols*/
        {
            *Y2_Ptr++ = *tmp_ptr++;
            *CB_Ptr++ = *tmp_ptr++;
            *Y2_Ptr++ = *tmp_ptr++;
            *CR_Ptr++ = *tmp_ptr++;
        }

        tmp_ptr += incr; /* next row (width - mcu_width)*/
    }
    tmp_ptr = NULL;/*clean*/
    return (input_ptr);
}

int GVJpeg::encode (UINT8 *input_ptr, UINT8 *output_ptr, bool huff)
{
    int size;
    UINT16 i, j;
    UINT8 *tmp_ptr=NULL;
    UINT8 *tmp_iptr=NULL;
    UINT8 *tmp_optr=NULL;
    tmp_iptr=input_ptr;
    tmp_optr=output_ptr;

    /* clean jpeg parameters*/
    clean();

    /* Writing Marker Data */
    tmp_optr = write_markers (tmp_optr, huff);

    for (i=0; i< vertical_mcus; i++) /* height /8 */
    {
        tmp_ptr=tmp_iptr;
        for (j=0; j< horizontal_mcus; j++) /* width /16 */
        {
            /*reads a block*/
            read_422_format (tmp_iptr); /*YUYV*/

            /* Encode the data in MCU */
            tmp_optr = encodeMCU (tmp_optr);

            if(j<(horizontal_mcus -1)) 
            {
                tmp_iptr += mcu_width_size;
            }
            else 
            {
                tmp_iptr=tmp_ptr;
            }
        }
        tmp_iptr += offset;
    }

    /* Close Routine */
    tmp_optr=close_bitstream (tmp_optr);
    size=tmp_optr-output_ptr;
    tmp_iptr=NULL;
    tmp_optr=NULL;

    return (size);
}

END_LIBGVENCODER_NAMESPACE
