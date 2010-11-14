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
#  GVideo V4L2 video grabber and control panel - video render thread            #
#                                                                               #
********************************************************************************/

#ifndef GV_VIDEO_THREADS_H
#define GV_VIDEO_THREADS_H

#include <string>
#include "gvcommon.h"
#include "libgvthreads/GVThreads.h"
#include "libgvideo/gvideo.h"
#include "libgvaudio/GVAudio.h"
#include "libgvencoder/GVCodec.h"

START_GVIDEOGTK_NAMESPACE

class GVVideoCapture : public libgvthreads::GVThread
{
  protected:
    libgvideo::GVDevice* dev;
    libgvaudio::GVAudio* audio;
    libgvencoder::GVCodec* encoder;
    libgvideo::GVBuffer* buf;
    libgvideo::VidBuff* capbuf;

    //audio device index
    int audio_dev_index;
    // video and audio codecs
    unsigned vcodec_ind;
    unsigned acodec_ind;
    
    libgvideo::GVFps fps;
    
    std::string video_filename;
    
    bool quit;
    int pix_order;
    bool capturing_video;
    bool set_fps;
    unsigned vcaptime;

  public:
    GVVideoCapture(
        libgvideo::GVDevice* _dev,
        libgvaudio::GVAudio* _audio,
        libgvencoder::GVCodec* _encoder,
        libgvideo::GVBuffer* _buf,
        int aindex,
        unsigned _vcodec_ind,
        unsigned _acodec_ind,
        libgvideo::GVFps _fps
        );
    bool capture_video;
    void run();

};

class GVVideoRender: public libgvthreads::GVThread
{
  protected:
    libgvideo::GVDevice* dev;
    libgvideo::GVBuffer* buf;
    libgvideo::VidBuff* framebuf;
    
    libgvideo::GVFps fps;
    
    std::string image_filename;
    
    int pix_order;
    bool set_fps;
    bool capture_image;
    bool capturing_video;
    

  public:
    GVVideoRender(
        libgvideo::GVDevice* _dev,
        libgvideo::GVBuffer* _buf,
        libgvideo::GVFps _fps
        );
    bool quit;
    void set_cap_video();
    void unset_cap_video();
    void cap_image(std::string _image_filename);
    void run();
};

class GVVideoThreads
{
  protected:
    libgvideo::GVDevice* dev;
    libgvideo::GVBuffer* buf;
    libgvencoder::GVCodec*  encoder;
    
    libgvideo::GVFps fps;
    
    GVVideoRender* th_video_render;
    GVVideoCapture* th_video_capture;

    bool capturing_video;

  public:
    GVVideoThreads(
        libgvideo::GVDevice* _dev,
        libgvencoder::GVCodec* encoder);
    ~GVVideoThreads();
    
    int set_format(std::string fourcc, int width, int height);
    void set_fps(libgvideo::GVFps* _fps);
    void quit();
    void start_video_capture(
        libgvaudio::GVAudio* _audio, 
        int audio_device,
        unsigned vcodec_index, 
        unsigned acodec_index);
    void stop_video_capture();
    void cap_image(std::string _image_filename);
    bool is_capturing_video();
};

END_GVIDEOGTK_NAMESPACE

#endif
