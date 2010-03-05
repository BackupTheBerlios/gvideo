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
#  GVideo V4L2 video grabber and control panel                                  #
#                                                                               #
********************************************************************************/

#include <iostream>
#include <string.h>
#include "libgvideo/gvideo.h"
#include "libgvrender/render.h"


using namespace std;

class GVOpt : public GVOptions
{
    struct data
    {
        int sc, val, width, height;
        int gc;
        int fps;
        bool list, verbose, lcontrols, render;
        string devicename;
        string fourcc;
        string picture; 
    };
    
  public:
    GVOpt( vector<GV_options> _options ) :
        GVOptions( _options )
        {
            opts = new data;
            
            opts->sc = -1;
            opts->val = 0;
            opts->width = 0;
            opts->height = 0;
            opts->fps = 0;
            opts->gc = -1;
            opts->list = false;
            opts->verbose = false;
            opts->lcontrols = false;
            opts->render = false;  
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
            case 'l':   /* -l or --list */
                /* This option takes an argument, the name of the output file.  */
                opts->list = true;
                break;
            case 'c':   /* -c or --controls */
                opts->lcontrols = true;
                break;
            case 'v':   /* -v or --verbose */
                opts->verbose = true;
                break;
            case 'j':  /* -j or --render */
                opts->render = true;
                break;
            case 'p':   /* -f or --format */
                opts->picture = string(optarg);
                break;
            case 'f':   /* -f or --format */
                opts->fourcc = string(optarg);
                break;
            case 'i': /* -i or --fps */
            {
                string frt = string(optarg);
                if (!from_string<int>(opts->fps, frt, dec))
                {
                    cerr << "couldn't convert width " << frt << " to int value\n";
                    opts->fps = 0;
                } 
            }
                break;
            case 'r':
            {
                string res = string(optarg);
                int token_pos = res.find("x");
                string wid = res.substr(0, token_pos);
                string hei = res.substr(token_pos+1, res.size());
                if (!from_string<int>(opts->width, wid, dec))
                {
                    cerr << "couldn't convert width " << wid << " to int value\n";
                    opts->width = 0;
                }
                if (!from_string<int>(opts->height, hei, dec))
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
                if (!from_string<int>(opts->sc, ind, dec))
                {
                    cerr << "couldn't convert index " << ind << " to int value\n";
                    opts->sc = -1;
                }
                if (!from_string<int>(opts->val, cval, dec))
                {
                    cerr << "couldn't convert val " << cval << " to int value\n";
                    opts->val = 0;
                }
            }
                break;
            case 'g':   /* -g or --get */
                
                if (!from_string<int>(opts->gc, string(optarg), dec))
                {
                    cerr << "couldn't convert index " << optarg << " to int value\n";
                    opts->gc = -1;
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
    Fps_s *fps;
    //parse command line options
    vector<GV_options> opts;
    
    opts.push_back((GV_options){"device",'d', true, "DEVICENAME", "sets device to DEVICENAME"});
    opts.push_back((GV_options){"format",'f', true, "FOURCC", "sets video format to FOURCC if supported"});
    opts.push_back((GV_options){"resolution",'r', true, "WIDTHxHEIGHT", "sets video resolution"});
    opts.push_back((GV_options){"fps",'i', true, "FPS", "sets frame interval"});
    opts.push_back((GV_options){"list",'l', false, "", "lists supported video formats and controls"});
    opts.push_back((GV_options){"controls",'c', false, "", "lists device controls"});
    opts.push_back((GV_options){"set",'s', true, "INDEX=VAL", "sets control with list INDEX to VAL"});
    opts.push_back((GV_options){"get",'g', true, "INDEX", "gets value from control with list INDEX"});
    opts.push_back((GV_options){"picture",'p', true, "FILENAME", "grab and save frame picure"});
    opts.push_back((GV_options){"render",'j', false, "", "grab and render frames"});
    opts.push_back((GV_options){"verbose",'v', false, "", "prints a lot of debug messages"});
    
    GVOpt *options = new GVOpt(opts);
    options->parse(argc, argv);
    if(options->isHelp())
    {
        delete options;
        return (0);
    }
    
    //get device
    GVDevice *dev = new GVDevice(options->opts->devicename, options->opts->verbose);
    GVBuffer *buf = NULL;

    //list formats
    if(options->opts->list)
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
    if(options->opts->list || options->opts->lcontrols)
    {
        cout << dev->listControls.size() << " controls detected:\n";
        
        for(i=0; i<dev->listControls.size(); i++)
        {
            cout << "  [" << i << "] " << dev->listControls[i].name 
                << " { min=" << dev->listControls[i].min 
                << " max=" << dev->listControls[i].max 
                << " step=" << dev->listControls[i].step << " }\n";
            if (dev->listControls[i].type == V4L2_CTRL_TYPE_MENU)
            {
                cout << "    menu:\n";
                for(j=0; j<dev->listControls[i].entries.size();j++)
                    cout << "    ---> (" << j << ") " << dev->listControls[i].entries[j] << endl;
            }
        }
        goto finish;
    }
    
    //set control
    if(options->opts->sc >= 0)
    {
        if(!(dev->set_control_val (options->opts->sc, options->opts->val)))
            cout << dev->listControls[options->opts->sc].name << " =" << options->opts->val << endl;
    }
    //get control value
    if(options->opts->gc >= 0)
    {
        int val;
        dev->get_control_val (options->opts->gc, &val);
        cout << dev->listControls[options->opts->gc].name << " =" << val << endl;
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
    if(options->opts->verbose) cout << "using width=" << width << endl;
    
    if(options->opts->height > 0)
        height = options->opts->height;
    else 
    {
        int fi = dev->get_format_index(fourcc);
        if (fi < 0) fi = 0;
        height = dev->listVidFormats[fi].listVidCap[0].height; // force a valid default
    }
    if (options->opts->verbose) cout << "using height=" << height << endl;
    
    if(options->opts->fps > 0) 
    { 
        fps = dev->get_framerate();
        fps->denom = options->opts->fps;
        dev->setFPS=true;
    }
    
    if(options->opts->picture.size() > 0)
    {
        if(dev->set_format(fourcc, width, height) < 0)
            goto finish;
        int format = dev->get_format();
        width = dev->get_width();
        height = dev->get_height();
        if(options->opts->verbose) cout << "format is " << format << " " << width << "x" << height << endl;
        buf = new GVBuffer(format, width, height);
        dev->stream_on();
        dev->grab_frame(buf->raw_data);
        buf->frame_decode(dev->get_bytesused());
        GVImage *img = new GVImage;
        GVConvert *conv = new GVConvert;
        UINT8 *rgb_buff = new UINT8 [width*height*3];
        conv->yuyv2bgr (buf->frame_data, rgb_buff, width, height);
        img->save_BPM(options->opts->picture.c_str(), width, height, 24, rgb_buff); 
        
        delete img;
        delete conv;
        delete[] rgb_buff;
    }
    
    if(options->opts->render)
    {
        bool quit = false;
        UINT32 key;
        if(dev->set_format(fourcc, width, height) < 0)
            goto finish;
        int format = dev->get_format();
        width = dev->get_width();
        height = dev->get_height();
        if(options->opts->verbose) cout << "format is " << format << " " << width << "x" << height << endl;
        buf = new GVBuffer(format, width, height);
        GVSdlRender * video = new GVSdlRender(width, height);
        cout << "press Q to exit(on video window)\n";
        dev->stream_on();
        while( !( quit || video->quit_event() ) )
        {
            dev->grab_frame(buf->raw_data);
            buf->frame_decode(dev->get_bytesused());
            video->render(buf->frame_data);
            if((dev->get_timestamp() - timestamp) > 2 * G_NSEC_PER_SEC)
            {
                float frate = (dev->get_framecount() - framecount) / 2;
                ostringstream s;
                s << "Video fps:" << frate;

                video->setCaption(s.str());
                framecount= dev->get_framecount();
                timestamp = dev->get_timestamp();
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
                    default:
                        cout << "Key nº" << key << " ('" << video->get_key_name(key) << "') pressed\n";
                        break;
                }
            }
        }
        delete video;
    }
    
    
    
  finish: 
    cout << "deleting dev\n"; 
    delete dev;
    cout << "deleting options\n";
    delete options;
    cout << "deleting buf\n";
    delete buf;
    
    return (0);
}
