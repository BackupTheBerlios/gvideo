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


#ifndef GVID_SDL_RENDER_H
#define GVID_SDL_RENDER_H

#include <SDL/SDL.h>
#include <string>
#include <queue>
#include "common/gvcommon.h"

/*
 * for key events check SDL key list at:
 *    http://www.libsdl.org/cgi/docwiki.cgi/SDLKey
 */

class GVSdlRender
{
    int width, height, bpp;
    bool hard_surf; // use hardware surfaces: default true
    std::string caption;
    SDL_Surface *screen;
    SDL_Overlay *overlay;
    SDL_Event event;
    SDL_Rect drect;
    std::queue<UINT32> KeyEvents; //queue of pressed SDL keys
    bool _quit_event;
    int video_init();
    
    
  public:
    GVSdlRender(int width, int height, int bpp = 0, bool _hard_surf = true);
    ~GVSdlRender();
    int render(UINT8 *data); // render frame data
    // SDL Events interface
    int poll_events(); //get events
    int get_nk_events(); // get number of key events in KeyEvents
    bool key_events_empty(); // true if KeyEvents queue is empty
    UINT32 get_pkey(); // get a pressed key from KeyEvents
    char * get_key_name(UINT32 key); // get the key name corresponding to key
    bool quit_event();
    void setCaption(std::string _caption);
};

#endif

