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

#include <iostream>
#include <string>
#include <cstdlib>
#include <string.h> //memcpy
#include "GVMatroska.h"


using namespace std;

GVMatroska::GVMatroska(const char* filename, 
                       const char *writingApp,
                       const char *codecID,
                       const char *AcodecID,
                       const void *codecPrivate, unsigned codecPrivateSize,
                       UINT64 default_frame_duration,  /*video */
                       const void *AcodecPrivate, unsigned AcodecPrivateSize,
                       UINT64 default_aframe_duration, /*audio */
                       INT64 _timescale,
                       unsigned width, unsigned height,
                       unsigned d_width, unsigned d_height,
                       float SampRate, int channels, int bitsSample)
{
    wrote_header = false;
    
    root = mk_createContext(NULL, 0);
    if (!root)
        goto error;

    myfile.open (filename,  ios::out | ios::app | ios::binary);
    if(myfile.is_open())
    {
        mk_Context *c, *ti, *v, *ti2, *ti3, *a;
        UINT8 empty[8] = {0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        int extra=6; /* use six extra bytes for segment size */
        unsigned pos = 0; /* current position in header */

        timescale = _timescale;
        def_duration = default_frame_duration; 

        if ((c = mk_createContext(root, EBML_ID_HEADER)) == NULL) /* EBML */
            goto error;
        
        //TEST(mk_writeUInt(c, EBML_ID_EBMLVERSION, 1));       // EBMLVersion
        //TEST(mk_writeUInt(c, EBML_ID_EBMLREADVERSION, 1));   // EBMLReadVersion
        //TEST(mk_writeUInt(c, EBML_ID_EBMLMAXIDLENGTH, 4));   // EBMLMaxIDLength
        //TEST(mk_writeUInt(c, EBML_ID_EBMLMAXSIZELENGTH, 8)); // EBMLMaxSizeLength
        TEST(mk_writeStr(c, EBML_ID_DOCTYPE, "matroska"));     // DocType
        TEST(mk_writeUInt(c, EBML_ID_DOCTYPEVERSION, 2));      // DocTypeVersion
        TEST(mk_writeUInt(c, EBML_ID_DOCTYPEREADVERSION, 2));  // DocTypeReadversion
        pos=c->d_cur;
        TEST(mk_closeContext(c, &pos));

        /* SEGMENT starts at position 24  */
        if ((c = mk_createContext(root, MATROSKA_ID_SEGMENT)) == NULL)
            goto error;
   
        pos+=4; /*pos=28*/

        segment_size_ptr = pos;
        /*needs full segment size here (including clusters) but we only know the header size for now.(2 bytes)*/
        /*add extra (6) bytes - reserve a total of 8 bytes for segment size in ebml format =6+2(header size)  */
        mk_appendContextData(c, empty, extra);
        pos+=extra+2;/*(pos=36) account 2 bytes from header size -these will only be available after closing c context*/

        seekhead_pos = pos; /* SeekHead Position */
        if ((ti = mk_createContext(c, MATROSKA_ID_SEEKHEAD)) == NULL)    /* SeekHead */
            goto error;
        
        if ((ti2 = mk_createContext(ti, MATROSKA_ID_SEEKENTRY)) == NULL) /* Seek */
            goto error;
        
        TEST(mk_writeUInt (ti2, MATROSKA_ID_SEEKID, MATROSKA_ID_INFO));    /* seekID */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_SEEKPOSITION, 4135-seekhead_pos));
        TEST(mk_closeContext(ti2, 0));

        if ((ti2 = mk_createContext(ti, MATROSKA_ID_SEEKENTRY)) == NULL) /* Seek */
            goto error;
        
        TEST(mk_writeUInt(ti2, MATROSKA_ID_SEEKID, MATROSKA_ID_TRACKS));   /* seekID */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_SEEKPOSITION, 4220-seekhead_pos));
        TEST(mk_closeContext(ti2, 0));
        pos = ti->d_cur + segment_size_ptr + 2;/* add 2 bytes from the header size */
        TEST(mk_closeContext(ti, &pos)); /* pos=71 */
        /* allways start Segment info at pos 4135
         * this will be overwritten by seek entries for cues and the final seekhead   */
        TEST(mk_writeVoid(c, 4135-(pos+3))); /* account 3 bytes from Void ID (1) and size(2) */

        /* Segment Info at position 4135 (fixed)*/
        pos=4135;
        if ((ti = mk_createContext(c, MATROSKA_ID_INFO)) == NULL)
            goto error;
        
        TEST(mk_writeUInt(ti, MATROSKA_ID_TIMECODESCALE, timescale));
        TEST(mk_writeStr(ti, MATROSKA_ID_MUXINGAPP, "GVEncoder Mux -2010.03"));
        TEST(mk_writeStr(ti, MATROSKA_ID_WRITINGAPP, writingApp));
        /* signed 8 byte integer in nanoseconds 
         * with 0 indicating the precise beginning of the millennium (at 2001-01-01T00:00:00,000000000 UTC)
         * value: ns_time - 978307200000000000  ( Unix Epoch is 01/01/1970 ) */
        GVTime *time = new GVTime;
        UINT64 date = time->ns_time() - 978307200000000000LL;
        delete time;
        TEST(mk_writeUIntRaw(ti, MATROSKA_ID_DATEUTC, date)); /* always use the full 8 bytes */
        /*generate seg uid - 16 byte random int*/
        GVRand* rand_uid= new GVRand;
        int seg_uid[4] = {0,0,0,0};
        seg_uid[0] = rand_uid->generate();
        seg_uid[1] = rand_uid->generate();
        seg_uid[2] = rand_uid->generate();
        seg_uid[3] = rand_uid->generate();
        TEST(mk_writeBin(ti, MATROSKA_ID_SEGMENTUID, seg_uid, 16));
        TEST(mk_writeFloat(ti, MATROSKA_ID_DURATION, 0)); //Track Duration
        /*ti->d_cur - float size(4) + EBML header size(24) + extra empty bytes for segment size(6)*/
        duration_ptr = ti->d_cur + 20 + extra;
        TEST(mk_closeContext(ti, &duration_ptr)); /* add ti->parent->d_cur to duration_ptr */

        /*segment tracks start at 4220 (fixed)*/
        pos=4220;
        if ((ti = mk_createContext(c, MATROSKA_ID_TRACKS)) == NULL)
            goto error;
        
        /*VIDEO TRACK entry start at 4226 = 4220 + 4 + 2(tracks header size)*/
        pos+=6;
        if ((ti2 = mk_createContext(ti, MATROSKA_ID_TRACKENTRY)) == NULL)
            goto error;
        
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKNUMBER, 1));        /* TrackNumber */

        int track_uid1 = rand_uid->generate();
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKUID, track_uid1));  /* Track UID  */
        /* TrackType 1 -video 2 -audio */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKTYPE, MATROSKA_TRACK_TYPE_VIDEO)); 
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKFLAGENABLED, 1));   /* enabled    */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKFLAGDEFAULT, 1));   /* default    */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKFLAGFORCED, 0));    /* forced     */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKFLAGLACING, 0));    /* FlagLacing */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKMINCACHE, 1));      /* MinCache   */
        TEST(mk_writeFloat(ti2, MATROSKA_ID_TRACKTIMECODESCALE, 1));/* Timecode scale (float) */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_TRACKMAXBLKADDID, 0));   /* Max Block Addition ID  */
        TEST(mk_writeStr(ti2, MATROSKA_ID_CODECID, codecID));       /* CodecID    */
        TEST(mk_writeUInt(ti2, MATROSKA_ID_CODECDECODEALL, 1));     /* Codec Decode All       */

        /* CodecPrivate (40 bytes) */
        if (codecPrivateSize)
            TEST(mk_writeBin(ti2, MATROSKA_ID_CODECPRIVATE, codecPrivate, codecPrivateSize));

        /* Default video frame duration - for fixed frame rate only (reset when closing matroska file)
         * not required by the spec, but at least vlc seems to need it to work properly
         * default duration position will be set after closing the current context */
        unsigned delta_pos = ti2->d_cur;
        TEST(mk_writeUIntRaw(ti2, MATROSKA_ID_TRACKDEFAULTDURATION, def_duration));

        if ((v = mk_createContext(ti2, MATROSKA_ID_TRACKVIDEO)) == NULL) /* Video track */
            goto error;
        
        TEST(mk_writeUInt(v, MATROSKA_ID_VIDEOPIXELWIDTH, width));
        TEST(mk_writeUInt(v, MATROSKA_ID_VIDEOPIXELHEIGHT, height));
        TEST(mk_writeUInt(v, MATROSKA_ID_VIDEOFLAGINTERLACED, 0));  /* interlaced flag */
        TEST(mk_writeUInt(v, MATROSKA_ID_VIDEODISPLAYWIDTH, d_width));
        TEST(mk_writeUInt(v, MATROSKA_ID_VIDEODISPLAYHEIGHT, d_height));
        unsigned delta_pos1=v->d_cur;
        TEST(mk_closeContext(v, &delta_pos1));
        delta_pos = delta_pos1-delta_pos; /* video track header size + default duration entry */
        pos+=ti2->d_cur;
        TEST(mk_closeContext(ti2, &pos));
        /* default duration position = current postion - (video track header size + def dur. entry) */
        def_duration_ptr =  pos-delta_pos;

        if (SampRate > 0)
        {
            /*AUDIO TRACK entry */
            if ((ti3 = mk_createContext(ti, MATROSKA_ID_TRACKENTRY)) == NULL)
                goto error;
            
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKNUMBER, 2));        /* TrackNumber            */

            int track_uid2 = rand_uid->generate();
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKUID, track_uid2));
            /* TrackType 1 -video 2 -audio */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKTYPE, MATROSKA_TRACK_TYPE_AUDIO));
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKFLAGENABLED, 1));   /* enabled                */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKFLAGDEFAULT, 1));   /* default                */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKFLAGFORCED, 0));    /* forced                 */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKFLAGLACING, 0));    /* FlagLacing             */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKMINCACHE, 0));      /* MinCache               */
            TEST(mk_writeFloat(ti3, MATROSKA_ID_TRACKTIMECODESCALE, 1));/* Timecode scale (float) */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKMAXBLKADDID, 0));   /* Max Block Addition ID  */
            TEST(mk_writeStr(ti3, MATROSKA_ID_CODECID, AcodecID));      /* CodecID                */
            TEST(mk_writeUInt(ti3, MATROSKA_ID_CODECDECODEALL, 1));     /* Codec Decode All       */

            if (default_aframe_duration) /* for fixed sample rate */
                TEST(mk_writeUInt(ti3, MATROSKA_ID_TRACKDEFAULTDURATION, default_aframe_duration)); /* DefaultDuration audio*/

            if ((a = mk_createContext(ti3, MATROSKA_ID_TRACKAUDIO)) == NULL) /* Audio track */
                goto error;
            
            TEST(mk_writeFloat(a, MATROSKA_ID_AUDIOSAMPLINGFREQ, SampRate));
            TEST(mk_writeUInt(a, MATROSKA_ID_AUDIOCHANNELS, channels));
            if (bitsSample > 0) /* for pcm and aac (16) */
                TEST(mk_writeUInt(a, MATROSKA_ID_AUDIOBITDEPTH, bitsSample));
            TEST(mk_closeContext(a, 0));
            /* AudioCodecPrivate */
            if (AcodecPrivateSize)
                TEST(mk_writeBin(ti3, MATROSKA_ID_CODECPRIVATE, AcodecPrivate, AcodecPrivateSize));
            TEST(mk_closeContext(ti3, 0));
        }
        else
        {
            video_only = 1;
        }

        delete rand_uid; /* free random uid generator */
        TEST(mk_closeContext(ti, 0));
        TEST(mk_writeVoid(c, 1024));
        TEST(mk_closeContext(c, 0));
        TEST(mk_flushContextData(root));
        cluster_index = 1;
        INT64 fpos = myfile.tellp();
        cluster_pos.push_back((INT64) fpos - 36);
        wrote_header = true;
    }
    else
    {
        cerr << "ERROR: Could not open file " << filename << " for write\n";
    }
    
  error:
    if (!wrote_header)
        cerr << "ERROR: failed to create " << filename << " header\n";
};

