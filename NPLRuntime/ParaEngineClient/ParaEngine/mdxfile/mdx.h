#pragma once

#include "mdxCustomVertex.h"
namespace ParaEngine
{
//
// we have to inverse the byte order, intel platform
//
#define TAG(x) (DWORD)(  (((DWORD)x&0x0000ff00)<<8)+(((DWORD)x&0x000000ff)<<24)+(((DWORD)x&0x00ff0000)>>8)+(((DWORD)x&0xff000000)>>24) )

//
// 3-short vertex indexing Triangle 
//
struct Triangle
{
	short ind[3];
};

struct Layer1
{ 
	int		mLayerSize; // in Bytes
	int		mBlendMode;
	int		mUnk3;
	int		mTexture;
	int		mUnk5;
	int		mUnk6;
	float	mUnk7;		// alpha ?
	BOOL	mKMTA;
};


struct SequenceName
{
	char	mName[80];
	int		mUnk1;

	int		mUnk2;
	float	mUnk3;
	int		mUnk4;
	float	mUnk5;
	int		mUnk6;
	float	mUnk7;

	Vec3	mMins;
	Vec3	mMaxs;
};

struct Texture
{
	DWORD	mUnk1;
	char	mName[MAX_PATH]; // 260
	DWORD	mUnk2;
};


struct Camera
{
	char cName[80];
	float posx;
	float posy;
	float posz;
	float cFOV;
	float farClip;
	float nearClip;
	float targetx;
	float targety;
	float targetz;
};


//
// Auxiliary union TypePointer
//
// Used for better readability
//

union TypePointer1
{
	DWORD*		  dw;
	char*		  c;
	void*		  p;
	float*		  f;
	Vec3*		  v3;
	Vec2*		  v2;
	Triangle*	  tri;
	SequenceName* seq;
	Texture*	  tex;
	Camera*		  cam;

	TypePointer1(void* in) :p(in)
	{
	}
};
}