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
#  GVBuffer class : gvideo buffer and decoding class                            #
#                                                                               #
********************************************************************************/

#include <iostream>
#include <string>
#include <string.h> //memcpy
#include "libgvideo/GVid_buff.h"


using namespace std;

//constructor
GVBuffer::GVBuffer(int b_format, int b_width, int b_height)
{
    format = b_format;
    width = b_width;
    height = b_height;
    
    conversor = new GVConvert;
    jpgdec = new GVMjpgDec;
    
    raw_data = NULL;
    frame_data = NULL;
    tmp_data = NULL;
    
    alloc_frame_buff();
}

//destructor
GVBuffer::~GVBuffer()
{        
    delete[] raw_data;
    delete[] frame_data;
    delete[] tmp_data;
    delete conversor;
    delete jpgdec;
}

int GVBuffer::alloc_frame_buff()
{
    int ret = 0;
    size_t framebuf_size=0;
    size_t tmpbuf_size=0;

    int framesizeIn = (width * height << 1); //2 bytes per pixel
    switch (format) 
    {
        case V4L2_PIX_FMT_JPEG:
        case V4L2_PIX_FMT_MJPEG:
            tmpbuf_size= framesizeIn;
            raw_data = new UINT8 [tmpbuf_size];
            framebuf_size = width * (height + 8) * 2;
            frame_data = new UINT8 [framebuf_size]; 
            break;
            
        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_YVYU:
        case V4L2_PIX_FMT_YYUV:
        case V4L2_PIX_FMT_YUV420: // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
        case V4L2_PIX_FMT_YVU420: // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
        case V4L2_PIX_FMT_Y41P:   // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV16:
        case V4L2_PIX_FMT_NV61:
        case V4L2_PIX_FMT_SPCA501:
        case V4L2_PIX_FMT_SPCA505:
        case V4L2_PIX_FMT_SPCA508:
            tmpbuf_size= framesizeIn;
            raw_data = new UINT8 [tmpbuf_size];
            framebuf_size = framesizeIn;
            frame_data = new UINT8 [framebuf_size];
            break;

        case V4L2_PIX_FMT_GREY:
            tmpbuf_size= width * height; // 1 byte per pixel
            raw_data = new UINT8 [tmpbuf_size];
            framebuf_size = framesizeIn;
            frame_data = new UINT8 [framebuf_size];
            break;

        case V4L2_PIX_FMT_YUYV:
            framebuf_size = framesizeIn;
            raw_data = new UINT8 [framebuf_size];
            frame_data = new UINT8 [framebuf_size];
            break;

        case V4L2_PIX_FMT_SGBRG8: //0
        case V4L2_PIX_FMT_SGRBG8: //1
        case V4L2_PIX_FMT_SBGGR8: //2
        case V4L2_PIX_FMT_SRGGB8: //3
            // Raw 8 bit bayer 
            // when grabbing use:
            //    bayer_to_rgb24(bayer_data, RGB24_data, width, height, 0..3)
            //    rgb2yuyv(RGB24_data, vd->frame_data, width, height)

            // alloc a temp buffer for converting to YUYV
            // rgb buffer for decoding bayer data
            tmpbuf_size = width * height * 3;
            tmp_data = new UINT8 [tmpbuf_size];
            raw_data = new UINT8 [tmpbuf_size];
            framebuf_size = framesizeIn;
            frame_data = new UINT8 [framebuf_size];
            break;
        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_BGR24:
            tmpbuf_size = width * height * 3;
            raw_data = new UINT8 [tmpbuf_size];
            framebuf_size = framesizeIn;
            frame_data = new UINT8 [framebuf_size];
            break;

        default:
            cerr << "(alloc_frame_buff) should never arrive (1)- exit fatal !!\n";
            ret = -1;
            goto error;
            break;
    }

    if ((!frame_data) || (framebuf_size <=0)) 
    {
        cerr << "couldn't allocate "<< framebuf_size 
            << "bytes of memory for frame buffer\n";
        ret = -3;
        goto error;
    } 
    else
    {
        int i = 0;
        // set frame_data to black (y=0x00 u=0x80 v=0x80) by default
        for (i=0; i<(framebuf_size-4); i+=4)
        {
            frame_data[i]=0x00;  //Y
            frame_data[i+1]=0x80;//U
            frame_data[i+2]=0x00;//Y
            frame_data[i+3]=0x80;//V
        }
    }
    return (ret);
error:
    delete[] frame_data;
    delete[] raw_data;

    return (ret);
}

