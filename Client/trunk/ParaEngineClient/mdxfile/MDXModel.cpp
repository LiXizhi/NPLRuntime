//----------------------------------------------------------------------
// Class:	CMDXModel
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date: 2005/02
// MDX loader is based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer
/*
TODO: there is an unknown bug about loading x file from memory. the directX call always fail.
Fixes 0.1: animation warping is implemented.
The user of CMDModel can set blending[*] parameters in AnimInfo struct to set motion blending.
The blending time can be set by SetBlendingTime(seconds) function. 
Fixes 0.2: BoneID is fixed by calling SortBones(). SortBones() also makes it possible to order
the bones by their parent-child dependecies.
Fixes 0.2: multiple animations set in para-X file are automatically concartenated.
each animation should be no longer than MAX_ANIMATION_LENGTH(10 seconds).However,
it is safer to use only one animation set in each para-X file.
Fixes 0.2: redundant keyframes are not loaded (removed).
if three successive keys are of the same value, the middle key is removed.
*/

#include "dxstdafx.h"
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "utils.h"
#include "MDXModel.h"
#include "math.h"
#include "mdx.h"
#include "ShadowVolume.h"

// for Paraengine x file template registartion
#include <rmxfguid.h>
#include "ParaEngineXFileDef.h"
#include "ParaXFileGUID.h"

namespace ParaEngine
{

ParaXFile* CMDXModel::xData =NULL;

int g_nTimeOffset = 0;
/// the maximum length of animation in milliseconds.
/// animation set are also concatenated by [k*MAX_ANIMATION_LENGTH, (k+1)*MAX_ANIMATION_LENGTH]
#define MAX_ANIMATION_LENGTH		100000


CMDXModel::CMDXModel(void)
{
	dataSize=0;
	modelColor.r=1.0f;
	modelColor.g=0.0f;
	modelColor.b=0.0f;
	modelColor.a=1.0f;

	fBlendingTime = 0.25f;	// this is the default value.

	nDefaultStandIndex = 0;
	nDefaultWalkIndex = 0;
	m_nUseShadowCap = 0;
}
CMDXModel::~CMDXModel(void)
{
	assert(xData == NULL);
}

void CMDXModel::SetBlendingTime(float fTime)
{
	fBlendingTime = fTime;
}

void CMDXModel::InitDeviceObjects()
{
	//texture.InitDeviceObjects();
}

void CMDXModel::DeleteDeviceObjects()
{
	//texture.DeleteDeviceObjects();
}

void CMDXModel::SetModelColor(float r,float g,float b)
{
	modelColor.r=r;
	modelColor.g=g;
	modelColor.b=b;
	modelColor.a=1.0f;
}
void CMDXModel::SetModelColor(D3DXCOLOR modelcolor1)
{
	this->modelColor = modelcolor1;
}

void CMDXModel::PreviousAnim()
{
	UpdateAnim(currentAnimInfo.currentAnim - 1);
}
void CMDXModel::NextAnim()
{
	UpdateAnim(currentAnimInfo.currentAnim + 1);
}
void CMDXModel::UpdateAnim(int animNum)
{
	if(animNum >=0 && animNum < sequences.GetSequencesNum())
	{
		SequenceAnim *anim=sequences.GetSequence(animNum);
		currentAnimInfo.currentAnim=animNum;
		currentAnimInfo.currentFrame = anim->startFrame;
		currentAnimInfo.startFrame = anim->startFrame;
		currentAnimInfo.endFrame = anim->endFrame;
	}
}

/// if not found(at least 2 characters match) return 0, which is the default animation
/// this is tailored to work for warcraft3 naming conventions.
/// return the index of the sequence that matches most characters
/// in the animName
/// any spaces are omitted
int CMDXModel::GetAnimNum(const char* animName)
{
	int n = sequences.GetSequencesNum();
	int nLen = (int)strlen(animName);
	int nLastMatch = 0;
	int nLastMatchCount = 1;
	for(int i = 0; i< n; i++)
	{
		int k,L;
		for(k=0, L=0; k<nLen; k++, L++)
		{
			if(animName[k] != sequences.sequences[i].name[L])
			{
				if(animName[k] == ' ')
				{
					k++;
					continue;
				}
				if(sequences.sequences[i].name[L] == ' '){
					L++;
					continue;
				}
				break;
			}
		}
		if(nLastMatchCount<k)
		{
			nLastMatch = i;
			nLastMatchCount=k;
			if(nLen == nLastMatchCount)
				break;
		}
	}
	return nLastMatch;
}

/// advance the animation. it may loop to the beginning. 
/// currentAnimInfo.currentFrame = currentAnimInfo.startFrame, if the end of animation is reached.
/// the caller can test the above expression to see whether the end of animation has been reached.
void CMDXModel::AdvanceTime( double dTimeDelta )
{
	currentAnimInfo.currentFrame+=(int)(dTimeDelta*1000); //(100-tinterval)/4;
	
	// blending factor is also decreased
	if(currentAnimInfo.blendingFactor >0 )
	{
		currentAnimInfo.blendingFactor -= (float)(dTimeDelta/(double)fBlendingTime); // BLENDING_TIME blending time
		if(currentAnimInfo.blendingFactor<0)
			currentAnimInfo.blendingFactor = 0;
	}

	if(currentAnimInfo.currentFrame>currentAnimInfo.endFrame)
	{
		SequenceAnim *anim=sequences.GetSequence(currentAnimInfo.currentAnim);

		// TODO: motion warping
		if(anim->nonLooping == 1)
		{
			/// load the stand-0 animation.
			UpdateAnim(nDefaultStandIndex);
		}
		else
		{
			/// randomly decide whether to loop or start loiter.
			/// rarity value is used in this case.
			//if(anim->rarity>0.0f && 
			//	(1.0f/(anim->rarity+1))>((float)rand()/(float)RAND_MAX))
			//{
			//	// load the stand-0 animation.
			//	UpdateAnim(nDefaultStandIndex);
			//}
			//else
			{
				// loop
				currentAnimInfo.currentFrame = currentAnimInfo.startFrame; //+(currentAnimInfo.currentFrame- currentAnimInfo.endFrame);
			}
		}
	}
}

void CMDXModel::Render(SceneState* sceneState)
{
	SequenceAnim *anim=sequences.GetSequence(currentAnimInfo.currentAnim);

	skeleton.CalcTransformMatrix(currentAnimInfo);

	sceneState->pd3dDevice->SetVertexShader(NULL);
	sceneState->pd3dDevice->SetFVF(D3D9T_CUSTOMVERTEX);
	DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM_DIF);
	sceneState->pd3dDevice->SetStreamSource( 0, pBufEntity->GetBuffer(), 0, pBufEntity->m_nUnitSize );

