//----------------------------------------------------------------------
// Class:	CGeoset
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class  based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer

#include "dxStdAfx.h"
#include "ParaEngine.h"
#include "Geoset.h"
#include "MDXModel.h"
#include "..\ShadowVolume.h"
#include "memdebug.h"
using namespace ParaEngine;


extern BOOL g_showmeshes[MAX_CHUNK_NUM];


CGeoset::CGeoset(void)
{
	chunks.clear();
}

CGeoset::~CGeoset(void)
{
	Cleanup();
}

void CGeoset::Cleanup()
{
	list< CGeoChunk* >::iterator itCurCP, itEndCP = chunks.end();

	for( itCurCP = chunks.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		delete (*itCurCP);
	}
	chunks.clear();
}

CGeoChunk* CGeoset::AddNewChunk()
{
	CGeoChunk* tmp = new CGeoChunk();
	chunks.push_back(tmp);
	return tmp;
}

void CGeoset::Read(TypePointer inP,int inSize)
{
	TypePointer p(inP);

	while(p.c<inP.c+inSize)
	{
		CGeoChunk* chunk = AddNewChunk();
		int size = (*p.dw++)-4;
		chunk->Read(p,size);
		p.c += size;
	}
}

void CGeoset::Render(CMDXModel *model,const AnimInfo& animInfo)
{
	list< CGeoChunk* >::iterator itCurCP, itEndCP = chunks.end();

	int i=0;
	for( itCurCP = chunks.begin(); itCurCP != itEndCP; ++ itCurCP, i++)
	{
		if(g_showmeshes[i] == TRUE) 
			(*itCurCP)->Render(model,animInfo);
	}
}

void CGeoset::BuildShadowVolume(CMDXModel *model,const AnimInfo& animInfo, ShadowVolume * pShadowVolume, LightParams* pLight, D3DXMATRIX* mxWorld)
{
	list< CGeoChunk* >::iterator itCurCP, itEndCP = chunks.end();

	int i=0;
	for( itCurCP = chunks.begin(); itCurCP != itEndCP; ++ itCurCP, i++)
	{
		if(g_showmeshes[i] == TRUE) 
			(*itCurCP)->BuildShadowVolume(model,animInfo,pShadowVolume, pLight, mxWorld);
	}
}
