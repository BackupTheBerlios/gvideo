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
#  GVideo V4L2 video grabber and control panel  - command line                  #
#                                                                               #
********************************************************************************/

#include <iostream>
#include <string.h>
#include "libgvideo/gvideo.h"
#include "libgvrender/render.h"
#include "libgvaudio/GVAudio.h"
#include "libgvencoder/GVJpeg_encoder.h"
#include "libgvencoder/GVCodec.h"
#include "libgvencoder/GVMatroska.h"


using namespace std;

class GVOpt : public libgvideo::GVOptions
{
    struct data
    {
        int sc, val, width, height;
        int gc;
        int fps;
        int adevice;
        int vcaptime;
        int vidcodec;
        int audcodec;
        bool lformats, verbose, lcontrols, render, laudio;
        string devicename;
        string fourcc;
        string picture;
        string videofile;
    };
    
  public:
    GVOpt( vector<libgvideo::GV_options> _options ) :
        GVOptions( _options )
        {
            opts = new data;
            
            opts->sc = -1;
            opts->val = 0;
            opts->width = 0;
            opts->height = 0;
            opts->fps = 0;
            opts->gc = -1;
            opts->adevice = -1;
            opts->vcaptime = 5; //in seconds
            opts->vidcodec = 0; //MJPG
            opts->audcodec = 0; //PCM
            opts->lformats = false;
            opts->verbose = false;
            opts->lcontrols = false;
            opts->render = false;
            opts->laudio = false;  
        };
    ~GVOpt()
    {
        delete opts;
    }

    data* opts;
    int onOption (int option, char* optarg)
    {
        switch(option)
        {
            case 'd':   /* -d or --device */
                opts->devicename = string(optarg);
                break;
            case 'l':   /* -l or --lformats */
                opts->lformats = true;
                break;
             case 'q':   /* -q or --laudio */
                opts->laudio = true;
                break;
            case 'c':   /* -c or --lcontrols */
                opts->lcontrols = true;
                break;
            case 'v':   /* -v or --verbose */
                opts->verbose = true;
                break;
            case 'j':  /* -j or --render */
                opts->render = true;
                break;
            case 'p':   /* -p or --picture */
                opts->picture = string(optarg);
                break;
            case 'e':   /* -e or --video */
                opts->videofile = string(optarg);
                break;
            case 't':   /* -t or --captime */
            {
                if (!gvcommon::from_string<int>(opts->vcaptime, string(optarg), dec))
                {
                    cerr << "couldn't convert index " << optarg << " to int value\n";
                    opts->vcaptime = 5;
                }
            }   
                break;
            case 'o':   /* -o or --vidcodec */
            {
                if (!gvcommon::from_string<int>(opts->vidcodec, string(optarg), dec))
                {
                    cerr << "couldn't convert video codec index " << optarg << " to int value\n";
                    opts->vidcodec = 0;
                }
            }   
                break;
            case 'n':   /* -n or --audcodec */
            {
                if (!gvcommon::from_string<int>(opts->audcodec, string(optarg), dec))
                {
                    cerr << "couldn't convert audio codec index " << optarg << " to int value\n";
                    opts->audcodec = 0;
                }
            }   
                break;
            case 'f':   /* -f or --format */
                opts->fourcc = string(optarg);
                break;
            case 'i': /* -i or --fps */
            {
                if (!gvcommon::from_string<int>(opts->fps, string(optarg), dec))
                {
                    cerr << "couldn't convert width " << optarg << " to int value\n";
                    opts->fps = 0;
                } 
            }
                break;
            case 'r': /* -r or --resolution */
            {
                string res = string(optarg);
                int token_pos = res.find("x");
                string wid = res.substr(0, token_pos);
                string hei = res.substr(token_pos+1, res.size());
                if (!gvcommon::from_string<int>(opts->width, wid, dec))
                {
                    cerr << "couldn't convert width " << wid << " to int value\n";
                    opts->width = 0;
                }
                if (!gvcommon::from_string<int>(opts->height, hei, dec))
                {
                    cerr << "couldn't convert height " << hei << " to int value\n";
                    opts->height = 0;
                }
                
            }
                break;
            case 's':   /* -s or --set */
            {
                string par = string(optarg);
                int token_pos = par.find("=");
                string ind = par.substr(0, token_pos);
                string cval = par.substr(token_pos+1, par.size()); 
                if (!gvcommon::from_string<int>(opts->sc, ind, dec))
                {
                    cerr << "couldn't convert index " << ind << " to int value\n";
                    opts->sc = -1;
                }
                if (!gvcommon::from_string<int>(opts->val, cval, dec))
                {
                    cerr << "couldn't convert val " << cval << " to int value\n";
                    opts->val = 0;
                }
            }
                break;
            case 'g':   /* -g or --get */
                
                if (!gvcommon::from_string<int>(opts->gc, string(optarg), dec))
                {
                    cerr << "couldn't convert index " << optarg << " to int value\n";
                    opts->gc = -1;
                }    
                break;
            case 'a':   /* -a or --adevice */
                
                if (!gvcommon::from_string<int>(opts->adevice, string(optarg), dec))
                {
                    cerr << "couldn't convert index " << optarg << " to int value\n";
                    opts->adevice = -1;
                }    
                break;
            case '?':   /* The user specified an invalid option.  */
                /* Print usage information to standard error, and exit with exit
                code one (indicating abnormal termination).  */
                cerr << "unknown option: " << option << endl;
                return (-1);
                break;
            default:
                /* Something else: unexpected.  */
                cerr << "unexpected command option:" << option << endl;
                return (-1);
        }
        return 0;
    }
};

