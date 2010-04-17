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

#include <sstream>
#include "gv_video_threads.h"
#include "libgvrender/render.h"
#include "libgvencoder/GVJpeg_encoder.h"
#include "libgvencoder/GVCodec.h"
#include "libgvencoder/GVMatroska.h"

START_GVIDEOGTK_NAMESPACE

GVVideoCapture::GVVideoCapture(
    libgvideo::GVDevice* _dev,
    libgvaudio::GVAudio* _audio,
    libgvideo::GVBuffer* _buf,
    int aindex,
    unsigned _vcodec_ind,
    unsigned _acodec_ind,
    libgvideo::GVFps _fps
    )
{
    dev = _dev;
    audio = _audio;
    buf = _buf;
    audio_dev_index = aindex;
    vcodec_ind = _vcodec_ind;
    acodec_ind = _acodec_ind;
    fps = _fps;
    video_filename = "gvideo_cap.mkv";
    capbuf = new libgvideo::VidBuff;

    capture_video = true;
    capturing_video = false;
}

void GVVideoCapture::run()
{
    capturing_video = true;
    int format = dev->get_format();
    int width = dev->get_width();
    int height = dev->get_height();
    UINT64 frame_count = 0;
    //update fps
    dev->get_fps(&fps);
    
    /*get a new audio instance*/
    if(!audio)
        audio = new libgvaudio::GVAudio();
    /*get a new audio buffer*/
    libgvaudio::AudBuff *aud_buf = new libgvaudio::AudBuff;
    /*get a new encoder instance*/
    libgvencoder::GVCodec*  encoder= new libgvencoder::GVCodec();
        
    /*get audio parameters*/
    int adev = audio->setDevice(audio_dev_index);
    std::cout << "using audio device id:" << adev << std::endl;
    int channels = audio->listAudioDev[adev].channels;
    int samprate = audio->listAudioDev[adev].samprate;
    UINT8* audio_out_buff = NULL;
    int asize = 0;
    if(acodec_ind > 0) 
    {
        asize = 240000;
        audio_out_buff = new UINT8 [asize];
        std::cout << "outbuf:" << long(audio_out_buff) << "\n";
    }
    int frame_size = encoder->open_acodec(acodec_ind, 
                                        audio_out_buff, 
                                        asize, 
                                        samprate,
                                        channels);
    if (frame_size <= 0)
    {
        std::cerr << "couldn't audio open codec (ind=" << acodec_ind << ")\n";
        /*clean up*/
        delete[] audio_out_buff;
        delete encoder;
        delete aud_buf;
        capturing_video = false;
        return;
    }
        
    std::cout << "audio: samprate=" << samprate << " channels=" 
        << channels << " frame size:" << frame_size << std::endl;
        
    /*audio frame duration*/
    UINT64 afdur = (UINT64) (1000*frame_size)/samprate;
    afdur = afdur * 1000000;
    /*video frame duration (use the set fps (25 default))*/
    UINT64 vfdur = 1 * 1e9/fps.denom;
        
    UINT32 size = width*height*2; /*enough for yuy2 (worst case)*/
    UINT8* video_out_buff = new UINT8 [size];
        
    if(!(encoder->open_vcodec(vcodec_ind, video_out_buff, size, width, height, fps.denom, fps.num)))
    {
        std::cerr << "couldn't open codec (ind=" << vcodec_ind << ")\n";
        /*clean up*/
        delete[] video_out_buff;
        delete[] audio_out_buff;
        delete encoder;
        delete aud_buf;
        capturing_video = false;
        return;
    }
        
    /*matroska video codec private data*/
    libgvideo::BITMAPINFOHEADER* mkv_priv = new libgvideo::BITMAPINFOHEADER;
    mkv_priv->biSize = 0x00000028; // 40 bytes
    mkv_priv->biWidth = width;
    mkv_priv->biHeight = height;
    mkv_priv->biPlanes = 1;
    mkv_priv->biBitCount = 24;
    mkv_priv->biCompression = V4L2_PIX_FMT_MJPEG;
    mkv_priv->biSizeImage = width * height * 2; //for rgb use w*h*3
    mkv_priv->biXPelsPerMeter = 0;
    mkv_priv->biYPelsPerMeter = 0;
    mkv_priv->biClrUsed = 0;
    mkv_priv->biClrImportant = 0;
    /*define a new matroska container*/
    libgvencoder::GVMatroska* matroska = new libgvencoder::GVMatroska( 
                                            video_filename.c_str(),
                                            "-gvideo-",
                                            encoder->vcodec_list[vcodec_ind].mkv_codec,
                                            encoder->acodec_list[acodec_ind].mkv_codec,
                                            encoder->vcodec_list[vcodec_ind].codecPriv,
                                            encoder->vcodec_list[vcodec_ind].codecPriv_size,
                                            vfdur,
                                            encoder->acodec_list[acodec_ind].codecPriv,
                                            encoder->acodec_list[acodec_ind].codecPriv_size,
                                            afdur,
                                            1000000,
                                            width, height,
                                            width, height,
                                            samprate, channels, 
                                            encoder->acodec_list[acodec_ind].bits,
                                            true);
    delete mkv_priv;
        
    bool try_next = true;
    int count = 0;
    UINT64 timestamp_f0 = 0;
    UINT64 vts = 0;
    UINT64 ats = 0;
        
    /*get video start time to sync with audio start time*/
    gvcommon::GVTime *timer = new gvcommon::GVTime;
    UINT64 vid_start_time = timer->ns_time_monotonic();
    delete timer; 
        
    std::cout << "vid start=" << vid_start_time << std::endl;
    buf->consume_nextFrame(capbuf);
    timestamp_f0 = capbuf->time_stamp;
    vts = 0;
            
    size = encoder->encode_video_frame (capbuf->yuv_frame);
    matroska->add_VideoFrame(video_out_buff, size, vts);
        
    std::cout << "ref ts=" << timestamp_f0 << std::endl;
    /*start audio stream*/
    audio->startStream(samprate, channels, frame_size);

    while( capture_video )
    {
        try_next = true; 
        while ( try_next )
        {
            /*check audio buffer*/
            try_next = audio->getNext(aud_buf);
            if (try_next)
            {
                ats = aud_buf->time_stamp - vid_start_time;
                if(acodec_ind > 0)
                {
                    asize = encoder->encode_audio_frame(aud_buf->i_frame);
                    matroska->add_AudioFrame(audio_out_buff, asize, ats);
                }
                else // use pcm buffer
                matroska->add_AudioFrame(aud_buf->i_frame, 
                                        aud_buf->samples * sizeof(INT16), 
                                        ats);
                /*audio timestamp more recent than video - go and process another video frame*/
                if(ats >= vts)
                    try_next = false;
            }
        }
        /*grab and process another video frame*/
        if (buf->consume_nextFrame(capbuf) >= 0);
        {
            vts = capbuf->time_stamp - timestamp_f0;
            //if (verbose) cout << "vts:" << vts << " max:" << (UINT64) options->opts->vcaptime * GV_NSEC_PER_SEC << endl;
            if( vcaptime && (vts > ((UINT64) vcaptime * GV_NSEC_PER_SEC)))
            {
                /*we already captured the defined amount of time so let's finish*/
                capture_video = false;
            }
            size = encoder->encode_video_frame (capbuf->yuv_frame);
            matroska->add_VideoFrame(video_out_buff, size, vts);
            frame_count++;
        }
    }
    /* stop the audio stream*/
    audio->stopStream();
        
    /*calc and set the default video frame duration (fps)*/
    vfdur = vts / frame_count;
    matroska->set_DefDuration(vfdur);
        
    /*clean up*/
    delete matroska;
    std::cout << "delete out buff\n";
    delete[] video_out_buff;
    delete[] audio_out_buff;
    std::cout << "delete encoder buff\n";
    delete encoder;
    std::cout << "delete aud buff\n";
    delete aud_buf;
    capturing_video = false;
}


