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
#  GVCodec class : gvideo libavcodec compressor class                           #
#                                                                               #
********************************************************************************/

#ifndef GVCODEC_H
#define GVCODEC_H

#include "gvcommon.h"
#include "libgvencoder/GVJpeg_encoder.h"
extern "C" {
/*for older libavcodec versions use:*/
/*#include <avcodec.h>*/
#include "libavcodec/avcodec.h"
}
#include <vector>

START_LIBGVENCODER_NAMESPACE

/* Audio formats (fourcc)*/
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IEEE_FLOAT          (0x0003)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_DSP_TRUESPEECH      (0x0022)
#define WAVE_FORMAT_GSM610              (0x0031)
#define WAVE_FORMAT_MP3                 (0x0055)
#define WAVE_FORMAT_MPEG12              (0x0050)
#define WAVE_FORMAT_AAC                 (0x00ff)
#define WAVE_FORMAT_IBM_MULAW           (0x0101)
#define WAVE_FORMAT_IBM_ALAW            (0x0102)
#define WAVE_FORMAT_IBM_ADPCM           (0x0103)
#define WAVE_FORMAT_AC3                 (0x2000)
/*extra audio formats (codecs)*/
#define ANTEX_FORMAT_ADPCME             (0x0033)
#define AUDIO_FORMAT_APTX               (0x0025)
#define AUDIOFILE_FORMAT_AF10           (0x0026)
#define AUDIOFILE_FORMAT_AF36           (0x0024) 
#define BROOKTREE_FORMAT_BTVD           (0x0400)
#define CANOPUS_FORMAT_ATRAC            (0x0063)
#define CIRRUS_FORMAT_CIRRUS            (0x0060)
#define CONTROL_FORMAT_CR10             (0x0037)
#define CONTROL_FORMAT_VQLPC            (0x0034)
#define CREATIVE_FORMAT_ADPCM           (0x0200)
#define CREATIVE_FORMAT_FASTSPEECH10    (0x0203)
#define CREATIVE_FORMAT_FASTSPEECH8     (0x0202)
#define IMA_FORMAT_ADPCM                (0x0039)
#define CONSISTENT_FORMAT_CS2           (0x0260)
#define HP_FORMAT_CU                    (0x0019)
#define DEC_FORMAT_G723                 (0x0123)
#define DF_FORMAT_G726                  (0x0085)
#define DSP_FORMAT_ADPCM                (0x0036)
#define DOLBY_FORMAT_AC2                (0x0030)
#define DOLBY_FORMAT_AC3_SPDIF          (0x0092)
#define ESS_FORMAT_ESPCM                (0x0061)
#define IEEE_FORMAT_FLOAT               (0x0003)
#define MS_FORMAT_MSAUDIO1_DIVX         (0x0160)
#define MS_FORMAT_MSAUDIO2_DIVX         (0x0161)
#define OGG_FORMAT_VORBIS1              (0x674f)
#define OGG_FORMAT_VORBIS1P             (0x676f)
#define OGG_FORMAT_VORBIS2              (0x6750)
#define OGG_FORMAT_VORBIS2P             (0x6770)
#define OGG_FORMAT_VORBIS3              (0x6751)
#define OGG_FORMAT_VORBIS3P             (0x6771)
#define MS_FORMAT_WMA9                  (0x0163)
#define MS_FORMAT_WMA9_PRO              (0x0162)

struct BMPINFOHEADER
{
    UINT32   biSize; 
    UINT32   biWidth; 
    UINT32   biHeight; 
    UINT16   biPlanes; 
    UINT16   biBitCount; 
    UINT32   biCompression; 
    UINT32   biSizeImage; 
    UINT32   biXPelsPerMeter; 
    UINT32   biYPelsPerMeter; 
    UINT32   biClrUsed; 
    UINT32   biClrImportant; 
}  __attribute__ ((packed));