int main (int argc, char *argv[])
{
    int i = 0;
    int j = 0;
    int k = 0;
    int framecount = 0;
    UINT64 timestamp = 0;
    string fourcc;
    int width, height;
    libgvideo::GVFps* fps = new libgvideo::GVFps;
    gvcommon::GVString* gvstr = new gvcommon::GVString;
    //parse command line options
    vector<libgvideo::GV_options> opts;
    
    opts.push_back((libgvideo::GV_options){"device",'d', true, "DEVICENAME", "sets device to DEVICENAME"});
    opts.push_back((libgvideo::GV_options){"adevice",'a', true, "INDEX", "sets audio device to INDEX"});
    opts.push_back((libgvideo::GV_options){"format",'f', true, "FOURCC", "sets video format to FOURCC if supported"});
    opts.push_back((libgvideo::GV_options){"resolution",'r', true, "WIDTHxHEIGHT", "sets video resolution"});
    opts.push_back((libgvideo::GV_options){"fps",'i', true, "FPS", "sets frame interval"});
    opts.push_back((libgvideo::GV_options){"lformats",'l', false, "", "lists supported video formats"});
    opts.push_back((libgvideo::GV_options){"lcontrols",'c', false, "", "lists device controls"});
    opts.push_back((libgvideo::GV_options){"set",'s', true, "INDEX=VAL", "sets control with list INDEX to VAL"});
    opts.push_back((libgvideo::GV_options){"get",'g', true, "INDEX", "gets value from control with list INDEX"});
    opts.push_back((libgvideo::GV_options){"picture",'p', true, "FILENAME", "grab and save frame picure"});
    opts.push_back((libgvideo::GV_options){"video",'e', true, "FILENAME", "capture video to file"});
    opts.push_back((libgvideo::GV_options){"captime",'t', true, "TIME_IN_SEC", "capture time in seconds"});
    opts.push_back((libgvideo::GV_options){"vidcodec",'o', true, "CODEC_INDEX", "set video codec for encoding"});
    opts.push_back((libgvideo::GV_options){"audcodec",'n', true, "CODEC_INDEX", "set audio codec for encoding"});
    opts.push_back((libgvideo::GV_options){"render",'j', false, "", "grab and render frames"});
    opts.push_back((libgvideo::GV_options){"laudio",'q', false, "", "lists input audio devices"});
    opts.push_back((libgvideo::GV_options){"verbose",'v', false, "", "prints a lot of debug messages"});
    
    GVOpt *options = new GVOpt(opts);
    options->parse(argc, argv);
    if(options->isHelp())
    {
        delete options;
        return (0);
    }
    
    //get device
    libgvideo::GVDevice *dev = new libgvideo::GVDevice(options->opts->devicename, options->opts->verbose);
    libgvideo::GVBuffer *buf = NULL;
    libgvideo::VidBuff *framebuf = NULL;

    libgvaudio::GVAudio *audio = NULL;
    bool verbose = options->opts->verbose;
    //list formats
    if(options->opts->lformats)
    {
        cout << dev->listVidFormats.size() << " supported video formats detected:\n";
        for(i=0; i<dev->listVidFormats.size(); i++)
        {
            cout << "  { fourcc='"<< dev->listVidFormats[i].fourcc 
                << "'  description='" << dev->listVidFormats[i].description << "' }\n";
            for(j=0;j<dev->listVidFormats[i].listVidCap.size();j++)
            {
                cout << "    { discrete: width=" << dev->listVidFormats[i].listVidCap[j].width
                    << "  height=" << dev->listVidFormats[i].listVidCap[j].height << " }\n";
                cout << "      Time interval between frame:";
                for(k=0;k<dev->listVidFormats[i].listVidCap[j].fps.size();k++)
                {
                    cout << " " << dev->listVidFormats[i].listVidCap[j].fps[k].num << "/"
                        <<  dev->listVidFormats[i].listVidCap[j].fps[k].denom;
                }
                cout << endl;
            }
        }
    }
    //list controls
    if(options->opts->lcontrols)
    {
        cout << dev->listControls.size() << " controls detected:\n";
        
        for(i=0; i<dev->listControls.size(); i++)
        {
            cout << "  [" << i << "] " << dev->listControls[i].control.name 
                << " { min=" << dev->listControls[i].control.minimum 
                << " max=" << dev->listControls[i].control.maximum 
                << " step=" << dev->listControls[i].control.step 
                << " default="<< dev->listControls[i].control.default_value <<" }\n";
            if (dev->listControls[i].control.type == V4L2_CTRL_TYPE_MENU)
            {
                cout << "    menu:\n";
                for(j=0; j<dev->listControls[i].entries.size();j++)
                    cout << "    ---> (" << j << ") " << dev->listControls[i].entries[j] << endl;
            }
        }
        goto finish;
    }
    
     //list audio devices
    if(options->opts->laudio)
    {
        audio = new libgvaudio::GVAudio(verbose);
        cout << audio->listAudioDev.size() << " input audio devices detected:\n";
        
        for(i=0; i< audio->listAudioDev.size(); i++)
        {
            cout << "device -" << i << " (id[" << audio->listAudioDev[i].id << "]) :\n";
            cout << "      name    : " << audio->listAudioDev[i].name << endl;
            cout << "      channels: " << audio->listAudioDev[i].channels << endl;
            cout << "      samprate: " << audio->listAudioDev[i].samprate << endl;
            cout << "      Llatency: " << audio->listAudioDev[i].low_input_latency << endl;
            cout << "      Hlatency: " << audio->listAudioDev[i].high_input_latency << endl;   
        }   
    }
    
    //set control
    if(options->opts->sc >= 0)
    {
        if(dev->listControls.size() > options->opts->sc)
        {
            dev->listControls[options->opts->sc].value = options->opts->val;
            if(!(dev->set_control_val (options->opts->sc)))
                cout << dev->listControls[options->opts->sc].control.name 
                    << " =" << options->opts->val << endl;
        }
        else
            cerr << "invalid control index\n";
    }
    //get control value
    if(options->opts->gc >= 0)
    {
        if(dev->listControls.size() > options->opts->sc)
        {
            dev->get_control_val (options->opts->gc);
            cout << dev->listControls[options->opts->gc].control.name 
                << " =" << dev->listControls[options->opts->gc].value << endl;
        }
         else
            cerr << "invalid control index\n";
    }
    
    if(options->opts->fourcc.size() > 0)
        fourcc = options->opts->fourcc;
    else fourcc = dev->listVidFormats[0].fourcc;

    if(options->opts->width > 0)
        width = options->opts->width;
    else 
    {
        int fi = dev->get_format_index(fourcc);
        if (fi < 0) fi = 0;
        width = dev->listVidFormats[fi].listVidCap[0].width; // force a valid default
    }
    if(verbose) cout << "using width=" << width << endl;
    
    if(options->opts->height > 0)
        height = options->opts->height;
    else 
    {
        int fi = dev->get_format_index(fourcc);
        if (fi < 0) fi = 0;
        height = dev->listVidFormats[fi].listVidCap[0].height; // force a valid default
    }
    if (verbose) cout << "using height=" << height << endl;
    
    if(options->opts->fps > 0) 
    { 
        fps->num = 1;
        fps->denom = options->opts->fps;
        if (verbose) cout << "trying to set fps to " << fps->num << "/" << fps->denom << endl;
        dev->set_fps(fps);
        if(!fps->num || !fps->denom)
        {
            cerr << "set fps failed: using default value 1/15 \n";
            fps->num = 1;
            fps->denom = 15;
        }
    }
    else
    {
        /* use default fps */
        if (verbose) cout << "trying to set fps to " << fps->num << "/" << fps->denom << endl;
        dev->set_fps(fps);
    }
    
    if(options->opts->picture.size() > 0)
    {
        if(dev->set_format(fourcc, width, height) < 0)
            goto finish;
        int format = dev->get_format();
        width = dev->get_width();
        height = dev->get_height();
        if(verbose) cout << "format is " << format << " " << width << "x" << height << endl;
        
        buf = new libgvideo::GVBuffer(dev, 1);
        framebuf = new libgvideo::VidBuff;
        
        dev->stream_on();
        
        int ntries = 0;
        
        while( (buf->produce_nextFrame() < 0) && (ntries < 10))
        {
            ntries++;
        }
        
        buf->consume_nextFrame(framebuf);
        
        string filename = gvstr->get_Filename(options->opts->picture);
        string ext = gvstr->get_Extension(filename);
        ext = gvstr->ToLower(ext);
        
        libgvideo::GVImage *img = new libgvideo::GVImage;

        if(ext.compare("bmp") == 0)
        {
            libgvideo::GVConvert *conv = new libgvideo::GVConvert;
            UINT8 *rgb_buff = new UINT8 [width*height*3];
            conv->yuyv2bgr (framebuf->yuv_frame, rgb_buff, width, height);
            img->save_BPM(options->opts->picture.c_str(), width, height, rgb_buff);
            delete conv;
            delete[] rgb_buff;
        }
        else if (ext.compare("jpg") == 0)
        {
            UINT32 size = width*height/2;
            UINT8* jpg_buff = new UINT8 [size];
            libgvencoder::GVJpeg* jpeg = new libgvencoder::GVJpeg(width, height);
            size = jpeg->encode (framebuf->yuv_frame, jpg_buff, true);
            img->save_buffer(options->opts->picture.c_str(), jpg_buff, size);
            delete[] jpg_buff;
            delete jpeg;
        }
        else
        {
            cerr << "Image " << filename << ", type:" << ext << " not supported (jpg, bmp only)\n";
        }
        
        delete img;
    }
    if(options->opts->videofile.size() > 0)
    {
        bool quit = false;
        unsigned vcodec_ind = options->opts->vidcodec;
        unsigned acodec_ind = options->opts->audcodec;
        
        if(dev->set_format(fourcc, width, height) < 0)
            goto finish;
        int format = dev->get_format();
        width = dev->get_width();
        height = dev->get_height();
        if(verbose) cout << "format is " << format << " " << width << "x" << height << endl;
        
        buf = new libgvideo::GVBuffer(dev, 1);
        framebuf = new libgvideo::VidBuff;
        
        /*get a new audio instance*/
        if(!audio)
            audio = new libgvaudio::GVAudio(verbose);
        /*get a new audio buffer*/
        libgvaudio::AudBuff *aud_buf = new libgvaudio::AudBuff;
        /*get a new encoder instance*/
        libgvencoder::GVCodec*  encoder= new libgvencoder::GVCodec();
        
        /*get audio parameters*/
        int adev = audio->setDevice(options->opts->adevice);
        if(verbose) cout << "using audio device id:" << adev << endl;
        int channels = audio->listAudioDev[adev].channels;
        int samprate = audio->listAudioDev[adev].samprate;
        UINT8* audio_out_buff = NULL;
        int asize = 0;
        if(acodec_ind > 0) 
        {
            asize = 240000;
            audio_out_buff = new UINT8 [asize];
            cout << "outbuf:" << long(audio_out_buff) << "\n";
        }
        int frame_size = encoder->open_acodec(acodec_ind, 
                                              audio_out_buff, 
                                              asize, 
                                              samprate,
                                              channels);
        if (frame_size <= 0)
        {
            cerr << "couldn't audio open codec (ind=" << acodec_ind << ")\n";
            /*clean up*/
            delete[] audio_out_buff;
            delete encoder;
            delete aud_buf;
            goto finish;
        }
        
        if(verbose) cout << "audio: samprate=" << samprate << " channels=" 
            << channels << " frame size:" << frame_size << endl;
        
        /*audio frame duration*/
        UINT64 afdur = (UINT64) (1000*frame_size)/samprate;
        afdur = afdur * 1000000;
        /*video frame duration (use the set fps (25 default))*/
        UINT64 vfdur = 1 * 1e9/fps->denom;
        
        UINT32 size = width*height*2; /*enough for yuy2 (worst case)*/
        UINT8* video_out_buff = new UINT8 [size];
        
        if(!(encoder->open_vcodec(vcodec_ind, video_out_buff, size, width, height, fps->denom, fps->num)))
        {
            cerr << "couldn't open codec (ind=" << vcodec_ind << ")\n";
            /*clean up*/
            delete[] video_out_buff;
            delete[] audio_out_buff;
            delete encoder;
            delete aud_buf;
            goto finish;
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
                                                options->opts->videofile.c_str(),
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
        dev->stream_on();
        
        bool try_next = true;
        int count = 0;
        UINT64 timestamp_f0 = 0;
        UINT64 vts = 0;
        UINT64 ats = 0;
        /*try to get the first video frame and use it's timestamp as reference*/
        /* so that we start at timestamp=0*/
        while ((buf->produce_nextFrame() < 0) && (count < 15))
        {
            count++;
            cout << "loop count: " << count << endl;
        }
        
        /*get video start time to sync with audio start time*/
        gvcommon::GVTime *timer = new gvcommon::GVTime;
        UINT64 vid_start_time = timer->ns_time_monotonic();
        delete timer; 
        
        if(verbose) cout << "vid start=" << vid_start_time << endl;
        if(count < 15)
        {
            buf->consume_nextFrame(framebuf);
            timestamp_f0 = framebuf->time_stamp;
            vts = 0;
            
            size = encoder->encode_video_frame (framebuf->yuv_frame);
            matroska->add_VideoFrame(video_out_buff, size, vts);
        }
        else
        {
            /*clean up*/
            delete matroska;
            delete[] video_out_buff;
            delete encoder;
            delete aud_buf;
            goto finish;
        }
        
        if(verbose) cout << "ref ts=" << timestamp_f0 << endl;
        /*start audio stream*/
        audio->startStream(samprate, channels, frame_size);

        while( !quit )
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
            if (buf->produce_nextFrame() >= 0);
            {
                buf->consume_nextFrame(framebuf);
                vts = framebuf->time_stamp - timestamp_f0;
                //if (verbose) cout << "vts:" << vts << " max:" << (UINT64) options->opts->vcaptime * GV_NSEC_PER_SEC << endl;
                if( vts > ((UINT64) options->opts->vcaptime * GV_NSEC_PER_SEC))
                {
                    /*we already captured the defined amount of time so let's finish*/
                    quit = true;
                }
                size = encoder->encode_video_frame (framebuf->yuv_frame);
                matroska->add_VideoFrame(video_out_buff, size, vts);
            }
        }
        /* stop the audio stream*/
        audio->stopStream();
        
        /*calc and set the default video frame duration (fps)*/
        vfdur = vts / dev->get_framecount();
        matroska->set_DefDuration(vfdur);
        
        /*clean up*/
        delete matroska;
        cout << "delete out buff\n";
        delete[] video_out_buff;
        delete[] audio_out_buff;
        cout << "delete encoder buff\n";
        delete encoder;
        cout << "delete aud buff\n";
        delete aud_buf;
    }
    
    if(options->opts->render)
    {
        bool quit = false;
        bool cap_file = false;
        UINT32 key;
        if(dev->set_format(fourcc, width, height) < 0)
            goto finish;
        int format = dev->get_format();
        width = dev->get_width();
        height = dev->get_height();
        if(verbose) cout << "format is " << format << " " << width << "x" << height << endl;
        
        buf = new libgvideo::GVBuffer(dev, 1);
        framebuf = new libgvideo::VidBuff;
        libgvrender::GVSdlRender * video = new libgvrender::GVSdlRender(width, height);
        cout << "on video window press:\n  Q to exit\n";
        cout << "  C to capture jpeg image\n";
        dev->stream_on();
        while( !( quit || video->quit_event() ) )
        {
            buf->produce_nextFrame();
            buf->consume_nextFrame(framebuf);
            video->render(framebuf->yuv_frame);
            if((framebuf->time_stamp - timestamp) > 2 * GV_NSEC_PER_SEC)
            {
                float frate = (dev->get_framecount() - framecount) / 2;
                ostringstream s;
                s << "Video fps:" << frate;

                video->setCaption(s.str());
                framecount= dev->get_framecount();
                timestamp = framebuf->time_stamp;
            }
            if (cap_file)
            {
                libgvideo::GVImage *img = new libgvideo::GVImage;
                UINT32 size = width*height/2;
                UINT8* jpg_buff = new UINT8 [size];
                libgvencoder::GVJpeg* jpeg = new libgvencoder::GVJpeg(width, height);
                size = jpeg->encode (framebuf->yuv_frame, jpg_buff, true);
                img->save_buffer("gvideo_capture.jpg", jpg_buff, size);
                delete jpeg;
                delete[] jpg_buff;
                delete img;
                cap_file = false;
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
                        cap_file = true;
                        break;
                    default:
                        cout << "Key nº" << key << " ('" << video->get_key_name(key) << "') pressed\n";
                        break;
                }
            }
        }
        delete video;
    }
    
    
    
  finish: 
    if(verbose) cout << "deleting dev\n"; 
    delete dev;
    if(verbose) cout << "deleting options\n";
    delete options;
    if(verbose) cout << "deleting buf\n";
    delete framebuf;
    delete buf;
    if(verbose) cout << "deleting audio\n";
    delete audio;
    return (0);
}