	geosets.Render(this,currentAnimInfo);
	
	// LiXizhi: TODO: Add particle system:turn off emitter
	//if(gEmitters == TRUE)
	//skeleton.BlizEmitters2.Render(currentAnimInfo, &skeleton, &texture);
}

float CMDXModel::ComputeBoundingBox(D3DXVECTOR3 *pMin, D3DXVECTOR3 *pMax )
{
	SequenceAnim *anim=sequences.GetSequence(0); // use the 0 sequence only
	if(anim)
	{
		D3DXVECTOR3 mCorners[8];
		*pMin = anim->mins;
		*pMax = anim->maxs;
		return anim->boundsRadius;
	}
	return 0;
}

void CMDXModel::BuildShadowVolume(ShadowVolume * pShadowVolume, LightParams* pLight, D3DXMATRIX* mxWorld)
{
	/// calculate the skeleton matrices
	SequenceAnim *anim=sequences.GetSequence(currentAnimInfo.currentAnim);
	skeleton.CalcTransformMatrix(currentAnimInfo);

#ifdef SHADOW_ZFAIL_WITHOUTCAPS
	pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_PASS;
	geosets.BuildShadowVolume(this,currentAnimInfo, pShadowVolume, pLight, mxWorld);
	pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_FAIL;
#else
	// get the bounding box and check which shadow rendering method we use.
	D3DXVECTOR3 mCorners[8];
	D3DXVECTOR3 mMinimum = anim->mins;
	D3DXVECTOR3 mMaximum = anim->maxs;

	/** check if object is too small, if so its shadow will not be rendered */
	bool bTestScreenDistance = true;
	D3DXVECTOR3 bottom, top;
	if(mMinimum == mMaximum)
	{
		if(anim->boundsRadius > 0)
		{
			bottom = D3DXVECTOR3(0,0,0);
			top = bottom;
			top.z = 2*anim->boundsRadius;
			float fR = anim->boundsRadius;
			mMinimum = D3DXVECTOR3(-fR,-fR,0);
			mMaximum = D3DXVECTOR3(fR,fR,2*fR);
		}
		else
		{
			/** we will not test screen distance if the object neither has a bounding box or a bounding sphere 
			i.e. we will draw its shadow anyway
			*/
			bTestScreenDistance = false; 
		}
	}
	else
	{
		bottom = (mMinimum+mMaximum)/2.0f;
		bottom.z = mMinimum.z;
		top = bottom;
		top.z = mMaximum.z;
	}

	if(bTestScreenDistance)
	{
		D3DXVECTOR3 screenTop, screenBottom;
		pShadowVolume->ProjectPoint(&screenBottom, &bottom, mxWorld);
		pShadowVolume->ProjectPoint(&screenTop, &top, mxWorld);

		float screenDistX = (float)fabs(screenTop.x - screenBottom.x);
		float screenDistY = (float)fabs(screenTop.y - screenBottom.y);
		if(screenDistX<pShadowVolume->m_fMinShadowCastDistance && screenDistY<pShadowVolume->m_fMinShadowCastDistance)
		{
			pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_NONE;
			return;
		}
	}
	/** check to see if object may be in shadow. We only performance this test if object does not
	enforce capping */
	if(m_nUseShadowCap<=0)
	{
		mCorners[0] = mMinimum;
		mCorners[1].x = mMinimum.x; mCorners[1].y = mMaximum.y; mCorners[1].z = mMinimum.z;
		mCorners[2].x = mMaximum.x; mCorners[2].y = mMaximum.y; mCorners[2].z = mMinimum.z;
		mCorners[3].x = mMaximum.x; mCorners[3].y = mMinimum.y; mCorners[3].z = mMinimum.z;            

		mCorners[4] = mMaximum;
		mCorners[5].x = mMinimum.x; mCorners[5].y = mMaximum.y; mCorners[5].z = mMaximum.z;
		mCorners[6].x = mMinimum.x; mCorners[6].y = mMinimum.y; mCorners[6].z = mMaximum.z;
		mCorners[7].x = mMaximum.x; mCorners[7].y = mMinimum.y; mCorners[7].z = mMaximum.z;

		for(int i = 0; i<8;i++)
		{
			D3DXVec3TransformCoord(&mCorners[i], &mCorners[i], mxWorld);
		}

		if( pShadowVolume->PointsInsideOcclusionPyramid(mCorners, 8) || (!bTestScreenDistance))
		{
			/** we use z-fail algorithm, if camera may be in the shadow or the object does not
			performance screen distance testing */
			pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_FAIL;
		}
		else
			pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_PASS;
	}
	else
	{
		/** if object enforces capping, we will use Z_FAIL algorithm */
		pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_FAIL;
	}
	geosets.BuildShadowVolume(this,currentAnimInfo, pShadowVolume, pLight, mxWorld);
#endif
}
/**
* Load mode from file. If it is a para x file, it must has the file extension .x.
* otherwise it will be recognized as MDX file. 
* Files will first be searched in the current directory, then in the directory of
* the current MPQ file.
*/
bool CMDXModel::Load(const char* inName)
{
	// if the file extension is x, we will treat it as Para X file.
	if(strcmp((inName+(int)strlen(inName)-2), ".x")==0)
	{
		bool res;
		try
		{
			res = LoadFromX(inName);
		}
		catch(...)
		{
			OUTPUT_LOG("LoadfromX error");
		}
		return res;
	}

	CParaFile modelfile(inName);
	char* mdxData = modelfile.getBuffer();
	if(mdxData == 0)
		return false;
	dataSize = (int)modelfile.getSize();

	TypePointer p(mdxData);

	if('MDLX' != TAG(*p.dw))
	{
		return false;
	}
	p.dw++;

	while(p.c < mdxData + dataSize)
	{
		switch(TAG(*p.dw))
		{
			case 'ATCH':
			{
				p.dw++;
				int size = *p.dw++;
				//attachs.Read(p,size);
				p.c += size;
			}
			break;

			case 'HELP':
			{
				p.dw++;
				int size = *p.dw++;
				skeleton.ReadHelpers(p,size);
				p.c += size;
			}
			break;

			case 'BONE':
			{
				p.dw++;
				int size = *p.dw++;
				skeleton.ReadBones(p,size);
				p.c += size;
			}
			break;

			case 'VERS':
			{
				p.dw++;
				int size = *p.dw++;
				p.c += size;
			}
			break;

			case 'MODL':
			{
				p.dw++;
				int size = *p.dw++;
				p.c += size;
			}
			break;

			case 'GEOA':
			{
				p.dw++;
				int size = *p.dw++;
				skeleton.ReadGeosetAnims(p,size);
				p.c += size;
			}
			break;

			case 'GEOS':
			{
				p.dw++;
				int size = *p.dw++;
				geosets.Read(p,size);
				p.c += size;
			}
			break;

			case 'SEQS':
			{
				p.dw++;
				int size = *p.dw++;
				sequences.Read(p,size);
				p.c += size; 
			}
			break;

			case 'TEXS':
				{
				p.dw++;
				int size = *p.dw++;
				texture.Read(p,size);
				p.c += size; 				
			}
			break;

			case 'MTLS':
			{
				p.dw++;
				int size = *p.dw++;
				materialMap.Read(p,size);
				p.c += size;
			}
			break;

			case 'PIVT':
			{
				p.dw++;
				int size = *p.dw++;
				skeleton.ReadPivotPoints(p,size);
				p.c += size;  
			}
			break;

			case 'CAMS':
			{
				p.dw++;
				int size = *p.dw++;
				cameras.Read(p, size);
				p.c += size;  
			}
			break;

			case 'PRE2':
			{
				p.dw++;
				int size = *p.dw++;
				skeleton.BlizEmitters2.Read(p, size);
				p.c += size;  
			}
			break;
			//case 'CLID': /* LiXizhi: collision sphere, not all models have it*/
			//{
			//	p.dw++;
			//	int size = *p.dw++;
			//	double collisionSphere = (double)*p.dw;
			//	p.c += size;  
			//}
			//break;
			default:
			{
				p.dw++;
				int size = *p.dw++;
				p.c += size; 
			}
			break;
		}//end of switch
	}//end of while

	skeleton.SortBones();

	// These are the default animation
	nDefaultStandIndex = GetAnimNum("Stand");
	nDefaultWalkIndex = GetAnimNum("Walk");

	modelfile.close();

	return true;
}

