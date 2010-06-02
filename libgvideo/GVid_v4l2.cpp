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
#include <cctype>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/select.h>

#include "libgvideo/GVid_v4l2.h"
#include "libgvideo/GVid_v4l2_formats.h"

START_LIBGVIDEO_NAMESPACE

/*constructors*/
GVFps::GVFps()
{
    /*use 1/25 as default*/
    num = 1;
    denom = 25;
}

GVDevice::GVDevice(std::string device_name /*= "/dev/video0"*/, bool verbosity /*= false*/)
{
    int i = 0;
    verbose = verbosity;
    buff_index = 0;
    fps = new GVFps;
    /* use automatic fps detection*/
    fps->num = 0;
    time = new gvcommon::GVTime;
    
    if(device_name.size() > 0)
        dev_name.assign(device_name);
    else
        dev_name="/dev/video0"; //empty string cases

    fd = v4l2_open(dev_name.c_str(), O_RDWR | O_NONBLOCK, 0);
    if(fd < 0 ) std::cerr << "Couldn't open video device:" << dev_name << std::endl;
    else
    {
        check_input();
        check_controls();
    }
    stream_ready = false;
    streaming = false;
    setFPS = false;
    mmaped = false;
    framecount = 0;
    
    //defaults
    method = IO_MMAP;
    
    for (i=0; i < NB_BUFFER; i++)
        timestamp[i] = 0;
}
/*destructor*/
GVDevice::~GVDevice()
{
    freeFormats();
    freeControls();
    clean_buffers();
    
    delete fps;
    delete time;
    
    v4l2_close(fd);
}

void GVDevice::set_verbose(bool verbosity)
{
    verbose = verbosity;
}

/* ioctl with a number of retries in the case of failure
* args:
* IOCTL_X - ioctl reference
* arg - pointer to ioctl data
* returns - ioctl result
*/
int GVDevice::xioctl(int IOCTL_X, void *arg)
{
    int ret = 0;
    int tries= 4;
    do 
    {
        ret = v4l2_ioctl(fd, IOCTL_X, arg);
    } 
    while (ret && tries-- &&
            ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

    if (ret && (tries <= 0)) 
        std::cerr << "ioctl " << IOCTL_X 
            << " retried " << 4 << " times - giving up: " << strerror(errno) << std::endl;
    return (ret);
}

bool GVDevice::isOpen()
{
    if (fd < 0) return false;
    else return true;
}

/* get Control index from available format list
 * args:
 * format: Control ID
 *
 * returns index of the control or -1 if not found 
 */
int GVDevice::get_control_index(int id)
{
    int i = 0;
    for (i=0; i<listControls.size(); i++)
    {
        if(listControls[i].control.id == id)
            return (i);
    }
    
    return (-1);
}
/* get Format index from available format list
 * args:
 * format: v4l2 pix format
 *
 * returns format list index */
int GVDevice::get_format_index(std::string fourcc)
{
    gvcommon::GVString *strconv = new gvcommon::GVString;
    int i=0;
    for(i=0;i<listVidFormats.size();i++)
    {
        //std::cout << listVidFormats[i].fourcc << " <-> " << fourcc << std::endl;
        if(listVidFormats[i].fourcc.compare(strconv->ToUpper(fourcc)) == 0)
        {
            delete strconv;
            return (i);
        }
    }
    delete strconv;
    std::cerr << "libgvideo: format " << fourcc << " not supported\n";
    return (-1);
}

int GVDevice::get_res_index(int format_index, int twidth, int theight)
{
    int i=0;
    for(i=0;i<listVidFormats[format_index].listVidCap.size();i++)
    {
        if((listVidFormats[format_index].listVidCap[i].width == twidth) &&
            listVidFormats[format_index].listVidCap[i].height == theight)
            return (i);
    }
    std::cerr << "libgvideo: resolution " << twidth << "x" << theight << " not supported for " 
        << listVidFormats[format_index].fourcc << std::endl;
    return(-1);
}

int GVDevice::get_fps_index(int format_index, int res_index, GVFps* frate)
{
    int i=0;
    for(i=0;i<listVidFormats[format_index].listVidCap[res_index].fps.size(); i++)
    {
        if((frate->num == listVidFormats[format_index].listVidCap[res_index].fps[i].num) &&
            (frate->denom == listVidFormats[format_index].listVidCap[res_index].fps[i].denom))
            return i;
    }
    std::cerr << "libgvideo: fps " << frate->num << "/" 
        << frate->denom << " not supported for selected format\n";
    return(-1);
}

void GVDevice::clean_buffers()
{
    struct v4l2_requestbuffers rb;

    if (stream_ready)
    {
        switch(method)
        {
            case IO_READ:
                delete[] mem[buff_index];
                break;

            case IO_MMAP:
            default:
                //delete requested buffers
                if(mmaped) unmap_buff();
                memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
                rb.count = 0;
                rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(VIDIOC_REQBUFS, &rb)<0)
                {
                    std::cerr << "libgvideo: VIDIOC_REQBUFS Failed to delete buffers: " 
                        << strerror(errno) << "(errno " << errno << ")\n";
                }
                break;
        }
    }
}

/* clean video formats list
 *
 * returns: void  */
void GVDevice::freeFormats()
{
    int i=0;
    int j=0;
    for(i=0;i<listVidFormats.size();i++)
    {
        for(j=0;j<listVidFormats[i].listVidCap.size();j++)
        {
            if(listVidFormats[i].listVidCap[j].fps.size() > 0)
                listVidFormats[i].listVidCap[j].fps.clear();
        }
        listVidFormats[i].listVidCap.clear();
    }
    listVidFormats.clear();
    if(verbose) std::cout << "cleaned Formats list\n";
}

/* clean controls list
 *
 * returns: void  */
void GVDevice::freeControls()
{
    int i=0;
    
    for(i=0;i<listControls.size();i++)
    {
        if(listControls[i].control.type == V4L2_CTRL_TYPE_MENU)
            listControls[i].entries.clear();
        if(listControls[i].control.type == V4L2_CTRL_TYPE_STRING)
            delete[] listControls[i].value_str;
    }
    listControls.clear();
    if(verbose) std::cout << "cleaned Controls list\n";
}


