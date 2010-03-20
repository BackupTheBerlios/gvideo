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
#include <iostream>

START_LIBGVENCODER_NAMESPACE

GVCodec::GVCodec(int width, int height, int fps_denom/*=15*/, int fps_num/*=1*/)
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
    out = NULL;
    
    mkv_codecPriv.biSize = 0x00000028; //40 bytes 
    mkv_codecPriv.biWidth = width;       //default values (must be set before use)
    mkv_codecPriv.biHeight = height; 
    mkv_codecPriv.biPlanes = 1; 
    mkv_codecPriv.biBitCount = 24; 
    mkv_codecPriv.biCompression = V4L2_PIX_FMT_MJPEG; 
    mkv_codecPriv.biSizeImage = width*height*2; //2 bytes per pixel (max buffer - use x3 for RGB)
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
    vcodec_list.push_back( (GVCodec_vdata)
    {
        false, true, "RGB ", v4l2_fourcc('R','G','B',' '), "V_MS/VFW/FOURCC", &mkv_codecPriv, 40,
        "RGB - uncomp BMP", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
    
    // set the default codec index to 0
    codec_index = 0;
    _fps_denom = fps_denom;
    _fps_num = fps_num;
    
    _buff_size = 0;
}

GVCodec::~GVCodec()
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
    if(tmpbuf) delete[] tmpbuf;
}

bool GVCodec::open_vcodec(unsigned codec_ind, UINT8 *out_buff, unsigned buff_size)
{
    out = out_buff;
    _buff_size = buff_size;
    
    if (codec_ind >= vcodec_list.size())
    {
        std::cerr << "libgvencoder: codec index not valid [2-" << vcodec_list.size()-1 << "]\n";
        return (false);
    }
    if(codec_ind == 0)
    {
        jpeg = new GVJpeg(mkv_codecPriv.biWidth, mkv_codecPriv.biHeight);
    }
    else if (codec_ind > 2) /* 0, 1 and 2 are built-in codecs*/
    {
        //alloc picture
        picture= avcodec_alloc_frame();
        //alloc tmpbuff (yuv420p)
        tmpbuf = new UINT8 [(mkv_codecPriv.biWidth * mkv_codecPriv.biHeight*3)/2];
        
        vcodec = avcodec_find_encoder(vcodec_list[codec_ind].codec_id);
        if (!vcodec) 
        {
            std::cerr << "libgvencoder: " 
                << vcodec_list[codec_ind].compressor << " codec not found\n";
            return(false);
        }
        
        //alloc context
        vcodec_context = avcodec_alloc_context();
        // resolution must be a multiple of two
        vcodec_context->width = mkv_codecPriv.biWidth; 
        vcodec_context->height = mkv_codecPriv.biHeight;
        
        vcodec_context->bit_rate = vcodec_list[codec_ind].bit_rate;
        vcodec_context->flags |= vcodec_list[codec_ind].flags;
        /* 
         * mb_decision
         *0 (FF_MB_DECISION_SIMPLE) Use mbcmp (default).
         *1 (FF_MB_DECISION_BITS)   Select the MB mode which needs the fewest bits (=vhq).
         *2 (FF_MB_DECISION_RD)     Select the MB mode which has the best rate distortion.
         */
        vcodec_context->mb_decision = vcodec_list[codec_ind].mb_decision;
        /*use trellis quantization*/
        vcodec_context->trellis = vcodec_list[codec_ind].trellis;

        //motion estimation method epzs
        vcodec_context->me_method = vcodec_list[codec_ind].me_method; 

        vcodec_context->dia_size = vcodec_list[codec_ind].dia;
        vcodec_context->pre_dia_size = vcodec_list[codec_ind].pre_dia;
        vcodec_context->pre_me = vcodec_list[codec_ind].pre_me;
        vcodec_context->me_pre_cmp = vcodec_list[codec_ind].me_pre_cmp;
        vcodec_context->me_cmp = vcodec_list[codec_ind].me_cmp;
        vcodec_context->me_sub_cmp = vcodec_list[codec_ind].me_sub_cmp;
        vcodec_context->me_subpel_quality = vcodec_list[codec_ind].subq; //NEW
        vcodec_context->refs = vcodec_list[codec_ind].framerefs;         //NEW
        vcodec_context->last_predictor_count = vcodec_list[codec_ind].last_pred;

        vcodec_context->mpeg_quant = vcodec_list[codec_ind].mpeg_quant; //h.263
        vcodec_context->qmin = vcodec_list[codec_ind].qmin;             // best detail allowed - worst compression
        vcodec_context->qmax = vcodec_list[codec_ind].qmax;             // worst detail allowed - best compression
        vcodec_context->max_qdiff = vcodec_list[codec_ind].max_qdiff;
        vcodec_context->max_b_frames = vcodec_list[codec_ind].max_b_frames;
        vcodec_context->gop_size = vcodec_list[codec_ind].gop_size;

        vcodec_context->qcompress = vcodec_list[codec_ind].qcompress;
        vcodec_context->qblur = vcodec_list[codec_ind].qblur;
        vcodec_context->strict_std_compliance = FF_COMPLIANCE_NORMAL;
        vcodec_context->codec_id = vcodec_list[codec_ind].codec_id;
        vcodec_context->codec_type = CODEC_TYPE_VIDEO;
        vcodec_context->pix_fmt = PIX_FMT_YUV420P; //only yuv420p available for mpeg
        if(vcodec_list[codec_ind].fps) /*for gscpa must set fps in properties*/
            vcodec_context->time_base = (AVRational){1, vcodec_list[codec_ind].fps}; //use properties fps
        else
            vcodec_context->time_base = (AVRational){_fps_num, _fps_denom};
            
        // open codec
        if (avcodec_open(vcodec_context, vcodec) < 0) 
        {
            std::cerr << "libgvencoder: could not open video codec\n";
            return(false);
        }
    }
    
    codec_index = codec_ind;

    if(vcodec_list[codec_index].codecPriv != NULL)
    {
        mkv_codecPriv.biCompression = vcodec_list[codec_index].fourcc;
        if(codec_index == 2)
            mkv_codecPriv.biSizeImage = mkv_codecPriv.biWidth * mkv_codecPriv.biHeight * 3; /*rgb*/
        else
            mkv_codecPriv.biSizeImage = mkv_codecPriv.biWidth * mkv_codecPriv.biHeight * 2;
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
    if(!out)
    {
        std::cerr << "libgvencoder: can't copy frame - output buffer is NULL\n";
        return (0);
    }
    
    switch(codec_index)
    {
        case 0: //MJPG
            size = jpeg->encode (in_buf, out, false);
            break;
        case 1: //YUY2
            size = 0;
            break;
        case 2: //RGB
            size = 0;
            break;
        default:
            //convert to 4:2:0
            fill_frame(in_buf);
            /* encode the image */
            size = avcodec_encode_video(vcodec_context, out, _buff_size, picture);
            break;
    }
    return (size);
}


END_LIBGVENCODER_NAMESPACE
