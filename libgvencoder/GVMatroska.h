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
#  GVMatroska class : gvideo matroska container class                           #
#                                                                               #
********************************************************************************/

#ifndef GVMATROSKA_H
#define GVMATROSKA_H

#include <iostream>
#include <fstream>
#include <vector>
#include "gvcommon.h"

START_LIBGVENCODER_NAMESPACE

struct mk_Context 
{
    mk_Context *next, **prev, *parent;
    unsigned id;
    void *data;
    unsigned d_cur, d_max;
};


class GVMatroska
{
    bool verbose;
    std::ofstream myfile;
    /*-------header------*/
    int video_only;            //not muxing audio   
    unsigned duration_ptr;     //file location pointer for duration
    INT64 segment_size_ptr;  //file location pointer for segment size
    INT64 cues_pos;
    INT64 seekhead_pos;  
    mk_Context *root, *cluster, *frame;
    mk_Context *freelist;
    mk_Context *actlist;
    bool wrote_header;
    /*-------video-------*/
    UINT64 def_duration;
    unsigned def_duration_ptr;
    INT64 timescale;
    INT64 cluster_tc_scaled;
    INT64 frame_tc, prev_frame_tc_scaled, max_frame_tc;
    bool in_frame, keyframe;
    bool b_writing_frame;
    /*-------audio-------*/
    mk_Context *audio_frame;
    INT64 audio_frame_tc, audio_prev_frame_tc_scaled; 
    INT64 audio_block, block_n;
    bool audio_in_frame;
    /*--------cues-------*/
    mk_Context *cues;
    INT64 cue_time;
    INT64 cue_video_track_pos;
    INT64 cue_audio_track_pos;
    /*-----seek head-----*/
    unsigned cluster_index;
    std::vector<INT64> cluster_pos;
    
    mk_Context *mk_createContext(mk_Context *parent, unsigned id);
    int mk_appendContextData(mk_Context *c, const void *data, unsigned size);
    int mk_writeID(mk_Context *c, unsigned id);
    int mk_writeSize(mk_Context *c, unsigned size);
    int mk_flushContextData(mk_Context *c);
    int mk_closeContext(mk_Context *c, unsigned *off);
    void mk_destroyContexts();
    int mk_writeStr(mk_Context *c, unsigned id, const char *str);
    int mk_writeBin(mk_Context *c, unsigned id, const void *data, unsigned size);
    int mk_writeVoid(mk_Context *c, unsigned size);
    int mk_writeUIntRaw(mk_Context *c, unsigned id, INT64 ui);
    int mk_writeUInt(mk_Context *c, unsigned id, INT64 ui);
    int mk_writeSegPos(mk_Context *c, INT64 ui);
    int mk_writeFloatRaw(mk_Context *c, float f);
    int mk_writeFloat(mk_Context *c, unsigned id, float f);
    unsigned mk_ebmlSizeSize(unsigned s);
    int mk_flushFrame();
    int mk_flushAudioFrame();
    int mk_closeCluster();
    int mk_startFrame();
    int mk_startAudioFrame();
    int mk_setFrameFlags(INT64 timestamp, int keyframe);
    int mk_setAudioFrameFlags(INT64 timestamp, int keyframe);
    int mk_addFrameData(const void *data, unsigned size);
    int mk_addAudioFrameData(const void *data, unsigned size);
    int write_cues();
    int write_SeekHead();
    int write_SegSeek(INT64 cues_pos, INT64 seekHeadPos);
    
  public:
    GVMatroska(const char* filename, 
               const char *writingApp,
               const char *codecID,
               const char *AcodecID,
               const void *codecPrivate, unsigned codecPrivateSize,
               UINT64 default_frame_duration,  /*video */
               void *AcodecPrivate, unsigned AcodecPrivateSize,
               UINT64 default_aframe_duration, /*audio */
               INT64 timescale,
               unsigned width, unsigned height,
               unsigned d_width, unsigned d_height,
               int samprate, int channels, int bitsSample,
               bool _verbose = false);
    ~GVMatroska();
    bool failed();
    void set_DefDuration(UINT64 _def_duration);
    int add_VideoFrame(const void *data, unsigned size, INT64 timestamp, int keyframe = 0);
    int add_AudioFrame(const void *data, unsigned size, INT64 timestamp, int keyframe = 0);    
};

END_LIBGVENCODER_NAMESPACE

#endif

