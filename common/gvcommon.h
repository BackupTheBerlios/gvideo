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
#  gvideo common header definitions                                             #
#                                                                               #
********************************************************************************/

#ifndef GVCOMMON_H
#define GVCOMMON_H

#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <string>
#include <sstream>


typedef int8_t     INT8;
typedef uint8_t    UINT8;
typedef int16_t    INT16;
typedef uint16_t   UINT16;
typedef int32_t    INT32;
typedef uint32_t   UINT32;
typedef int64_t    INT64;
typedef uint64_t   UINT64;

/*clip value between 0 and 255*/
#define CLIP(value) (UINT8)(((value)>0xFF)?0xff:(((value)<0)?0:(value)))

/*V4L2 related defintions*/
#ifndef V4L2_CID_CAMERA_CLASS_LAST
#define V4L2_CID_CAMERA_CLASS_LAST		(V4L2_CID_CAMERA_CLASS_BASE +20)
#endif
#define V4L2_CID_BASE_EXTCTR				0x0A046D01
#define V4L2_CID_BASE_LOGITECH				V4L2_CID_BASE_EXTCTR
#define V4L2_CID_PANTILT_RESET_LOGITECH			V4L2_CID_BASE_LOGITECH+2
//this should realy be replaced by V4L2_CID_FOCUS_ABSOLUTE in libwebcam
#define V4L2_CID_FOCUS_LOGITECH				V4L2_CID_BASE_LOGITECH+3
#define V4L2_CID_LED1_MODE_LOGITECH			V4L2_CID_BASE_LOGITECH+4
#define V4L2_CID_LED1_FREQUENCY_LOGITECH		V4L2_CID_BASE_LOGITECH+5
#define V4L2_CID_DISABLE_PROCESSING_LOGITECH		V4L2_CID_BASE_LOGITECH+0x70
#define V4L2_CID_RAW_BITS_PER_PIXEL_LOGITECH		V4L2_CID_BASE_LOGITECH+0x71
#define V4L2_CID_LAST_EXTCTR				V4L2_CID_RAW_BITS_PER_PIXEL_LOGITECH

#ifndef V4L2_PIX_FMT_SRGGB8
#define V4L2_PIX_FMT_SRGGB8 v4l2_fourcc('R', 'G', 'G', 'B') /* RGRG.. GBGB..    */
#endif 

//convert from string template
// args:
// destination (type T - ex: int)
// source (string)
// source type (hex dec oct)
template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
};

//convert string to upper or lower case
// args:
// destination (type T - ex: int)
// source (string)
// source type (hex dec oct)
class GVString
{

  public:
    std::string ToUpper(std::string strToConvert)
    {//change each element of the string to upper case
        for(unsigned int i=0;i<strToConvert.length();i++)
        {
            strToConvert[i] = toupper(strToConvert[i]);
        }
        return strToConvert;//return the converted string
    };

    std::string ToLower(std::string strToConvert)
    {//change each element of the string to lower case
        for(unsigned int i=0;i<strToConvert.length();i++)
        {
            strToConvert[i] = tolower(strToConvert[i]);
        }
        return strToConvert;//return the converted string
    };

};

class GVSleep
{
    long sleep_us;
    int n_loops;
    bool *var;
    pthread_mutex_t *mutex;

  public:
    GVSleep(int sleep_ms, bool* _var=NULL, pthread_mutex_t * _mutex=NULL, int _n_loops=30)
    {
        sleep_us = sleep_ms * 1000; /*convert to microseconds*/
        var = _var;
        mutex = _mutex;
        n_loops = _n_loops;
    };
    /*wait on cond by sleeping for n_loops of sleep_ms ms (test var==val every loop)*/
    /*return remaining number of loops (if 0 then a stall ocurred)              */
    int wait_ms(bool val=true)
    {
        int n=n_loops;
        pthread_mutex_lock( mutex );
        if(var != NULL)
        {
            while( (*var != val) && ( n > 0 ) ) /*wait at max (n_loops*sleep_ms) ms */
            {
                pthread_mutex_unlock( mutex );
                n--;
                usleep( sleep_us );/*sleep for sleep_ms ms*/
                pthread_mutex_lock( mutex );
            }
            pthread_mutex_unlock( mutex );
        }
        else
        {
            pthread_mutex_unlock( mutex );
            usleep( sleep_us );/*sleep for sleep_ms ms*/
        }
        
        return (n);
    };
};

#endif