GVVideoRender::GVVideoRender(
    libgvideo::GVDevice* _dev,
    libgvideo::GVBuffer* _buf,
    libgvideo::GVFps _fps
    )
{
    dev = _dev;
    buf = _buf;
    fps = _fps;
 
    framebuf = new libgvideo::VidBuff;

    quit = false;
    capture_image = false;
    capturing_video = false;
    pix_order = -1;
}

void GVVideoRender::set_cap_video()
{
    capturing_video = true;
}

void GVVideoRender::unset_cap_video()
{
    capturing_video = false;
}

void GVVideoRender::cap_image(std::string _image_filename)
{
    image_filename = _image_filename;
    capture_image = true;
}

void GVVideoRender::run()
{
    UINT32 key;
    int framecount = 0;
    UINT64 timestamp = 0;
    
    int format = dev->get_format();
    int width = dev->get_width();
    int height = dev->get_height();
    std::cout << "format is " << format << " " << width << "x" << height << std::endl;
    
    libgvrender::GVSdlRender * video = new libgvrender::GVSdlRender(width, height);
    std::cout << "on video window press:\n  Q to exit\n";
    std::cout << "  C to capture jpeg image\n";
    dev->stream_on();
    
    while( !( quit || video->quit_event() ) )
    {
        if(buf->produce_nextFrame(pix_order, framebuf) < 0)
        {  
         // don't render
        }
        else
        {
            video->render(framebuf->yuv_frame);
            if((framebuf->time_stamp - timestamp) > 2 * GV_NSEC_PER_SEC)
            {
                float frate = (dev->get_framecount() - framecount) / 2;
                std::ostringstream s;
                s << "Video fps:" << frate;
                video->setCaption(s.str());
                framecount= dev->get_framecount();
                timestamp = framebuf->time_stamp;
            }
            if (capture_image)
            {
                libgvideo::GVImage *img = new libgvideo::GVImage;
                UINT32 size = width*height/2;
                UINT8* jpg_buff = new UINT8 [size];
                libgvencoder::GVJpeg* jpeg = new libgvencoder::GVJpeg(width, height);
                size = jpeg->encode (framebuf->yuv_frame, jpg_buff, true);
                img->save_buffer(image_filename.c_str(), jpg_buff, size);
                delete jpeg;
                delete[] jpg_buff;
                delete img;
                capture_image = false;
            }
            if (!capturing_video)
                buf->consume_nextFrame(framebuf);
        }
        
        video->poll_events();
        while(!video->key_events_empty())
        {
            key = video->get_pkey();
            switch(key)
            {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_q:
                    quit = true;
                    break;
                case SDLK_c:
                    capture_image = true;
                    break;
                default:
                    std::cout << "Key nº" << key << " ('" << video->get_key_name(key) << "') pressed\n";
                    break;
            }
        }
    }
    delete video;
    delete framebuf;
}

