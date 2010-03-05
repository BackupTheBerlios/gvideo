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
#  GVDevice  class : gvideo V4L2 interface definition                           #
#                                                                               #
********************************************************************************/

#ifndef GVID_V4L2_FORMATS_H
#define GVID_V4L2_FORMATS_H

#include <libv4l2.h>
#include <string>
#include <vector>
#include "libgvideo/gvideo.h"

#define SUP_PIX_FMT 23


// Class Definition
class GVFormats 
{
    struct SupFormats
    {
	    int format;          //v4l2 software(guvcview) supported format
	    std::string mode;          //mode (fourcc - lower case)
	    int hardware;        //hardware supported (1 or 0)
    };
    
    std::vector<SupFormats> listSupFormats;

  public:
    GVFormats();
    ~GVFormats();
    int check_PixFormat(int pixfmt);
    int set_SupPixFormat(int pixfmt);
    int check_SupPixFormat(int pixfmt);
    std::string get_PixMode(int pixfmt);
    int get_PixFormat(std::string mode);
};

#endif
