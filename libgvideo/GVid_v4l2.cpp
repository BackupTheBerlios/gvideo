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
GVDevice::GVDevice(std::string device_name /*= "/dev/video0"*/, bool verbosity /*= false*/)
{
    verbose = verbosity;
    buff_index = 0;
    fps = new Fps_s;
    fps->num = 1;
    fps->denom = 25;
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
    framecount = 0;
    
    //defaults
    method = IO_MMAP;
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
    std::cerr << "Warn: format " << fourcc << " not supported\n";
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
    std::cerr << "Warn: resolution " << twidth << "x" << theight << " not supported for " 
        << listVidFormats[format_index].fourcc << std::endl;
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
                unmap_buff();
                memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
                rb.count = 0;
                rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(VIDIOC_REQBUFS, &rb)<0)
                {
                    std::cerr << "VIDIOC_REQBUFS - Failed to delete buffers: " 
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
        if(listControls[i].type == V4L2_CTRL_TYPE_MENU)
            listControls[i].entries.clear();
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
        std::cerr << "VIDIOC_ENUM_FRAMESIZES - Error enumerating frame sizes:" << strerror(errno) << std::endl;
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
        std::cerr << "VIDIOC_QUERYCAP error" << ret << std::endl;
        return -1;
    }

    if ( ( cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) == 0) 
    {
        std::cerr << "Error opening device " << dev_name << ": video capture not supported" << std::endl;
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
    {
        std::cerr << dev_name << " does not support streaming i/o" << std::endl; 
        return -1;
    }

    if(cap_meth == IO_READ) 
    {
        mem[buff_index] = NULL;
        if (!(cap.capabilities & V4L2_CAP_READWRITE)) 
        {
            std::cerr << dev_name << " does not support read i/o" << std::endl;
            return -2;
        }
    }
    
    if (verbose)
        std::cout << "Init. " << cap.card << " (location:" << cap.bus_info <<")" << std::endl;

    enum_frame_formats();

    if ( listVidFormats.size() <= 0)
        std::cerr << "Couldn't detect any supported formats on your device" 
            << listVidFormats.size() << std::endl;

    return 0;
}

int GVDevice::add_control(int n, struct v4l2_queryctrl *queryctrl)
{
    // Add control to control list
    listControls.resize(n);
    listControls[n-1].i = n-1;
    listControls[n-1].id = queryctrl->id;
    listControls[n-1].type = queryctrl->type;
    //allocate control name (must free it on exit)
    listControls[n-1].name = strdup ((char *) queryctrl->name);
    listControls[n-1].min = queryctrl->minimum;
    listControls[n-1].max = queryctrl->maximum;
    listControls[n-1].step = queryctrl->step;
    listControls[n-1].default_val = queryctrl->default_value;
    listControls[n-1].enabled = (queryctrl->flags & V4L2_CTRL_FLAG_GRABBED) ? 0 : 1;
 
    if (queryctrl->type == V4L2_CTRL_TYPE_BOOLEAN)
    {
        listControls[n-1].min = 0;
        listControls[n-1].max = 1;
        listControls[n-1].step = 1;
        /*get the first bit*/
        listControls[n-1].default_val=(queryctrl->default_value & 0x0001);
    } 
    else if (queryctrl->type == V4L2_CTRL_TYPE_MENU) 
    {
        struct v4l2_querymenu querymenu;
        memset(&querymenu,0,sizeof(struct v4l2_querymenu));
        listControls[n-1].min = 0;
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
        listControls[n-1].max = querymenu.index - 1;
    }
}

int GVDevice::check_controls()
{
    int ret=0;
    int tries=4;
    int n = 0;
    struct v4l2_queryctrl queryctrl; 
    memset(&queryctrl,0,sizeof(struct v4l2_queryctrl));

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
                    std::cerr << "Failed to query control id=" << currentctrl 
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
}

/* get device control value
 * args:
 * control_index: control index from listControls 
 * val: pointer to control value
 * returns: VIDIOC_G_CTRL return value ( failure  < 0 )                                                  */
int GVDevice::get_control_val (int control_index, int * val)
{
    int ret=0;
    struct v4l2_control c;
    memset(&c,0,sizeof(struct v4l2_control));

    c.id  = listControls[control_index].id;
    ret = xioctl (VIDIOC_G_CTRL, &c);
    if (ret == 0) 
        *val = c.value;
    else 
        std::cerr << "VIDIOC_G_CTRL - Unable to get control:" << strerror(errno) << std::endl;

    return ret;
}

/* set device control value
 * args:
 * control_index: control index from listControls
 * val: control value 
 *
 * returns: VIDIOC_S_CTRL return value ( failure  < 0 )                   */
