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
#  GVImage class : gvideo image class                                           #
#                                                                               #
********************************************************************************/

#include <iostream>
#include <fstream>
#include "libgvideo/GVid_image.h"

using namespace std;

int GVImage::save_BPM(const char* filename, int width, int height, int BitCount, UINT8 *ImagePix) 
{
	int ret=0;
	BITMAPFILEHEADER *BmpFileh = new BITMAPFILEHEADER;
	BITMAPINFOHEADER *BmpInfoh = new BITMAPINFOHEADER;
	UINT32 imgsize;
	ofstream myfile;

	imgsize=width*height*BitCount/8;

	BmpFileh->bfType=0x4d42;//must be BM (x4d42) 
	/*Specifies the size, in bytes, of the bitmap file*/
	BmpFileh->bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+imgsize;
	BmpFileh->bfReserved1=0; //Reserved; must be zero
	BmpFileh->bfReserved2=0; //Reserved; must be zero
	/*Specifies the offset, in bytes,                      */
	/*from the beginning of the BITMAPFILEHEADER structure */
	/* to the bitmap bits                                  */
	BmpFileh->bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); 

	BmpInfoh->biSize=40;
	BmpInfoh->biWidth=width; 
	BmpInfoh->biHeight=height; 
	BmpInfoh->biPlanes=1; 
	BmpInfoh->biBitCount=BitCount; 
	BmpInfoh->biCompression=0; // 0 
	BmpInfoh->biSizeImage=imgsize; 
	BmpInfoh->biXPelsPerMeter=0; 
	BmpInfoh->biYPelsPerMeter=0; 
	BmpInfoh->biClrUsed=0; 
	BmpInfoh->biClrImportant=0;
	
	myfile.open (filename,  ios::out | ios::app | ios::binary);
	
	if(myfile.is_open())
	{
        myfile.write((char *) BmpFileh, sizeof(BITMAPFILEHEADER));
        myfile.write((char *) BmpInfoh, sizeof(BITMAPINFOHEADER));
        myfile.write((char *) ImagePix, imgsize); 
	} 
	else 
	{
		ret=1;
		cerr << "ERROR: Could not open file " << filename << " for write\n";
	}
	
	myfile.close();
	delete BmpFileh;
	delete BmpInfoh;
	return ret;
}