GVVideoThreads::GVVideoThreads(libgvideo::GVDevice* _dev)
{
    int err = 0;
    dev = _dev;
    //set a 30 frame buffer
    buf = new libgvideo::GVBuffer(dev, 30);
    
    dev->get_fps(&fps);

    th_video_capture = NULL;
    capturing_video = false;
    
    th_video_render = new GVVideoRender(dev, buf, fps);
    th_video_render->start();
    err = th_video_render->get_err();
    if(err)
    {
        std::cerr << "video render thread failed to start: error " << err << std::endl;
        delete th_video_render;
        th_video_render = NULL;
    }
}

GVVideoThreads::~GVVideoThreads()
{
    stop_video_capture();

    quit();

    delete buf;
}

void GVVideoThreads::quit()
{
    if(th_video_render)
    {
        th_video_render->quit = true;
        th_video_render->join();
        delete th_video_render;
        th_video_render = NULL;
    }
}

void GVVideoThreads::stop_video_capture()
{
    if(capturing_video)
    {
        th_video_capture->capture_video = false;
        th_video_capture->join();    
        delete th_video_capture;
        th_video_capture = NULL;
        
        capturing_video = false;
        
        th_video_render->unset_cap_video();
    }
}

bool GVVideoThreads::is_capturing_video()
{
    return capturing_video;
}

void GVVideoThreads::start_video_capture(libgvaudio::GVAudio* _audio, int audio_device, unsigned vcodec_index, unsigned acodec_index)
{
    int err = 0;
    capturing_video = true;
    th_video_render->set_cap_video();
   
    th_video_capture = new GVVideoCapture(dev, _audio, buf, audio_device, vcodec_index, acodec_index, fps);
    th_video_capture->start();
    err = th_video_capture->get_err();
    if(err)
    {
        std::cerr << "video capture thread failed to start: error " << err << std::endl;
        delete th_video_capture;
        th_video_capture = NULL;
        capturing_video = false;
        th_video_render->unset_cap_video();
    }
}

void GVVideoThreads::cap_image(std::string _image_filename)
{
   th_video_render->cap_image(_image_filename); 
}

int GVVideoThreads::set_format(std::string fourcc, int width, int height)
{
    int err = 0;
    std::cout << "removing threads\n";
    //exit current threads
    stop_video_capture();
    quit();
        
    std::cout << "setting new format\n";
    // set new format
    if(dev->set_format(fourcc, width, height) < 0)
    {
        std::cerr << "couldn't set format: " << fourcc << " (" << width << "x" << height <<")\n";
        return -1;
    }

    std::cout << "deleting buff\n";
    delete buf;
    //create new buffers
    //set a 30 frame buffer
    std::cout << "new buff\n";
    buf = new libgvideo::GVBuffer(dev, 30);

    std::cout << "restarting threads\n";
    //restart the render thread
    th_video_render = new GVVideoRender(dev, buf, fps);
    th_video_render->start();
    err = th_video_render->get_err();
    if(err)
    {
        std::cerr << "video render thread failed to start: error " << err << std::endl;
        delete th_video_render;
        th_video_render = NULL;
    }
    return err;
}

END_GVIDEOGTK_NAMESPACE