/* decode video stream (frame buffer in yuyv format)
 * args:
 * returns: error code ( 0 )
*/
int GVBuffer::frame_decode(size_t size, int pix_order)
{
    int ret = 0;
    int framesizeIn =(width * height << 1);//2 bytes per pixel
    switch (format) 
    {
        case V4L2_PIX_FMT_JPEG:
        case V4L2_PIX_FMT_MJPEG:
            if(size <= HEADERFRAME1) 
            {
                // Prevent crash on empty image
                cerr << "Ignoring empty buffer ...\n";
                return (ret);
            }

            if (jpgdec->decode(&frame_data, raw_data, width, height) < 0) 
            {
                cerr << "jpeg decode errors\n";
                ret = -1;
                goto err;
            }
			break;
		
		case V4L2_PIX_FMT_UYVY:
			conversor->uyvy_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_YVYU:
			conversor->yvyu_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_YYUV:
			conversor->yyuv_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_YUV420:
			conversor->yuv420_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_YVU420:
			conversor->yvu420_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_NV12:
			conversor->nv12_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_NV21:
			conversor->nv21_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_NV16:
			conversor->nv16_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_NV61:
			conversor->nv61_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_Y41P: 
			conversor->y41p_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_GREY:
			conversor->grey_to_yuyv(frame_data, raw_data, width, height);
			break;
			
		case V4L2_PIX_FMT_SPCA501:
			conversor->s501_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_SPCA505:
			conversor->s505_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_SPCA508:
			conversor->s508_to_yuyv(frame_data, raw_data, width, height);
			break;
		
		case V4L2_PIX_FMT_YUYV:
			if(pix_order >= 0) 
			{
				if (!(tmp_data)) 
				{
					// rgb buffer for decoding bayer data
					tmp_data = new UINT8 [width * height * 3];
				}
				conversor->bayer_to_rgb24 (raw_data, tmp_data, width, height, pix_order);
				// raw bayer is only available in logitech cameras in yuyv mode
				conversor->rgb2yuyv (tmp_data, frame_data, width, height);
			} 
			else 
			{
				if (size > framesizeIn)
					memcpy(frame_data, raw_data,
						(size_t) framesizeIn);
				else
					memcpy(frame_data, raw_data, size);
			}
			break;
			
		case V4L2_PIX_FMT_SGBRG8: //0
			conversor->bayer_to_rgb24 (raw_data, tmp_data, width, height, 0);
			conversor->rgb2yuyv (tmp_data, frame_data, width, height);
			break;
			
		case V4L2_PIX_FMT_SGRBG8: //1
			conversor->bayer_to_rgb24 (raw_data, tmp_data, width, height, 1);
			conversor->rgb2yuyv (tmp_data, frame_data, width, height);
			break;
			
		case V4L2_PIX_FMT_SBGGR8: //2
			conversor->bayer_to_rgb24 (raw_data, tmp_data, width, height, 2);
			conversor->rgb2yuyv (tmp_data, frame_data, width, height);
			break;
		case V4L2_PIX_FMT_SRGGB8: //3
			conversor->bayer_to_rgb24 (raw_data, tmp_data, width, height, 3);
			conversor->rgb2yuyv (tmp_data, frame_data, width, height);
			break;
			
		case V4L2_PIX_FMT_RGB24:
			conversor->rgb2yuyv(raw_data, frame_data, width, height);
			break;
		case V4L2_PIX_FMT_BGR24:
			conversor->bgr2yuyv(raw_data, frame_data, width, height);
			break;
		
		default:
			cerr << "error grabbing (v4l2uvc.c) unknown format:" << format << endl;
			ret = -1;
			goto err;
			break;
	}
	return ret;
err:
	return ret;
};