/* enumerate frame intervals (fps)
 * args:
 * pixfmt: v4l2 pixel format that we want to list frame intervals for
 * fwidth: video width that we want to list frame intervals for
 * fheight: video height that we want to list frame intervals for
 * fmtind: current index of format list
 * fsizeind: current index of frame size list
 * 
 * returns 0 if enumeration succeded or errno otherwise               */
int GVDevice::enum_frame_intervals( int pixfmt, __u32 fwidth, __u32 fheight, int fmtind, int fsizeind)
{
    int ret=0;
    struct v4l2_frmivalenum fival;
    int list_fps=0;
    memset(&fival, 0, sizeof(fival));
    fival.index = 0;
    fival.pixel_format = pixfmt;
    fival.width = fwidth;
    fival.height = fheight;
    if(verbose) std::cout << "Time intervals between frames ";
    
    while ((ret = xioctl(VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) 
    {
        fival.index++;
        if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) 
        {
            if (verbose && !list_fps)
                std::cout << "(discrete): ";
            list_fps++;
            listVidFormats[fmtind-1].listVidCap[fsizeind-1].fps.resize(list_fps);
            if (verbose)
                std::cout << fival.discrete.numerator << "/" << fival.discrete.denominator << " ";
            listVidFormats[fmtind-1].listVidCap[fsizeind-1].fps[list_fps-1].num = fival.discrete.numerator;
            listVidFormats[fmtind-1].listVidCap[fsizeind-1].fps[list_fps-1].denom = fival.discrete.denominator;
        } 
        else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) 
        {
            if(verbose) 
                std::cout << "(continuous): {min {" << fival.stepwise.min.numerator 
                    << "/" << fival.stepwise.min.numerator << "} .. max {"
                    << fival.stepwise.max.denominator << "/" 
                    << fival.stepwise.max.denominator <<"} }, \n";
            break;
        } 
        else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) 
        {
            if(verbose) 
                std::cout << "(stepwise): {min {" << fival.stepwise.min.numerator << "/" 
                    << fival.stepwise.min.denominator << " } .. max {"
                    << fival.stepwise.max.numerator << "/" 
                    << fival.stepwise.max.denominator << "} / "
                    "step { " << fival.stepwise.step.numerator 
                    << "/" << fival.stepwise.step.denominator << "} }, \n";
            break;
        }
    }
    
    if(verbose) std::cout << std::endl;
    
    if (list_fps==0)
    {
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].fps.resize(1);

        listVidFormats[fmtind-1].listVidCap[fsizeind-1].fps[0].num = 1;
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].fps[0].denom = 1;
    }

    if (ret != 0 && errno != EINVAL) 
    {
        std::cerr << "VIDIOC_ENUM_FRAMEINTERVALS - Error enumerating frame intervals:" << strerror(errno) << std::endl;
        return errno;
    }
    return 0;
}

/* enumerate frame sizes
 * args:
 * listVidFormats: array of VidFormats (list of video formats)
 * pixfmt: v4l2 pixel format that we want to list frame sizes for
 * fmtind: format list current index
 * width: pointer to integer containing the selected video width
 * height: pointer to integer containing the selected video height
 * fd: device file descriptor
 * 
 * returns 0 if enumeration succeded or errno otherwise               */
int GVDevice::enum_frame_sizes( int pixfmt, int fmtind )
{
    int ret=0;
    int fsizeind=0; /*index for supported sizes*/

    struct v4l2_frmsizeenum fsize;

    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;
    fsize.pixel_format = pixfmt;
    while ((ret = xioctl(VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) 
    {
        fsize.index++;
        if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) 
        {
            fsizeind++;
            listVidFormats[fmtind-1].listVidCap.resize(fsizeind);

            listVidFormats[fmtind-1].listVidCap[fsizeind-1].width = fsize.discrete.width;
            listVidFormats[fmtind-1].listVidCap[fsizeind-1].height = fsize.discrete.height;
            
            if (verbose)
            {
                std::cout << "    { discrete: width=" << fsize.discrete.width 
                    << " height=" << fsize.discrete.height << " }\n";
            }
            
            ret = enum_frame_intervals( pixfmt,
                fsize.discrete.width, 
                fsize.discrete.height,
                fmtind, 
                fsizeind);

                if (ret != 0) std::cerr <<"  Unable to enumerate frame sizes\n";
        } 
        else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) 
        {
            if (verbose)
            {
                std::cout << "    { continuous: min { width = " << fsize.stepwise.min_width 
                    <<", height = " << fsize.stepwise.min_height 
                    << "} .. max { width = " << fsize.stepwise.max_width 
                    << ", height = " << fsize.stepwise.max_height << " } }\n";
                std::cout << "  will not enumerate frame intervals.\n";
            }
        } 
        else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) 
        {
            if (verbose)
            {
                std::cout << "    { stepwise: min { width = " << fsize.stepwise.min_width 
                    << ", height = " << fsize.stepwise.min_height 
                    << " } .. max { width = " << fsize.stepwise.max_width 
                    << ", height = " << fsize.stepwise.max_height 
                    << " } / stepsize { width = " << fsize.stepwise.step_width 
                    << ", height = " << fsize.stepwise.step_height << " } }\n";
                std::cout << "  will not enumerate frame intervals.\n";
            }
        } 
        else 
        {
            std::cerr << "  fsize.type not supported: " << fsize.type << "\n";
            std::cerr << "     (Discrete: " << V4L2_FRMSIZE_TYPE_DISCRETE << "Continuous: " << V4L2_FRMSIZE_TYPE_CONTINUOUS 
                << "  Stepwise: " << V4L2_FRMSIZE_TYPE_STEPWISE << ")\n";
        }
    }
    if (ret != 0 && errno != EINVAL) 
    {
        std::cerr << "libgvideo: VIDIOC_ENUM_FRAMESIZES Error enumerating frame sizes:" << strerror(errno) << std::endl;
        return errno;
    } 
    else if ((ret != 0) && (fsizeind == 0)) 
    {
        /* ------ older gspca doesn't enumerate frame sizes ------ */
        /*          negotiate with VIDIOC_TRY_FMT instead          */

        fsizeind++;
        struct v4l2_format fmt;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = pixfmt;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        ret = xioctl(VIDIOC_TRY_FMT, &fmt);
        /*use the returned values*/
        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;
        if (verbose)
        {
            std::cout << "    { ?GSPCA? : width = " << width << ", height = " << height << " }\n";
            std::cout << "    fmtind:" << fmtind << " fsizeind: " << fsizeind << "\n";
        }
        if(listVidFormats[fmtind-1].listVidCap.size() > 0) 
        {
            listVidFormats[fmtind-1].listVidCap.resize(fsizeind);
            listVidFormats[fmtind-1].listVidCap[0].fps.resize(1);
        } 
        else
        {
            std::cerr << "assert failed: listVidCap not Null\n";
            return (-2);
        }
        listVidFormats[fmtind-1].listVidCap[0].width = width;
        listVidFormats[fmtind-1].listVidCap[0].height = height;
        listVidFormats[fmtind-1].listVidCap[0].fps[0].num = 1;
        listVidFormats[fmtind-1].listVidCap[0].fps[0].denom = 25;
    }

    return 0;
}

