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

#ifndef GVID_V4L2_H
#define GVID_V4L2_H

#include <string>
#include <vector>
#include <libv4l2.h>
#include "gvcommon.h"


#define NB_BUFFER 4

#define IO_MMAP 1
#define IO_READ 2

START_LIBGVIDEO_NAMESPACE

class GVFps
{
  public:
    GVFps();
    int num;
    int denom;
};

struct VidCap 
{
    int width;            //width 
    int height;           //height
    std::vector<GVFps> fps;    //list of fps values
};

struct VidFormats
{
    int format;              //v4l2 pixel format
    std::string fourcc;      //corresponding fourcc (mode)
    std::string description;
    std::vector<VidCap> listVidCap;  //list of VidCap for format
};
    
struct InputControl 
{
    unsigned int i;
    unsigned int id;
    v4l2_ctrl_type type;
    std::string name;
    int min, max, step, default_val, enabled;
    std::vector<std::string> entries;
};

class GVDevice 
{
    bool verbose;
    bool stream_ready; //flag if we have set a format for the device
    bool streaming; //flag if video stream is on
    bool mmaped;
    bool setFPS; //flag fps change
    GVFps *fps;
    int fd;
    int width;
    int height;
    int format;
    int method;
    std::string dev_name;
    int current_format; //index of current format in listVidFormats
    UINT64 timestamp[NB_BUFFER];
    int framecount;
    
    gvcommon::GVTime *time;

    //struct v4l2_buffer buf;             // v4l2 buffer struct
    //struct v4l2_requestbuffers rb;      // v4l2 request buffers struct
    //struct v4l2_streamparm streamparm;  // v4l2 stream parameters struct
    UINT8 *mem[NB_BUFFER];                // memory buffers for mmap driver frames
    int buff_index;                       // memory buffers index
    size_t buff_bytesused[NB_BUFFER];     // bytes used by frame
    std::vector<UINT32> buff_length;      // memory buffers length as set by VIDIOC_QUERYBUF
    std::vector<UINT32> buff_offset;      // memory buffers offset as set by VIDIOC_QUERYBUF
    int cap_meth;
    
    int xioctl(int IOCTL_X, void *arg);
    int check_input();
    int check_controls();
    
    void clean_buffers();
    void freeFormats();
    void freeControls();
    int enum_frame_intervals( int pixfmt, __u32 fwidth, __u32 fheight, int fmtind, int fsizeind);
    int enum_frame_sizes( int pixfmt, int fmtind );
    int enum_frame_formats();
    int add_control(int n, struct v4l2_queryctrl *queryctrl);
    int map_buff();
    int unmap_buff();
    int query_buff();
    int queue_buff();
    int alloc_frame_buff();
    int set_framerate(GVFps* frate = NULL);
    
  public:
    std::vector<VidFormats> listVidFormats; //list of supported Pixel Formats;
    std::vector<InputControl> listControls; //list of controls
    GVDevice(std::string device_name = "/dev/video0", bool verbosity = false);
    ~GVDevice();
    bool isOpen();
    void set_verbose(bool verbosity);
    int get_control_val (int control_index, int * val);
    int set_control_val (int control_index, int val);
    int set_format (std::string fourcc, int twidth, int theight);
    int get_format_index(std::string fourcc);
    int get_res_index(int format_index, int twidth, int theight);
    int get_fps_index(int format_index, int res_index, GVFps* frate);
    int get_format();
    int get_width();
    int get_height();
    bool get_fps(GVFps* frate = NULL);
    bool set_fps(GVFps* frate);
    int get_framecount();
    UINT64 get_timestamp();
    int stream_on();
    int stream_off();
    int grab_frame(UINT8* data);
    size_t get_bytesused();
};

END_LIBGVIDEO_NAMESPACE

#endif
