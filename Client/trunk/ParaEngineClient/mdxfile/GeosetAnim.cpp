//----------------------------------------------------------------------
// Class:	CGeosetAnim
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class  based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer

#include "DxStdAfx.h"
#include "GeosetAnim.h"
#include "math.h"
#include "mdx.h"
#include "memdebug.h"
using namespace ParaEngine;


CGeosetAnim::CGeosetAnim(void)
{
	numGeosetAnim=0;
}

CGeosetAnim::~CGeosetAnim(void)
{
	{
		vector <GEOA*>::iterator itCurCP, itEndCP = geosetAnim.end();

		for( itCurCP = geosetAnim.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete (*itCurCP);
		}
		geosetAnim.clear();
	}
	{
		vector <KGAO*>::iterator itCurCP, itEndCP = geosetAlpha.end();

		for( itCurCP = geosetAlpha.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete (*itCurCP);
		}
		geosetAlpha.clear();
	}
}

void CGeosetAnim::Read(TypePointer inP,int inSize)
{
	TypePointer p(inP);

	while(p.c<inP.c+inSize)
	{
		TypePointer nextAddress(p);
		int size=*p.dw++;
		nextAddress.c+=size;

		// load anim
		GEOA * pGeosetAnim = new GEOA();
		memcpy(pGeosetAnim, p.c, sizeof(GEOA));
		geosetAnim.push_back(pGeosetAnim);

		p.c+=sizeof(GEOA);

		// load keys
		KGAO * pGeosetAlphaKeys = new KGAO();
		
		if(TAG(*p.dw)=='KGAO')
		{
			p.dw++;
			pGeosetAlphaKeys->chunkNum=*p.dw++;
			pGeosetAlphaKeys->lineType=*p.dw++;
			p.dw++;
			pGeosetAlphaKeys->data = new char[nextAddress.c - p.c];
			memcpy(pGeosetAlphaKeys->data, p.c, nextAddress.c - p.c);

		}else{
			pGeosetAlphaKeys->chunkNum=0;
			pGeosetAlphaKeys->lineType=0;
			pGeosetAlphaKeys->data=NULL;
		}
		geosetAlpha.push_back(pGeosetAlphaKeys);

		numGeosetAnim++;
		p=nextAddress;
	}
}

float CGeosetAnim::GetAnimAlpha(int geoID,const AnimInfo& animInfo)
{

	if(geoID>numGeosetAnim-1 || geoID < 0)
		return 1.0f;

	AnimAlpha *alpha=(AnimAlpha*)geosetAlpha[geoID]->data;

	if(alpha==NULL)
		return 1.0f;

	int max_num=geosetAlpha[geoID]->chunkNum-1;
	if(	alpha[max_num].frameNum <= animInfo.currentFrame )
	{
		if(animInfo.startFrame == alpha[max_num].frameNum)
			return alpha[max_num].alphaValue;
		else
			return 1.0f;
	}

	if(	alpha[0].frameNum > animInfo.currentFrame )
		return 1.0f;

	for(int i=0;i<max_num;i++)
	{
		if(	alpha[i].frameNum <= animInfo.currentFrame &&
			alpha[i+1].frameNum >= animInfo.currentFrame )
		{
			if(	alpha[i].frameNum >= animInfo.startFrame &&
				alpha[i].frameNum <= animInfo.endFrame )
			{
				return alpha[i].alphaValue;
			}	
		}
	}
	return 1.0f;
}