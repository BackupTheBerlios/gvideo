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
#  GVAudio class : gvideo audio interface definition                            #
#                                                                               #
********************************************************************************/

#ifndef GVAUDIO_H
#define GVAUDIO_H

#include <portaudio.h>
#include <queue>
#include <vector>
#include "gvcommon.h"
#include "../config.h"

#ifdef PULSEAUDIO
  #include <pulse/simple.h>
  #include <pulse/error.h>
  #include <pulse/gccmacro.h>
#endif

/*------------- portaudio defs ----------------*/

#define DEFAULT_LATENCY_DURATION 100.0
#define DEFAULT_LATENCY_CORRECTION -130.0

#define PA_SAMPLE_TYPE  paFloat32

#define SAMPLE_SILENCE  (0.0f)
#define MAX_SAMPLE (1.0f)

//API index
#define PORT  0
#define PULSE 1

//buffer defs
#define AUDBUFF_SIZE 100   //max number of audio buffers in the queue
#define MPG_NUM_SAMP 1152  //number of samples in a audio buffer (MPEG frame) 

struct audioDevice
{
    int id;
    std::string name;
    int channels;
    double samprate;
    double high_input_latency;
    double low_input_latency;
};

struct AudBuff
{
    UINT64 time_stamp;
    float *frame;
};

class GVAudio
{
    bool verbose;
    int api; //0-Portaudio 1-pulse audio
    bool enabled;
    int defDevice; //index of default audio device
    int indDevice; //index of audio device in use
    PaStreamParameters inputParameters;
    PaStream *stream;
    int channels;                   // channels
    bool streaming;                 // audio streaming flag
    bool complete;                  // flag callback to complete streaming
    int samprate;                   // samp rate
    int numsec;                     // aprox. number of seconds for out buffer size
    int aud_numBytes;               // bytes copied to out buffer*/
    int aud_numSamples;             // samples copied to out buffer*/
    UINT64 snd_begintime;           // audio recording start time*/
    int capVid;                     // video capture flag
    float *recordedSamples;         // callback frame buffer
    int frame_size;                 // audio frame size (def to mpg size =1152 samp per chan)
    int sampleIndex;                // callback frame buffer index
    std::queue<AudBuff> audio_buff; // buffer queue for audio data
    int64_t time_stamp;             // audio frame time stamp
    int64_t ts_ref;                 // timestamp sync reference
    UINT16 *pcm_sndBuff;            // buffer for pcm coding with int16
    int snd_Flags;                  // effects flag
    int skip_n;                     // video frames to skip
    UINT64 delay;                   // in nanosec - h264 has a two frame delay that must be compensated
    //GMutex *mutex;                // audio mutex
    int recordCallback (const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData );
    
    //PULSE SUPPORT
#ifdef PULSEAUDIO
    pa_simple *pulse_simple;
    //GThread *pulse_read_th;
#endif
  public:
    GVAudio(bool verbose=false);
    ~GVAudio();
    void float_to_int16 (AudBuff *proc_buff);
    std::vector<audioDevice> listAudioDev;
    AudBuff* popAudBuff();
    int setDevice(int index);
    int getDevice();
};


#endif
