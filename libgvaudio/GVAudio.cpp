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

#include <string.h>
#include <iostream>
#include "GVAudio.h"

START_LIBGVAUDIO_NAMESPACE

AudBuff::AudBuff()
{
    f_frame = NULL;
    i_frame = NULL;
    processed = true;
    time_stamp = 0;
    samples = 0;
}

AudBuff::~AudBuff()
{
    delete[] f_frame;
    delete[] i_frame;
}

GVAudio::GVAudio(bool _verbose /*=false*/, int audio_buffers /*=AUDBUFF_SIZE*/)
{
    int it = 0;
    int i = 0;
    int defId = 0;
    int defaultDisplayed = 0;
    const PaDeviceInfo *deviceInfo;
    
    pthread_mutex_init( &mutex, NULL );
    
    maxIndex = audio_buffers;
    channels = 0;
    samprate = 0;
    int ch = 0;
    verbose = _verbose;
    frame_size = DEF_AUD_FRAME_SIZE;
    
    complete = false;
    streaming = false;
    enabled = true;
    stream_ready = false;
    
    stream = NULL;
    sampleIndex = 0;
    max_samples = 0;
    
    time_stamp = 0;
    ts_ref = 0;
    
    //Init Portaudio API
    Pa_Initialize();
    
    //list audio devices
    int numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 )
    {
        std::cerr << "libgvideo: SOUND DISABLE Pa_CountDevices returned " << numDevices << std::endl;
        enabled = false;
    } 
    else 
    {
        defDevice = 0;
    
        for( it=0; it<numDevices; it++ )
        {
            deviceInfo = Pa_GetDeviceInfo( it );
            if (verbose) std::cout << "--------------------------------------- device [" << it << "]\n";
            // Mark global and API specific default devices
            defaultDisplayed = 0;

            // with pulse, ALSA is now listed first and doesn't set a API default- 11-2009
            if( it == Pa_GetDefaultInputDevice() )
            {
                if (verbose) std::cout << "[ Default Input ";
                defaultDisplayed = 1;
                defId = it;
            }
            else if( it == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultInputDevice )
            {
                const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
                if (verbose) std::cout << "[ Default " <<  hostInfo->name << " Input";
                defaultDisplayed = 2;
            }
            // OUTPUT device doesn't matter for capture
            if( it == Pa_GetDefaultOutputDevice() )
            {
                if (verbose) 
                {
                    if (defaultDisplayed) std::cout << ",";
                    else std::cout << "[";
                    std::cout << " Default Output";
                }
                defaultDisplayed = 3;
            }
            else if( it == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice )
            {
                const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
                if (verbose)
                {
                    if(defaultDisplayed) std::cout << ",";
                    else std::cout << "[";
                    std::cout << " Default " << hostInfo->name << " Output";/* OSS ALSA etc*/
                }
                defaultDisplayed = 4;
            }

            if( defaultDisplayed != 0 )
                if (verbose) std::cout << " ]\n";

            /* print device info fields */
            if (verbose) 
            {
                std::cout << "Name                     = " << deviceInfo->name << std::endl;
                std::cout << "Host API                 = " 
                    << Pa_GetHostApiInfo( deviceInfo->hostApi )->name << std::endl;
                std::cout << "Max inputs = " << deviceInfo->maxInputChannels;
            }
            
            // INPUT devices (if it has input channels it's a capture device)
            if (deviceInfo->maxInputChannels > 0) 
            {
                ch = (deviceInfo->maxInputChannels >= 2)?2:1;
                listAudioDev.push_back(
                    (audioDevice) { it, 
                                    std::string(deviceInfo->name), 
                                    ((deviceInfo->maxInputChannels >= 2)?2:1), 
                                    int(deviceInfo->defaultSampleRate),
                                    deviceInfo->defaultHighInputLatency,
                                    deviceInfo->defaultLowInputLatency }); /*saves dev id*/
                if(defId == it) 
                    defDevice = listAudioDev.size() - 1;
            }
            if (verbose) 
            {
                std::cout << ", Max outputs = " << deviceInfo->maxOutputChannels << std::endl;
                std::cout << "Def. low input latency   = " << deviceInfo->defaultLowInputLatency << std::endl;
                std::cout << "Def. low output latency  = " << deviceInfo->defaultLowOutputLatency << std::endl;
                std::cout << "Def. high input latency  = " << deviceInfo->defaultHighInputLatency << std::endl;
                std::cout << "Def. high output latency = " << deviceInfo->defaultHighOutputLatency << std::endl;
                std::cout << "Def. sample rate         = " << deviceInfo->defaultSampleRate << std::endl;
            }

        }

        if (verbose) std::cout << "----------------------------------------------\n";
    }

    /* allocate when starting stream */
    audio_buff = NULL;
    recordedSamples = NULL;
    /* by default we the use default device :D */
    indDevice = defDevice;
    
    pIndex = 0;
    cIndex = 0;   

}

