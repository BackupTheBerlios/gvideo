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
#  GVMjpgDec class : gvideo mjpg decompression class                            #
#                                                                               #
********************************************************************************/

#ifndef GVID_MJPG_DEC_H
#define GVID_MJPG_DEC_H

#include "gvcommon.h"
#include "GVid_color_convert.h"

#define ISHIFT 11

#define IFIX(a) ((int)((a) * (1 << ISHIFT) + .5))

#ifndef __P
# define __P(x) x
#endif

/* special markers */
#define M_BADHUFF	-1
#define M_EOF		0x80

#define DECBITS 10		/* seems to be the optimum */

/******** Markers *********/
#define M_SOI   0xd8
#define M_APP0  0xe0
#define M_DQT   0xdb
#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_DRI   0xdd
#define M_SOS   0xda
#define M_RST0  0xd0
#define M_EOI   0xd9
#define M_COM   0xfe

/*******Error codes *******/
#define ERR_NO_SOI 1
#define ERR_NOT_8BIT 2
#define ERR_HEIGHT_MISMATCH 3
#define ERR_WIDTH_MISMATCH 4
#define ERR_BAD_WIDTH_OR_HEIGHT 5
#define ERR_TOO_MANY_COMPPS 6
#define ERR_ILLEGAL_HV 7
#define ERR_QUANT_TABLE_SELECTOR 8
#define ERR_NOT_YCBCR_221111 9
#define ERR_UNKNOWN_CID_IN_SCAN 10
#define ERR_NOT_SEQUENTIAL_DCT 11
#define ERR_WRONG_MARKER 12
#define ERR_NO_EOI 13
#define ERR_BAD_TABLES 14
#define ERR_DEPTH_MISMATCH 15

struct comp 
{
    int cid;
    int hv;
    int tq;
};

#define MAXCOMP 4
struct jpginfo 
{
    int nc;     /* number of components */
    int ns;     /* number of scans */
    int dri;    /* restart interval */
    int nm;     /* mcus til next marker */
    int rm;     /* next restart marker */
};

// jpeg decoder structs
struct jpeg_decdata 
{
    int dcts[6 * 64 + 16];
    int out[64 * 6];
    int dquant[3][64];
};

#define HEADERFRAME1 0xaf

struct inp 
{
    UINT8 *p;
    UINT32 bits;
    int left;
    int marker;
    //int (*func) __P((void *));
    void *data;
};
    
struct dec_hufftbl 
{
    int maxcode[17];
    int valptr[16];
    UINT8 vals[256];
    UINT32 llvals[1 << DECBITS];
};

struct scan 
{
    int dc;     // old dc value

    struct dec_hufftbl *hudc;
    struct dec_hufftbl *huac;
    int next;       // when to switch to next scan

    int cid;        // component id
    int hv;         // horiz/vert, copied from comp
    int tq;         // quant tbl, copied from comp
};



class GVMjpgDec
{
    GVConvert *conversor;
    UINT8 * datap;

    //(M)jpg decode info
    struct jpginfo info;
    
    //(M)jpg components
    struct comp comps[MAXCOMP];
    struct scan dscans[MAXCOMP];

    UINT8 quant[4][64];

    struct dec_hufftbl dhuff[4];

#define dec_huffdc (dhuff + 0)
#define dec_huffac (dhuff + 2)

    struct inp *in;
    
    //IDCT
    static const UINT8 zig[64];
    //zigzag order used by idct
    static const UINT8 zig2[64];

    //coef used in idct
    static const int aaidct[8];

    // internal functions
    int getbyte();
    int getword();
    int readtables(int till, int *isDHT);
    int huffman_init();
    void dec_makehuff(struct dec_hufftbl *hu, int *hufflen, UINT8 *huffvals);
    void setinput();
    int fillbits( int le, unsigned int bi );
    int dec_readmarker();
    int dec_rec2( struct dec_hufftbl *hu, int *runp, int c, int i );
    void decode_mcus( int *dct, int n, struct scan *sc, int *maxp );
    void dec_initscans();
    int dec_checkmarker();
    void idct(int *in, int *out, int *quant, long off, int max);
    void idctqtab(UINT8 *qin, int *qout);
    
  public:
    GVMjpgDec();
    ~GVMjpgDec();
    int decode(UINT8 **frame_data, UINT8 *raw_data, int width, int height);
    
};


#endif
