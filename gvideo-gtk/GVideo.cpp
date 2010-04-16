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
#  GVideo V4L2 video grabber and control panel  - GTK GUI                       #
#                                                                               #
********************************************************************************/

#include <gtkmm/main.h>
#include <iostream>
#include <string.h>
#include "libgvideo/gvideo.h"
#include "libgvaudio/GVAudio.h"
#include "gv_gtk_window.h"
#include "gv_video_threads.h"

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
    //init gtk
    Gtk::Main kit(argc, argv);
    gvideogtk::GVVideoThreads* th_video = NULL;
    gvideogtk::GtkWindow* window = NULL;
    
    int i = 0;
    int j = 0;
    int k = 0;
    int framecount = 0;
    UINT64 timestamp = 0;
    string fourcc;
    int width=0, height=0;
    libgvideo::GVFps* fps = new libgvideo::GVFps;
    //gvcommon::GVString* gvstr = new gvcommon::GVString;
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
    
    bool verbose = options->opts->verbose;
    
    //get device
    libgvideo::GVDevice* dev = new libgvideo::GVDevice(options->opts->devicename, options->opts->verbose);
    libgvaudio::GVAudio* audio = new libgvaudio::GVAudio(verbose);

    int format_index = 0;
    int resolution_index = 0;
    int fps_index = 0;
    
    if(options->opts->fourcc.size() > 0)
    {
        format_index = dev->get_format_index(options->opts->fourcc);
        if (format_index < 0) 
            format_index = 0;
    }
    fourcc = dev->listVidFormats[format_index].fourcc;

    if((options->opts->width > 0) && (options->opts->height > 0))
    {
        //validate resolutions
        resolution_index = dev->get_res_index(format_index, options->opts->width, options->opts->height);
        if(resolution_index < 0) 
            resolution_index = dev->listVidFormats[format_index].listVidCap.size() - 1;
    }
    width = dev->listVidFormats[format_index].listVidCap[resolution_index].width;
    height = dev->listVidFormats[format_index].listVidCap[resolution_index].height;
    
    if(options->opts->fps > 0) 
    { 
        fps->num = 1;
        fps->denom = options->opts->fps;
        fps_index = dev->get_fps_index(format_index, resolution_index, fps);
        if(fps_index < 0) 
            fps_index = dev->listVidFormats[format_index].listVidCap[resolution_index].fps.size() - 1;
    }

    if(dev->set_format(fourcc, width, height) < 0)
    {
        cerr << "couldn't set format: " << fourcc << " (" << width << "x" << height <<")\n";
        goto finish;
    }
    else
        th_video = new gvideogtk::GVVideoThreads(dev);
    
    //main window
    window = new gvideogtk::GtkWindow( 
        dev, 
        audio,
        th_video,
        format_index, 
        resolution_index, 
        fps_index );

    //main loop
    Gtk::Main::run(*window);   

    //remove window
    delete window;
    delete th_video;

  finish: 
    if(verbose) cout << "deleting dev\n"; 
    delete dev;
    if(verbose) cout << "deleting options\n";
    delete options;
    
    if(verbose) cout << "deleting audio\n";
    delete audio;
    return (0);
}