GVAudio::~GVAudio()
{
    int err = 0;
    int i = 0;
    if(verbose) std::cout << "stopping stream\n";
    stopStream();
    /* Pa_Terminate seems to segfault when using pulseaudio (Ubuntu) */
    if ((err=Pa_Terminate()) != paNoError) 
        std::cerr << "Error: Pa_Terminate return error " << err << std::endl;
    if(verbose) std::cout << "removing samples\n";
    if (recordedSamples != NULL) delete[] recordedSamples;
    
    pthread_mutex_destroy(&mutex);

    delete[] audio_buff;
}

/* saturate float samples to int16 limits*/
INT16 GVAudio::clip_int16 (float in)
{
	in = (in < -32768) ? -32768 : (in > 32767) ? 32767 : in;
	
	return ((INT16) in);
}

void GVAudio::float_to_int16 (AudBuff *ab)
{
    if(!(ab->i_frame))
        ab->i_frame = new INT16[max_samples];
    
    int samp = 0;
	for(samp=0; samp < ab->samples; samp++)
	{
		ab->i_frame[samp] = clip_int16(ab->f_frame[samp] * 32767.0); //* 32768 + 385;
	}
    
}

int GVAudio::stopStream()
{
    int ret = 0;
    PaError err = paNoError;
    pthread_mutex_lock( &mutex );
    complete = true;
    pthread_mutex_unlock( &mutex );
    //now we should wait a while for streaming = false
    
    if(stream || streaming)
    {
        if(Pa_IsStreamActive( stream ) > 0)
        {
            std::cout << "Aborting audio stream\n";
            err = Pa_AbortStream( stream );
        }
        else
        {
            std::cout << "Stoping audio stream\n";
            err = Pa_StopStream( stream );
        }
        if( err != paNoError )
        {
            std::cerr << "An error occured while stoping the audio stream\n" ;
            std::cerr << "Error number: " << err << std::endl;
            std::cerr << "Error message: " << Pa_GetErrorText( err ) << std::endl;
            ret = -1;
        }
    }
    if (stream)
    {
        err = Pa_CloseStream( stream );
        if( err != paNoError )
        {
            std::cerr << "An error occured while closing the audio stream\n" ;
            std::cerr << "Error number: " << err << std::endl;
            std::cerr << "Error message: " << Pa_GetErrorText( err ) << std::endl;
            ret = -1;
        }
    }
    streaming = false;
    stream = NULL;
    return ret;
}

int GVAudio::startStream(int samp_rate/*=0*/, int chan/*=0*/, int frm_size/*=0*/, UINT64 timestamp_ref /*=0*/)
{
    stream_ready = false;
    
    if(!enabled)
    {
        std::cerr << "libgvaudio: can't start audio stream : audio is disabled \n";
        return -1;
    }
    int i=0;
    if(stream || streaming)
    {
        stopStream();
    }
    
    ts_ref = timestamp_ref;
    
    PaError err = paNoError;
     
    pthread_mutex_lock( &mutex );
    complete = false;
    pthread_mutex_unlock( &mutex );
    
    if((chan > 0) && (chan <= listAudioDev[indDevice].channels))
        channels = chan;
    else 
        channels = listAudioDev[indDevice].channels;
    
    if((samp_rate > 0) && (samp_rate <= listAudioDev[indDevice].samprate))
        samprate = samp_rate;
    else 
        samprate = listAudioDev[indDevice].samprate;
    
    if (frm_size > 0)
        frame_size = frm_size;
    /*else use default (frame_size = DEF_AUD_FRAME_SIZE)*/

    max_samples = frame_size * channels;
    
    if(recordedSamples) delete[] recordedSamples;
    recordedSamples = new float[max_samples];
    
    if(audio_buff)
    {
        delete[] audio_buff;
    }
    
    audio_buff = new AudBuff[maxIndex];
    
    for ( i=0; i<maxIndex; i++ )
    {
        audio_buff[i].f_frame = new float[max_samples];
        audio_buff[i].samples = max_samples;
    }
    sampleIndex = 0;
    
    //select device
    inputParameters.device = listAudioDev[indDevice].id;
    
    inputParameters.channelCount = channels;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    if (Pa_GetDeviceInfo( inputParameters.device ))
        inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency;
    else
        inputParameters.suggestedLatency = DEFAULT_LATENCY_DURATION/1000.0;
    inputParameters.hostApiSpecificStreamInfo = NULL; 

    /*---------------------------- start recording Audio. ----------------------------- */
    err = Pa_OpenStream(
                &stream,
                &inputParameters,
                NULL,                  /* &outputParameters, */
                samprate,              /* sample rate        */
                frame_size,            /* buffer in frames => Mpeg frame size (samples = 1152 samples * channels)*/
                paNoFlag,              /* PaNoFlag - clip and dhiter*/
                &myRecordCallback,        /* sound callback     */
                this );                /* callback userData  */

    if( err != paNoError ) return (-1);
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) 
    {
        std::cerr << "Error opening audio stream, Pa_OpenStream returned error " << err << std::endl;
        streaming = false;
        
        if(stream) Pa_AbortStream( stream );
        
        return -1;
    }
    gvcommon::GVTime *timer = new gvcommon::GVTime;
    snd_begintime = timer->ns_time_monotonic();
    delete timer;
    
    stream_ready = true;
    
    if(verbose) std::cout << "audio initial timecode:" << snd_begintime << std::endl;
    return 0;
}

