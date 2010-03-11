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

#include <iostream>
#include <fstream>
#include <string.h>
#include "libgvideo/GVid_image.h"

START_LIBGVIDEO_NAMESPACE

int GVImage::save_buffer(const char* filename, UINT8 *Picture, UINT32 image_size)
{
    int ret = 0;
    std::ofstream myfile;
    
    myfile.open(filename, std::ios::out | std::ios::binary);
    
    if(myfile.is_open())
    {
        myfile.write((char *) Picture, image_size); 
    } 
    else 
    {
        ret=-1;
        std::cerr << "ERROR: Could not open file " << filename << " for write\n";
    }

    myfile.close();
    return (ret);
}

int GVImage::save_BPM(const char* filename, int width, int height, UINT8 *ImagePix, int BitCount/*=24*/) 
{
    int ret=0;
    BITMAPFILEHEADER *BmpFileh = new BITMAPFILEHEADER;
    BITMAPINFOHEADER *BmpInfoh = new BITMAPINFOHEADER;
    UINT32 pixsize = width*height*BitCount/8;
    UINT32 image_size = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+pixsize;
    UINT8* Picture = new UINT8[image_size];


    BmpFileh->bfType = 0x4d42;//must be BM (x4d42) 
    /*Specifies the size, in bytes, of the bitmap file*/
    BmpFileh->bfSize = image_size;
    BmpFileh->bfReserved1 = 0; //Reserved; must be zero
    BmpFileh->bfReserved2 = 0; //Reserved; must be zero
    /*Specifies the offset, in bytes,                      */
    /*from the beginning of the BITMAPFILEHEADER structure */
    /* to the bitmap bits                                  */
    BmpFileh->bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); 

    BmpInfoh->biSize=40;
    BmpInfoh->biWidth=width; 
    BmpInfoh->biHeight=height; 
    BmpInfoh->biPlanes=1; 
    BmpInfoh->biBitCount=BitCount; 
    BmpInfoh->biCompression=0; // 0 
    BmpInfoh->biSizeImage=pixsize; 
    BmpInfoh->biXPelsPerMeter=0; 
    BmpInfoh->biYPelsPerMeter=0; 
    BmpInfoh->biClrUsed=0; 
    BmpInfoh->biClrImportant=0;

    memcpy(Picture, BmpFileh, sizeof(BITMAPFILEHEADER));
    memcpy(Picture+sizeof(BITMAPFILEHEADER), BmpInfoh, sizeof(BITMAPINFOHEADER));
    memcpy(Picture+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), ImagePix, pixsize);
    
    save_buffer(filename, Picture, image_size);
    
    delete BmpFileh;
    delete BmpInfoh;
    delete[] Picture;
    return ret;
}

END_LIBGVIDEO_NAMESPACE
