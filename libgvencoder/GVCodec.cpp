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

#include <libgvencoder/GVCodec.h>
#include <string.h>
#include <iostream>

START_LIBGVENCODER_NAMESPACE

GVCodec::GVCodec()
{
    // must be called before using avcodec lib
    avcodec_init();

    // register all the codecs (you can also register only the codec
    //you wish to have smaller code
    avcodec_register_all();

    vcodec_context = NULL;
    vcodec = NULL;
    picture= NULL;
    jpeg = NULL;
    tmpbuf = NULL;
    video_out = NULL;
    audio_out = NULL;
    // set the default codec index to 0
    vcodec_index = 0;
    acodec_index = 0;
    audio_frame_size = 0;
    _video_buff_size = 0;
    _audio_buff_size = 0;
    
    acodec_context = NULL;
    
    /*video codecs*/
    mkv_codecPriv.biSize = 0x00000028; //40 bytes 
    mkv_codecPriv.biWidth = 640;       //default values 
    mkv_codecPriv.biHeight = 480; 
    mkv_codecPriv.biPlanes = 1; 
    mkv_codecPriv.biBitCount = 24; 
    mkv_codecPriv.biCompression = V4L2_PIX_FMT_MJPEG; 
    mkv_codecPriv.biSizeImage = 640*480*2; //2 bytes per pixel (max buffer - use x3 for RGB)
    mkv_codecPriv.biXPelsPerMeter = 0; 
    mkv_codecPriv.biYPelsPerMeter = 0; 
    mkv_codecPriv.biClrUsed = 0; 
    mkv_codecPriv.biClrImportant = 0;

    vcodec_list.push_back( (GVCodec_vdata)
    {
        false, true, "MJPG", v4l2_fourcc('M','J','P','G'), "V_MS/VFW/FOURCC", &mkv_codecPriv, 40,
        "MJPG - compressed", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        CODEC_ID_MJPEG, 0, 0, 0, 0, 0, 0
    } );
    vcodec_list.push_back( (GVCodec_vdata)
    {
        false, true, "YUY2", v4l2_fourcc('Y','U','Y','2'), "V_MS/VFW/FOURCC", &mkv_codecPriv, 40,
        "YUY2 - uncomp YUV", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        CODEC_ID_NONE, 0, 0, 0, 0, 0, 0
    } );
    
    if(avcodec_find_encoder(CODEC_ID_MPEG1VIDEO))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "MPEG", v4l2_fourcc('M','P','E','G'), "V_MPEG1", NULL, 0,
            "MPEG video 1", 30, 3000000, 8, 2, 2, 2, 2, 2, 0, 3, 3, 2, 12, 0.5, 0.5, 0, 0,
            CODEC_ID_MPEG1VIDEO, FF_MB_DECISION_RD, 1, ME_EPZS, 0, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_MPEG1VIDEO missing\n";
    if(avcodec_find_encoder(CODEC_ID_FLV1))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "FLV1", v4l2_fourcc('F','L','V','1'), "V_MS/VFW/FOURCC", &mkv_codecPriv, 40,
            "FLV1 - flash video 1", 0, 3000000, 31, 2, 3, 2, 2, 2, 0, 3, 3, 2, 100, 0.5, 0.5, 0, 0,
            CODEC_ID_FLV1, FF_MB_DECISION_RD, 1, ME_EPZS, 0, 0, CODEC_FLAG_4MV
      } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_FLV1 missing\n";
    if(avcodec_find_encoder(CODEC_ID_WMV1))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "WMV1", v4l2_fourcc('W','M','V','1'), "V_MS/VFW/FOURCC", &mkv_codecPriv, 40,
            "WMV1 - win. med. video 7", 0, 3000000, 8, 2, 2, 2, 2, 2, 0, 3, 3, 2, 100, 0.5, 0.5, 0, 0,
            CODEC_ID_WMV1, FF_MB_DECISION_RD, 1, ME_EPZS, 0, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_WMV1 missing\n";
    if(avcodec_find_encoder(CODEC_ID_MPEG2VIDEO))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "MPG2", v4l2_fourcc('M','P','G','2'), "V_MPEG2", NULL, 0,
            "MPG2 - MPG2 format", 30, 3000000, 31, 2, 3, 2, 2, 2, 0, 3, 3, 2, 12, 0.5, 0.5, 0, 0,
            CODEC_ID_MPEG2VIDEO, FF_MB_DECISION_RD, 1, ME_EPZS, 0, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_MPEG2VIDEO missing\n";
    if(avcodec_find_encoder(CODEC_ID_MSMPEG4V3))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "MP43", v4l2_fourcc('M','P','4','3'), "V_MPEG4/MS/V3", NULL, 0,
            "MS MP4 V3", 0, 3000000, 31, 2, 3, 2, 2, 2, 0, 3, 3, 2, 100, 0.5, 0.5, 0, 0,
            CODEC_ID_MSMPEG4V3, FF_MB_DECISION_RD, 1, ME_EPZS, 0, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_MSMPEG4V3 missing\n";
    if(avcodec_find_encoder(CODEC_ID_MPEG4))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "DX50", v4l2_fourcc('D','X','5','0'), "V_MPEG4/ISO/ASP", NULL, 0,
            "MPEG4-ASP", 0, 1500000, 31, 2, 3, 2, 2, 2, 0, 3, 3, 2, 100, 0.5, 0.5, 0, 0,
            CODEC_ID_MPEG4, FF_MB_DECISION_RD, 1, ME_EPZS, 1, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_MPG4 missing\n";
    if(avcodec_find_encoder(CODEC_ID_H264))
        vcodec_list.push_back( (GVCodec_vdata)
        {
            true, true, "H264", v4l2_fourcc('H','2','6','4'), "V_MPEG4/ISO/AVC", NULL, 0,
            "MPEG4-AVC (H264)", 0, 1500000, 31, 2, 3, 2, 2, 2, 0, 3, 3, 2, 100, 0.7, 0.5, 5, 2,
            CODEC_ID_H264, FF_MB_DECISION_RD, 1, ME_HEX, 1, 2, 
            CODEC_FLAG2_BPYRAMID | CODEC_FLAG2_WPRED | CODEC_FLAG2_FASTPSKIP
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_H264 missing\n";
    
    /*Audio codecs*/
    AAC_OBJ_TYPE[0]=FF_PROFILE_UNKNOWN;
    AAC_OBJ_TYPE[1]=FF_PROFILE_AAC_MAIN;
    AAC_OBJ_TYPE[2]=FF_PROFILE_AAC_LOW;
    AAC_OBJ_TYPE[3]=FF_PROFILE_AAC_SSR;
    AAC_OBJ_TYPE[4]=FF_PROFILE_AAC_LTP;
    
    AAC_SAMP_FREQ[0]= 96000;
    AAC_SAMP_FREQ[1]= 88200;
    AAC_SAMP_FREQ[2]= 64000;
    AAC_SAMP_FREQ[3]= 48000;
    AAC_SAMP_FREQ[4]= 44100;
    AAC_SAMP_FREQ[5]= 32000;
    AAC_SAMP_FREQ[6]= 24000;
    AAC_SAMP_FREQ[7]= 22050;
    AAC_SAMP_FREQ[8]= 16000;
    AAC_SAMP_FREQ[9]= 12000;
    AAC_SAMP_FREQ[10]= 11025;
    AAC_SAMP_FREQ[11]= 8000;
    AAC_SAMP_FREQ[12]= 7350;
    AAC_SAMP_FREQ[13]= -1;
    AAC_SAMP_FREQ[14]= -1;
    AAC_SAMP_FREQ[15]= 0;
    
    AAC_ESDS[0]= 0x0A;
    AAC_ESDS[1]= 0x10;
    
    acodec_list.push_back( (GVCodec_adata)
    {
        false, true, 16, WAVE_FORMAT_PCM, "A_PCM/INT/LIT", "PCM - uncompressed (16 bit)",
        0, CODEC_ID_PCM_S16LE, FF_PROFILE_UNKNOWN, NULL, 0, 0
    } );
    if(avcodec_find_encoder(CODEC_ID_MP2))
        acodec_list.push_back( (GVCodec_adata)
        {
            true, true, 0, WAVE_FORMAT_MPEG12, "A_MPEG/L2", "MPEG2 - (lavc)",
            160000, CODEC_ID_MP2, FF_PROFILE_UNKNOWN, NULL, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_MP2 missing\n";
    if(avcodec_find_encoder(CODEC_ID_MP3))
        acodec_list.push_back( (GVCodec_adata)
        {
            true, true, 0, WAVE_FORMAT_MP3, "A_MPEG/L3", "MPEG3 - (lavc)",
            160000, CODEC_ID_MP3, FF_PROFILE_UNKNOWN, NULL, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_MP3 missing\n";
    if(avcodec_find_encoder(CODEC_ID_AC3))
        acodec_list.push_back( (GVCodec_adata)
        {
            true, true, 0, WAVE_FORMAT_AC3, "A_AC3", "Dolby AC3 - (lavc)",
            160000, CODEC_ID_AC3, FF_PROFILE_UNKNOWN, NULL, 0, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_AC3 missing\n";
    if(avcodec_find_encoder(CODEC_ID_AAC))
        acodec_list.push_back( (GVCodec_adata)
        {
            true, true, 16, WAVE_FORMAT_AAC, "A_AAC", "ACC Low - (faac)",
            64000, CODEC_ID_AAC, FF_PROFILE_AAC_LOW, AAC_ESDS, 2, 0
        } );
    else
        std::cerr << "libgvencoder: ffmpeg CODEC_ID_AAC missing\n";
}

GVCodec::~GVCodec()
{
    close_codecs();
}

void GVCodec::close_codecs()
{
    close_vcodec();
    close_acodec();
}

void GVCodec::close_vcodec()
{
    if(vcodec_context)
    {
        avcodec_flush_buffers(vcodec_context);
        //close codec 
        avcodec_close(vcodec_context);
        //free codec context 
        free(vcodec_context);
    }
    
    if(picture) free(picture);
    picture = NULL;
    if(tmpbuf) delete[] tmpbuf;
    tmpbuf = NULL;
    
    vcodec_context = NULL;
}

bool GVCodec::open_vcodec(unsigned codec_ind, 
                          UINT8 *out_buff, 
                          unsigned buff_size,
                          int width,
                          int height,
                          int fps_denom/*=15*/, 
                          int fps_num/*=1*/)
{
    if (codec_ind >= vcodec_list.size())
    {
        std::cerr << "libgvencoder: codec index not valid [0-" << vcodec_list.size()-1 << "]\n";
        return (false);
    }
    
    vcodec_index = codec_ind;
    video_out = out_buff;
    _video_buff_size = buff_size;
    
    /*sets the matroska codec private data if required*/
    if((vcodec_list[vcodec_index].codecPriv != NULL) && (vcodec_list[vcodec_index].codecPriv_size > 0))
    {
        mkv_codecPriv.biWidth = width;
        mkv_codecPriv.biHeight = height;
        mkv_codecPriv.biCompression = vcodec_list[vcodec_index].fourcc;
        if(vcodec_index == 2)
            mkv_codecPriv.biSizeImage = width * height * 3; /*rgb*/
        else
            mkv_codecPriv.biSizeImage = width * height * 2;
    }

    if(vcodec_index == 0)
    {
        jpeg = new GVJpeg(width, height);
    }
    else if (vcodec_index > 1) /* 0 and 1 are built-in codecs*/
    {
        //alloc picture
        picture= avcodec_alloc_frame();
        //alloc tmpbuff (yuv420p)
        tmpbuf = new UINT8 [(width * height*3)/2];
        
        vcodec = avcodec_find_encoder(vcodec_list[vcodec_index].codec_id);
        if (!vcodec) 
        {
            std::cerr << "libgvencoder: " 
                << vcodec_list[vcodec_index].compressor << " codec not found\n";
            return(false);
        }
        
        //alloc context
        vcodec_context = avcodec_alloc_context();
        // resolution must be a multiple of two
        vcodec_context->width = width; 
        vcodec_context->height = height;
        
        vcodec_context->bit_rate = vcodec_list[vcodec_index].bit_rate;
        vcodec_context->flags |= vcodec_list[vcodec_index].flags;
        /* 
         * mb_decision
         *0 (FF_MB_DECISION_SIMPLE) Use mbcmp (default).
         *1 (FF_MB_DECISION_BITS)   Select the MB mode which needs the fewest bits (=vhq).
         *2 (FF_MB_DECISION_RD)     Select the MB mode which has the best rate distortion.
         */
        vcodec_context->mb_decision = vcodec_list[vcodec_index].mb_decision;
        /*use trellis quantization*/
        vcodec_context->trellis = vcodec_list[vcodec_index].trellis;

        //motion estimation method epzs
        vcodec_context->me_method = vcodec_list[vcodec_index].me_method; 

        vcodec_context->dia_size = vcodec_list[vcodec_index].dia;
        vcodec_context->pre_dia_size = vcodec_list[vcodec_index].pre_dia;
        vcodec_context->pre_me = vcodec_list[vcodec_index].pre_me;
        vcodec_context->me_pre_cmp = vcodec_list[vcodec_index].me_pre_cmp;
        vcodec_context->me_cmp = vcodec_list[vcodec_index].me_cmp;
        vcodec_context->me_sub_cmp = vcodec_list[vcodec_index].me_sub_cmp;
        vcodec_context->me_subpel_quality = vcodec_list[vcodec_index].subq; //NEW
        vcodec_context->refs = vcodec_list[vcodec_index].framerefs;         //NEW
        vcodec_context->last_predictor_count = vcodec_list[vcodec_index].last_pred;

        vcodec_context->mpeg_quant = vcodec_list[vcodec_index].mpeg_quant; //h.263
        vcodec_context->qmin = vcodec_list[vcodec_index].qmin;             // best detail allowed - worst compression
        vcodec_context->qmax = vcodec_list[vcodec_index].qmax;             // worst detail allowed - best compression
        vcodec_context->max_qdiff = vcodec_list[vcodec_index].max_qdiff;
        vcodec_context->max_b_frames = vcodec_list[vcodec_index].max_b_frames;
        vcodec_context->gop_size = vcodec_list[vcodec_index].gop_size;

        vcodec_context->qcompress = vcodec_list[vcodec_index].qcompress;
        vcodec_context->qblur = vcodec_list[vcodec_index].qblur;
        vcodec_context->strict_std_compliance = FF_COMPLIANCE_NORMAL;
        vcodec_context->codec_id = vcodec_list[vcodec_index].codec_id;
        vcodec_context->codec_type = CODEC_TYPE_VIDEO;
        vcodec_context->pix_fmt = PIX_FMT_YUV420P; //only yuv420p available for mpeg
        if(vcodec_list[vcodec_index].fps) /*for gscpa must set fps in properties*/
            vcodec_context->time_base = (AVRational){1, vcodec_list[vcodec_index].fps}; //use properties fps
        else if ((fps_num > 0) && (fps_denom > 0))
            vcodec_context->time_base = (AVRational){fps_num, fps_denom};
        else 
            vcodec_context->time_base = (AVRational){1, 15}; // set a default value
            
        // open codec
        if (avcodec_open(vcodec_context, vcodec) < 0) 
        {
            std::cerr << "libgvencoder: could not open video codec\n";
            return(false);
        }
    }
    
    return (true);
}

unsigned GVCodec::get_real_vcodec_index (unsigned codec_ind)
{
    unsigned i = 0;
    unsigned ind = -1;
    for (i=0;i<vcodec_list.size(); i++)
    {
        if(vcodec_list[i].valid)
            ind++;
        if(ind == codec_ind)
            return i;
    }
    return (codec_ind); //should never arrive
}

void GVCodec::fill_frame(UINT8* yuv422)
{
    int i,j;
    int width = vcodec_context->width;
    int height = vcodec_context->height;
    int linesize=width*2;
    int size = width * height;

    UINT8 *y;
    UINT8 *y1;
    UINT8 *u;
    UINT8* v;
    y = tmpbuf;
    y1 = tmpbuf + width;
    u = tmpbuf + size;
    v = u + size/4;

    for(j=0;j<(height-1);j+=2)
    {
        for(i=0;i<(linesize-3);i+=4)
        { 
            *y++ = yuv422[i+j*linesize];
            *y++ = yuv422[i+2+j*linesize]; 
            *y1++ = yuv422[i+(j+1)*linesize]; 
            *y1++ = yuv422[i+2+(j+1)*linesize]; 
            *u++ = (yuv422[i+1+j*linesize] + yuv422[i+1+(j+1)*linesize])>>1; // div by 2 
            *v++ = (yuv422[i+3+j*linesize] + yuv422[i+3+(j+1)*linesize])>>1; 
        } 
        y += width; 
        y1 += width;//2 lines
    }

    picture->data[0] = tmpbuf;                    //Y
    picture->data[1] = tmpbuf + size;             //U
    picture->data[2] = picture->data[1] + size/4; //V
    picture->linesize[0] = vcodec_context->width;
    picture->linesize[1] = vcodec_context->width / 2;
    picture->linesize[2] = vcodec_context->width / 2;
}

int GVCodec::encode_video_frame (UINT8 *in_buf)
{
    int size = 0;
    if(!video_out)
    {
        std::cerr << "libgvencoder: can't copy frame - output buffer is NULL\n";
        return (0);
    }
    
    switch(vcodec_index)
    {
        case 0: //MJPG
            size = jpeg->encode (in_buf, video_out, false);
            break;
        case 1: //YUY2
            size = mkv_codecPriv.biWidth*mkv_codecPriv.biHeight*2; //frame in yuv format already (2 bytes per pixel)
            memcpy(video_out, in_buf, size);
            break;
        default:
            //convert to 4:2:0
            fill_frame(in_buf);
            /* encode the image */
            size = avcodec_encode_video(vcodec_context, video_out, _video_buff_size, picture);
            break;
    }
    return (size);
}

int GVCodec::get_aac_obj_ind(int profile)
{
    int i = 0;

    for (i=0; i<4; i++)
        if(AAC_OBJ_TYPE[i] == profile) break;

    return i;
}

int GVCodec::get_aac_samp_ind(int samprate)
{
    int i = 0;

    for (i=0; i<13; i++)
        if(AAC_SAMP_FREQ[i] == samprate) break;

    if (i>12) 
    {
        std::cerr << "WARNING: invalid sample rate for AAC encoding\n";
        std::cerr << "valid(96000, 88200, 64000, 48000, 44100, 32000, 24000,";
        std::cerr << " 22050, 16000, 12000, 11025, 8000, 7350)\n";
        i=4; /*default 44100*/
    }
    return i;
}


void GVCodec::close_acodec()
{
    if(acodec_context)
    {
        avcodec_flush_buffers(acodec_context);
        //close codec 
        avcodec_close(acodec_context);
        //free codec context 
        free(acodec_context);
    }
    
    acodec_context = NULL;
}

/* returns -1 on error or 
 * expected audio frame size in samples
 */
int GVCodec::open_acodec(unsigned codec_ind, 
                          UINT8 *out_buff, 
                          unsigned buff_size,
                          int samprate, 
                          int channels)
{
    if (codec_ind >= acodec_list.size())
    {
        std::cerr << "libgvencoder: audio codec index not valid [0-" << acodec_list.size()-1 << "]\n";
        return (-1);
    }
    acodec_index = codec_ind;
    audio_out = out_buff;
    _audio_buff_size = buff_size;
    audio_frame_size = DEF_AUD_FRAME_SIZE;
    
    /*sets the matroska codec private data if required*/
    if (acodec_list[acodec_index].codecPriv != NULL)
    {
        if (acodec_list[acodec_index].codec_id == CODEC_ID_AAC)
        {
            int obj_type = get_aac_obj_ind(acodec_list[acodec_index].profile);
            int sampind  = get_aac_samp_ind(samprate);
            AAC_ESDS[0] = (UINT8) ((obj_type & 0x1F) << 3 ) + ((sampind & 0x0F) >> 1);
            AAC_ESDS[1] = (UINT8) ((sampind & 0x0F) << 7 ) + ((channels & 0x0F) << 3);
        }
    }
    std::cout << "libgvencoder: using audio codec n." << acodec_index << std::endl;
    if (acodec_index > 0) /* 0, is a built-in codec (PCM)*/
    {
        acodec = avcodec_find_encoder(acodec_list[acodec_index].codec_id);
        if (!acodec) 
        {
            std::cerr << "libgvencoder: 4cc(" 
                << acodec_list[acodec_index].fourcc << ") audio codec not found\n";
            return(-1);
        }
        acodec_context = avcodec_alloc_context();

        // define bit rate (lower = more compression but lower quality)
        acodec_context->bit_rate = acodec_list[acodec_index].bit_rate;
        acodec_context->profile = acodec_list[acodec_index].profile; /*for AAC*/

        acodec_context->flags |= acodec_list[acodec_index].flags;
        acodec_context->sample_rate = samprate;
        acodec_context->channels = channels;
        acodec_context->cutoff = 0; /*automatic*/
        //data->codec_context->sample_fmt = SAMPLE_FMT_FLT; /* floating point sample */
        acodec_context->codec_id = acodec_list[acodec_index].codec_id;
        acodec_context->codec_type = CODEC_TYPE_AUDIO;

        // open codec
        if (avcodec_open(acodec_context, acodec) < 0) 
        {
            std::cerr << "libgvencoder: could not open audio codec\n";
            return(-1);
        }

        /* the codec gives us the frame size, in samples */
        audio_frame_size = acodec_context->frame_size;  
        std::cout << "Audio frame size is" << audio_frame_size <<"samples for selected codec\n";
    }
    
    return(audio_frame_size);
}

int GVCodec::encode_audio_frame (INT16 *in_buf)
{
    int size = 0;
    if(!audio_out)
    {
        std::cerr << "libgvencoder: can't copy audio frame - output buffer is NULL\n";
        return (0);
    }
    
    switch(acodec_index)
    {
        case 0: //PCM
            //std::cerr << "libgvencoder: input buffer already in pcm format\n";
            size = audio_frame_size * sizeof(UINT16); //frame in pcm format already( 2 bytes per sample)
            if(size > _audio_buff_size)
            {
                std::cerr << "libgvencoder: ouput buffer not big enough, needs " << size << " bytes\n";
                std::cerr << "libgvencoder: input buffer already in pcm format no need for encoding\n";
                return (0);
            }
            memcpy(audio_out, in_buf, size);
            break;
        default:
            /* encode the image */
            size = avcodec_encode_audio(acodec_context, audio_out, _audio_buff_size, in_buf);
            break;
    }
    return (size);
}

END_LIBGVENCODER_NAMESPACE