class GVCodec_vdata
{
  public:
    bool islavc;              //is a lavc codec or a gvideo builtin (MJPG and YUYV)
    bool valid;               //the encoding codec exists in ffmpeg
    const char* compressor;   //fourcc - upper case
    int fourcc;               //fourcc WORD value
    const char* mkv_codec;    //mkv codecID
    void* codecPriv;          //mkv codec private data
    int codecPriv_size;       //mkv codec private data size (usually 40)
    const char* description;  //codec description
    int fps;                  // encoder frame rate (used for time base)
    int bit_rate;             //lavc default bit rate
    int qmax;                 //lavc qmax
    int qmin;                 //lavc qmin
    int max_qdiff;            //lavc qmin
    int dia;                  //lavc dia_size
    int pre_dia;              //lavc pre_dia_size
    int pre_me;               //lavc pre_me
    int me_pre_cmp;           //lavc me_pre_cmp
    int me_cmp;               //lavc me_cmp
    int me_sub_cmp;           //lavc me_sub_cmp
    int last_pred;            //lavc last_predictor_count
    int gop_size;             //lavc gop_size
    float qcompress;          //lavc qcompress
    float qblur;              //lavc qblur
    int subq;                 //lavc subq
    int framerefs;            //lavc refs
    enum CodecID codec_id;    //lavc codec_id
    int mb_decision;          //lavc mb_decision
    int trellis;              //lavc trellis quantization
    int me_method;            //lavc motion estimation method
    int mpeg_quant;           //lavc mpeg quantization
    int max_b_frames;         //lavc max b frames
    int flags;                //lavc flags
};

class GVCodec_adata
{
  public:
    bool islavc;              //is a avcodec codec
    bool valid;               //the encoding codec exists in ffmpeg
    int bits;                 //bits per sample (pcm only) 
    int fourcc;               //fourcc WORD value
    const char *mkv_codec;    //mkv codecID
    const char *description;  //codec description
    int bit_rate;             //lavc default bit rate
    enum CodecID codec_id;    //lavc codec_id
    int profile;              //for AAC only
    void *codecPriv;          //pointer for codec private data
    int codecPriv_size;       //size in bytes of private data
    int flags;                //lavc flags
};

class GVCodec
{
    unsigned vcodec_index;
    AVCodec *vcodec;
    AVCodecContext *vcodec_context;
    AVFrame *picture;
    GVJpeg* jpeg;
    
    unsigned acodec_index;
    AVCodec *acodec;
    AVCodecContext *acodec_context;
    int audio_frame_size; /*frame size in samples*/
    
    UINT8 *tmpbuf;
    UINT8 *video_out;
    unsigned _video_buff_size;
    UINT8 *audio_out;
    unsigned _audio_buff_size;
    
    /*mkv codec privated data*/
    /*video codecs*/
    BMPINFOHEADER mkv_codecPriv;
    /*audio codecs - AAC*/
    /* AAC object types index: MAIN = 1; LOW = 2; SSR = 3; LTP = 4*/
    int AAC_OBJ_TYPE[5]; 
    /*-1 = reserved; 0 = freq. is writen explictly (increases header by 24 bits)*/
    int AAC_SAMP_FREQ[16];
    /*NORMAL AAC HEADER*/
    /*2 bytes: object type index(5 bits) + sample frequency index(4bits) + channels(4 bits) + flags(3 bit) */
    /*default = MAIN(1)+44100(4)+stereo(2)+flags(0) = 0x0A10*/
    UINT8 AAC_ESDS[2];
    /* if samprate index == 15 AAC_ESDS[5]: 
     * object type index(5 bits) + sample frequency index(4bits) + samprate(24bits) + channels(4 bits) + flags(3 bit)
     */
    
    void fill_frame(UINT8* yuv422);
    int get_aac_obj_ind(int profile);
    int get_aac_samp_ind(int samprate);
    
  public:
    GVCodec();
    ~GVCodec();
    std::vector<GVCodec_vdata> vcodec_list;
    std::vector<GVCodec_adata> acodec_list;
    unsigned get_real_vcodec_index (unsigned codec_ind);
    void close_codecs(); // calls both close_vcodec and close_acodec
    void close_vcodec();
    void close_acodec();
    bool open_vcodec(unsigned codec_ind, UINT8 *out_buff, 
                     unsigned buff_size, 
                     int width, 
                     int height, 
                     int fps_denom=15, 
                     int fps_num=1);
    int encode_video_frame (UINT8 *in_buf);
    int open_acodec(unsigned codec_ind, 
                    UINT8 *out_buff, 
                    unsigned buff_size,
                    int samprate, 
                    int channels);
    int encode_audio_frame (INT16 *in_buf);
};


END_LIBGVENCODER_NAMESPACE

#endif