bool CMDXModel::LoadFromX(const char* inName)
{
	/// Load animation file ParaEngine X file format

	ID3DXFileEnumObject *pDXEnum = NULL;
	ID3DXFileData       *pDXData = NULL;
	g_nTimeOffset = -MAX_ANIMATION_LENGTH;
	
	// create XFile object and register templates, if we have not done so.
	LPD3DXFILE pDXFile = CGlobals::GetAssetManager()->GetParaXFileParser();
	
	CParaFile modelfile(inName);
	char* mdxData = modelfile.getBuffer();
	if(mdxData == 0)
		return FALSE;
	DWORD dataSize = (int)modelfile.getSize();

	if(dataSize>0)
	{
		D3DXF_FILELOADMEMORY memData;
		memData.dSize = dataSize;
		memData.lpMemory = mdxData;

		if(SUCCEEDED(pDXFile->CreateEnumObject(&memData, D3DXF_FILELOAD_FROMMEMORY, &pDXEnum)))
		{
			// loaded.	
		}
		else
		{
			return FALSE;
		}
	}

	xData = new ParaXFile();
	// Enumerate all top-level templates
	SIZE_T nCount;
	pDXEnum->GetChildren(&nCount);
	for(int i = 0; i<(int)nCount;i++)
	{
		pDXEnum->GetChild(i, &pDXData);
		ParseXFileData(pDXData);
		SAFE_RELEASE(pDXData);
	}

	// Release objects
	SAFE_RELEASE(pDXEnum);
	
	// Build data from ParaXFile
	BuildFromX();

	// These are the default animation
	nDefaultStandIndex = GetAnimNum("Stand");
	nDefaultWalkIndex = GetAnimNum("Walk");

	SAFE_DELETE(xData);
	modelfile.close();
	return true;
}