GVMatroska::~GVMatroska()
{
    int ret = 0;
    if (mk_flushFrame() < 0 || mk_flushAudioFrame() < 0 || mk_closeCluster() < 0)
        ret = -1;
    if (wrote_header)
    {
        /* move to end of file */
        myfile.seekp(0, ios_base::end);
        /* store last position */
        INT64 fpos = myfile.tellp();
        INT64 CuesPos = (INT64) fpos - 36;
        //printf("cues at %llu\n",(unsigned long long) CuesPos);
        write_cues();
        /* move to end of file */
        myfile.seekp(0, ios_base::end);
        fpos = myfile.tellp();
        INT64 SeekHeadPos = (INT64) fpos - 36;
        //printf("SeekHead at %llu\n",(unsigned long long) SeekHeadPos);
        /* write seekHead */
        write_SeekHead();
        /* move to end of file */
        myfile.seekp(0, ios_base::end);
        INT64 lLastPos = myfile.tellp();
        INT64 seg_size = lLastPos - segment_size_ptr;
        seg_size |= 0x0100000000000000LL;
        /* move to segment entry */
        myfile.seekp(segment_size_ptr, ios_base::beg);
        if (mk_writeSegPos (root, seg_size ) < 0 || mk_flushContextData(root) < 0)
            ret = -1;
        /* move to seekentries */
        myfile.seekp(seekhead_pos, ios_base::beg);
        write_SegSeek (CuesPos, SeekHeadPos);
        /* move to default video frame duration entry and set the correct value*/
        myfile.seekp(def_duration_ptr, ios_base::beg);
        if ((mk_writeUIntRaw(root, MATROSKA_ID_TRACKDEFAULTDURATION, def_duration)) < 0 ||
            mk_flushContextData(root) < 0)
                ret = -1;

        /* move to segment duration entry */
        myfile.seekp(duration_ptr, ios_base::beg);
        if (mk_writeFloatRaw(root, (float)(double)(max_frame_tc/ timescale)) < 0 ||
            mk_flushContextData(root) < 0)
                ret = -1;
    }
    mk_destroyContexts();
    cluster_pos.clear();

    cout << "closed matroska file\n";
};