int GVDevice::set_control_val (int control_index, int val)
{
    int ret=0;
    struct v4l2_control c;

    c.id  = listControls[control_index].id;
    c.value = val;
    ret = xioctl (VIDIOC_S_CTRL, &c);
    if (ret < 0) 
        std::cerr << "VIDIOC_S_CTRL - Unable to get control:" << strerror(errno) << std::endl;

    return ret;
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
            std::cerr << "Unable to map buffer:" << strerror(errno) << std::endl;
            return -1;
        }
    }
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
                        std::cerr << "couldn't unmap buff:" << strerror(errno) << std::endl;
                    }
            }
    }
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
                    std::cerr << "VIDIOC_QUERYBUF - Unable to query buffer:" << strerror(errno) << std::endl;
                    if(errno == EINVAL)
                    {
                        std::cerr << "trying with read method instead\n";
                        method = IO_READ;
                    }
                    return -1;
                }
                if (buf.length <= 0) 
                    std::cerr << "WARNING VIDIOC_QUERYBUF - buffer length is " << buf.length << std::endl;

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
    if(stream_ready)
    {
        //cleanup previous format allocations
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
    
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = format;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    ret = xioctl(VIDIOC_S_FMT, &fmt);
    if (ret < 0) 
    {
        std::cerr << "VIDIOC_S_FORMAT - Unable to set format:" << strerror(errno) << std::endl;
        return -1;
    }

    stream_ready = true; //we have sucefully set a format so we are in stream mode

    if ((fmt.fmt.pix.width != width) ||
        (fmt.fmt.pix.height != height)) 
    {
        std::cerr << "Requested Format unavailable: get width=" << fmt.fmt.pix.width
            << " height=" << fmt.fmt.pix.height << std::endl;

        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;
    }
    
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
                std::cerr << "VIDIOC_REQBUFS - Unable to allocate buffers:" << strerror(errno) << std::endl;
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
                    std::cerr << "VIDIOC_REQBUFS - Unable to delete buffers:" << strerror(errno) << std::endl;
                return -3;
            }
            // Queue the buffers
            if (queue_buff()) 
            {
                //delete requested buffers
                unmap_buff();
                memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
                rb.count = 0;
                rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                rb.memory = V4L2_MEMORY_MMAP;
                if(xioctl(VIDIOC_REQBUFS, &rb)<0)
                    std::cerr << "VIDIOC_REQBUFS - Unable to delete buffers:" << strerror(errno) << std::endl;
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
        std::cerr << "WARNING: stream seems to be already on\n";
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
                std::cerr << "VIDIOC_STREAMON - Unable to start stream:" << strerror(errno) << std::endl;
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
        std::cerr << "WARNING: stream seems to be already off\n";
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
                std::cerr << "VIDIOC_STREAMOFF - Unable to stop stream:" << strerror(errno) << std::endl;
                if(errno == 9) streaming = false;/*capture as allready stoped*/
                return -1;
            }
            break;
    }
    streaming = false;
    return 0;
}

/* Grabs video frame and decodes it if necessary
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
    timeout.tv_sec = 1; // 2 sec timeout 
    timeout.tv_usec = 0;
    // select - wait for data or timeout
    ret = select(fd + 1, &rdset, NULL, NULL, &timeout);
    if (ret < 0) 
    {
        std::cerr << " Could not grab frame (select error)" << strerror(errno) << std::endl;
        return -2;
    } 
    else if (ret == 0)
    {
        std::cerr << " Could not grab frame (select timeout)" << strerror(errno) << std::endl;
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
                            std::cerr << "No data available for read\n";
                            return -3;
                            break;
                        case EINVAL:
                            std::cerr << "Read method error, try mmap instead:" << strerror(errno) << std::endl;
                            return -4;
                            break;
                        case EIO:
                            std::cerr << "read I/O Error:" << strerror(errno) << std::endl;
                            return -4;
                            break;
                        default:
                            std::cerr << "read:" << strerror(errno) << std::endl;
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
                        std::cerr << "VIDIOC_DQBUF - Unable to dequeue buffer:" << strerror(errno) << std::endl;
                        ret = -5;
                        return ret;
                    }
                    buff_index = buf.index;
                    ts = (UINT64) buf.timestamp.tv_sec * GV_NSEC_PER_SEC +  buf.timestamp.tv_usec * GV_MSEC_PER_SEC; //in nanosec
                    /* use buffer timestamp if set by the driver, otherwise use current system time */
                    if(ts > 0) timestamp[buff_index] = ts; 
                    else timestamp[buff_index] = time->ns_time_monotonic();
                    buff_bytesused[buff_index] = buf.bytesused;

                    ret = xioctl(VIDIOC_QBUF, &buf);
                    if (ret < 0) 
                    {
                        std::cerr << "VIDIOC_QBUF - Unable to queue buffer:" << strerror(errno) << std::endl;
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
int GVDevice::set_framerate ()
{  
    int ret=0;
    struct v4l2_streamparm streamparm;
    
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = fps->num;
    streamparm.parm.capture.timeperframe.denominator = fps->denom;

    ret = xioctl(VIDIOC_S_PARM, &streamparm);
    if (ret < 0) 
    {
        std::cerr << "Unable to set framerate to " << fps->num << "/" << fps->denom << std::endl;
        std::cerr << "VIDIOC_S_PARM error:" << strerror(errno) << std::endl;
    } 

    /*make sure we now have the correct fps*/
    get_framerate ();

    return ret;
}

/* gets video device defined frame rate (not real - consider it a maximum value)
 * args:
 *
 * returns: a pointer to struct Fps_s
*/
Fps_s* GVDevice::get_framerate ()
{
    int ret=0;
    struct v4l2_streamparm streamparm;
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    ret = xioctl(VIDIOC_G_PARM, &streamparm);
    if (ret < 0) 
    {
        std::cerr << "VIDIOC_G_PARM - Unable to get timeperframe:" << strerror(errno) << std::endl;
    } 
    else 
    {
        fps->denom = streamparm.parm.capture.timeperframe.denominator;
        fps->num = streamparm.parm.capture.timeperframe.numerator;
    }
    return fps;
}

size_t GVDevice::get_bytesused ()
{
    return buff_bytesused[buff_index];
}

END_LIBGVIDEO_NAMESPACE