/* enumerate frames (formats, sizes and fps)
 * args:
 * width: current selected width
 * height: current selected height
 * fd: device file descriptor
 *
 * returns: pointer to LFormats struct containing list of available frame formats */
int GVDevice::enum_frame_formats()
{
    int ret=0;
    int fmtind=0;
    struct v4l2_fmtdesc fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while ((ret = xioctl(VIDIOC_ENUM_FMT, &fmt)) == 0) 
    {
        fmt.index++;
        /*check if format is supported by gvideo, set hardware flag and allocate on device list*/
        GVFormats *formats = new GVFormats;
        if((ret=formats->set_SupPixFormat(fmt.pixelformat)) >= 0) 
        {
            fmtind++;
            listVidFormats.resize(fmtind);
            listVidFormats[fmtind-1].format=fmt.pixelformat;
            //fourcc is in upper case
            listVidFormats[fmtind-1].fourcc.push_back(fmt.pixelformat & 0xFF);
            listVidFormats[fmtind-1].fourcc.push_back((fmt.pixelformat >> 8) & 0xFF);
            listVidFormats[fmtind-1].fourcc.push_back((fmt.pixelformat >> 16) & 0xFF);
            listVidFormats[fmtind-1].fourcc.push_back((fmt.pixelformat >> 24) & 0xFF);
            
            listVidFormats[fmtind-1].description = std::string(( char* ) fmt.description);
            
            if(verbose)
            {
                std::cout << "  { format='" << fmt.pixelformat 
                    << "'  description='" << fmt.description << " }\n";
            }
            //enumerate frame sizes
            ret = enum_frame_sizes( fmt.pixelformat, fmtind );
            if (ret != 0)
               std::cerr << "  Unable to enumerate frame sizes.:" << strerror(errno) << std::endl;
        }
        else
        {
            std::cerr << "   { not supported - request format " << fmt.pixelformat << " support at http://guvcview.berlios.de }\n";
        }
    }
    if (errno != EINVAL) {
        std::cerr << "VIDIOC_ENUM_FMT - Error enumerating frame formats:" << strerror(errno) << std::endl;
    }

    return (0);
}


/* Query video device capabilities and supported formats
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 *
 * returns: error code  (0- OK)
*/
int GVDevice::check_input()
{
    int ret = 0;
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(struct v4l2_capability));

    if ( (ret = xioctl(VIDIOC_QUERYCAP, &cap)) < 0 ) 
    {
        std::cerr << "libgvideo: VIDIOC_QUERYCAP error" << ret << std::endl;
        return -1;
    }

    if ( ( cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) == 0) 
    {
        std::cerr << "libgvideo: Error opening device " << dev_name << ": video capture not supported" << std::endl;
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
    {
        std::cerr << "libgvideo: " << dev_name << " does not support streaming i/o" << std::endl; 
        return -1;
    }

    if(cap_meth == IO_READ) 
    {
        mem[buff_index] = NULL;
        if (!(cap.capabilities & V4L2_CAP_READWRITE)) 
        {
            std::cerr << "libgvideo: " << dev_name << " does not support read i/o" << std::endl;
            return -2;
        }
    }
    
    if (verbose)
        std::cout << "Init. " << cap.card << " (location:" << cap.bus_info <<")" << std::endl;

    enum_frame_formats();

    if ( listVidFormats.size() <= 0)
        std::cerr << "libgvideo: Couldn't detect any supported formats on your device " 
            << listVidFormats.size() << std::endl;

    return 0;
}

int GVDevice::add_control(int n, struct v4l2_queryctrl *queryctrl)
{
    // Add control to control list
    //std::cout << "adding control [" << n-1 << "] " << queryctrl->name << "...";
    listControls.resize(n);
    listControls[n-1].i = n-1;
    listControls[n-1].ctrl_class = (queryctrl->id & 0xFFFF0000);
    memcpy(&(listControls[n-1].control), queryctrl, sizeof(struct v4l2_queryctrl));
   
    if (queryctrl->type == V4L2_CTRL_TYPE_MENU) 
    {
        struct v4l2_querymenu querymenu;
        memset(&querymenu,0,sizeof(struct v4l2_querymenu));
        listControls[n-1].control.minimum = 0;
        querymenu.id = queryctrl->id;
        querymenu.index = 0;
        while ( xioctl(VIDIOC_QUERYMENU, &querymenu) == 0 ) 
        {
            //allocate entries list
            listControls[n-1].entries.resize(querymenu.index+1);
            //allocate entrie name
            listControls[n-1].entries[querymenu.index] = std::string((char *) querymenu.name);
            querymenu.index++;
        }
        if(listControls[n-1].control.maximum != querymenu.index - 1)
            std::cerr << "libgvideo: menu maximum(" << listControls[n-1].control.maximum << "max index(" 
                << querymenu.index - 1 << ")\n";
    }
    else if (queryctrl->type == V4L2_CTRL_TYPE_STRING) 
    {
        listControls[n-1].value_str = new char[queryctrl->maximum + 1];
    }
    //std::cout << "finished \n";
}

