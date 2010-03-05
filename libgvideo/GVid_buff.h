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
#include "libgvideo/GVid_color_convert.h"
#include "libgvideo/GVid_mjpg_dec.h"

class GVBuffer
{
    GVConvert *conversor;
    GVMjpgDec *jpgdec;
    int format, width, height;
    
    UINT8 *tmp_data;

    // internal functions
    int alloc_frame_buff();
    
  public:
    UINT8 *frame_data;
    UINT8 *raw_data;
    GVBuffer(int b_format=V4L2_PIX_FMT_MJPEG, int b_width=640, int b_height=480);
    ~GVBuffer();
    int frame_decode(size_t size, int pix_order=-1);
};

#endif