bool GVMatroska::failed()
{
    return wrote_header;
}

mk_Context* GVMatroska::mk_createContext(mk_Context *parent, unsigned id)
{
    mk_Context *c;

    if (freelist)
    {
        c = freelist;
        freelist = freelist->next;
    }
    else 
    {
        c = new mk_Context;
    }

    if (!c)
        return NULL;

    c->parent = parent;
    c->id = id;

    if (actlist)
        actlist->prev = &c->next;
    c->next = actlist;
    c->prev = &actlist;
    actlist = c;

    return c;
}

int GVMatroska::mk_appendContextData(mk_Context *c, const void *data, unsigned size)
{
    if (!size) return 0;

    unsigned  ns = c->d_cur + size;
    if (ns > c->d_max)
    {
        void *dp;
        unsigned  dn = c->d_max ? c->d_max << 1 : 16;
        while (ns > dn)
            dn <<= 1;

        dp = realloc(c->data, dn * sizeof(UINT8));
        if (dp == NULL)
            return -1;

        c->data = dp;
        c->d_max = dn;
    }

    memcpy((UINT8*)c->data + c->d_cur, data, size);
    c->d_cur = ns;

    return 0;
}

int GVMatroska::mk_writeID(mk_Context *c, unsigned id)
{
    UINT8 c_id[4] = { id >> 24, id >> 16, id >> 8, id };
    if (c_id[0])
        return mk_appendContextData(c, c_id, 4);
    if (c_id[1])
        return mk_appendContextData(c, c_id+1, 3);
    if (c_id[2])
        return mk_appendContextData(c, c_id+2, 2);
    return mk_appendContextData(c, c_id+3, 1);
}

