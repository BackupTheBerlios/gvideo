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

using namespace std;

GVAudio::GVAudio(bool _verbose /*=false*/)
{
    int it = 0;
    int defaultDisplayed = 0;
    const PaDeviceInfo *deviceInfo;
    pthread_mutex_init( &mutex, NULL );
    channels = 0;
    samprate = 0;
    verbose = _verbose;
    frame_size = MPG_NUM_SAMP;
    
    complete = false;
    streaming = false;
    sampleIndex = 0;
    
    time_stamp = 0;
    ts_ref = 0;
    
    //Init Portaudio API
    Pa_Initialize();
    
    //list audio devices
    int numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 )
    {
        cerr << "SOUND DISABLE: Pa_CountDevices returned" << numDevices << endl;
        enabled = false;
    } 
    else 
    {
        defDevice = 0;
    
        for( it=0; it<numDevices; it++ )
        {
            deviceInfo = Pa_GetDeviceInfo( it );
            if (verbose) cout << "--------------------------------------- device [" << it << "]\n";
            // Mark global and API specific default devices
            defaultDisplayed = 0;

            // with pulse, ALSA is now listed first and doesn't set a API default- 11-2009
            if( it == Pa_GetDefaultInputDevice() )
            {
                if (verbose) cout << "[ Default Input ";
                defaultDisplayed = 1;
                defDevice = it;
            }
            else if( it == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultInputDevice )
            {
                const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
                if (verbose) cout << "[ Default " <<  hostInfo->name << " Input";
                defaultDisplayed = 2;
            }
            // OUTPUT device doesn't matter for capture
            if( it == Pa_GetDefaultOutputDevice() )
            {
                if (verbose) 
                {
                    if (defaultDisplayed) cout << ",";
                    else cout << "[";
                    cout << " Default Output";
                }
                defaultDisplayed = 3;
            }
            else if( it == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice )
            {
                const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
                if (verbose)
                {
                    if(defaultDisplayed) cout << ",";
                    else cout << "[";
                    cout << " Default " << hostInfo->name << " Output";/* OSS ALSA etc*/
                }
                defaultDisplayed = 4;
            }

            if( defaultDisplayed != 0 )
                if (verbose) cout << " ]\n";

            /* print device info fields */
            if (verbose) 
            {
                cout << "Name                     = " << deviceInfo->name << endl;
                cout << "Host API                 = " 
                    << Pa_GetHostApiInfo( deviceInfo->hostApi )->name << endl;
                cout << "Max inputs = " << deviceInfo->maxInputChannels;
            }
            
            // INPUT devices (if it has input channels it's a capture device)
            if (deviceInfo->maxInputChannels > 0) 
            {
                listAudioDev.push_back(
                    (audioDevice) { it, 
                                    string(deviceInfo->name), 
                                    deviceInfo->maxInputChannels, 
                                    deviceInfo->defaultSampleRate,
                                    deviceInfo->defaultHighInputLatency,
                                    deviceInfo->defaultLowInputLatency }); /*saves dev id*/
            }
            if (verbose) 
            {
                cout << ", Max outputs = " << deviceInfo->maxOutputChannels << endl;
                cout << "Def. low input latency   = " << deviceInfo->defaultLowInputLatency << endl;
                cout << "Def. low output latency  = " << deviceInfo->defaultLowOutputLatency << endl;
                cout << "Def. high input latency  = " << deviceInfo->defaultHighInputLatency << endl;
                cout << "Def. high output latency = " << deviceInfo->defaultHighOutputLatency << endl;
                cout << "Def. sample rate         = " << deviceInfo->defaultSampleRate << endl;
            }

        }

        if (verbose) cout << "----------------------------------------------\n";
    }

    recordedSamples = NULL;
    indDevice = defDevice; /* by default we the use default device :D */

}

