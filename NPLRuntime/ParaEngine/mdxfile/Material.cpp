//----------------------------------------------------------------------
// Class:	CMaterial
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class  based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer

#include "dxstdafx.h"
#include "material.h"
#include "math3d.h"
#include "math.h"
#include "mdx.h"
#include "memdebug.h"
using namespace ParaEngine;


CMaterial::CMaterial()
{
	layers.clear();
}

CMaterial::~CMaterial()
{
	Cleanup();
}

void CMaterial::Cleanup()
{
	{
		vector <Layer*>::iterator itCurCP, itEndCP = layers.end();

		for( itCurCP = layers.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete (*itCurCP);
		}
		layers.clear();
	}
}

Layer* CMaterial::CreateLayer()
{
	Layer* pLayer = new Layer();
	layers.push_back(pLayer);
	return pLayer;
}

/// one time read, each model can only call this once
void CMaterial::Read(TypePointer inP,int inSize)
{
	/// one time call, any previous call to Read(...)  will be overriden
	Cleanup(); 

	TypePointer p(inP);
	*p.dw++;
	*p.dw++;
	if( TAG(*p.dw) != 'LAYS' )
		return;

	p.dw++;
	int numLayers = *p.dw++;
	
	for(int i=0;i<numLayers;++i)
	{
		Layer* pLayer = CreateLayer();
		pLayer->layerSize = p.dw[0];
		pLayer->filterMode = p.dw[1];
		pLayer->shading = p.dw[2];
		pLayer->textureID = p.dw[3];
		pLayer->unk5 = p.dw[4];
		pLayer->unk6 = p.dw[5];
		pLayer->alpha = p.f[6];

		// ktma stuff here 
		if(TAG(p.dw[7]) == 'KMTA')
		{
			pLayer->existKMTA = true;
			pLayer->KMTA.chunkNum = p.dw[8];
			pLayer->KMTA.LineType = p.dw[9];
			
			int nLen = pLayer->layerSize-4*11;
			pLayer->KMTA.data = new char[nLen];
			memcpy(pLayer->KMTA.data, (void*)&p.dw[11], nLen); 
		} else
		{
			pLayer->existKMTA = false;
		}
		p.c += pLayer->layerSize;
	}
}

float CMaterial::getFrameAlpha(int frameNum,int layerNum)
{
	int numLayers = (int)layers.size();
	if(layerNum >= numLayers || !layers[layerNum]->existKMTA)
		return 1.0f;
	if(layers[layerNum]->KMTA.LineType == LINEAR)
	{
		LinearAlpha *p=(LinearAlpha *)layers[layerNum]->KMTA.data;
		for(int i=0;i<layers[layerNum]->KMTA.chunkNum-1;i++)
		{
			if(frameNum<17600 && frameNum>16967)
				frameNum=frameNum;
			LinearAlpha *startA=p++;
			LinearAlpha *endA=p;
			if(frameNum == startA->frameNum)
				return startA->alphaValue;
			else
			if(frameNum == endA->frameNum)
				return endA->alphaValue;
			else
			if(frameNum > startA->frameNum && frameNum < endA->frameNum)
			{
				float step=(float)(frameNum - startA->frameNum)/(float)(endA->frameNum - startA->frameNum);

				D3DXVECTOR3 v1(startA->alphaValue,startA->alphaValue,startA->alphaValue);
				D3DXVECTOR3 v2(endA->alphaValue,endA->alphaValue,endA->alphaValue);
				D3DXVECTOR3 out;
				D3DXVec3Lerp (&out, &v1, &v2, step);
				return out.x;
			}
		}
		return 0.0f;
	}
	else
	if(layers[layerNum]->KMTA.LineType > LINEAR)
	{
		NonLinearAlpha *p=(NonLinearAlpha *)layers[layerNum]->KMTA.data;
		for(int i=0;i<layers[layerNum]->KMTA.chunkNum-1;i++)
		{
			NonLinearAlpha *startA=p++;
			NonLinearAlpha *endA=p;
			if(frameNum == startA->frameNum)
				return startA->alphaValue;
			else
			if(frameNum == endA->frameNum)
				return endA->alphaValue;
			else
			if(frameNum > startA->frameNum && frameNum < endA->frameNum)
			{
				float step=(float)(frameNum - startA->frameNum)/(float)(endA->frameNum - startA->frameNum);

				D3DXVECTOR3 v1(startA->alphaValue,startA->alphaValue,startA->alphaValue);
				D3DXVECTOR3 v2(endA->alphaValue,endA->alphaValue,endA->alphaValue);
				D3DXVECTOR3 inTan(startA->inTan,startA->inTan,startA->inTan);
				D3DXVECTOR3 outTan(startA->outTan,startA->outTan,startA->outTan);
				D3DXVECTOR3 out(1.0f,1.0f,1.0f);
				if(layers[layerNum]->KMTA.LineType == HERMITE)
				{
					D3DXVec3Hermite(&out, &v1, &inTan, &v2, &outTan, step);
				}else
				if(layers[layerNum]->KMTA.LineType == BEZIER)
				{
					D3DXVec3Bezier(&out, &v1, &inTan, &v2, &outTan, step);
				}
				return out.x;
			}
		}
		return 0.0f;
	}
	return 1.0f;
}

CMaterialMap::~CMaterialMap()
{
	Cleanup();
}

void CMaterialMap::Cleanup()
{
	{
		vector <CMaterial*>::iterator itCurCP, itEndCP = materials.end();

		for( itCurCP = materials.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete (*itCurCP);
		}
		materials.clear();
	}
}

CMaterial* CMaterialMap::CreateMaterial()
{
	CMaterial* pMat = new CMaterial();
	materials.push_back(pMat);
	return pMat;
}

void CMaterialMap::Read(TypePointer inP,int inSize)
{
	TypePointer p(inP);
	
	while(p.c < inP.c+inSize)
	{
		int size = (*p.dw++)-4;
		CreateMaterial()->Read(p,size);
		p.c+=size;
	}	
}