int GVMatroska::mk_writeSize(mk_Context *c, unsigned size)
{
    UINT8 c_size[5] = { 0x08, size >> 24, size >> 16, size >> 8, size };

    if (size < 0x7f)
    {
        c_size[4] |= 0x80;
        return mk_appendContextData(c, c_size+4, 1);
    }
    if (size < 0x3fff)
    {
        c_size[3] |= 0x40;
        return mk_appendContextData(c, c_size+3, 2);
    }
    if (size < 0x1fffff)
    {
        c_size[2] |= 0x20;
        return mk_appendContextData(c, c_size+2, 3);
    }
    if (size < 0x0fffffff)
    {
        c_size[1] |= 0x10;
        return mk_appendContextData(c, c_size+1, 4);
    }
    return mk_appendContextData(c, c_size, 5);
}

int GVMatroska::mk_flushContextData(mk_Context *c)
{
    if (c->d_cur == 0)
        return 0;

    if (c->parent)
        CHECK(mk_appendContextData(c->parent, c->data, c->d_cur));
    else if (myfile.is_open())
        myfile.write((char *) c->data, c->d_cur);
    else
        return (-1);
    c->d_cur = 0;

    return 0;
}

int GVMatroska::mk_closeContext(mk_Context *c, unsigned *off)
{
    if (c->id)
    {
        CHECK(mk_writeID(c->parent, c->id));
        CHECK(mk_writeSize(c->parent, c->d_cur));
    }

    if (c->parent && off != NULL)
        *off += c->parent->d_cur;

    CHECK(mk_flushContextData(c));

    if (c->next)
        c->next->prev = c->prev;
    *(c->prev) = c->next;
    c->next = freelist;
    freelist = c;

    return 0;
}

