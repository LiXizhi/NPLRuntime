//----------------------------------------------------------------------
// Class:	BLP image loader
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date: 2005/02
// BLP loader is based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer
/**
some replaceable ID is implemented here. see also ReplaceIDWithName();
define EXTRACT_MPQ_FILE_TO_DISK to extract BLP to disk
*/
#include "DxStdAfx.h"
#include "ParaEngine.h"

#ifndef BLP_SUPPORT
LPDIRECT3DTEXTURE9 LoadBLP(char* blpdata, DWORD blpsize)
{
	return false;
}
#else

#include "math.h"

#include "mdx.h"
#include "blp.h"
#include "SFmpq_static.h"
#include "utility.h"
#include "memdebug.h"
using namespace ParaEngine;

// LiXizhi added
extern "C" DWORD IJGRead (unsigned char *buf, unsigned long buflen, long *w, long *h, long *bpp, unsigned char *destbuf);


extern void *malloc_func(const size_t size);
extern void free_func(void* ptr);
DWORD ConvertBLP(char *srcBuf, char *destBuf, DWORD *width, DWORD *height, DWORD *type, DWORD *subtype);

void FatalError(const char *error_msg);

// Added LiXizhi

struct BLPHeader
{
	char ident[4];
	unsigned long compress, nummipmaps, sizex, sizey, pictype, picsubtype;
	unsigned long poffs[16], psize[16];
};

struct BLPHeader2
{
	char ident[4];
	unsigned long compress, nummipmaps, sizex, sizey, pictype, picsubtype;
	unsigned long poffs[16], psize[16];
	unsigned long JPEGHeaderSize;
};

struct RGBAPix
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

struct PAPix
{
	unsigned char i;
	unsigned char A;
};

struct PPix
{
	unsigned char i;
};

void FatalError(const char* a)
{
	// LiXizhi: place holder
}

