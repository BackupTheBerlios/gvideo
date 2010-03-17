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

#ifndef GVID_BUFFER_H
#define GVID_BUFFER_H

#include "gvcommon.h"
#include "libgvideo/GVid_v4l2.h"
#include "libgvideo/GVid_color_convert.h"
#include "libgvideo/GVid_mjpg_dec.h"


START_LIBGVIDEO_NAMESPACE

struct VidBuff
{
    bool processed;
    int raw_size;
    UINT64 time_stamp;
    UINT8 * raw_frame;
    UINT8 * yuv_frame;
};

class GVBuffer
{
    GVConvert *conversor;
    GVMjpgDec *jpgdec;
    GVDevice *dev;
    
    pthread_mutex_t mutex;
    int format, width, height;
    
    UINT8 *tmp_data;
    
    int _frame_buff_size;
    int _raw_size;
    int _yuv_size;
    
    VidBuff *frameBuff;
    int pIndex;
    int cIndex;
    // internal functions
    int alloc_frame_buff();
    
  public:
    GVBuffer(int b_format=V4L2_PIX_FMT_MJPEG, int b_width=640, int b_height=480, int frame_buff_size=1, GVDevice *device=NULL);
    ~GVBuffer();
    VidBuff* new_VidBuff(VidBuff *frame);
    void delete_VidBuff(VidBuff *frame);
    int frame_decode(size_t size, int pix_order = -1);
    int produce_nextFrame(int pix_order = -1);
    int consume_nextFrame(VidBuff *frame);
};

END_LIBGVIDEO_NAMESPACE
#endif