void GVMatroska::mk_destroyContexts()
{
    mk_Context  *cur, *next;

    for (cur = freelist; cur; cur = next)
    {
        next = cur->next;
        free(cur->data);
        delete cur;
    }

    for (cur = actlist; cur; cur = next)
    {
        next = cur->next;
        free(cur->data);
        delete cur;
    }

    freelist = actlist = root = NULL;
}

int GVMatroska::mk_writeStr(mk_Context *c, unsigned id, const char *str)
{
    size_t  len = strlen(str);

    CHECK(mk_writeID(c, id));
    CHECK(mk_writeSize(c, len));
    CHECK(mk_appendContextData(c, str, len));
    return 0;
}

int GVMatroska::mk_writeBin(mk_Context *c, unsigned id, const void *data, unsigned size)
{
    CHECK(mk_writeID(c, id));
    CHECK(mk_writeSize(c, size));
    CHECK(mk_appendContextData(c, data, size));

    return 0;
}

int GVMatroska::mk_writeVoid(mk_Context *c, unsigned size)
{
    UINT8 EbmlVoid = 0x00;
    int i=0;
    CHECK(mk_writeID(c, EBML_ID_VOID));
    CHECK(mk_writeSize(c, size));
    for (i=0; i<size; i++)
        CHECK(mk_appendContextData(c, &EbmlVoid, 1));
    return 0;
}

int GVMatroska::mk_writeUIntRaw(mk_Context *c, unsigned id, INT64 ui)
{
    UINT8 c_ui[8] = { ui >> 56, ui >> 48, ui >> 40, ui >> 32, ui >> 24, ui >> 16, ui >> 8, ui };

    CHECK(mk_writeID(c, id));
    CHECK(mk_writeSize(c, 8));
    CHECK(mk_appendContextData(c, c_ui, 8));
    return 0;
}

int GVMatroska::mk_writeUInt(mk_Context *c, unsigned id, INT64 ui)
{
    UINT8 c_ui[8] = { ui >> 56, ui >> 48, ui >> 40, ui >> 32, ui >> 24, ui >> 16, ui >> 8, ui };
    unsigned i = 0;

    CHECK(mk_writeID(c, id));
    while (i < 7 && c_ui[i] == 0)
        ++i;
    CHECK(mk_writeSize(c, 8 - i));
    CHECK(mk_appendContextData(c, c_ui+i, 8 - i));
    return 0;
}

int GVMatroska::mk_writeSegPos(mk_Context *c, INT64 ui)
{
    UINT8 c_ui[8] = { ui >> 56, ui >> 48, ui >> 40, ui >> 32, ui >> 24, ui >> 16, ui >> 8, ui };

    CHECK(mk_appendContextData(c, c_ui, 8 ));
    return 0;
}