void CMDXModel::ParseXFileData(ID3DXFileData *pData)
{
	static int nParantID;
	static int nCurrentAnimFrame;
	ID3DXFileData   *pSubData = NULL;
	GUID Type;
	DWORD       dwSize;
	const char       *pBuffer;
	  
	// Get the template type
	if(FAILED(pData->GetType(&Type)))
		return;
	
	// See what the template was and deal with it
	/**
	* for frame information
	*/
	if(Type == TID_D3DRMFrame) 
	{
		// Frame template found
		ParaXFrame* pFrame = new ParaXFrame();
		if(xData->frames.empty())
		{
			pFrame->parentID = -1;
		}
		else
			pFrame->parentID = nParantID;
		xData->frames.push_back(pFrame);
		nParantID = ((int)xData->frames.size()-1);

		// Get the frame name (if any)
		if(FAILED(pData->GetName(NULL, &dwSize)))
			pFrame->name[0] = '\0';
		if(SUCCEEDED(pData->GetName(pFrame->name, &dwSize)))
		{
			pFrame->name[dwSize] = '\0';
		}

		// Scan for embedded templates
		SIZE_T nCount;
		pData->GetChildren(&nCount);
		for(int i = 0; i<(int)nCount;i++)
		{
			pData->GetChild(i, &pSubData);
			if(pSubData->IsReference() == false)
			{
				ParseXFileData(pSubData);
			}
			SAFE_RELEASE(pSubData);
		}

		nParantID = pFrame->parentID;
		return;
	}
	else if(Type == TID_D3DRMFrameTransformMatrix) 
	{
		// Frame transformation matrix template found

		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		D3DXMATRIX matTrans((FLOAT*)pBuffer);
		xData->frames.back()->matrix = matTrans;
		pData->Unlock();
	}
	/**
	* for mesh information
	*/
	else if(Type == TID_D3DRMMesh) 
	{
		// Mesh template found
		xData->geoChunks.push_back(new ParaXGeoChunk());
		xData->geoChunks.back()->nParentFrame = nParantID;
		
		ParaXMesh* myMesh = &(xData->geoChunks.back()->mesh);
		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		myMesh->nVertices =  *((DWORD*)pBuffer);
		pBuffer+=sizeof(DWORD);

		myMesh->vertices =  new D3DXVECTOR3[myMesh->nVertices];
		memcpy(myMesh->vertices, (D3DXVECTOR3*)pBuffer, sizeof(D3DXVECTOR3)*myMesh->nVertices);
		pBuffer+=sizeof(D3DXVECTOR3)*myMesh->nVertices;

		myMesh->nFaces =  *((DWORD*)pBuffer);
		pBuffer+=sizeof(DWORD);

		myMesh->faces = new ParaXMeshFace[myMesh->nFaces];
		memcpy(myMesh->faces , pBuffer, sizeof(ParaXMeshFace)*myMesh->nFaces);
		pData->Unlock();

		// Scan for embedded templates
		SIZE_T nCount;
		pData->GetChildren(&nCount);
		for(int i = 0; i<(int)nCount;i++)
		{
			pData->GetChild(i, &pSubData);
			if(pSubData->IsReference() == false)
			{
				ParseXFileData(pSubData);
			}
			SAFE_RELEASE(pSubData);
		}
		return;
	}
	else if(Type == TID_D3DRMMeshNormals) 
	{
		// Mesh normal template found
		ParaXNorm* myNorm = &(xData->geoChunks.back()->norm);
		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		myNorm->nNormals =  *((DWORD*)pBuffer);pBuffer+=sizeof(DWORD);
		
		myNorm->normals =  new D3DXVECTOR3[myNorm->nNormals ];
		memcpy(myNorm->normals, pBuffer, sizeof(D3DXVECTOR3)*myNorm->nNormals);
		pBuffer+=sizeof(D3DXVECTOR3)*myNorm->nNormals;

		myNorm->nFaces =  *((DWORD*)pBuffer);pBuffer+=sizeof(DWORD);
		
		myNorm->faces = new ParaXMeshFace[myNorm->nFaces];
		memcpy(myNorm->faces , pBuffer, sizeof(ParaXMeshFace)*myNorm->nFaces);
		pData->Unlock();
	}
	else if(Type == TID_D3DRMMeshTextureCoords) 
	{
		// Mesh Texture template found
		ParaXTexUV* myTex = &(xData->geoChunks.back()->uvs);
		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		myTex->nTextureCoords =  *((DWORD*)pBuffer);pBuffer+=sizeof(DWORD);
		
		myTex->vertices =  new D3DXVECTOR2[myTex->nTextureCoords];
		memcpy(myTex->vertices, pBuffer, sizeof(D3DXVECTOR2)*myTex->nTextureCoords);
		pData->Unlock();
	}
	else if(Type == TID_D3DRMMeshMaterialList) 
	{
		// Materials list found
		ParaXMeshMaterialList* pMatlist = &(xData->geoChunks.back()->matList);
		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		pMatlist->nMaterials = *((DWORD*)pBuffer);
		pBuffer+=sizeof(DWORD);
		pMatlist->nFaceIndexes = *((DWORD*)pBuffer);
		pBuffer+=sizeof(DWORD);
		pData->Unlock();
	}
	else if(Type == TID_D3DRMMaterial) 
	{
		ParaXMeshMaterialList* pMatlist = &(xData->geoChunks.back()->matList);
		ParaXMaterial* pMat = new ParaXMaterial();
		pMatlist->materials.push_back(pMat);
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		pMat->faceColor = D3DXCOLOR(*((FLOAT*)pBuffer), *(((FLOAT*)pBuffer)+1), *(((FLOAT*)pBuffer)+2), *(((FLOAT*)pBuffer)+3));
		pBuffer+=16;
		pMat->power = *((FLOAT*)pBuffer);
		pBuffer+=4;
		pMat->specularColor = D3DXCOLOR(*((FLOAT*)pBuffer), *(((FLOAT*)pBuffer)+1), *(((FLOAT*)pBuffer)+2), 1.0f);
		pBuffer+=12;
		pMat->emissiveColor = D3DXCOLOR(*((FLOAT*)pBuffer), *(((FLOAT*)pBuffer)+1), *(((FLOAT*)pBuffer)+2), 1.0f);
		pBuffer+=12;
		pData->Unlock();
	}
	else if(Type == TID_D3DRMTextureFilename) 
	{
		ParaXMaterial* pMat = xData->geoChunks.back()->matList.materials.back();
		if(pMat)
		{
			if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
				return;
			int nLen;
			const char* pName = (const char*)pBuffer;
			if((nLen=(int)strlen(pName))>0)
			{
				pMat->TextureFilename = new char[nLen+1];
				strcpy(pMat->TextureFilename, pName);
			}
			else
				pMat->TextureFilename = NULL;
			pData->Unlock();
		}
	}
	else if(Type == TID_SkinWeights) 
	{
		// Skin weights template found
		ParaXSkinWeight* pSkinWeight = new ParaXSkinWeight ();
		xData->geoChunks.back()->listWeights.push_back(pSkinWeight);

		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		strcpy(pSkinWeight->transformNodeName, (const char*)(pBuffer));
		pBuffer+=strlen((const char*)(pBuffer))+1;
		pSkinWeight->nWeights = *((DWORD*)pBuffer);pBuffer+=4;
		pSkinWeight->vertexIndices = new DWORD[pSkinWeight->nWeights];
		memcpy (pSkinWeight->vertexIndices, pBuffer, 4*pSkinWeight->nWeights);
		pBuffer+=4*pSkinWeight->nWeights;
		pSkinWeight->weights = new FLOAT[pSkinWeight->nWeights];
		memcpy (pSkinWeight->weights, pBuffer, 4*pSkinWeight->nWeights);
		pBuffer+=4*pSkinWeight->nWeights;
		pSkinWeight->matrixOffset = *((D3DXMATRIX*)pBuffer);
		pData->Unlock();
	}
	/** 
	* for paraengine specific info 
	*/
	else if(Type == TID_ParaEngine)
	{
		// search child
	}
	else if(Type == TID_Anim) 
	{
		ParaXAnim* pAnim = new ParaXAnim ();
		xData->anims.push_back(pAnim);

		// Get the template data
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;

		int nNameLen = (int)strlen(pBuffer);
		pAnim->name = new char[nNameLen+1];
		strcpy(pAnim->name, pBuffer);
		pBuffer += nNameLen+1;
		memcpy(((char*)pAnim+4), pBuffer, sizeof(ParaXAnim)-4);
		pData->Unlock();
	}
	else if(Type == TID_Sequences)
	{
		// search child
	}
	else if(Type == TID_AlphaSet)
	{
		// search child
	}
	else if(Type == TID_Alpha)
	{
		// search child
	}
	else if(Type == TID_AlphaKey)
	{
		// search child
	}
	/** 
	* for X animation 
	*/
	else if(Type == TID_D3DRMAnimation)
	{
		bool bFound = false;
	
		// Scan for embedded templates
		SIZE_T nCount;
		pData->GetChildren(&nCount);
		for(int i = 0; i<(int)nCount;i++)
		{
			pData->GetChild(i, &pSubData);
			// Process embedded references
			if(pSubData->IsReference())
			{
				// get the frame reference name
				if(SUCCEEDED(pSubData->GetName(NULL, &dwSize)))
				{
					char name[80];
					if(SUCCEEDED(pSubData->GetName(name, &dwSize)))
					{
						nCurrentAnimFrame = xData->GetFrameByName(name);
						if(nCurrentAnimFrame>0)
						{
							bFound = true;
							/// add a new animation
							ParaXFrame::Animation* anim = new ParaXFrame::Animation();
							anim->timeOffset = g_nTimeOffset;
							ParaXFrame* pFrame = xData->frames[nCurrentAnimFrame];
							pFrame->animations.push_back(anim);
						}
					}	
				}
			}
			if(bFound)
			{
				// Scan for animation keys
				// Process non-referenced embedded templates
				if(pSubData->IsReference() == false)
				{
					/// parse animation keys
					ParseXFileData(pSubData);
				}
			}
			SAFE_RELEASE(pSubData);
		}
		
		return;
	}
	else if(Type == TID_D3DRMAnimationSet)
	{
		// search child
		g_nTimeOffset += MAX_ANIMATION_LENGTH;
	}
	else if(Type == TID_D3DRMAnimationKey)
	{
		if(FAILED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			return;
		ParaXFrame* pFrame = xData->frames[nCurrentAnimFrame];
		DWORD keyType = *((DWORD*)pBuffer);
		if((dwSize > 0) && (!pFrame->animations.empty()))
		{
			ParaXFrame::Animation* anim = pFrame->animations.back();

			anim->keyFrameSize[keyType] = dwSize;
			anim->keyFrames[keyType] = new char[dwSize];
			memcpy(anim->keyFrames[keyType], pBuffer, dwSize);
		}
		pData->Unlock();
	}
	else
	{
		return;
	}

	/** 
	* Scan for embedded templates 
	*/
	SIZE_T nCount;
	pData->GetChildren(&nCount);
	for(int i = 0; i<(int)nCount;i++)
	{
		pData->GetChild(i, &pSubData);
		
		if(pSubData->IsReference())
		{
			// Process embedded references
			// references are not parsed
		}
		else 
		{
			// Process non-referenced embedded templates
			ParseXFileData(pSubData);
		}
		SAFE_RELEASE(pSubData);
	}
}

bool CMDXModel::BuildFromX()
{
	int k=0;
	{ /// build animations

		sequences.numSequences = (int) xData->anims.size();
		sequences.sequences = new SequenceAnim[sequences.numSequences];
		
		vector< ParaXAnim* >::iterator itCurCP, itEndCP = xData->anims.end();
		
		int i=0;
		for( itCurCP = xData->anims.begin(); itCurCP != itEndCP; ++ itCurCP, ++i)
		{
			ParaXAnim* pAnim = (*itCurCP);
			strcpy(sequences.sequences[i].name, pAnim->name);
			sequences.sequences[i].boundsRadius = pAnim->boundsRadius;
			sequences.sequences[i].endFrame = pAnim->to;
			sequences.sequences[i].startFrame = pAnim->from;
			
			sequences.sequences[i].maxs = pAnim->maxEntent;
			sequences.sequences[i].mins = pAnim->minEntent;
			sequences.sequences[i].nonLooping = (pAnim->nextAnim!=-1);
			sequences.sequences[i].moveSpeed = pAnim->moveSpeed;
		}
	}
	{ /// build GeoChunks
		
		vector< ParaXGeoChunk* >::iterator itCurCP, itEndCP = xData->geoChunks.end();

		for( itCurCP = xData->geoChunks.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			CGeoChunk* pChunk = this->geosets.AddNewChunk();
			ParaXGeoChunk * pXchunk = (*itCurCP);
			/* load vertice and triangles */
			// copy vertices
			pChunk->numVertices = pXchunk->mesh.nVertices;
			pChunk->vertices = new D3DXVECTOR3[pChunk->numVertices];
			memcpy(pChunk->vertices, pXchunk->mesh.vertices, sizeof(D3DXVECTOR3)*pChunk->numVertices);
			
			// copy triangles
			pChunk->numTriangles = pXchunk->mesh.nFaces;
			pChunk->triangles = new short[pChunk->numTriangles*3];
			
			for(int i=0; i<(int)pXchunk->mesh.nFaces; i++)
			{
				int nVCount = pXchunk->mesh.faces[i].nFaceVertexIndices; // normally this value is 3
				// TODO: if the face has more than 3 vertex, than multiple triangles should be added
				for (int k=0; k<3; k++)
				{
					pChunk->triangles[i*3+k]= (short)(pXchunk->mesh.faces[i].faceVertexIndices[k]); 
				}
			}

			/* load norm */
			/// calculate and save norms for each vertice
			pChunk->normals = new D3DXVECTOR3[pChunk->numVertices];
			memset(pChunk->normals, 0, sizeof(D3DXVECTOR3)*pChunk->numVertices);
			
			assert(pXchunk->norm.nFaces == pChunk->numTriangles);

			for(int i=0; i<(int)pXchunk->norm.nFaces; i++)
			{
				int nVCount = pXchunk->norm.faces[i].nFaceVertexIndices; // normally this value is 3
				// TODO: if the face has more than 3 vertex
				for (int k=0; k<3; k++)
				{
					pChunk->normals[pChunk->triangles[i*3+k]] = pXchunk->norm.normals[pXchunk->norm.faces[i].faceVertexIndices[k]]; 
				}
			}	

			/* load UV */
			/// copy UV coordinates
			assert(pXchunk->uvs.nTextureCoords == pChunk->numVertices);
			pChunk->uvs = new D3DXVECTOR2[pChunk->numVertices];
			memcpy(pChunk->uvs, pXchunk->uvs.vertices, sizeof(D3DXVECTOR2)*pChunk->numVertices);
			
			/// build material maps
			{
				CMaterial* pMat =  materialMap.CreateMaterial();
				pChunk->materialID = (int)materialMap.materials.size()-1;
				Layer* pLayer =  pMat->CreateLayer();

				/// TODO: currently only a single layer is supported for any geo chunk
				pLayer->filterMode = 0; // none
				pLayer->shading = 0;
				pLayer->alpha = 1.0;
				pLayer->textureID = NULL;
				pLayer->existKMTA = false;

				vector< ParaXMaterial* >::iterator itCurCP, itEndCP = pXchunk->matList.materials.end();

				for( itCurCP = pXchunk->matList.materials.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					if((*itCurCP)->TextureFilename)
					{
						MyD3DTexture* pTex = texture.CreateTexture((*itCurCP)->TextureFilename);
						pLayer->textureID = (int)texture.m_pTextures.size()-1;
						// TODO: set bitmap width and height.
					}
					pLayer->alpha = (*itCurCP)->faceColor.a;
					break;
				}
			}
			
			/* build groups*/
			{
				/// build bone binding groups
				int nVerticeNum = pChunk->numVertices;

				/// we allow each vertex to bind to at most BIND_BONE_NUM bones at a time
				/// if more than three bones are available, we will select the three with the highest weight
				FrameBind (* vertexFrameBind)[BIND_BONE_NUM] = new FrameBind[nVerticeNum][BIND_BONE_NUM];
				memset(vertexFrameBind, 0, sizeof(FrameBind)*BIND_BONE_NUM*nVerticeNum);
				
				/// get the transform node index(ID)
				list< ParaXSkinWeight* >::iterator itCurCP, itEndCP = pXchunk->listWeights.end();

				for( itCurCP = pXchunk->listWeights.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					(*itCurCP)->transformNodeID = xData->GetFrameByName((*itCurCP)->transformNodeName);
					xData->frames[(*itCurCP)->transformNodeID]->offsetMatrix = (*itCurCP)->matrixOffset;

					for(int i=0; i<(int)(*itCurCP)->nWeights; i++)
					{
						int vertexindex = (*itCurCP)->vertexIndices[i];
						float weight = (*itCurCP)->weights[i];
						int FrameID = (*itCurCP)->transformNodeID;
						// insert from big to small
						for(k=0;k<BIND_BONE_NUM; k++)
						{
							if(weight>vertexFrameBind[vertexindex][k].fWeight)
							{
								float tmp = vertexFrameBind[vertexindex][k].fWeight;
								vertexFrameBind[vertexindex][k].fWeight = weight;
								weight = tmp;
								int tmpFM = vertexFrameBind[vertexindex][k].nFrameIndex;
								vertexFrameBind[vertexindex][k].nFrameIndex = FrameID;
								FrameID = tmpFM;
							}
						}
					}
				}
				pChunk->vertexGroups = new char[nVerticeNum];
				vector <int> groups;
				list <int> matrice;

				for(int i=0; i<nVerticeNum;i++)
				{
					/** build matrix
					count=1: w[0]=1
					count=2: w[0]=0.6, w[1]=0.4
					count=3: w[0]=0.4, w[1]=0.3, w[2]=0.3
					*/
					int matrix[BIND_BONE_NUM];
					int count=0;
					for(k=0;k<BIND_BONE_NUM; k++)
					{
						if(vertexFrameBind[i][k].fWeight>0.8)
						{
							matrix[0] = vertexFrameBind[i][k].nFrameIndex;
							count = 1;
							break;
						}
						else if(vertexFrameBind[i][k].fWeight<0.2)
						{
							break;
						}
						else
						{
							matrix[k] = vertexFrameBind[i][k].nFrameIndex;
							count++;
						}
					}
					/// if the vertex is not bound to any frame, then the parent frame of this mesh is used
					if(count == 0)
					{
						count=1;
						matrix[0] = pXchunk->nParentFrame;
					}
					/// convert to matrix: i.e. frame array

					/// search in existing groups
					bool bFound = false;
					// group index
					list< int >::iterator itCurCP, itEndCP = matrice.end();
					k=0;
					for( itCurCP = matrice.begin(); itCurCP != itEndCP; k++)
					{
						bFound = true;
						int group = groups[k];
						if (count != group)
						{
							bFound = false;
							for(int m=0; m<group; m++)
								++itCurCP;
						}
						else
						{
							for(int m=0; m<group; m++)
							{
								if(matrix[m] != *itCurCP)
								{
									bFound = false;
								}
								++itCurCP;
							}
						}
						if(bFound)
						{
							pChunk->vertexGroups[i] = (BYTE)k;
							break;
						}
					} // for
					if(!bFound)
					{
						groups.push_back(count);
						for(k=0; k<count; k++)
						{
							matrice.push_back(matrix[k]);
						}
						pChunk->vertexGroups[i] = (BYTE)(groups.size()-1);
					}
				}//for(int i=0; i<nVerticeNum;i++)

				delete [] vertexFrameBind;

				/// build matrices
				pChunk->numGroups = (int)groups.size();
				pChunk->groups = new int[pChunk->numGroups];
				for(k=0; k<pChunk->numGroups;k++)
				{
					pChunk->groups[k] = groups[k];
				}
				
				pChunk->numMatrixGroups = (int)matrice.size();
				pChunk->matrixGroups = new int[pChunk->numMatrixGroups];
				{
					list< int >::iterator itCurCP, itEndCP = matrice.end();
					k=0; int n=0;
					for( itCurCP = matrice.begin(); itCurCP != itEndCP; k++)
					{
						int group = pChunk->groups[k];
						for(int m=0; m<group; m++)
						{
							pChunk->matrixGroups[n++] = *itCurCP;
							++itCurCP;
						}
					}
				}
			}
		}//for( itCurCP = xData->geoChunks.begin()
	}
	{ // build skeletons
		vector< ParaXFrame* >::iterator itCurCP, itEndCP = xData->frames.end();
		int i=0;
		for( itCurCP = xData->frames.begin(); itCurCP != itEndCP; ++ itCurCP, ++i)
		{
			ParaXFrame* pFrame = *itCurCP;
			CBone1* pBone = skeleton.CreateNewBone();
			pBone->objectID = i;
			pBone->parentID = pFrame->parentID;
			int nLen = (int)strlen(pFrame->name);
			if(nLen>0)
			{
				pBone->boneName = new char[nLen+1];
				strcpy(pBone->boneName, pFrame->name);
			}
			pBone->geosetID = 0;
			pBone->geosetAnimID = 0;
			pBone->localMatrix = pFrame->matrix;
			pBone->offsetMatrix = pFrame->offsetMatrix;

			
			// load animation for translation, rotation, scaling, etc
			for(int m=0;m<CBone1::MAX_MOTION;m++)
			{
				/// remove redundent keys.
				/// and load in to memory only significant keys
				list <void*> listKeyframes; // store useful keys

				/// search all keys, combine keys in different animation set and combine redundent keys
				list< ParaXFrame::Animation* >::iterator itAnimCurCP, itAnimEndCP = pFrame->animations.end();
				for( itAnimCurCP = pFrame->animations.begin(); itAnimCurCP != itAnimEndCP; ++ itAnimCurCP)
				{
					ParaXFrame::Animation* pAnim = *itAnimCurCP;
					if(pAnim->keyFrameSize[m] == 0)
						continue;
					char * pBuffer = (char *)(pAnim->keyFrames[m]);

					DWORD keyType = *((DWORD*)pBuffer);
					assert(keyType == m);
					pBuffer+=sizeof(DWORD);
					int numKeys = (int)(*((DWORD*)pBuffer));
					pBuffer+=sizeof(DWORD);
					if(numKeys <= 0)
						continue;
					if(keyType == CBone1::KGRT)
					{
						for(k=0; k<numKeys; k++)
						{
							ParaXKeyFrame4* curFrame =(ParaXKeyFrame4*) pBuffer;
							curFrame->frameNum += pAnim->timeOffset;
							pBuffer+=sizeof(ParaXKeyFrame4);
							if((listKeyframes.size()<2) || (k<2))
							{
								/// at least two keys at the beginning and end of an animation set
								listKeyframes.push_back(curFrame);
							}
							else
							{
								/// remove redundent keys, if three succesive keys are of the same value, the middle key is removed.
								ParaXKeyFrame4* lastFrame = (ParaXKeyFrame4*)listKeyframes.back();
								list< void* >::iterator lastSecond = listKeyframes.end(); 
								lastSecond--;lastSecond--;
								ParaXKeyFrame4* lastFrame2 = (ParaXKeyFrame4*)(*lastSecond);

								if (lastFrame->comparekey(lastFrame2) && lastFrame->comparekey(curFrame))
								{
									listKeyframes.back() = curFrame;
								}
								else
									listKeyframes.push_back(curFrame);
							}
						}
					}
					else
					{
						for(k=0; k<numKeys; k++)
						{
							ParaXKeyFrame3* curFrame =(ParaXKeyFrame3*) pBuffer;
							curFrame->frameNum += pAnim->timeOffset;
							pBuffer+=sizeof(ParaXKeyFrame3);
							if((listKeyframes.size()<2) || (k<2))
							{
								/// at least two keys at the beginning and end of an animation set
								listKeyframes.push_back(curFrame);
							}
							else
							{
								/// remove redundent keys, if there succesive keys are of the same value, the middle key is removed.
								ParaXKeyFrame3* lastFrame = (ParaXKeyFrame3*)listKeyframes.back();
								list< void* >::iterator lastSecond = listKeyframes.end(); 
								lastSecond--;lastSecond--;
								ParaXKeyFrame3* lastFrame2 = (ParaXKeyFrame3*)(*lastSecond);

								if (lastFrame->comparekey(lastFrame2) && lastFrame->comparekey(curFrame))
								{
									listKeyframes.back() = curFrame;
								}
								else
									listKeyframes.push_back(curFrame);
							}
						}
					}
				}

				/// load compressed keys into bone animation key frames

				pBone->lineType[m] = CBone1::LINEAR;
				int numKeys = (int)listKeyframes.size();
				pBone->keyFrameCount[m] = numKeys;
				if(numKeys<=0)
					continue;
				int nKeyOffset = 0;
				if(m == CBone1::KGRT)
				{
					pBone->keyFrames[m] = new char[pBone->keyFrameCount[m]*sizeof(LinearKeyFrame4)];
					LinearKeyFrame4 * pkeys = (LinearKeyFrame4 *) (pBone->keyFrames[m]);

					list< void* >::iterator itKeyCurCP, itKeyEndCP = listKeyframes.end();

					k=0;
					for( itKeyCurCP = listKeyframes.begin(); itKeyCurCP != itKeyEndCP; ++ itKeyCurCP, ++k)
					{
						ParaXKeyFrame4* curKey = ((ParaXKeyFrame4*)(*itKeyCurCP));
						pkeys[k].frameNum = curKey->frameNum + nKeyOffset;
						if((k>=1) && (pkeys[k].frameNum <= pkeys[k-1].frameNum))
						{
							OutputDebugString("keyframe overrun");
							int diff = pkeys[k].frameNum - pkeys[k-1].frameNum+10;
							nKeyOffset+=diff;
							pkeys[k].frameNum += diff;
						}
						/** 
						* So far, I donot know why the quaternion keyframe in X file format is 
						* defined so strange. instead of (x,y,z,w), it is saved as (w, -x,-y,-z)
						* if the file format standard is changed, we need to reedit the following code.
						*/
						pkeys[k].vec.w = curKey->key[0];
						pkeys[k].vec.x = -curKey->key[1];
						pkeys[k].vec.y = -curKey->key[2];
						pkeys[k].vec.z = -curKey->key[3];
					}
				}
				else
				{
					pBone->keyFrames[m] = new char[pBone->keyFrameCount[m]*sizeof(LinearKeyFrame3)];
					LinearKeyFrame3 * pkeys = (LinearKeyFrame3 *) (pBone->keyFrames[m]);

					list< void* >::iterator itKeyCurCP, itKeyEndCP = listKeyframes.end();

					k=0;
					for( itKeyCurCP = listKeyframes.begin(); itKeyCurCP != itKeyEndCP; ++ itKeyCurCP, ++k)
					{
						ParaXKeyFrame3* curKey = ((ParaXKeyFrame3*)(*itKeyCurCP));
						pkeys[k].frameNum = curKey->frameNum + nKeyOffset;
						if((k>=1) && (pkeys[k].frameNum <= pkeys[k-1].frameNum))
						{
							OutputDebugString("keyframe overrun");
							int diff = pkeys[k].frameNum - pkeys[k-1].frameNum+10;
							nKeyOffset+=diff;
							pkeys[k].frameNum += diff;
						}
						pkeys[k].vec.x = curKey->key[0];
						pkeys[k].vec.y = curKey->key[1];
						pkeys[k].vec.z = curKey->key[2];
					}
				}
			}
		}
	}

	return true;
}

/// if bText is true, it will be saved as text version, otherwise, binary format
bool CMDXModel::SaveToX(const char* inName, bool bText)
{
	/// save animation to ParaEngine X file
	// TODO: 
	return true;
}


CCameras::~CCameras()
{
	Cleanup();
}

void CCameras::Cleanup()
{
	if(mCameras)
	{
		delete [] mCameras;
		mCameras = NULL;
	}
}

// one time read. only call this function once for every instance
void CCameras::Read(TypePointer inP, int inSize)
{
	// one time call, any previous call to Read(...)  will be overridden
	Cleanup(); 

	long csize = *inP.dw++;
	mNumCameras = inSize/csize;

	// create cameras
	mCameras = new camera[mNumCameras];

	for(int k=0; k<mNumCameras; ++k)
	{
		memcpy(&mCameras[k], inP.p, csize);
		inP.c += csize;
	}
	mLoaded = TRUE;
}


void CCameras::Render()
{

}
}

