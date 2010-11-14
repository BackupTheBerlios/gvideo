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
#  GVDevice  class : gvideo V4L2 interface                                      #
#                                                                               #
********************************************************************************/

#include <iostream>
#include <cstring>

#include "libgvideo/GVid_v4l2_formats.h"
#include "libgvideo/GVid_v4l2.h"
//using namespace std;

#define SUP_PIX_FMT 23

START_LIBGVIDEO_NAMESPACE

/*constructors*/
GVFormats::GVFormats()
{
    listSupFormats.resize(SUP_PIX_FMT);
    
    listSupFormats[0].format   = V4L2_PIX_FMT_MJPEG;
    listSupFormats[0].mode     = "mjpg";
    listSupFormats[0].hardware = 0;

    listSupFormats[1].format   = V4L2_PIX_FMT_JPEG;
    listSupFormats[1].mode     = "jpeg";
    listSupFormats[1].hardware = 0;

    listSupFormats[2].format   = V4L2_PIX_FMT_YUYV;
    listSupFormats[2].mode     = "yuyv";
    listSupFormats[2].hardware = 0;

    listSupFormats[3].format   = V4L2_PIX_FMT_YVYU;
    listSupFormats[3].mode     = "yvyu";
    listSupFormats[3].hardware = 0;

    listSupFormats[4].format   = V4L2_PIX_FMT_UYVY;
    listSupFormats[4].mode     = "uyvy";
    listSupFormats[4].hardware = 0;

    listSupFormats[5].format   = V4L2_PIX_FMT_YYUV;
    listSupFormats[5].mode     = "yyuv";
    listSupFormats[5].hardware = 0;

    listSupFormats[6].format   = V4L2_PIX_FMT_Y41P;
    listSupFormats[6].mode     = "y41p";
    listSupFormats[6].hardware = 0;
   
    listSupFormats[7].format   = V4L2_PIX_FMT_GREY;
    listSupFormats[7].mode     = "grey";
    listSupFormats[7].hardware = 0;

    listSupFormats[8].format   = V4L2_PIX_FMT_YUV420;
    listSupFormats[8].mode     = "yu12";
    listSupFormats[8].hardware = 0;

    listSupFormats[9].format   = V4L2_PIX_FMT_YVU420;
    listSupFormats[9].mode     = "yv12";
    listSupFormats[9].hardware = 0;       

    listSupFormats[10].format   = V4L2_PIX_FMT_NV12;
    listSupFormats[10].mode     = "nv12";
    listSupFormats[10].hardware = 0;

    listSupFormats[11].format   = V4L2_PIX_FMT_NV21;
    listSupFormats[11].mode     = "nv21";
    listSupFormats[11].hardware = 0;

    listSupFormats[12].format   = V4L2_PIX_FMT_NV16;
    listSupFormats[12].mode     = "nv16";
    listSupFormats[12].hardware = 0;              

    listSupFormats[13].format   = V4L2_PIX_FMT_NV61;
    listSupFormats[13].mode     = "nv61";
    listSupFormats[13].hardware = 0;

    listSupFormats[14].format   = V4L2_PIX_FMT_SPCA501;
    listSupFormats[14].mode     = "s501";
    listSupFormats[14].hardware = 0;

    listSupFormats[15].format   = V4L2_PIX_FMT_SPCA505;
    listSupFormats[15].mode     = "s505";
    listSupFormats[15].hardware = 0;             

    listSupFormats[16].format   = V4L2_PIX_FMT_SPCA508;
    listSupFormats[16].mode     = "s508";
    listSupFormats[16].hardware = 0;

    listSupFormats[17].format   = V4L2_PIX_FMT_SGBRG8;
    listSupFormats[17].mode     = "gbrg";
    listSupFormats[17].hardware = 0;

    listSupFormats[18].format   = V4L2_PIX_FMT_SGRBG8;
    listSupFormats[18].mode     = "grbg";
    listSupFormats[18].hardware = 0;       
       
    listSupFormats[19].format   = V4L2_PIX_FMT_SBGGR8;
    listSupFormats[19].mode     = "ba81";
    listSupFormats[19].hardware = 0;

    listSupFormats[20].format   = V4L2_PIX_FMT_SRGGB8;
    listSupFormats[20].mode     = "rggb";
    listSupFormats[20].hardware = 0;               
        
    listSupFormats[21].format   = V4L2_PIX_FMT_RGB24;
    listSupFormats[21].mode     = "rgb3";
    listSupFormats[21].hardware = 0;

    listSupFormats[22].format   = V4L2_PIX_FMT_BGR24;
    listSupFormats[22].mode     = "bgr3";
    listSupFormats[22].hardware = 0;

}

/*destructor*/
GVFormats::~GVFormats()
{
    listSupFormats.clear();
}

/* check if format is supported
 * args:
 * pixfmt: V4L2 pixel format
 * return index from supported devices list 
 * or -1 if not supported                    */
int GVFormats::check_PixFormat(int pixfmt)
{
    int i=0;
    for (i=0; i<SUP_PIX_FMT; i++)
    {
        if (pixfmt == listSupFormats[i].format)
        {
            return (i); /*return index from supported formats list*/
        }
    }
    return (-1);
};

/* set hardware flag for v4l2 pix format
 * args:
 * pixfmt: V4L2 pixel format
 * return index from supported devices list
 * or -1 if not supported                    */
int GVFormats::set_SupPixFormat(int pixfmt)
{
	int i=0;
	for (i=0; i<SUP_PIX_FMT; i++)
	{
		if (pixfmt == listSupFormats[i].format)
		{
			listSupFormats[i].hardware = 1; /*supported by hardware*/
			return (i);
		}
	}
	return(-1); /*not supported*/
}

/* check if format is supported by hardware
 * args:
 * pixfmt: V4L2 pixel format
 * return index from supported devices list
 * or -1 if not supported                    */
int GVFormats::check_SupPixFormat(int pixfmt)
{
	int i=0;
	for (i=0; i<SUP_PIX_FMT; i++)
	{
		if (pixfmt == listSupFormats[i].format)
		{
			if(listSupFormats[i].hardware > 0) return (i); /*supported by hardware*/
		}
	}
	return (-1);

}

/* convert v4l2 pix format to mode (Fourcc)
 * args:
 * pixfmt: V4L2 pixel format
 * mode: fourcc string (lower case)
 * returns 1 on success 
 * and -1 on failure (not supported)         */
std::string GVFormats::get_PixMode(int pixfmt)
{
	int i=0;
	for (i=0; i<SUP_PIX_FMT; i++)
	{
		if (pixfmt == listSupFormats[i].format)
		{
			return(listSupFormats[i].mode);
		}
	}
	return ("not found");
}

/* converts mode (fourcc) to v4l2 pix format
 * args:
 * mode: fourcc string (lower case)
 * returns v4l2 pix format 
 * defaults to MJPG if mode not supported    */
int GVFormats::get_PixFormat(std::string mode)
{
	int i=0;
	for (i=0; i<SUP_PIX_FMT; i++)
	{
		//g_printf("mode: %s - check %s\n", mode, listSupFormats[i].mode);
		if (listSupFormats[i].mode.compare(mode) == 0)
		{
			return (listSupFormats[i].format);
		}
	}
	return (listSupFormats[0].format); /*default is- MJPG*/
}

END_LIBGVIDEO_NAMESPACE