int GVMatroska::mk_writeFloatRaw(mk_Context *c, float f)
{
    union
    {
        float f;
        unsigned u;
    } u;
    UINT8 c_f[4];

    u.f = f;
    c_f[0] = (UINT8) u.u >> 24;
    c_f[1] = (UINT8) u.u >> 16;
    c_f[2] = (UINT8) u.u >> 8;
    c_f[3] = (UINT8) u.u;

    return mk_appendContextData(c, c_f, 4);
}

int GVMatroska::mk_writeFloat(mk_Context *c, unsigned id, float f)
{
    CHECK(mk_writeID(c, id));
    CHECK(mk_writeSize(c, 4));
    CHECK(mk_writeFloatRaw(c, f));
    return 0;
}

unsigned GVMatroska::mk_ebmlSizeSize(unsigned s)
{
    if (s < 0x7f)
        return 1;
    if (s < 0x3fff)
        return 2;
    if (s < 0x1fffff)
        return 3;
    if (s < 0x0fffffff)
        return 4;
    return 5;
}

int GVMatroska::mk_closeCluster()
{
    if (cluster == NULL)
        return 0;
    CHECK(mk_closeContext(cluster, 0));
    cluster = NULL;
    CHECK(mk_flushContextData(root));
    cluster_index++;
    INT64 fpos = myfile.tellp();
    cluster_pos.push_back((INT64) fpos - 36);
    return 0;
}

int GVMatroska::mk_flushFrame()
{
    //int64_t ref = 0;
    INT64 delta = 0;
    unsigned fsize, bgsize;
    unsigned char c_delta_flags[3];

    if (!in_frame)
        return 0;

    delta = frame_tc / timescale - cluster_tc_scaled;

    if (cluster == NULL) 
    {
        cluster_tc_scaled = frame_tc / timescale ;
        cluster = mk_createContext(root, MATROSKA_ID_CLUSTER); /* New Cluster */
        if (cluster == NULL)
            return -1;

        CHECK(mk_writeUInt(cluster, MATROSKA_ID_CLUSTERTIMECODE, cluster_tc_scaled)); /* Timecode */

        delta = 0;
        block_n=0;
    }

    fsize = frame ? frame->d_cur : 0;
    bgsize = fsize + 4 + mk_ebmlSizeSize(fsize + 4) + 1;

    CHECK(mk_writeID(cluster, MATROSKA_ID_BLOCKGROUP)); /* BlockGroup */
    CHECK(mk_writeSize(cluster, bgsize));
    CHECK(mk_writeID(cluster, MATROSKA_ID_BLOCK)); /* Block */
    block_n++;
    CHECK(mk_writeSize(cluster, fsize + 4));
    CHECK(mk_writeSize(cluster, 1)); /* track number (for guvcview 1-video  2-audio) */

    c_delta_flags[0] = delta >> 8;
    c_delta_flags[1] = delta;
    c_delta_flags[2] = 0;
    CHECK(mk_appendContextData(cluster, c_delta_flags, 3)); /* block timecode */
    if (frame)
    {
        CHECK(mk_appendContextData(cluster, frame->data, frame->d_cur));
        frame->d_cur = 0;
    }
    //if (!w->keyframe)
    //    CHECK(mk_writeSInt(w->cluster, MATROSKA_ID_BLOCKREFERENCE, ref)); /* ReferenceBlock */

    in_frame = false; /* current frame processed */

    prev_frame_tc_scaled = cluster_tc_scaled + delta;

    /*******************************/
    if (delta > 32767ll || delta < -32768ll || (cluster->d_cur) > CLSIZE)
    {
        CHECK(mk_closeCluster());
    }
    /*******************************/

    return 0;
}