DWORD ConvertBLP(char *srcBuf, char *destBuf, DWORD *width, DWORD *height, DWORD *type, DWORD *subtype)
{
	BLPHeader blph;
	unsigned long curpos = 0;

	if(srcBuf==NULL)
		return 0;

	memcpy(&blph, srcBuf+curpos, sizeof(BLPHeader));
	if (memcmp(blph.ident, "BLP1", 4)!=0)
	{
		if(width!=NULL)	*width = 0;
		if(height!=NULL) *height = 0;
		if(type!=NULL) *type = 0;
		if(subtype!=NULL) *subtype = 0;
		return 0;
	}
	curpos+=sizeof(BLPHeader);

	if(blph.compress == 1) // non compressée
	{
		if(destBuf==NULL)
		{
			if(width!=NULL)	*width = blph.sizex;
			if(height!=NULL)	*height = blph.sizey;
			if(type!=NULL) *type = blph.pictype;
			if(subtype!=NULL) *subtype = blph.picsubtype;

			return (blph.sizex*blph.sizey*4);
		}

		RGBAPix Pal[256];
		memcpy(&Pal, srcBuf+curpos, 256*4);
		curpos+=256*4;

		if((blph.pictype==4)||(blph.pictype==3)) // a un canal alpha
		{
			PAPix *tdata = (PAPix*)malloc_func(blph.sizex*blph.sizey*2);
			memcpy(tdata, srcBuf+curpos, blph.sizex*blph.sizey*2);
			curpos+=blph.sizex*blph.sizey*2;

			RGBAPix *pic = (RGBAPix*)malloc_func(blph.sizex*blph.sizey*4);
			
			unsigned long k,j,i;

			j=0;
			i=(blph.sizex*blph.sizey)/2;
			for(k=0; k<i; ++k)
			{
				pic[j].R = Pal[tdata[k].i].R;
				pic[j].G = Pal[tdata[k].i].G;
				pic[j].B = Pal[tdata[k].i].B;
				pic[j].A = tdata[k+i].i;
				++j;

				pic[j].R = Pal[tdata[k].A].R;
				pic[j].G = Pal[tdata[k].A].G;
				pic[j].B = Pal[tdata[k].A].B;
				pic[j].A = tdata[k+i].A;
				++j;
			}

			free_func(tdata);
			memcpy(destBuf, pic, blph.sizex*blph.sizey*4);
			free_func(pic);
			if(width!=NULL)	*width = blph.sizex;
			if(height!=NULL) *height = blph.sizey;
			if(type!=NULL) *type = blph.pictype;
			if(subtype!=NULL) *subtype = blph.picsubtype;

			return (blph.sizex*blph.sizey*4);
		}

		if(blph.pictype==5) // n'a pas de canal alpha
		{
			PPix *tdata = (PPix*)malloc_func(blph.sizex*blph.sizey);
			memcpy(tdata, srcBuf+curpos, blph.sizex*blph.sizey);
			curpos+=blph.sizex*blph.sizey;

			RGBAPix *pic = (RGBAPix*)malloc_func(blph.sizex*blph.sizey*4);
			
			unsigned long k,i;

			i=blph.sizex*blph.sizey;
			for(k=0; k<i; ++k)
			{
				pic[k].R = Pal[tdata[k].i].R;
				pic[k].G = Pal[tdata[k].i].G;
				pic[k].B = Pal[tdata[k].i].B;
				pic[k].A = 255-Pal[tdata[k].i].A;
			}

			free_func(tdata);
			memcpy(destBuf, pic, blph.sizex*blph.sizey*4);
			free_func(pic);
			if(width!=NULL)	*width = blph.sizex;
			if(height!=NULL) *height = blph.sizey;
			if(type!=NULL) *type = blph.pictype;
			if(subtype!=NULL) *subtype = blph.picsubtype;

			return (blph.sizex*blph.sizey*4);
		}

	}

	if(blph.compress == 0) // compressée en JPEG
	{
		if(destBuf==NULL)
		{
			if(width!=NULL)	*width = blph.sizex;
			if(height!=NULL)	*height = blph.sizey;
			if(type!=NULL) *type = blph.pictype;
			if(subtype!=NULL) *subtype = blph.picsubtype;

			return (blph.sizex*blph.sizey*4);
		}

		curpos=sizeof(BLPHeader);
		long JPEGHeaderSize;
		memcpy(&JPEGHeaderSize, srcBuf+curpos, 4);
		curpos+=4;
		unsigned char *tdata = (unsigned char*)malloc_func(blph.psize[0] + JPEGHeaderSize);
		curpos=sizeof(BLPHeader)+4;
		memcpy(tdata, srcBuf+curpos, JPEGHeaderSize);
		curpos+=JPEGHeaderSize;
		curpos=blph.poffs[0];
		memcpy((tdata+JPEGHeaderSize), srcBuf+curpos, blph.psize[0]);

		if(IJGRead(tdata, blph.psize[0] + JPEGHeaderSize, NULL, NULL, NULL, (unsigned char*)destBuf) == 0)
		{
			free_func(tdata);
			if(width!=NULL)	*width = 0;
			if(height!=NULL)	*height = 0;
			if(type!=NULL) *type = 0;
			if(subtype!=NULL) *subtype = 0;

			return (0);
		}

		free_func(tdata);
		

		if(width!=NULL)	*width = blph.sizex;
		if(height!=NULL)	*height = blph.sizey;
		if(type!=NULL) *type = blph.pictype;
		if(subtype!=NULL) *subtype = blph.picsubtype;

		return (blph.sizex*blph.sizey*4);
	}

	if(width!=NULL)	*width = 0;
	if(height!=NULL) *height = 0;
	if(type!=NULL) *type = 0;
	if(subtype!=NULL) *subtype = 0;

	return 0;
}

LPDIRECT3DTEXTURE9 LoadBLP(char* blpdata, DWORD blpsize)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	DWORD width,height,type,subtype,nSizeBuffer = ConvertBLP(blpdata, NULL, &width,&height,&type,&subtype);

	char *timage = new char[nSizeBuffer];
	ConvertBLP(blpdata, timage, &width,&height,&type,&subtype);
	

	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
	
	if(SUCCEEDED(D3DXCreateTexture(pd3dDevice, width, height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture)))
	{
		D3DLOCKED_RECT lr;
		if(SUCCEEDED(pTexture->LockRect(0, &lr, NULL, 0)))
		{
			char *pp = (char*)lr.pBits;
			unsigned long index = 0, x = 0, y = 0;

			for (y=0; y < height; y++)
			{
				for (x=0; x < width; x++)
				{
					pp[index++] = timage[(y*4*width)+4*x];
					pp[index++] = timage[(y*4*width)+4*x+1];
					pp[index++] = timage[(y*4*width)+4*x+2];
					pp[index++] = timage[(y*4*width)+4*x+3];
				}
				index += lr.Pitch - (width*4);
			}
			pTexture->UnlockRect(0);
		}
	}
	delete []timage;
	return pTexture;
}
#endif