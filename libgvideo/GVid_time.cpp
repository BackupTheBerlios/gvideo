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

#include <sys/time.h>
#include <time.h>
#include "libgvideo/GVid_time.h"

/*------------------------------ get time ------------------------------------*/

/*REAL TIME CLOCK*/
/*in nanoseconds*/
UINT64 GVTime::ns_time ()
{
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((UINT64) ts.tv_sec * G_NSEC_PER_SEC + (UINT64) ts.tv_nsec);
}

/*MONOTONIC CLOCK*/
/*in nanosec*/
UINT64 GVTime::ns_time_monotonic()
{
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((UINT64) ts.tv_sec * G_NSEC_PER_SEC + (UINT64) ts.tv_nsec);
}

/*REAL TIME CLOCK*/
/*in miliseconds*/
UINT64 GVTime::ms_time ()
{
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((UINT64) ts.tv_sec * G_MSEC_PER_SEC + (UINT64) (ts.tv_nsec / G_USEC_PER_SEC));
}

/*MONOTONIC CLOCK*/
/*in miliseconds*/
UINT64 GVTime::ms_time_monotonic()
{
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((UINT64) ts.tv_sec * G_MSEC_PER_SEC + (UINT64) (ts.tv_nsec / G_USEC_PER_SEC));
}
