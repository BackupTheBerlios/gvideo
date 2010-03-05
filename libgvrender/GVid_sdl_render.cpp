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
#  GVideo SDL video render                                                      #
#                                                                               #
********************************************************************************/

#include <iostream>
#include <cstdlib>
#include "libgvrender/GVid_sdl_render.h"

using namespace std;

//constructor
GVSdlRender::GVSdlRender(int _width, int _height, int _bpp /* = 0 */, bool _hard_surf /* = true */)
{
    width = _width;
    height = _height;
    bpp = _bpp;
    hard_surf = _hard_surf;
    caption = "Video preview";
    screen = NULL;
    overlay = NULL;
    _quit_event = false;
    video_init();
}

//destructor
GVSdlRender::~GVSdlRender()
{
    SDL_FreeYUVOverlay(overlay);
    SDL_Quit();
}

/* 
 * video_init: 
 * args: SDL_Surface pointer address 
 * returns: SDL_Overlay pointer on success
 *          NULL on error
 */
int GVSdlRender::video_init()
{
    static UINT32 SDL_VIDEO_Flags =
        SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE;

    if (screen == NULL) //init SDL
    {
        const SDL_VideoInfo *info;
        char driver[128];
        /*----------------------------- Test SDL capabilities ---------------------*/
        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) 
        {
            cerr << "Couldn't initialize SDL:" << SDL_GetError() << endl;
            return (-1);
        }
        if (hard_surf)
        {   //hardware surfaces
            if ( ! getenv("SDL_VIDEO_YUV_HWACCEL") ) putenv(strdup("SDL_VIDEO_YUV_HWACCEL=1"));
            if ( ! getenv("SDL_VIDEO_YUV_DIRECT") ) putenv(strdup("SDL_VIDEO_YUV_DIRECT=1"));
        }
        else
        {   //software surfaces
            if ( ! getenv("SDL_VIDEO_YUV_HWACCEL") ) putenv(strdup("SDL_VIDEO_YUV_HWACCEL=0"));
            if ( ! getenv("SDL_VIDEO_YUV_DIRECT") ) putenv(strdup("SDL_VIDEO_YUV_DIRECT=0"));
        }
 
        if (SDL_VideoDriverName(driver, sizeof(driver))) 
        {
            cout << "Video driver:" << driver;
        }

        info = SDL_GetVideoInfo();

        if (info->wm_available) cout << "A window manager is available\n";

        if (info->hw_available) 
        {
            cout << "Hardware surfaces are available (" << info->video_mem <<"K video memory)\n";
            SDL_VIDEO_Flags |= SDL_HWSURFACE;
        }
        if (info->blit_hw) 
        {
            cout << "Copy blits between hardware surfaces are accelerated\n";
            SDL_VIDEO_Flags |= SDL_ASYNCBLIT;
        }

        if (info->blit_hw_CC) cout << "Colorkey blits between hardware surfaces are accelerated\n";
        if (info->blit_hw_A) cout << "Alpha blits between hardware surfaces are accelerated\n";
        if (info->blit_sw) cout << "Copy blits from software surfaces to hardware surfaces are accelerated\n";
        if (info->blit_sw_CC) cout << "Colorkey blits from software surfaces to hardware surfaces are accelerated\n";
        if (info->blit_sw_A) cout << "Alpha blits from software surfaces to hardware surfaces are accelerated\n";
        if (info->blit_fill) cout << "Color fills on hardware surfaces are accelerated\n";


        if (!(SDL_VIDEO_Flags & SDL_HWSURFACE))
        {
            SDL_VIDEO_Flags |= SDL_SWSURFACE;
        }

        SDL_WM_SetCaption(caption.c_str(), NULL); 

        /* enable key repeat */
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
    }
    /*------------------------------ SDL init video ---------------------*/

    screen = SDL_SetVideoMode( width, 
        height, 
        bpp,
        SDL_VIDEO_Flags);

    overlay = SDL_CreateYUVOverlay(width, height,
        SDL_YUY2_OVERLAY, screen);

    return (0);
}

void GVSdlRender::setCaption(string _caption)
{
    caption.assign(_caption);
    SDL_WM_SetCaption(caption.c_str(), NULL); 
}

int GVSdlRender::render(UINT8 *data)
{
    drect.x = 0;
    drect.y = 0;
    drect.w = screen->w;
    drect.h = screen->h;
    
    UINT8 *p = (UINT8 *) overlay->pixels[0];
    SDL_LockYUVOverlay(overlay);
    memcpy(p, data, width * height * 2);
    SDL_UnlockYUVOverlay(overlay);
    
    SDL_DisplayYUVOverlay(overlay, &drect);
}

int GVSdlRender::poll_events()
{
    /* Poll for events */
    while( SDL_PollEvent(&event) )
    {
        if(event.type==SDL_KEYDOWN) // key down event
        {
            KeyEvents.push(UINT32(event.key.keysym.sym));
        }
        if(event.type==SDL_QUIT)
            _quit_event = true;
    }
    return (0);
}

int GVSdlRender::get_nk_events()
{
    return KeyEvents.size();
}

bool GVSdlRender::key_events_empty()
{
    return KeyEvents.empty();
}

UINT32 GVSdlRender::get_pkey()
{
    UINT32 _key_pressed_ = KeyEvents.front(); // check first key in queue
    KeyEvents.pop(); // remove key from event queue
    return (_key_pressed_);
}

char *GVSdlRender::get_key_name(UINT32 key)
{
    return( SDL_GetKeyName(SDLKey(key)));
}

bool GVSdlRender::quit_event()
{
    return _quit_event;
}

