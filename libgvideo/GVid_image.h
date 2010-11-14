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
#  GVImage class : gvideo image class                                           #
#                                                                               #
********************************************************************************/

#ifndef GVID_IMAGE_H
#define GVID_IMAGE_H

#include <string>
#include "libgvideo/gvideo.h"

START_LIBGVIDEO_NAMESPACE

struct BITMAPFILEHEADER 
{ 
    UINT16    bfType; //Specifies the file type, must be BM
    UINT32    bfSize; //Specifies the size, in bytes, of the bitmap file
    UINT16    bfReserved1; //Reserved; must be zero
    UINT16    bfReserved2; //Reserved; must be zero
    UINT32    bfOffBits; /*Specifies the offset, in bytes, 
                from the beginning of the BITMAPFILEHEADER structure 
                to the bitmap bits= FileHeader+InfoHeader+RGBQUAD(0 for 24bit BMP)=64*/
}   __attribute__ ((packed));


struct BITMAPINFOHEADER
{
    UINT32   biSize; 
    UINT32   biWidth; 
    UINT32   biHeight; 
    UINT16   biPlanes; 
    UINT16   biBitCount; 
    UINT32   biCompression; 
    UINT32   biSizeImage; 
    UINT32   biXPelsPerMeter; 
    UINT32   biYPelsPerMeter; 
    UINT32   biClrUsed; 
    UINT32   biClrImportant; 
}  __attribute__ ((packed));

struct JPGFILEHEADER 
{
    UINT8 SOI[2];/*SOI Marker 0xFFD8*/
    UINT8 APP0[2];/*APP0 MARKER 0xFF0E*/
    UINT8 length[2];/*length of header without APP0 in bytes*/
    UINT8 JFIF[5];/*set to JFIF0 0x4A46494600*/
    UINT8 VERS[2];/*1-2 0x0102*/
    UINT8 density;/* 0 - No units, aspect ratio only specified
                1 - Pixels per Inch on quickcam5000pro
                2 - Pixels per Centimetre                */
    UINT8 xdensity[2];/*120 on quickcam5000pro*/
    UINT8 ydensity[2];/*120 on quickcam5000pro*/
    UINT8 WTN;/*width Thumbnail 0*/
    UINT8 HTN;/*height Thumbnail 0*/	
} __attribute__ ((packed));

class GVImage
{
  public:
    int save_buffer(const char* filename, UINT8 *Picture, UINT32 image_size);
    int save_BPM(const char* filename, int width, int height, UINT8 *ImagePix, int BitCount = 24);
};

END_LIBGVIDEO_NAMESPACE

#endif