int GVAudio::setDevice(int index /*= -1*/)
{
    if((index >=0) && (index < listAudioDev.size()))
        indDevice = index;
    else 
        indDevice = defDevice;
    
    return indDevice;
}

int GVAudio::getDevice()
{
    return indDevice;
}

void GVAudio::fill_audio_buffer()
{
        
    if(sampleIndex >= max_samples - 1)
    {
        if(!time_stamp)
        {
            if((ts_ref > 0) && (ts_ref < snd_begintime))
                time_stamp = snd_begintime - ts_ref;
            else 
                time_stamp = snd_begintime;
        }
        else
            time_stamp += (UINT64) (GV_NSEC_PER_SEC * (UINT64) frame_size)/((UINT64)samprate);
        
        pthread_mutex_lock( &mutex );
        if(!(audio_buff[pIndex].processed))
            std::cerr << "Dropping audio frame\n";
        else
        {
            memcpy(audio_buff[pIndex].f_frame, recordedSamples, max_samples * sizeof(float));
            audio_buff[pIndex].time_stamp = time_stamp;
            audio_buff[pIndex].samples = max_samples;
            audio_buff[pIndex].processed = false;
            pIndex++;
            if(pIndex >= maxIndex)
                pIndex = 0;
        }    
        sampleIndex = 0; //reset
        pthread_mutex_unlock( &mutex );
    }

}


/*--------------------------- sound callback ------------------------------*/
int GVAudio:: recordCallback (const void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags)
{
    const float* rptr = (const float*) inputBuffer;
    int i;

    pthread_mutex_lock( &mutex );
    bool streamOn = complete;
    int chan = channels;
    pthread_mutex_unlock( &mutex );

    int numSamples= framesPerBuffer * chan;

    /*set to false on paComplete*/
    streaming = true;
    
    //if(verbose) std::cout << "processing " << numSamples << " audio samples\n";
    if( inputBuffer == NULL )
    {
        for( i=0; i<numSamples; i++ )
        {
            recordedSamples[sampleIndex] = 0;/*silence*/
            sampleIndex++;
            
            fill_audio_buffer();
        }
    }
    else
    {
        for( i=0; i<numSamples; i++ )
        {
            recordedSamples[sampleIndex] = *rptr++;
            sampleIndex++;

            fill_audio_buffer();
        }
    }

    if(!streamOn) return (paContinue); /*still capturing*/
    else 
    {
        streaming=false;
        return (paComplete);
    }
}

//caller must clean buffer after consuming
bool GVAudio::getNext(AudBuff *ab)
{
    if(!stream_ready)
    {
        std::cerr << "libgvideo: stream isn't ready \n";
        return (false);
    }
    
    if(!(ab->f_frame))
            ab->f_frame = new float[max_samples];
    
    pthread_mutex_lock( &mutex );
    if(!(audio_buff[cIndex].processed) && audio_buff[cIndex].samples > 0)
    {
        memcpy(ab->f_frame, audio_buff[cIndex].f_frame, audio_buff[cIndex].samples * sizeof(float));
        ab->time_stamp = audio_buff[cIndex].time_stamp;
        ab->samples = audio_buff[cIndex].samples;
        float_to_int16(ab);
        audio_buff[cIndex].processed = true;
        ab->processed = true;
        cIndex++;
        if(cIndex >= maxIndex)
            cIndex = 0;
    }
    else
    {
        ab->processed = false;
    }
    pthread_mutex_unlock( &mutex );
    
    return (ab->processed);
}

END_LIBGVAUDIO_NAMESPACE

