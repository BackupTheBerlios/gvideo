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

START_LIBGVIDEO_NAMESPACE

VidBuff::VidBuff ()
{
    raw_frame = NULL;
    yuv_frame = NULL;
    time_stamp = 0;
    raw_size = 0;
    processed = true;
}

VidBuff::~VidBuff()
{
    delete[] raw_frame;
    delete[] yuv_frame;
}

//constructor
GVBuffer::GVBuffer(GVDevice *device, int frame_buff_size)
{
    dev=device;
    
    format = dev->get_format();
    width = dev->get_width();
    height = dev->get_height();
    _frame_buff_size = frame_buff_size;
    
    pthread_mutex_init( &mutex, NULL );
    
    frameBuff = new VidBuff[frame_buff_size];
    
    pIndex = 0;
    cIndex = 0;
    conversor = new GVConvert;
    jpgdec = new GVMjpgDec;
    
    tmp_data = NULL;
    
    alloc_frame_buff();
}

//destructor
GVBuffer::~GVBuffer()
{
    int i = 0;
    delete[] frameBuff;
    delete[] tmp_data;
    delete conversor;
    delete jpgdec;
    
    pthread_mutex_destroy(&mutex);
}

int GVBuffer::alloc_frame_buff()
{
    int ret = 0;
    int i = 0;
    
    int framesizeIn = (width * height << 1); //2 bytes per pixel
    switch (format) 
    {
        case V4L2_PIX_FMT_JPEG:
        case V4L2_PIX_FMT_MJPEG:
            _raw_size= framesizeIn;
            _yuv_size = width * (height + 8) * 2;
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
            _raw_size= framesizeIn;
            _yuv_size = framesizeIn;
            break;

        case V4L2_PIX_FMT_GREY:
            _raw_size= width * height; // 1 byte per pixel
            _yuv_size = framesizeIn;
            break;

        case V4L2_PIX_FMT_YUYV:
            _raw_size = framesizeIn;
            _yuv_size = framesizeIn;
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
            _raw_size = width * height * 3;
            _yuv_size = framesizeIn;
            tmp_data = new UINT8 [_raw_size];
            break;
        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_BGR24:
            _raw_size = width * height * 3;
            _yuv_size = framesizeIn;
            break;

        default:
            std::cerr << "(alloc_frame_buff) should never arrive (1)- exit fatal !!\n";
            ret = -1;
            goto error;
            break;
    }
    
    for(i=0; i<_frame_buff_size; i++)
    {
        frameBuff[i].raw_size = _raw_size;
        frameBuff[i].raw_frame = new UINT8 [_raw_size];
        frameBuff[i].yuv_frame = new UINT8 [_yuv_size];
        int j = 0;
        /*set yuv frame to black (y=0x00 u=0x80 v=0x80) by default*/
        for (j=0; j<(_yuv_size-4); j+=4)
        {
            frameBuff[i].yuv_frame[j]  =0x00;//Y
            frameBuff[i].yuv_frame[j+1]=0x80;//U
            frameBuff[i].yuv_frame[j+2]=0x00;//Y
            frameBuff[i].yuv_frame[j+3]=0x80;//V
        }
    }

    return (ret);
error:

    return (ret);
}