int GVMatroska::mk_flushAudioFrame()
{
    INT64 delta = 0;
    unsigned fsize, bgsize;
    unsigned char c_delta_flags[3];
    //unsigned char flags = 0x04; //lacing
    //unsigned char framesinlace = 0x07; //FIXME:  total frames -1

    delta = audio_frame_tc / timescale - cluster_tc_scaled;
    /* make sure we have a cluster */
    if (cluster == NULL) 
    {
        cluster_tc_scaled = audio_frame_tc / timescale ;
        cluster = mk_createContext(root, MATROSKA_ID_CLUSTER); /* Cluster */
        if (cluster == NULL)
            return -1;

        CHECK(mk_writeUInt(cluster, MATROSKA_ID_CLUSTERTIMECODE, cluster_tc_scaled)); /* Timecode */

        delta = 0;
        block_n=0;
    }

    if (!audio_in_frame)
        return 0;

    fsize = audio_frame ? audio_frame->d_cur : 0;
    bgsize = fsize + 4 + mk_ebmlSizeSize(fsize + 4) + 1;
    CHECK(mk_writeID(cluster, MATROSKA_ID_BLOCKGROUP)); /* BlockGroup */
    CHECK(mk_writeSize(cluster, bgsize));
    CHECK(mk_writeID(cluster, MATROSKA_ID_BLOCK)); /* Block */
    block_n++;
    if(audio_block == 0) 
    {
        audio_block = block_n;
    }
    CHECK(mk_writeSize(cluster, fsize + 4));
    CHECK(mk_writeSize(cluster, 2)); /* track number (1-video  2-audio) */
    c_delta_flags[0] = delta >> 8;
    c_delta_flags[1] = delta;
    c_delta_flags[2] = 0;
    CHECK(mk_appendContextData(cluster, c_delta_flags, 3)); /* block timecode (delta to cluster tc) */
    if (audio_frame)
    {
        CHECK(mk_appendContextData(cluster, audio_frame->data, audio_frame->d_cur));
        audio_frame->d_cur = 0;
    }

    audio_in_frame = false; /* current frame processed */

    audio_prev_frame_tc_scaled = cluster_tc_scaled + delta;

    /*******************************/
    if (delta > 32767ll || delta < -32768ll || (cluster->d_cur) > CLSIZE)
    {
        CHECK(mk_closeCluster());
    }
    /*******************************/

    return 0;
}

int GVMatroska::write_cues()
{
    mk_Context *cpe, *tpe;
    //printf("writng cues\n");
    cues = mk_createContext(root, MATROSKA_ID_CUES); /* Cues */
    if (cues == NULL)
        return -1;
    cpe = mk_createContext(cues, MATROSKA_ID_POINTENTRY); /* Cues */
    if (cpe == NULL)
        return -1;
    CHECK(mk_writeUInt(cpe, MATROSKA_ID_CUETIME, 0)); /* Cue Time */
    tpe = mk_createContext(cpe, MATROSKA_ID_CUETRACKPOSITION); /* track position */
    if (tpe == NULL)
        return -1;
    CHECK(mk_writeUInt(tpe, MATROSKA_ID_CUETRACK, 1)); /* Cue track video */
    CHECK(mk_writeUInt(tpe, MATROSKA_ID_CUECLUSTERPOSITION, cluster_pos[0]));
    CHECK(mk_writeUInt(tpe, MATROSKA_ID_CUEBLOCKNUMBER, 1));
    CHECK(mk_closeContext(tpe, 0));
    if(!(video_only) && (audio_block > 0))
    {
        tpe = mk_createContext(cpe, MATROSKA_ID_CUETRACKPOSITION); /* track position */
        if (tpe == NULL)
            return -1;
        CHECK(mk_writeUInt(tpe, MATROSKA_ID_CUETRACK, 2)); /* Cue track audio */
        CHECK(mk_writeUInt(tpe, MATROSKA_ID_CUECLUSTERPOSITION, cluster_pos[0]));
        CHECK(mk_writeUInt(tpe, MATROSKA_ID_CUEBLOCKNUMBER, audio_block));
        CHECK(mk_closeContext(tpe, 0));
    }
    CHECK(mk_closeContext(cpe, 0));
    CHECK(mk_closeContext(cues, 0));
    if(mk_flushContextData(root) < 0) 
        return -1;
    return 0;
}

