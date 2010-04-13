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
#include <cstdlib>
#include <ctime>
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

//namespaces macros
#define START_GVCOMMON_NAMESPACE namespace gvcommon {
#define END_GVCOMMON_NAMESPACE };
#define START_LIBGVIDEO_NAMESPACE namespace libgvideo {
#define END_LIBGVIDEO_NAMESPACE };
#define START_LIBGVAUDIO_NAMESPACE namespace libgvaudio {
#define END_LIBGVAUDIO_NAMESPACE };
#define START_LIBGVENCODER_NAMESPACE namespace libgvencoder {
#define END_LIBGVENCODER_NAMESPACE };
#define START_LIBGVRENDER_NAMESPACE namespace libgvrender {
#define END_LIBGVRENDER_NAMESPACE };

#define START_GVIDEOGTK_NAMESPACE namespace gvideogtk {
#define END_GVIDEOGTK_NAMESPACE };

//audio frame defs
#define MPG_NUM_SAMP 1152  //number of samples in a audio buffer (MPEG frame) 
#define DEF_AUD_FRAME_SIZE MPG_NUM_SAMP*4

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

#ifndef GV_NSEC_PER_SEC
#define GV_NSEC_PER_SEC 1000000000
#endif

#ifndef GV_USEC_PER_SEC
#define GV_USEC_PER_SEC 1000000
#endif

#ifndef GV_MSEC_PER_SEC
#define GV_MSEC_PER_SEC 1000
#endif

START_GVCOMMON_NAMESPACE

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
    
    // get filename from a full pathname
    std::string get_Filename (const std::string& str)
    {
        size_t found;
        found=str.find_last_of("/\\");
        //cout << " folder: " << str.substr(0,found) << endl;
        //cout << " file: " << str.substr(found+1) << endl;
        return str.substr(found+1);
    };
    
    //get only the folder path from a full pathname
    std::string get_Path (const std::string& str)
    {
        size_t found;
        found=str.find_last_of("/\\");
        //cout << " folder: " << str.substr(0,found) << endl;
        //cout << " file: " << str.substr(found+1) << endl;
        return str.substr(0, found);
    };
    
    //get the basename from filename (basename.ext)
    std::string get_Basename (const std::string& str)
    {
        size_t found;
        found=str.find_last_of(".");
        //cout << " folder: " << str.substr(0,found) << endl;
        //cout << " file: " << str.substr(found+1) << endl;
        return str.substr(0, found);
    };
    
    //get the extension (ext) from filename (basename.ext)
    std::string get_Extension (const std::string& str)
    {
        size_t found;
        found=str.find_last_of(".");
        //cout << " folder: " << str.substr(0,found) << endl;
        //cout << " file: " << str.substr(found+1) << endl;
        return str.substr(found+1);
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

class GVRand
{
    UINT8 generate_byte()
    {
       return ((UINT8) (rand() % 256));
    }

  public:
    GVRand(unsigned int seed=0)
    {
        if(seed > 0)
            srand(seed);
        else
            srand(time(NULL) + getpid());
    }
    int generate_range(int min=0, int max=RAND_MAX)
    {
        if (max > RAND_MAX)
            max = RAND_MAX;
        if (min < 0)
            min = 0;
        if (max < 0)
            max = 1;
        if (min > max)
            min = max - 1;
        return( rand() % (max-min+1) + min);
    }
    
    void generate_bytes(UINT8 *dest, int num_bytes)
    {
        int i = 0;
        for (i=0; i< num_bytes; i++)
            dest[i] = generate_byte();
    }
};

class GVTime
{
    struct timespec ts;
    
  public:
    /*REAL TIME CLOCK*/
    /*in nanoseconds*/
    UINT64 ns_time ()
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        return ((UINT64) ts.tv_sec * GV_NSEC_PER_SEC + (UINT64) ts.tv_nsec);
    }

    /*MONOTONIC CLOCK*/
    /*in nanosec*/
    UINT64 ns_time_monotonic()
    {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ((UINT64) ts.tv_sec * GV_NSEC_PER_SEC + (UINT64) ts.tv_nsec);
    }

    /*REAL TIME CLOCK*/
    /*in miliseconds*/
    UINT64 ms_time ()
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        return ((UINT64) ts.tv_sec * GV_MSEC_PER_SEC + (UINT64) (ts.tv_nsec / GV_USEC_PER_SEC));
    }

    /*MONOTONIC CLOCK*/
    /*in miliseconds*/
    UINT64 ms_time_monotonic()
    {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ((UINT64) ts.tv_sec * GV_MSEC_PER_SEC + (UINT64) (ts.tv_nsec / GV_USEC_PER_SEC));
    }
};

END_GVCOMMON_NAMESPACE

#endif