GVAudio::~GVAudio()
{
    int err = 0;
    stopStream();
    if ((err=Pa_Terminate()) != paNoError) 
        cerr << "Error: Pa_Terminate return error " << err << endl;
    
    delete[] recordedSamples;
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
            cout << "Aborting audio stream\n";
            err = Pa_AbortStream( stream );
        }
        else
        {
            cout << "Stoping audio stream\n";
            err = Pa_StopStream( stream );
        }
        if( err != paNoError )
        {
            cerr << "An error occured while stoping the audio stream\n" ;
            cerr << "Error number: " << err << endl;
            cerr << "Error message: " << Pa_GetErrorText( err ) << endl;
            ret = -1;
        }
    }
    if (stream)
    {
        err = Pa_CloseStream( stream );
        if( err != paNoError )
        {
            cerr << "An error occured while closing the audio stream\n" ;
            cerr << "Error number: " << err << endl;
            cerr << "Error message: " << Pa_GetErrorText( err ) << endl;
            ret = -1;
        }
    }
    streaming = false;
    stream = NULL;
    return ret;
}

int GVAudio::startStream(int samp_rate/*=0*/, int chan/*=0*/, int frm_size/*=0*/)
{

    if(stream || streaming)
    {
        stopStream();
    }
   
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
    else
        frame_size = MPG_NUM_SAMP;
    //mutex_unlock(mutex);

    int num_samples = frame_size * channels;
    
    if(recordedSamples) delete[] recordedSamples;
    recordedSamples = new float[num_samples];
    
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
                &GVAudio::myRecordCallback,        /* sound callback     */
                NULL );                /* callback userData  */

    if( err != paNoError ) return (-1);

    err = Pa_StartStream( stream );
    if( err != paNoError ) 
    {
        cerr << "Error opening audio stream, Pa_OpenStream returned error " << err << endl;
        streaming = false;
        if(stream) Pa_AbortStream( stream );
        
        return -1;
    }
    GVTime *timer = new GVTime;
    snd_begintime = timer->ns_time_monotonic();
    delete timer;
    return 0;
}

int GVAudio::setDevice(int index)
{
    if(index < listAudioDev.size())
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
    int n_samp = frame_size * channels;
        
    if(sampleIndex > n_samp)
    {
        if(time_stamp <= 0)
        {
            if((ts_ref > 0) && (ts_ref < snd_begintime))
             time_stamp = snd_begintime - ts_ref;
            else 
                time_stamp = 1; /*make it > 0 otherwise we will keep getting the same ts*/
        }
        else
            time_stamp += (GV_NSEC_PER_SEC * n_samp)/(samprate * channels);
            
        AudBuff* ab = new AudBuff;
        ab->frame = new float[n_samp];
        memcpy(ab->frame, recordedSamples, n_samp * sizeof(float));
        ab->time_stamp = time_stamp;
        
        pthread_mutex_lock( &mutex );
        if(audio_buff.size() > AUDBUFF_SIZE)
            audio_buff.push(*ab);
        else
            cerr << "Dropping audio frame - buffer treshold reached\n";
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

	// if (skip_n > 0) /*skip audio while were skipping video frames*/
	// {
		
		// if(capVid) 
		// {
			// g_mutex_lock( data->mutex );
				// data->snd_begintime = ns_time_monotonic(); /*reset first time stamp*/
			// g_mutex_unlock( data->mutex );
			// return (paContinue); /*still capturing*/
		// }
		// else
		// {	g_mutex_lock( data->mutex );
				// data->streaming=FALSE;
			// g_mutex_lock( data->mutex );
			// return (paComplete);
		// }
	// }

    int numSamples= framesPerBuffer * chan;

    /*set to false on paComplete*/
    streaming = true;
    
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
AudBuff GVAudio::popAudBuff()
{
    pthread_mutex_lock( &mutex );
    AudBuff ab = audio_buff.front();
    audio_buff.pop();
    pthread_mutex_unlock( &mutex );
    
    return (ab);
}

void GVAudio::free_buff(AudBuff *ab)
{
    delete[] ab->frame;
    delete ab;
}

