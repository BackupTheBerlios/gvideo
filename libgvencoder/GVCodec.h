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

class GVCodec
{
    unsigned codec_index;
    AVCodec *vcodec;
    AVCodecContext *vcodec_context;
    AVFrame *picture;
    GVJpeg* jpeg;
    
    UINT8 *tmpbuf;
    UINT8 *out;
    unsigned _buff_size;
    
    int _fps_num;
    int _fps_denom;
        
    BMPINFOHEADER mkv_codecPriv;
    
    void fill_frame(UINT8* yuv422);
    
  public:
    GVCodec(int width, int height, int fps_denom=15, int fps_num=1);
    ~GVCodec();
    std::vector<GVCodec_vdata> vcodec_list;
    unsigned get_real_vcodec_index (unsigned codec_ind);
    bool open_vcodec(unsigned codec_ind, UINT8 *out_buff, unsigned buff_size);
    int encode_video_frame (UINT8 *in_buf);
};


END_LIBGVENCODER_NAMESPACE

#endif