int GVDevice::check_controls()
{
    int ret=0;
    int tries=4;
    int n = 0;
    struct v4l2_queryctrl queryctrl = {0}; 

    queryctrl.id = 0 | V4L2_CTRL_FLAG_NEXT_CTRL;

    if ((ret=xioctl (VIDIOC_QUERYCTRL, &queryctrl)) == 0)
    {
        if (verbose) 
            std::cout << "V4L2_CTRL_FLAG_NEXT_CTRL supported\n";
        // The driver supports the V4L2_CTRL_FLAG_NEXT_CTRL flag
        queryctrl.id = 0;
        int currentctrl= queryctrl.id;
        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

        // Loop as long as ioctl does not return EINVAL
        // don't use xioctl here since we must reset queryctrl.id every retry (is this realy true ??)
        while((ret = v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)), ret ? errno != EINVAL : 1) 
        {

            if(ret && (errno == EIO || errno == EPIPE || errno == ETIMEDOUT))
            {
                // I/O error RETRY
                queryctrl.id = currentctrl | V4L2_CTRL_FLAG_NEXT_CTRL;
                tries = 4;
                while(tries-- &&
                    (ret = v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) &&
                    (errno == EIO || errno == EPIPE || errno == ETIMEDOUT)) 
                {
                    queryctrl.id = currentctrl | V4L2_CTRL_FLAG_NEXT_CTRL;
                }
                if ( ret && ( tries <= 0)) 
                    std::cerr << "libgvideo: Failed to query control id=" << currentctrl 
                        << "tried 4 times - giving up:" << strerror(errno) << std::endl;
            }
            // Prevent infinite loop for buggy NEXT_CTRL implementations
            if(ret && queryctrl.id <= currentctrl) 
            {
                currentctrl++;
                goto next_control;
            }
            currentctrl = queryctrl.id;

            // skip if control is disabled or failed
            if (ret || (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                goto next_control;

            // Add control to control list
            n++;
            add_control(n, &queryctrl);

next_control:
            queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
    }
    else //NEXT_CTRL flag not supported, use old method 
    {
        if (verbose)
            std::cout << "V4L2_CTRL_FLAG_NEXT_CTRL not supported\n";
        int currentctrl;
        for(currentctrl = V4L2_CID_BASE; currentctrl < V4L2_CID_LASTP1; currentctrl++) 
        {
        queryctrl.id = currentctrl;
            ret = xioctl(VIDIOC_QUERYCTRL, &queryctrl);

            if(ret || queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                continue;
            // Add control to control list
            n++;
            add_control(n, &queryctrl);
        }

        for(currentctrl = V4L2_CID_CAMERA_CLASS_BASE; currentctrl < V4L2_CID_CAMERA_CLASS_LAST; currentctrl++) 
        {
            queryctrl.id = currentctrl;
            // Try querying the control
            ret = xioctl(VIDIOC_QUERYCTRL, &queryctrl);

            if(ret || queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                continue;
            // Add control to control list
            n++;
            add_control(n, &queryctrl);
        }

        for(currentctrl = V4L2_CID_BASE_LOGITECH; currentctrl < V4L2_CID_LAST_EXTCTR; currentctrl++) 
        {
            queryctrl.id = currentctrl;
            // Try querying the control
            ret = xioctl(VIDIOC_QUERYCTRL, &queryctrl);

            if(ret || queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                continue;
            // Add control to control list
            n++;
            add_control(n, &queryctrl);
        }
    }
    
    //get the controls current value
    get_all_controls_val();
}

/* runs the control list and gets the controls values
 * args:
 * returns:
 */
void GVDevice::get_all_controls_val()
{
    int i = 0;
    int j = 0;
    int count = 0;
    bool process = false;
    struct v4l2_ext_control clist[listControls.size()];
    
    for (i=0; i< listControls.size(); i++)
    {
        if(listControls[i].control.flags & V4L2_CTRL_FLAG_WRITE_ONLY)
            continue;
        //std::cout << "control[" << i << "] ...";
        clist[count].id = listControls[i].control.id;
        clist[count].size = 0;
        if(listControls[i].control.type == V4L2_CTRL_TYPE_STRING)
        {
            clist[count].size = listControls[i].control.maximum + 1;
            clist[count].string = listControls[i].value_str; 
        }
        count++;
        
        if(listControls.size() <= i+1)
            process = true;
        else if (listControls[i+1].ctrl_class != listControls[i].ctrl_class)
            process = true;
            
        if (process)    
        {
            //get all controls in the same class
            struct v4l2_ext_controls ctrls = {0};
            ctrls.ctrl_class = listControls[i].ctrl_class;
            ctrls.count = count;
            ctrls.controls = clist;
            if(xioctl(VIDIOC_G_EXT_CTRLS, &ctrls))
            {
                std::cerr << "libgvideo: VIDIOC_G_EXT_CTRLS failed for control list\n";
                for(j=0; j< count; j++)
                {
                    //get one by one
                    get_control_val(get_control_index(clist[j].id));
                }
            }
            else
            {
                for(j=0; j<count; j++)
                {
                    int index = get_control_index(clist[j].id);
                    switch(listControls[index].control.type)
                    {
                    case V4L2_CTRL_TYPE_STRING:
                        //string gets set on VIDIOC_G_EXT_CTRLS
                        //add the maximum size to value
                        listControls[index].value = clist[j].size;
                        break;
                    case V4L2_CTRL_TYPE_INTEGER64:
                        listControls[index].value64 = clist[j].value64;
                        break;
                    default:
                        listControls[index].value = clist[j].value;
                        //printf("control %i [0x%08x] = %i\n", 
                        //    i, clist[i].id, clist[i].value);
                        break;
                }
                }
            }
            count = 0;
            process = false;
        }
        //std::cout << " finished\n";
    }
    
    update_control_list_flags();
}

/* get device control value and updates it in the control list
 * args:
 * control_index: control index from listControls 
 * returns: VIDIOC_G_CTRL return value ( failure  < 0 )
 */
int GVDevice::get_control_val (int control_index)
{
    int ret=0;
    if(listControls[control_index].ctrl_class == V4L2_CTRL_CLASS_USER)
    {
        struct v4l2_control c = {0};
        c.id  = listControls[control_index].control.id;
        ret = xioctl (VIDIOC_G_CTRL, &c);
        if (!ret) 
            listControls[control_index].value = c.value;
    }
    else
    {
        struct v4l2_ext_controls ctrls = {0};
        struct v4l2_ext_control ctrl = {0};
        ctrl.id = listControls[control_index].control.id;
        ctrl.size = 0;
        if(listControls[control_index].control.type == V4L2_CTRL_TYPE_STRING)
        {
            ctrl.size = listControls[control_index].control.maximum + 1;
            ctrl.string = listControls[control_index].value_str; 
        }
        ctrls.count = 1;
        ctrls.controls = &ctrl;
        ret = xioctl(VIDIOC_G_EXT_CTRLS, &ctrls);
        if(!ret)
        {
            switch(listControls[control_index].control.type)
            {
                case V4L2_CTRL_TYPE_STRING:
                    //string gets set on VIDIOC_G_EXT_CTRLS
                    //add the maximum size to value
                    listControls[control_index].value = ctrl.size;
                    break;
                case V4L2_CTRL_TYPE_INTEGER64:
                    listControls[control_index].value64 = ctrl.value64;
                    break;
                default:
                    listControls[control_index].value = ctrl.value;
                    //printf("control %i [0x%08x] = %i\n", 
                    //    i, clist[i].id, clist[i].value);
                    break;
            }
        }
    }
    
    update_control_flags(listControls[control_index].control.id);
    
    if(ret)
        std::cerr << "libgvideo: "<< listControls[control_index].control.name 
            << " failed to get value: " << strerror(errno) << std::endl;
    return ret;
}

/* sets all the control list values into the hardware
 * args:
 *
 * returns: 
 */
void GVDevice::set_all_controls_val ()
{
    int i = 0;
    int j = 0;
    int count = 0;
    bool process = false;
    
    struct v4l2_ext_control clist[listControls.size()];
    
    for (i=0; i< listControls.size(); i++)
    {
        if(listControls[i].control.flags & V4L2_CTRL_FLAG_READ_ONLY)
            continue;
        clist[count].id = listControls[i].control.id;
        clist[count].size = 0;
        if(listControls[i].control.type == V4L2_CTRL_TYPE_STRING)
        {
            clist[count].size = listControls[i].control.maximum + 1;
            clist[count].string = listControls[i].value_str; 
        }
        else if (listControls[i].control.type == V4L2_CTRL_TYPE_INTEGER64)
        {
            clist[count].value64 = listControls[i].value64;
        }
        else
            clist[count].value = listControls[i].value;
            
        count++;
        
        if(listControls.size() <= i+1)
            process = true;
        else if (listControls[i+1].ctrl_class != listControls[i].ctrl_class)
            process = true;
        
        if(process)
        {
            //set all controls in the same class
            struct v4l2_ext_controls ctrls = {0};
            ctrls.ctrl_class = listControls[i].ctrl_class;
            ctrls.count = count;
            ctrls.controls = clist;
            if(xioctl(VIDIOC_S_EXT_CTRLS, &ctrls))
            {
                for(j=0; j< count; j++)
                {
                    //set one by one
                    set_control_val(get_control_index(clist[j].id));
                }
            }
            count = 0;
            process = false;
        }
    }
    //get real values from the hardware
    get_all_controls_val();
}

/* sets the control value into the hardware
 * args:
 * control_index: control index from listControls
 *
 * returns: VIDIOC_S_CTRL return value ( failure  < 0 )                   */
int GVDevice::set_control_val (int control_index)
{
    int ret=0;
    
    if(listControls[control_index].control.flags & V4L2_CTRL_FLAG_READ_ONLY)
        return (-1);

    if( listControls[control_index].ctrl_class == V4L2_CTRL_CLASS_USER)
    {
        struct v4l2_control ctrl;
        //printf("   using VIDIOC_G_CTRL for user class controls\n");
        ctrl.id = listControls[control_index].control.id;
        ctrl.value = listControls[control_index].value;
        ret = xioctl(VIDIOC_S_CTRL, &ctrl);
    }
    else
    {
        //printf("   using VIDIOC_G_EXT_CTRLS on single controls for class: 0x%08x\n", 
        //    current->class);
        struct v4l2_ext_controls ctrls = {0};
        struct v4l2_ext_control ctrl = {0};
        ctrl.id = listControls[control_index].control.id;
        switch (listControls[control_index].control.type)
        {
            case V4L2_CTRL_TYPE_STRING:
                ctrl.size = listControls[control_index].value;
                ctrl.string = listControls[control_index].value_str;
                break;
            case V4L2_CTRL_TYPE_INTEGER64:
                ctrl.value64 = listControls[control_index].value64;
                break;
            default:
                ctrl.value = listControls[control_index].value;
                break;
        }
        ctrls.count = 1;
        ctrls.controls = &ctrl;
        ret = xioctl(VIDIOC_S_EXT_CTRLS, &ctrls);
    }
    
    if(ret)
        std::cerr << "libgvideo: "<< listControls[control_index].control.name 
            << " failed to set value: " << strerror(errno) << std::endl;
            
    //update with real value 
    get_control_val (control_index);
    return ret;
}

/* updates related control flags for control id
 * args:
 * control_id: control v4l2 id
 *
 * returns: 
 */
void GVDevice::update_control_flags(int id)
{
    int index = get_control_index( id );
    if (index < 0)
        return;
        
    switch (id) 
    {
        case V4L2_CID_EXPOSURE_AUTO:
            {   
                switch (listControls[index].value) 
                {
                    case V4L2_EXPOSURE_AUTO:
                        {
                            //printf("auto\n");
                            int ref_index = get_control_index(V4L2_CID_IRIS_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                    
                            ref_index = get_control_index(V4L2_CID_IRIS_RELATIVE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                                
                            ref_index = get_control_index(V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                        }
                        break;
                        
                    case V4L2_EXPOSURE_APERTURE_PRIORITY:
                        {
                            int ref_index = get_control_index(V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                                
                            ref_index = get_control_index(V4L2_CID_IRIS_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                                
                            ref_index = get_control_index(V4L2_CID_IRIS_RELATIVE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;
                        
                    case V4L2_EXPOSURE_SHUTTER_PRIORITY:
                        {
                            int ref_index = get_control_index(V4L2_CID_IRIS_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            
                            ref_index = get_control_index(V4L2_CID_IRIS_RELATIVE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                                
                            ref_index = get_control_index(V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;
                    
                    default:
                        {
                            int ref_index = get_control_index(V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                                
                            ref_index = get_control_index(V4L2_CID_IRIS_ABSOLUTE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                                
                            ref_index = get_control_index(V4L2_CID_IRIS_RELATIVE );
                            if (ref_index >= 0)
                                listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;
                }
            }
            break;

        case V4L2_CID_FOCUS_AUTO:
            {
                if(listControls[index].value > 0) 
                {
                    int ref_index = get_control_index(V4L2_CID_FOCUS_ABSOLUTE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            
                    ref_index = get_control_index(V4L2_CID_FOCUS_RELATIVE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    int ref_index = get_control_index(V4L2_CID_FOCUS_ABSOLUTE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                         
                    ref_index = get_control_index(V4L2_CID_FOCUS_RELATIVE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;
            
        case V4L2_CID_HUE_AUTO:
            {
                if(listControls[index].value > 0) 
                {
                    int ref_index = get_control_index(V4L2_CID_HUE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    int ref_index = get_control_index(V4L2_CID_HUE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;

        case V4L2_CID_AUTO_WHITE_BALANCE:
            {
                if(listControls[index].value > 0) 
                {
                    int ref_index = get_control_index(V4L2_CID_WHITE_BALANCE_TEMPERATURE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                        
                    ref_index = get_control_index(V4L2_CID_BLUE_BALANCE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                        
                    ref_index = get_control_index(V4L2_CID_RED_BALANCE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    int ref_index = get_control_index(V4L2_CID_WHITE_BALANCE_TEMPERATURE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        
                    ref_index = get_control_index(V4L2_CID_BLUE_BALANCE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        
                    ref_index = get_control_index(V4L2_CID_RED_BALANCE);
                    if (ref_index >= 0)
                        listControls[ref_index].control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;
    }
}

/*
 * updates the flags for all controls in the control list
 */
void GVDevice::update_control_list_flags()
{
    int i = 0;
    for (i = 0; i < listControls.size(); i++)
    {
        update_control_flags(listControls[i].control.id);
    }
}

int GVDevice::map_buff()
{
    int i = 0;
    // map new buffers
    for (i = 0; i < NB_BUFFER; i++) 
    {
        mem[i] = (UINT8 *) v4l2_mmap( NULL, // start anywhere
            buff_length[i], 
            PROT_READ | PROT_WRITE,
            MAP_SHARED, 
            fd,
            buff_offset[i]);
        if (mem[i] == MAP_FAILED) 
        {
            std::cerr << "libgvideo: Unable to map buffer[" << i << "]:" << strerror(errno) << std::endl;
            mmaped = false;
            return -1;
        }
    }
    mmaped = true;
    return (0);
}

int GVDevice::unmap_buff()
{
    int i=0;
    int ret=0;

    switch(method)
    {
        case IO_READ:
            break;

        case IO_MMAP:
            for (i = 0; i < NB_BUFFER; i++) 
            {
                // unmap old buffer
                if((mem[i] != MAP_FAILED) && buff_length[i])
                    if((ret=v4l2_munmap(mem[i], buff_length[i]))<0)
                    {
                        std::cerr << "libgvideo: couldn't unmap buff:" << strerror(errno) << std::endl;
                    }
            }
    }
    mmaped = false;
    return ret;
}

int GVDevice::query_buff()
{
    int i=0;
    int ret=0;
    struct v4l2_buffer buf;
    
    //clean vectors with buff parameters
    buff_length.clear();
    buff_offset.clear();
    
    switch(method)
    {
        case IO_READ:
            break;

        case IO_MMAP:
            for (i = 0; i < NB_BUFFER; i++) 
            {
                memset(&buf, 0, sizeof(struct v4l2_buffer));
                buf.index = i;
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                ret = xioctl(VIDIOC_QUERYBUF, &buf);
                if (ret < 0) 
                {
                    std::cerr << "libgvideo: VIDIOC_QUERYBUF Unable to query buffer:" << strerror(errno) << std::endl;
                    if(errno == EINVAL)
                    {
                        std::cerr << "trying with read method instead\n";
                        method = IO_READ;
                    }
                    return -1;
                }
                if (buf.length <= 0) 
                    std::cerr << "libgvideo: WARNING VIDIOC_QUERYBUF buffer length is " << buf.length << std::endl;

                buff_length.push_back(buf.length);
                buff_offset.push_back(buf.m.offset);
            }
            // map the new buffers
            if(map_buff() != 0) 
                return -2;
    }
    return 0;
}

int GVDevice::queue_buff()
{
    int i=0;
    int ret=0;
    struct v4l2_buffer buf;
    
    switch(method)
    {
        case IO_READ:
            break;

        case IO_MMAP:
        default:
            for (i = 0; i < NB_BUFFER; ++i) 
            {
                memset(&buf, 0, sizeof(struct v4l2_buffer));
                buf.index = i;
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                ret = xioctl(VIDIOC_QBUF, &buf);
                if (ret < 0) 
                {
                    std::cerr << "VIDIOC_QBUF - Unable to queue buffer:" << strerror(errno) << std::endl;
                    return -1;
                }
            }
            buf.index = 0; /*reset index*/
    }
    return 0;
}

int GVDevice::set_format(std::string fourcc, int twidth, int theight)
{
    int ret = 0;
    if(streaming) stream_off();
    
    if(stream_ready)
    {
        //cleanup previous format allocations
        clean_buffers();
    }

    int format_index = get_format_index(fourcc);
    if (verbose) std::cout << "format index:" << format_index << std::endl;
    int res_index = get_res_index(format_index, twidth, theight);
    if (verbose) std::cout << "resolution index:" << res_index << std::endl;
    
    struct v4l2_format fmt;
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers rb;
    
    // set format
    width = listVidFormats[format_index].listVidCap[res_index].width;
    height = listVidFormats[format_index].listVidCap[res_index].height;
    format = listVidFormats[format_index].format;
    
    if(!fps->num)
    {
        fps->num = listVidFormats[format_index].listVidCap[res_index].fps[0].num;
        fps->denom = listVidFormats[format_index].listVidCap[res_index].fps[0].denom;
    }
    
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = format;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    ret = xioctl(VIDIOC_S_FMT, &fmt);
    if (ret < 0) 
    {
        std::cerr << "libgvideo: VIDIOC_S_FORMAT Unable to set format:" << strerror(errno) << std::endl;
        return -1;
    }

    stream_ready = true; //we have sucefully set a format so we are in stream mode

    if ((fmt.fmt.pix.width != width) ||
        (fmt.fmt.pix.height != height)) 
    {
        std::cerr << "libgvideo: Requested Format unavailable: get width=" << fmt.fmt.pix.width
            << " height=" << fmt.fmt.pix.height << std::endl;

        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;
    }
    
    if(fps->num > 0)
        set_framerate ();
    setFPS = false;
    
    switch (method)
    {
        case IO_READ: //allocate only one buffer for read
            buff_index = 0;
            buff_length.push_back(width * height * 3); //worst case (rgb)
            mem[buff_index] = new UINT8 [buff_length[buff_index]];
            break;

        case IO_MMAP:
        default:
            // request buffers 
            memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
            rb.count = NB_BUFFER;
            rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            rb.memory = V4L2_MEMORY_MMAP;

            ret = xioctl(VIDIOC_REQBUFS, &rb);
            if (ret < 0) 
            {
                std::cerr << "libgvideo: VIDIOC_REQBUFS Unable to allocate buffers:" << strerror(errno) << std::endl;
                return -2;
            }
            // map the buffers 
            if (query_buff()) 
            {
                //delete requested buffers
                //no need to unmap as mmap failed for sure
                memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
                rb.count = 0;
                rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(VIDIOC_REQBUFS, &rb)<0)
                    std::cerr << "libgvideo: VIDIOC_REQBUFS Unable to delete buffers:" 
                        << strerror(errno) << std::endl;
                return -3;
            }
            // Queue the buffers
            if (queue_buff()) 
            {
                //delete requested buffers
                if(mmaped) unmap_buff();
                memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
                rb.count = 0;
                rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(VIDIOC_REQBUFS, &rb)<0)
                    std::cerr << "libgvideo: VIDIOC_REQBUFS Unable to delete buffers:" 
                        << strerror(errno) << std::endl;
                return -3;
            }
    }
    
    return (0);
}

int GVDevice::get_format()
{
    return (format);
}

int GVDevice::get_width()
{
    return (width);
}

int GVDevice::get_height()
{
    return (height);
}

int GVDevice::get_framecount()
{
    return (framecount);
}

UINT64 GVDevice::get_timestamp()
{
    return (timestamp[buff_index]);
}
/*
 * start video stream
 */
int GVDevice::stream_on()
{
    if(streaming) 
        std::cerr << "libgvideo: WARNING: stream seems to be already on\n";
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret=0;
    switch(method)
    {
        case IO_READ:
            //do nothing
            break;

        case IO_MMAP:
        default:
            ret = xioctl(VIDIOC_STREAMON, &type);
            if (ret < 0) 
            {
                std::cerr << "libgvideo: VIDIOC_STREAMON Unable to start stream:" 
                    << strerror(errno) << std::endl;
                return -1;
            }
            break;
    }
    streaming = true;
    return 0;
}

int GVDevice::stream_off()
{
    if(!streaming) 
        std::cerr << "libgvideo: WARNING: stream seems to be already off\n";
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret=0;
    switch(method)
    {
        case IO_READ:
            //do nothing
            break;

        case IO_MMAP:
        default:
            ret = xioctl(VIDIOC_STREAMOFF, &type);
            if (ret < 0) 
            {
                std::cerr << "libgvideo: VIDIOC_STREAMOFF Unable to stop stream:" << strerror(errno) << std::endl;
                if(errno == 9) streaming = false;/*capture as allready stoped*/
                return -1;
            }
            break;
    }
    streaming = false;
    return 0;
}

/* Grabs a video frame
 * args:
 *
 * returns: error code ( 0 - no error)
*/
int GVDevice::grab_frame(UINT8* data)
{
    int ret = 0;
    struct v4l2_buffer buf;
    fd_set rdset;
    struct timeval timeout;
    UINT64 ts = 0;
    //make sure streaming is on
    if (!streaming)
        if (stream_on()) goto err;
 
    FD_ZERO(&rdset);
    FD_SET(fd, &rdset);
    timeout.tv_sec = 1; // 1 sec timeout 
    timeout.tv_usec = 0;
    // select - wait for data or timeout
    ret = select(fd + 1, &rdset, NULL, NULL, &timeout);
    if (ret < 0) 
    {
        std::cerr << "libgvideo: Could not grab frame (select error)" << strerror(errno) << std::endl;
        return -2;
    } 
    else if (ret == 0)
    {
        std::cerr << "libgvideo: Could not grab frame (select timeout)" << strerror(errno) << std::endl;
        return -3;
    }
    else if ((ret > 0) && (FD_ISSET(fd, &rdset))) 
    {
        switch(method)
        {
            case IO_READ:
                if(setFPS && (framecount > 5))
                {
                    stream_off();
                    set_framerate ();
                    stream_on();
                    setFPS = false; /*no need to query and queue buufers*/
                }
                buff_bytesused[buff_index] = v4l2_read (fd, mem[buff_index], buff_length[buff_index]);
                if (buff_bytesused < 0) 
                {
                    switch (errno) 
                    {
                        case EAGAIN:
                            std::cerr << "ligbvideo: No data available for read\n";
                            return -3;
                            break;
                        case EINVAL:
                            std::cerr << "ligbvideo: Read method error, try mmap instead:" 
                                << strerror(errno) << std::endl;
                            return -4;
                            break;
                        case EIO:
                            std::cerr << "ligbvideo: read I/O Error:" << strerror(errno) << std::endl;
                            return -4;
                            break;
                        default:
                            std::cerr << "ligbvideo: read:" << strerror(errno) << std::endl;
                            return -4;
                            break;
                    }
                }
                timestamp[buff_index] = time->ns_time_monotonic();
                break;

            case IO_MMAP:
            default:
                /*query and queue buffers since fps or compression as changed*/
                if(setFPS && (framecount > 5))
                {
                    stream_off();
                    unmap_buff();
                    set_framerate ();
                    query_buff();
                    queue_buff();
                    stream_on();
                    setFPS = false;
                    return(0);
                }
                else
                {
                    memset(&buf, 0, sizeof(struct v4l2_buffer));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;

                    ret = xioctl(VIDIOC_DQBUF, &buf);
                    if (ret < 0) 
                    {
                        std::cerr << "ligbvideo: VIDIOC_DQBUF Unable to dequeue buffer:" 
                            << strerror(errno) << std::endl;
                        ret = -5;
                        return ret;
                    }
                    buff_index = buf.index;
                    ts = (UINT64) buf.timestamp.tv_sec * GV_NSEC_PER_SEC +  
                        buf.timestamp.tv_usec * GV_MSEC_PER_SEC; //in nanosec
                    /* use buffer timestamp if set by the driver, otherwise use current system time */
                    if(ts > 0) timestamp[buff_index] = ts; 
                    else timestamp[buff_index] = time->ns_time_monotonic();
                    buff_bytesused[buff_index] = buf.bytesused;

                    ret = xioctl(VIDIOC_QBUF, &buf);
                    if (ret < 0) 
                    {
                        std::cerr << "ligbvideo: VIDIOC_QBUF Unable to queue buffer:" 
                            << strerror(errno) << std::endl;
                        ret = -6;
                        return ret;
                    }
                }
        }
        
        memcpy(data, mem[buff_index], buff_bytesused[buff_index]);
        framecount++;
    }

    return 0;
err:
    return -1; //couldn't start the video stream
}

/* sets video device frame rate
 * args:
 *
 * returns: VIDIOC_S_PARM ioctl result value
*/
int GVDevice::set_framerate (GVFps* frate)
{  
    int ret=0;
    struct v4l2_streamparm streamparm;
    
    if(frate)
    {
        fps->num = frate->num;
        fps->denom = frate->denom;
    }
    
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = fps->num;
    streamparm.parm.capture.timeperframe.denominator = fps->denom;

    ret = xioctl(VIDIOC_S_PARM, &streamparm);
    if (ret < 0) 
    {
        std::cerr << "ligbvideo: Unable to set framerate to " 
            << fps->num << "/" << fps->denom << std::endl;
        std::cerr << "ligbvideo: VIDIOC_S_PARM error:" << strerror(errno) << std::endl;
    } 

    /*make sure we now have the correct fps*/
    get_fps (fps);
    if(verbose) std::cout << "framerate=" << fps->num << "/" << fps->denom << std::endl;

    if(frate)
    {
        frate->num = fps->num;
        frate->denom = fps->denom;
    }
    
    return ret;
}

/* 
 * gets video device defined frame rate (not real - consider it a maximum value)
*/
bool GVDevice::get_fps (GVFps* frate/* = NULL*/)
{
    int ret=0;
    struct v4l2_streamparm streamparm;
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    ret = xioctl(VIDIOC_G_PARM, &streamparm);
    if (ret < 0) 
    {
        std::cerr << "ligbvideo: VIDIOC_G_PARM Unable to get timeperframe:" 
            << strerror(errno) << std::endl;
    } 
    else 
    {
        /*update instance fps value*/
        fps->denom = streamparm.parm.capture.timeperframe.denominator;
        fps->num = streamparm.parm.capture.timeperframe.numerator;
    }
    
    if(frate)
    {
        frate->denom = fps->denom;
        frate->num = fps->num;
    }
    
    return true;
}

bool GVDevice::set_fps (GVFps* frate/* = NULL*/)
{
    bool res = true;
    if(streaming)
    {
        if(verbose) std::cout << " fps: streaming is on - delay\n";
        if(frate)
        {
            fps->num = frate->num;
            fps->denom = frate->denom;
        }
        setFPS=true; // delay until dequeue iteration
    }
    else if (mmaped) /*must unmap and requeue the buffers*/
    {
        if(verbose) std::cout << " fps: needs buffer requeuing\n";
        unmap_buff();
        if (set_framerate (frate) < 0)
            res = false;
        else
        {
            query_buff();
            queue_buff();
        }
    }
    else
        res = ( set_framerate (frate) < 0 );
    
    return res;
}

size_t GVDevice::get_bytesused ()
{
    return buff_bytesused[buff_index];
}

END_LIBGVIDEO_NAMESPACE
