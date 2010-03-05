/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
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
#  GVideo colorspace conversion class                                           #
#                                                                               # 
#                                                                               #
********************************************************************************/

#ifndef GVID_COLOR_CONVERT_H
#define GVID_COLOR_CONVERT_H

#include "gvcommon.h"

class GVConvert
{
    void convert_border_bayer_line_to_bgr24( UINT8* bayer, UINT8* adjacent_bayer, 
        UINT8 *bgr, int width, bool start_with_green, bool blue_line);
    void bayer_to_rgbbgr24(UINT8 *bayer, UINT8 *bgr, int width, int height,
        bool start_with_green, bool blue_line);
        
  public:
    void yuyv2rgb (UINT8 *pyuv, UINT8 *prgb, int width, int height);
    void yuyv2bgr1 (UINT8 *pyuv, UINT8 *pbgr, int width, int height);
    void yuyv2bgr (UINT8 *pyuv, UINT8 *pbgr, int width, int height);
    void yyuv_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void uyvy_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void yvyu_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void yuv420_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void yvu420_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void nv12_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void nv21_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void nv16_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void nv61_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void y41p_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void grey_to_yuyv (UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void s501_to_yuyv(UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void s505_to_yuyv(UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void s508_to_yuyv(UINT8 *framebuffer, UINT8 *tmpbuffer, int width, int height);
    void bayer_to_rgb24(UINT8 *pBay, UINT8 *pRGB24, int width, int height, int pix_order);
    void rgb2yuyv(UINT8 *prgb, UINT8 *pyuv, int width, int height);
    void bgr2yuyv(UINT8 *pbgr, UINT8 *pyuv, int width, int height);
    void yuv420pto422(int * out,unsigned char *pic,int width);
    void yuv422pto422(int * out,unsigned char *pic,int width);
    void yuv444pto422(int * out,unsigned char *pic,int width);
    void yuv400pto422(int * out,unsigned char *pic,int width);
    
};



#endif
