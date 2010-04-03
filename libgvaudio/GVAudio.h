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
//max number of audio buffers in the queue
#define AUDBUFF_SIZE 100

START_LIBGVAUDIO_NAMESPACE

struct audioDevice
{
    int id;
    std::string name;
    int channels;
    int samprate;
    double high_input_latency;
    double low_input_latency;
};

class AudBuff
{
  public:
    AudBuff();
    ~AudBuff();
    bool processed;
    UINT64 time_stamp;
    float*  f_frame;
    INT16* i_frame;
    int samples;
};

class GVAudio
{
    bool verbose;
    pthread_mutex_t mutex;
    int api; //0-Portaudio 1-pulse audio
    bool enabled;                   // audio enabled disabled
    bool stream_ready;               // stream ready flag
    bool streaming;                 // audio streaming flag
    bool complete;                  // flag callback to complete streaming
    int defDevice;                  //index of default audio device
    int indDevice;                  //index of audio device in use
    PaStreamParameters inputParameters;
    PaStream *stream;
    int channels;                   // channels
    int samprate;                   // samp rate
    int aud_numBytes;               // bytes copied to out buffer*/
    int aud_numSamples;             // samples copied to out buffer*/
    UINT64 snd_begintime;           // audio recording start time*/
    float *recordedSamples;         // callback frame buffer
    int frame_size;                 // audio frame size (def to mpg size =1152 samp per chan)
    int maxIndex;                   // size of audio ring buffer
    int sampleIndex;                // callback frame buffer index
    int max_samples;                // recordedSamples buffer size
    AudBuff *audio_buff;            // ring buffer for audio data
    int pIndex;                     // ring buffer producer index;
    int cIndex;                     // ring buffer consumer index;
    UINT64 time_stamp;              // audio frame time stamp
    UINT64 ts_ref;                  // timestamp sync reference
    void fill_audio_buffer();
    INT16 clip_int16 (float in);
    
    int recordCallback (const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags);
    
    static int myRecordCallback(
                        const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData )
    {
        return ((GVAudio*)userData)
            ->recordCallback(inputBuffer, outputBuffer, 
                             framesPerBuffer, 
                             timeInfo, 
                             statusFlags);
    }

    
    //PULSE SUPPORT
#ifdef PULSEAUDIO
    pa_simple *pulse_simple;
#endif
  public:
    GVAudio(bool verbose=false, int audio_buffers = AUDBUFF_SIZE);
    ~GVAudio();
    std::vector<audioDevice> listAudioDev;
    void float_to_int16 (AudBuff *proc_buff);
    int setDevice(int index = -1);
    int getDevice();
    int stopStream();
    int startStream(int samp_rate=0, int chan=0, int frm_size=0, UINT64 timestamp_ref=0);
    //caller must clean buffer after consuming by calling free_buff
    bool getNext(AudBuff* ab);
};

END_LIBGVAUDIO_NAMESPACE

#endif
