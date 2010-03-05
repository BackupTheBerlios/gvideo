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
#  GVTime  class : gvideo time interface                                        #
#                                                                               #
********************************************************************************/

#ifndef GVID_TIME_H
#define GVID_TIME_H

#include "gvcommon.h"

#ifndef G_NSEC_PER_SEC
#define G_NSEC_PER_SEC 1000000000
#endif

#ifndef G_USEC_PER_SEC
#define G_USEC_PER_SEC 1000000
#endif

#ifndef G_MSEC_PER_SEC
#define G_MSEC_PER_SEC 1000
#endif

class GVTime
{
    struct timespec ts;
  public:
    UINT64 ns_time ();
    UINT64 ns_time_monotonic();
    UINT64 ms_time ();
    UINT64 ms_time_monotonic();

};

#endif