/* decode video stream (frame buffer in yuyv format)
 * args:
 * returns: error code ( 0 )
*/
int GVBuffer::frame_decode(VidBuff *frame, int pix_order/*=-1*/)
{
    int ret = 0;
    int framesizeIn =(width * height * 2);//2 bytes per pixel
    UINT8 * frame_data = frame->yuv_frame;
    UINT8 * raw_data = frame->raw_frame;
    size_t size = frame->raw_size;
    
    if(size <= 0)
    {
        std::cerr << "libgvideo: raw frame with size 0\n";
        return( -1);
    }
    
    switch (format) 
    {
        case V4L2_PIX_FMT_JPEG:
        case V4L2_PIX_FMT_MJPEG:
            if(size <= HEADERFRAME1) 
            {
                // Prevent crash on empty image
                std::cerr << "Ignoring empty buffer ...\n";
                return (ret);
            }

            if (jpgdec->decode(&frame_data, raw_data, width, height) < 0) 
            {
                std::cerr << "jpeg decode errors\n";
                return (-1);
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
                memcpy(frame_data, raw_data,(size_t) framesizeIn);
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
            std::cerr << "libgvideo: unknown format for decoding:" << format << std::endl;
            return (-1);
            break;
    }
    return ret;
};

int GVBuffer::produce_nextFrame(int pix_order/*=-1*/, VidBuff *frame/*=NULL*/)
{
    if(!dev)
    {
        std::cerr << "buff error: no GVDevice (NULL) can't grab frame\n";
        return -1;
    }
    
    pthread_mutex_lock( &mutex );
    if(frameBuff[pIndex].processed)
    {
        if(dev->grab_frame(frameBuff[pIndex].raw_frame) < 0)
        {
             pthread_mutex_unlock( &mutex );
            return -1;
        }
        frameBuff[pIndex].raw_size = dev->get_bytesused();
        frame_decode(&(frameBuff[pIndex]), pix_order);
        frameBuff[pIndex].time_stamp = dev->get_timestamp();
        
        frameBuff[pIndex].processed = false;
        
        /* get a copy of the grabed frame if requested (frame!=NULL) */
        /* must still consume the frame or the buffer will overflow  */
        if(frame)
        {
            if(!(frame->raw_frame))
                frame->raw_frame = new UINT8 [_raw_size];
            
            if(!(frame->yuv_frame))
                frame->yuv_frame = new UINT8 [_yuv_size];
            
            memcpy(frame->raw_frame, frameBuff[pIndex].raw_frame, frameBuff[pIndex].raw_size);
            memcpy(frame->yuv_frame, frameBuff[pIndex].yuv_frame, _yuv_size);
            frame->time_stamp = frameBuff[pIndex].time_stamp;
        }
        
        pIndex++;
        if(pIndex >= _frame_buff_size)
        pIndex = 0;
        pthread_mutex_unlock( &mutex );
        
        return 0;
    }
    else if(frame)
    {
        //frame buffer full - get new frame directly, don't write it to frame buffer 
        if(!(frame->raw_frame))
            frame->raw_frame = new UINT8 [_raw_size];
            
        if(!(frame->yuv_frame))
            frame->yuv_frame = new UINT8 [_yuv_size];
        
        if(dev->grab_frame(frame->raw_frame) < 0)
        {
             pthread_mutex_unlock( &mutex );
            return -1;
        }
        frame->raw_size = dev->get_bytesused();
        frame_decode(frame, pix_order);
        frame->time_stamp = dev->get_timestamp();
    }
    
    pthread_mutex_unlock( &mutex );
    std::cerr << "libgvideo: frame buffer full (must consume more or produce less)\n";
    return -1;
}

int GVBuffer::consume_nextFrame(VidBuff *frame)
{
    if(!frame)
    {
        std::cerr << "libgvideo: no Video Buffer allocated (NULL)\n";
        return -1;
    }
    
    if(!(frame->raw_frame))
        frame->raw_frame = new UINT8 [_raw_size];
    
    if(!(frame->yuv_frame))
        frame->yuv_frame = new UINT8 [_yuv_size];
    
    pthread_mutex_lock( &mutex );
    if(!(frameBuff[cIndex].processed))
    {
        memcpy(frame->raw_frame, frameBuff[cIndex].raw_frame, frameBuff[cIndex].raw_size);
        memcpy(frame->yuv_frame, frameBuff[cIndex].yuv_frame, _yuv_size);
        frame->time_stamp = frameBuff[cIndex].time_stamp;
        
        frameBuff[cIndex].processed = true;
        
        cIndex++;
        if(cIndex >= _frame_buff_size)
        cIndex = 0;
        pthread_mutex_unlock( &mutex );
        
        return 0;
    }
    
    pthread_mutex_unlock( &mutex );
    std::cerr << "libgvideo: no frame available yet (produce a frame first)\n";
    return -1;
}

int GVBuffer::best_ms_delay()
{
    int diff_ind = 0;
	int sched_sleep = 0;

	//try to balance buffer overrun in read/write operations 
	pthread_mutex_lock( &mutex );
	if(pIndex >= cIndex)
		diff_ind = pIndex - cIndex;
	else
		diff_ind = (_frame_buff_size - cIndex) + pIndex;
    pthread_mutex_unlock( &mutex );
    
	if(diff_ind <= (_frame_buff_size - (_frame_buff_size/2))) 
	    sched_sleep = 0; /* full throttle */
	else if (diff_ind <= (_frame_buff_size - (_frame_buff_size/3))) 
	    sched_sleep = (diff_ind-(_frame_buff_size/2)) * 2;
	else if (diff_ind <= (_frame_buff_size - (_frame_buff_size/4))) 
	    sched_sleep = (diff_ind-(_frame_buff_size/2)) * 3;
	else if (diff_ind <= (_frame_buff_size - (_frame_buff_size/5))) 
	    sched_sleep = (diff_ind-(_frame_buff_size/2)) * 4;
	else if (diff_ind <= (_frame_buff_size - (_frame_buff_size/6))) 
	    sched_sleep = (diff_ind-(_frame_buff_size/2)) * 5;
	else 
	    sched_sleep = (diff_ind-(_frame_buff_size/2)) * 6;
	
	return sched_sleep;
}

END_LIBGVIDEO_NAMESPACE