int GVMatroska::write_SeekHead()
{
    mk_Context *sk,*se;
    int i=0;

    if ((sk = mk_createContext(root, MATROSKA_ID_SEEKHEAD)) == NULL) /* SeekHead */
        return -1;
    for(i=0; i < cluster_pos.size(); i++)
    {
        if ((se = mk_createContext(sk, MATROSKA_ID_SEEKENTRY)) == NULL) /* Seek */
            return -1;
        CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKID, MATROSKA_ID_CLUSTER));  /* seekID */
        CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKPOSITION, cluster_pos[i])); /* FIXME:  SeekPosition (should not be hardcoded) */
        CHECK(mk_closeContext(se, 0));
    }
    CHECK(mk_closeContext(sk, 0));
    if(mk_flushContextData(root) < 0) 
        return -1;
    return 0;
}

int GVMatroska::write_SegSeek(INT64 cues_pos, INT64 seekHeadPos)
{
    mk_Context *sh, *se;

    if ((sh = mk_createContext(root, MATROSKA_ID_SEEKHEAD)) == NULL) /* SeekHead */
        return -1;
    if ((se = mk_createContext(sh, MATROSKA_ID_SEEKENTRY)) == NULL)     /* Seek */
        return -1;
    CHECK(mk_writeUInt (se, MATROSKA_ID_SEEKID, MATROSKA_ID_INFO));        /* seekID */
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKPOSITION, 4135-seekhead_pos));
    CHECK(mk_closeContext(se, 0));

    if ((se = mk_createContext(sh, MATROSKA_ID_SEEKENTRY)) == NULL)     /* Seek */
        return -1;
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKID, MATROSKA_ID_TRACKS));       /* seekID */
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKPOSITION, 4220-seekhead_pos));
    CHECK(mk_closeContext(se, 0));

    //printf("cues@%d seekHead@%d\n", cues_pos, seekHeadPos);
    if ((se = mk_createContext(sh, MATROSKA_ID_SEEKENTRY)) == NULL) /* Seek */
        return -1;
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKID, MATROSKA_ID_SEEKHEAD)); /* seekID */
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKPOSITION, seekHeadPos)); 
    CHECK(mk_closeContext(se, 0));
    if ((se = mk_createContext(sh, MATROSKA_ID_SEEKENTRY)) == NULL) /* Seek */
        return -1;
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKID, MATROSKA_ID_CUES));  /* seekID */
    CHECK(mk_writeUInt(se, MATROSKA_ID_SEEKPOSITION, cues_pos)); 
    CHECK(mk_closeContext(se, 0));
    CHECK(mk_closeContext(sh, 0));

    if(mk_flushContextData(root) < 0) 
    return -1;
    long fpos = myfile.tellp();
    CHECK(mk_writeVoid(root, 4135 - (fpos + 3)));
    if(mk_flushContextData(root) < 0) 
        return -1;
    return 0;
}

void GVMatroska::mk_setDef_Duration(UINT64 _def_duration)
{
    def_duration = _def_duration;
}

int GVMatroska::mk_startFrame()
{
    if (mk_flushFrame() < 0)
        return -1;

    in_frame = true; /*first frame will have size zero (don't write it)*/
    keyframe = false;

    return 0;
}

int GVMatroska::mk_startAudioFrame()
{
    if (mk_flushAudioFrame() < 0)
        return -1;

    return 0;
}

int GVMatroska::mk_setFrameFlags(INT64 timestamp, int keyframe)
{
    if (!in_frame)
        return -1;

    frame_tc = timestamp;
    keyframe = (keyframe != 0);

    if (max_frame_tc < timestamp)
        max_frame_tc = timestamp;

    return 0;
}

int GVMatroska::mk_setAudioFrameFlags(INT64 timestamp, int keyframe)
{
    if (!audio_in_frame)
        return -1;

    audio_frame_tc = timestamp;

    return 0;
}

int GVMatroska::mk_addFrameData(const void *data, unsigned size)
{

    if (frame == NULL)
        if ((frame = mk_createContext(NULL, 0)) == NULL)
            return -1;

    in_frame = true;

    return mk_appendContextData(frame, data, size);
}

int GVMatroska::mk_addAudioFrameData(const void *data, unsigned size)
{

    if (audio_frame == NULL)
        if ((audio_frame = mk_createContext(NULL, 0)) == NULL)
            return -1;

    audio_in_frame = true;

    return mk_appendContextData(audio_frame, data, size);
}

