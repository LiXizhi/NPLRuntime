//----------------------------------------------------------------------
// Class:	CBone1
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class  based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer
#include "ParaEngine.h"
#include "math.h"
#include "math3d.h"
#include "bone.h"
#include "mdx.h"
#include "EngineUtil.h"
#include "memdebug.h"
using namespace ParaEngine;

#ifndef min 
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

CBone1::CBone1(void)
{
	boneName=NULL;
	objectID=-1;
	parentID=-1;
	nextID = -1;
	geosetID=-1;
	geosetAnimID=-1;

	geosetAnimAlpha=1.0f;

	D3DXMatrixIdentity(&localMatrix);
	D3DXMatrixIdentity(&offsetMatrix);
	
	for(int i=0;i<MAX_MOTION;i++)
	{
		keyFrameCount[i]=0;
		lineType[i]=0;
		keyFrames[i]=NULL;
	}
}

CBone1::~CBone1(void)
{
	if(boneName)
		delete [] boneName;

	for(int i=0;i<MAX_MOTION;i++)
	{
		if(keyFrames[i])
			delete [] keyFrames[i];
	}
}

void CBone1::Read(TypePointer inP,int inSize)
{
	TypePointer p(inP);
	int iread = 0;

	boneName = new char[(int)strlen(p.c)+1];
	strcpy(boneName, p.c);

	p.c+=80;
	objectID=*(p.dw++);
	
	parentID=*(p.dw++);
	nodeType=*(p.dw++);
	char *nextAddr=inP.c+inSize;

	while((p.c < inP.c+inSize) && (iread < 3))
	{
		//trasfer
		if( TAG(*p.dw) == 'KGTR' ){
			++iread;
			p.dw++;
			keyFrameCount[KGTR]=*(p.dw++);
			lineType[KGTR]=*(p.dw++);
			p.dw++;
			int nLen;
			//non linear trasfer
			if( lineType[KGTR] > LINEAR )
				nLen=keyFrameCount[KGTR]*sizeof(NonLinearKeyFrame3);
			else	 
				nLen=keyFrameCount[KGTR]*sizeof(LinearKeyFrame3);
			
			keyFrames[KGTR]= new char[nLen];
			memcpy(keyFrames[KGTR], p.c, nLen);

			p.c+=nLen;

		}
		else
		//rotation
		if(TAG(*p.dw) == 'KGRT'){
			++iread;
			p.dw++;
			keyFrameCount[KGRT]=*(p.dw++);
			lineType[KGRT]=*(p.dw++);
			p.dw++;
			//non linear rotaion
			int nLen;
			if( lineType[KGRT] > LINEAR )
				nLen=keyFrameCount[KGRT]*sizeof(NonLinearKeyFrame4);
			else
				nLen=keyFrameCount[KGRT]*sizeof(LinearKeyFrame4);

			keyFrames[KGRT]= new char[nLen];
			memcpy(keyFrames[KGRT], p.c, nLen);

			p.c+=nLen;

		}
		else
		//scale
		if(TAG(*p.dw) == 'KGSC'){
			++iread;
			p.dw++;
			keyFrameCount[KGSC]=*(p.dw++);
			lineType[KGSC]=*(p.dw++);
			p.dw++;
			//keyFrames[KGSC]=p.p;
			int nLen;
			//non linear scale
			if( lineType[KGSC] > LINEAR )
				nLen=keyFrameCount[KGSC]*sizeof(NonLinearKeyFrame3);
			else	 
				nLen=keyFrameCount[KGSC]*sizeof(LinearKeyFrame3);

			keyFrames[KGSC]= new char[nLen];
			memcpy(keyFrames[KGSC], p.c, nLen);

			p.c+=nLen;
		}
		else
			break;
	}
	
	if(p.c < inP.c+inSize-4){
		geosetID=*(p.dw++);
		geosetAnimID=*(p.dw++);
	}

}

bool CBone1::GetTransferMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo)
{
	if(animInfo.blendingFactor == 0.0f)
	{
		return GetTransferMatrixNoneBlending(vec, animInfo);
	}
	else 
	{
		AnimInfo animI;
		animI.currentAnim = animInfo.currentAnim;
		animI.startFrame = animInfo.blendingStartFrame;
		animI.currentFrame = animInfo.blendingFrame;
		animI.endFrame = animInfo.blendingEndFrame;
		animI.blendingFrame = animInfo.blendingFrame;

		if(animInfo.blendingFactor == 1.0f)
		{
			return GetTransferMatrixNoneBlending(vec, animI);
		}
		else
		{
			D3DXVECTOR3 vec1, vec2;
			if(! GetTransferMatrixNoneBlending(&vec1, animI))
				return GetTransferMatrixNoneBlending(vec, animInfo); 

			if (! GetTransferMatrixNoneBlending(&vec2, animInfo))
			{
				
				return false;
			}
			// Perform linear motion warping, here
			D3DXVec3Lerp (vec, &vec2, &vec1, animInfo.blendingFactor);
			return true;
		}
	}
	return false;
}

bool CBone1::GetTransferMatrixNoneBlending(D3DXVECTOR3* vec,const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGTR];
	int		type	=	lineType[KGTR];
	void*	frames	=	keyFrames[KGTR];

	/** by LiXizhi: 2005/02:
	* modify the brutal force search by binary search
	* and use interpolation.
	1.  location = -1;
	2.  while ((more than one item in list))
		2A.  look at the middle item
		2B.  if (middle item is target)
				have found target
				 return location as the result
			else
				2C.  if (target < middle item)
							list = first half of list
				2D.  else (target > middle item)
							list = last half of list
		end while
	*/
	if(type == LINEAR)
	{
		LinearKeyFrame3 *p = (LinearKeyFrame3 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			LinearKeyFrame3 *startP=&(p[nMid]);
			LinearKeyFrame3 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(animInfo.currentFrame == startP->frameNum)
				{
					*vec = startP->vec;
					return true;
				}
				else if(animInfo.currentFrame == endP->frameNum)
				{
					*vec = endP->vec;
					return true;
				}
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
				{
					break;
				}

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXVec3Lerp (vec, &startP->vec, &endP->vec, step);
				}else
					*vec=startP->vec;
				return true;
				// break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}else
	if(type > LINEAR)
	{
		NonLinearKeyFrame3 *p=(NonLinearKeyFrame3 *)frames;

		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			NonLinearKeyFrame3 *startP=&(p[nMid]);
			NonLinearKeyFrame3 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(animInfo.currentFrame == startP->frameNum)
				{
					*vec = startP->vec;
					return true;
				}
				else if(animInfo.currentFrame == endP->frameNum)
				{
					*vec = endP->vec;
					return true;
				}
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
				{
					break;
				}

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					// I didnot figure out the current Hermite parameter used in MDX model
					// So I will use Linear interpolation instead.
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXVec3Lerp (vec, &startP->vec, &endP->vec, step);
					/*if(type== HERMITE)
						D3DXVec3Hermite(vec, &startP->vec, &startP->outTan, &endP->vec, &endP->inTan, step);
					else
					if(type== BEZIER)
						D3DXVec3Bezier(vec, &startP->vec, &startP->outTan, &endP->vec, &endP->inTan, step);
					else
						break;*/
				}else
					*vec=startP->vec;
				return true;
				// break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}
	return false;
}

bool CBone1::GetRotationMatrix(D3DXQUATERNION* vec,const AnimInfo& animInfo)
{
	if(animInfo.blendingFactor == 0.0f)
	{
		return GetRotationMatrixNoneBlending(vec, animInfo);
	}
	else 
	{
		AnimInfo animI;
		animI.currentAnim = animInfo.currentAnim;
		animI.startFrame = animInfo.blendingStartFrame;
		animI.currentFrame = animInfo.blendingFrame;
		animI.endFrame = animInfo.blendingEndFrame;
		animI.blendingFrame = animInfo.blendingFrame;

		if(animInfo.blendingFactor == 1.0f)
		{
			return GetRotationMatrixNoneBlending(vec, animI);
		}
		else
		{
			D3DXQUATERNION vec1, vec2;
			if(! GetRotationMatrixNoneBlending(&vec1, animI))
				return GetRotationMatrixNoneBlending(vec, animInfo); 

			if (! GetRotationMatrixNoneBlending(&vec2, animInfo))
			{
				
				return false;
			}
			// Perform linear motion warping, here
			D3DXQuaternionSlerp(vec, &vec2, &vec1, animInfo.blendingFactor);
			D3DXQuaternionNormalize(vec, vec);
			return true;
		}
	}
	return false;
}

bool CBone1::GetRotationMatrixNoneBlending(D3DXQUATERNION* quat,const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGRT];
	int		type	=	lineType[KGRT];
	void*	frames	=	keyFrames[KGRT];

	if(type == LINEAR)
	{
		LinearKeyFrame4 *p = (LinearKeyFrame4 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			LinearKeyFrame4 *startP=&(p[nMid]);
			LinearKeyFrame4 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(animInfo.currentFrame == startP->frameNum)
				{
					*quat = startP->vec;
					return true;
				}
				else if(animInfo.currentFrame == endP->frameNum)
				{
					*quat = endP->vec;
					return true;
				}
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
				{
					return false;
				}

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step = (float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);

					D3DXQuaternionSlerp(quat, &startP->vec, &endP->vec, step);
					D3DXQuaternionNormalize(quat, quat);
				}else
					*quat = startP->vec;
				return true;
				// break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}
	

	if(type == HERMITE)
	{
		NonLinearKeyFrame4 *p = (NonLinearKeyFrame4 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			NonLinearKeyFrame4 *startP=&(p[nMid]);
			NonLinearKeyFrame4 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(animInfo.currentFrame == startP->frameNum)
				{
					*quat = startP->vec;
					return true;
				}
				else if(animInfo.currentFrame == endP->frameNum)
				{
					*quat = endP->vec;
					return true;
				}
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
				{
					return false;
				}

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step = (float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXQUATERNION A, B, C, Qt;
					D3DXQuaternionSquadSetup(&A, &B, &C, &startP->vec, &startP->inTan, &endP->vec, &endP->outTan);
					D3DXQuaternionSquad(quat, &startP->vec, &A, &B, &C, step);
				}else
					*quat = startP->vec;
				return true;
				// break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}

	return false;
}

bool CBone1::CheckHideByScale(const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGSC];
	int		type	=	lineType[KGSC];
	void*	frames	=	keyFrames[KGSC];

	if(type == LINEAR)
	{
		LinearKeyFrame3 *p = (LinearKeyFrame3 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			LinearKeyFrame3 *startP=&(p[nMid]);
			LinearKeyFrame3 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				
				break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}else
	if(type > LINEAR)
	{
		NonLinearKeyFrame3 *p = (NonLinearKeyFrame3 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			NonLinearKeyFrame3 *startP=&(p[nMid]);
			NonLinearKeyFrame3 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(	startP->vec.x<0.1f &&
					startP->vec.y<0.1f &&
					startP->vec.z<0.1f )
					return true;
				else
					return false;
				
				/*
				if(	startP->vec.x<myEPSILON &&
					startP->vec.y<myEPSILON &&
					startP->vec.z<myEPSILON )
					return true;
				else
					return false;
					*/
				return true;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}
	return false;
}

bool CBone1::GetScaleMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo)
{
	if(animInfo.blendingFactor == 0.0f)
	{
		return GetScaleMatrixNoneBlending(vec, animInfo);
	}
	else 
	{
		AnimInfo animI;
		animI.currentAnim = animInfo.currentAnim;
		animI.startFrame = animInfo.blendingStartFrame;
		animI.currentFrame = animInfo.blendingFrame;
		animI.endFrame = animInfo.blendingEndFrame;
		animI.blendingFrame = animInfo.blendingFrame;

		if(animInfo.blendingFactor == 1.0f)
		{
			if(GetScaleMatrixNoneBlending(vec, animI))
				return true;
			else
				return GetScaleMatrixNoneBlending(vec, animInfo);
		}
		else
		{
			D3DXVECTOR3 vec1, vec2;
			if(! GetScaleMatrixNoneBlending(&vec1, animI))
				return GetScaleMatrixNoneBlending(vec, animInfo); 

			if (! GetScaleMatrixNoneBlending(&vec2, animInfo))
			{
				
				return false;
			}
			// Perform linear motion warping, here
			D3DXVec3Lerp (vec, &vec2, &vec1, animInfo.blendingFactor);
			return true;
		}
	}
	return false;
}

bool CBone1::GetScaleMatrixNoneBlending(D3DXVECTOR3* vec,const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGSC];
	int		type	=	lineType[KGSC];
	void*	frames	=	keyFrames[KGSC];

	if(type == LINEAR)
	{
		LinearKeyFrame3 *p = (LinearKeyFrame3 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			LinearKeyFrame3 *startP=&(p[nMid]);
			LinearKeyFrame3 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(animInfo.currentFrame == startP->frameNum)
				{
					*vec = startP->vec;
					return true;
				}
				else if(animInfo.currentFrame == endP->frameNum)
				{
					*vec = endP->vec;
					return true;
				}
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
				{
					return false;
				}

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXVec3Lerp (vec, &startP->vec, &endP->vec, step);
				}else
					*vec = startP->vec;
				return true;
				// break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}else
	if(type > LINEAR)
	{
		NonLinearKeyFrame3 *p = (NonLinearKeyFrame3 *)frames;
		int nStart = 0;
		int nEnd = frameCount-1;
		while(nStart<nEnd)
		{
			int nMid = (nStart+nEnd)/2;
			NonLinearKeyFrame3 *startP=&(p[nMid]);
			NonLinearKeyFrame3 *endP=&(p[nMid+1]);
			
			if(startP->frameNum <= animInfo.currentFrame &&
				animInfo.currentFrame <= endP->frameNum )
			{ // if (middle item is target)
				if(animInfo.currentFrame == startP->frameNum)
				{
					*vec = startP->vec;
					return true;
				}
				else if(animInfo.currentFrame == endP->frameNum)
				{
					*vec = endP->vec;
					return true;
				}
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
				{
					return false;
				}

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);

					// use linear instead
					D3DXVec3Lerp (vec, &startP->vec, &endP->vec, step);
				}else
					*vec = startP->vec;
				return true;
				// break;
			}
			else if(animInfo.currentFrame < startP->frameNum )
			{ //if (target < middle item)
				nEnd = nMid;
			}
			else if(animInfo.currentFrame > endP->frameNum )
			{
				nStart = nMid+1;	
			}
		}// while(nStart<=nEnd)
	}
	return false;
}

D3DXMATRIX* CBone1::CalcTransformMatrix(D3DXVECTOR3* center, const AnimInfo& animInfo,const D3DXMATRIX* parentMatrix)
{
	D3DXVECTOR3		scaling;
	D3DXQUATERNION	rotation;
	D3DXVECTOR3		translation;

	D3DXVECTOR3		*pScaling=NULL;
	D3DXQUATERNION	*pRotation=NULL;
	D3DXVECTOR3		*pTranslation = NULL;

	//billboarded
	if(nodeType==264)
	{
		D3DXVECTOR3 new_center;
		D3DXVec3TransformCoord(&new_center,center,parentMatrix);
		translation=new_center-(*center);
		D3DXVECTOR3 scale_vector(center->x+1,center->y,center->z);
		D3DXVec3TransformCoord(&scale_vector,&scale_vector,parentMatrix);
		scale_vector=scale_vector-new_center;

		D3DXMATRIX mat;
		
		// the code is edited by LiXizhi for porting.
		D3DXMatrixIdentity(&mat); 
		//glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&mat);

		D3DXVECTOR3 sight_normal(mat[2],mat[6],mat[10]);
		D3DXVECTOR3 sight_right(mat[0],mat[4],mat[8]);

		float d1=-asinf(sight_normal.z);
		float d2=acosf(sight_normal.x/sqrtf(sight_normal.x*sight_normal.x+sight_normal.y*sight_normal.y));

		if(sight_normal.y<0)
			d2=TWO_PI-d2;

		D3DXMATRIX rotZ,rotAxis;
		D3DXMatrixRotationZ(&rotZ,d2);
		D3DXMatrixRotationAxis(&rotAxis,&sight_right,d1);
		D3DXMatrixMultiply(&mat,&rotZ,&rotAxis);
		D3DXQuaternionRotationMatrix(&rotation,&mat);

		D3DXMatrixTranslation(&mat,-new_center.x,-new_center.y,-new_center.z);
		D3DXMatrixMultiply(&mat,&mat,parentMatrix);
		MatrixResolve(&mat,NULL,NULL,&scaling);

		D3DXMatrixTransformation(	&combinedMatrix,
			center,
			NULL,
			&scaling,			
			center,
			&rotation,
			&translation
			);

		D3DXMatrixMultiply(&finalMatrix, &offsetMatrix, &combinedMatrix);

		return &combinedMatrix;
	}

	if(keyFrameCount[KGTR]>0)
	{
		if(GetTransferMatrix(&translation,animInfo))
			pTranslation=&translation;
	}

	if(keyFrameCount[KGRT]>0)
	{
		if(GetRotationMatrix(&rotation,animInfo))
			pRotation=&rotation;
	}

	if(keyFrameCount[KGSC]>0)
	{
		// TODO:LiXizhi: enable scaling 
		/*if(CheckHideByScale(animInfo))
		{
			pScaling=&scaling;
		}*/
		if(GetScaleMatrix(&scaling,animInfo))
			pScaling=&scaling;
	}	

	// transformation matrix for this bone.
	if((pRotation == NULL) && (pTranslation == NULL) && (pScaling == NULL))
	{
		combinedMatrix = localMatrix * (*parentMatrix);
	}
	else
	{
		if(pTranslation == NULL)
		{
			// default translation is loaded, for MDX support only
			// MDX use pivot point, instead of offset matrix.
			// however, I used offset matrix in the engine, so tranlation must 
			// be set.
			translation.x = localMatrix._41;
			translation.y = localMatrix._42;
			translation.z = localMatrix._43;
			pTranslation = &translation;
		}

		if(pScaling == NULL)
		{
			D3DXMATRIX T;
			D3DXMatrixTranslation(&T, pTranslation->x, pTranslation->y, pTranslation->z);
			if(pRotation == NULL)
			{
				combinedMatrix = ( T ) * (*parentMatrix);
			}
			else
			{
				D3DXMATRIX R;
				D3DXMatrixRotationQuaternion(&R, pRotation);
				combinedMatrix = (R * T ) * (*parentMatrix);
			}
		}
		else
		{
			D3DXMATRIX S;
			D3DXMatrixScaling(&S, pScaling->x, pScaling->y, pScaling->z);
			D3DXMATRIX T;
			D3DXMatrixTranslation(&T, pTranslation->x, pTranslation->y, pTranslation->z);
			if(pRotation == NULL)
			{
				combinedMatrix = (S * T ) * (*parentMatrix);
			}
			else
			{
				
				D3DXMATRIX R;
				D3DXMatrixRotationQuaternion(&R, pRotation);
				if(this->objectID ==22)
				{
					combinedMatrix = (R * S * T );
				}
				combinedMatrix = (R * S * T ) * (*parentMatrix);
			}
		}
	}

	finalMatrix = offsetMatrix*combinedMatrix;
	
	return &combinedMatrix;
}

D3DXMATRIX* CBone1::TransformMatrixMultiply(const D3DXMATRIX* matrix)
{

	return &combinedMatrix;
}



/*********************************************************************************/
/*                                                                               */
/* CBone2 definitions                                                            */
/*                                                                               */
/*********************************************************************************/

CBone2::CBone2(void)
{
	D3DXMatrixIdentity(&transformMatrix);

	for(int i=0;i<MAX_MOTION;i++)
	{
		keyFrameCount[i]=0;
		lineType[i]=0;
		keyFrames[i]=NULL;
	}
}

CBone2::~CBone2(void)
{
}


bool CBone2::GetTransferMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGTR];
	int		type	=	lineType[KGTR];
	void*	frames	=	keyFrames[KGTR];

	if(type == LINEAR)
	{
		LinearKeyFrame3 *p=(LinearKeyFrame3 *)frames;

		if(animInfo.startFrame == p[frameCount-1].frameNum)
		{
			*vec = p[frameCount-1].vec;
			return true;
		}

		for(int i=0;i<frameCount-1;i++)
		{
			LinearKeyFrame3 *startP=p++;
			LinearKeyFrame3 *endP=p;
			if(animInfo.currentFrame == startP->frameNum)
			{
				*vec = startP->vec;
				return true;
			}else
			if(animInfo.currentFrame == endP->frameNum)
			{
				*vec = endP->vec;
				return true;
			}else
			if(	startP->frameNum < animInfo.currentFrame &&
				animInfo.currentFrame < endP->frameNum )
			{
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
					return false;

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXVec3Lerp (vec, &startP->vec, &endP->vec, step);
				}else
					*vec=startP->vec;

				return true;
			}
		}		
	}else
	if(type > LINEAR)
	{
		NonLinearKeyFrame3 *p=(NonLinearKeyFrame3 *)frames;

		if(!p) return false;

		if(animInfo.startFrame == p[frameCount-1].frameNum)
		{
			*vec = p[frameCount-1].vec;
			return true;
		}

		for(int i=0;i<frameCount-1;i++)
		{
			NonLinearKeyFrame3 *startP=&p[i];
			NonLinearKeyFrame3 *endP=&p[i+1];
			if(animInfo.currentFrame == startP->frameNum)
			{
				*vec = startP->vec;
				return true;
			}else
			if(animInfo.currentFrame == endP->frameNum)
			{
				*vec = endP->vec;
				return true;
			}else
			if(	startP->frameNum < animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
					return false;

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum)
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					if(type== HERMITE)
						D3DXVec3Hermite(vec, &startP->vec, &startP->inTan, &endP->vec, &endP->outTan, step);
					else
					if(type== BEZIER)
						D3DXVec3Bezier(vec, &startP->vec, &startP->inTan, &endP->vec, &endP->outTan, step);
					else
						return false;
				}else{
					*vec=startP->vec;
				}
				return true;
			}
		}
	}


	return false;
}

bool CBone2::GetRotationMatrix(D3DXQUATERNION* quat,const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGRT];
	int		type	=	lineType[KGRT];
	void*	frames	=	keyFrames[KGRT];


	if(type == LINEAR)
	{
		LinearKeyFrame4 *p=(LinearKeyFrame4 *)frames;

		if(animInfo.startFrame == p[frameCount-1].frameNum)
		{
			*quat = p[frameCount-1].vec;
			return true;
		}


		for(int i=0;i<frameCount;i++)
		{
			LinearKeyFrame4 *startP=&p[i];
			LinearKeyFrame4 *endP=&p[(i+1)%frameCount];
			
			if(animInfo.currentFrame == startP->frameNum)
			{

					   *quat = startP->vec;
				return true;
			}else
			if(animInfo.currentFrame == endP->frameNum)
			{

				*quat = endP->vec;
				return true;
			}else
			if(	startP->frameNum < animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
					return false;

				if(	animInfo.startFrame <= startP->frameNum && 
					animInfo.endFrame >= endP->frameNum )
				{

					D3DXQUATERNION temp;
					float step = (float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);

					D3DXQuaternionSlerp(quat, &startP->vec, &endP->vec, step);
					D3DXQuaternionNormalize(quat, quat);
				}else
					*quat = startP->vec;

				return true;
			}
		}		

	}
	

	if(type == HERMITE)
	{
		NonLinearKeyFrame4 *p=(NonLinearKeyFrame4 *)frames;

		if(animInfo.startFrame == p[frameCount-1].frameNum)
		{
			*quat = p[frameCount-1].vec;
			return true;
		}

		for(int i=0;i<frameCount-1;i++)
		{
			NonLinearKeyFrame4 *startP=&p[i];
			NonLinearKeyFrame4 *endP=&p[i+1];
			
			if(animInfo.currentFrame == startP->frameNum)
			{
				*quat = startP->vec;
				return true;
			}else
			if(animInfo.currentFrame == endP->frameNum)
			{
				*quat = endP->vec;
				return true;
			}else
			if(	startP->frameNum < animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
					return false;

				if(	animInfo.startFrame <= startP->frameNum && 
					animInfo.endFrame >= endP->frameNum )
				{
					float step = (float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXQUATERNION A, B, C, Qt;
					D3DXQuaternionSquadSetup(&A, &B, &C, &startP->vec, &startP->inTan, &endP->vec, &endP->outTan);
					D3DXQuaternionSquad(quat, &startP->vec, &A, &B, &C, step);
				}else
					*quat = startP->vec;

				return true;
			}
		}
	}

	return false;
}

bool CBone2::CheckHideByScale(const AnimInfo& animInfo)
{
	int		frameCount	=	keyFrameCount[KGSC];
	int		type	=	lineType[KGSC];
	void*	frames	=	keyFrames[KGSC];

	if(type == LINEAR)
	{
		LinearKeyFrame3 *p=(LinearKeyFrame3 *)frames;
		for(int i=0;i<frameCount-1;i++)
		{
			LinearKeyFrame3 *startP=&p[i];
			LinearKeyFrame3 *endP=&p[i+1];
			if(	startP->frameNum <= animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				/*
				if(	startP->vec.x<0.1f &&
					startP->vec.y<0.1f &&
					startP->vec.z<0.1f )
					return true;
				else
					return false;
				*/
				/*
				if(	startP->vec.x<myEPSILON &&
					startP->vec.y<myEPSILON &&
					startP->vec.z<myEPSILON )
					return true;
				else
					return false;
				*/
			}
		}
	}else
	if(type > LINEAR)
	{
		NonLinearKeyFrame3 *p=(NonLinearKeyFrame3 *)frames;
		for(int i=0;i<frameCount-1;i++)
		{
			NonLinearKeyFrame3 *startP=p++;
			NonLinearKeyFrame3 *endP=p;
			if(	startP->frameNum <= animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				
				if(	startP->vec.x<0.1f &&
					startP->vec.y<0.1f &&
					startP->vec.z<0.1f )
					return true;
				else
					return false;
				
				/*
				if(	startP->vec.x<myEPSILON &&
					startP->vec.y<myEPSILON &&
					startP->vec.z<myEPSILON )
					return true;
				else
					return false;
					*/
			}
		}
	}
	return false;
}

bool CBone2::GetScaleMatrix(D3DXVECTOR3* vec,const AnimInfo& animInfo)
{
	return false;

	int		frameCount	=	keyFrameCount[KGSC];
	int		type	=	lineType[KGSC];
	void*	frames	=	keyFrames[KGSC];

	if(type == LINEAR)
	{
		LinearKeyFrame3 *p=(LinearKeyFrame3 *)frames;

		if(animInfo.startFrame == p[frameCount-1].frameNum)
		{
			*vec = p[frameCount-1].vec;
			return true;
		}

		for(int i=0;i<frameCount-1;i++)
		{
			LinearKeyFrame3 *startP=&p[i];
			LinearKeyFrame3 *endP=&p[i+1];
			if(animInfo.currentFrame == startP->frameNum)
			{
				*vec = startP->vec;
				return true;
			}else
			if(animInfo.currentFrame == endP->frameNum)
			{
				*vec = endP->vec;
				return true;
			}else
			if(	startP->frameNum < animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
					return false;

				if(	animInfo.startFrame <= startP->frameNum &&
					animInfo.endFrame >= endP->frameNum )
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);
					D3DXVec3Lerp (vec, &startP->vec, &endP->vec, step);
				}else
					*vec=startP->vec;

				return true;
			}
		}		
	}else
	if(type > LINEAR)
	{
		NonLinearKeyFrame3 *p=(NonLinearKeyFrame3 *)frames;
		for(int i=0;i<frameCount-1;i++)
		{
			NonLinearKeyFrame3 *startP=p++;
			NonLinearKeyFrame3 *endP=p;
			if(animInfo.currentFrame == startP->frameNum)
			{
				*vec = startP->vec;
				return true;
			}else
			if(animInfo.currentFrame == endP->frameNum)
			{
				*vec = endP->vec;
				return true;
			}else
			if(	startP->frameNum < animInfo.currentFrame && 
				animInfo.currentFrame < endP->frameNum)
			{
				if(	animInfo.startFrame > startP->frameNum && 
					animInfo.endFrame < endP->frameNum )
					return false;

				if(	animInfo.startFrame <= startP->frameNum  &&
					animInfo.endFrame >= endP->frameNum )
				{
					float step=(float)(animInfo.currentFrame - startP->frameNum)/(float)(endP->frameNum - startP->frameNum);

					if(type== HERMITE)
						return false;
						//D3DXVec3Hermite(vec, &startP->vec, &startP->inTan, &endP->vec, &endP->outTan, step);
					else
					if(type== BEZIER)
						//D3DXVec3Bezier(vec, &startP->vec, &startP->inTan, &endP->vec, &endP->outTan, step);
						return false;
					else
						return false;
				}else
					*vec=startP->vec;

				return true;
			}
		}
	}
	return false;
}

D3DXMATRIX* CBone2::CalcTransformMatrix(D3DXVECTOR3* center, const AnimInfo& animInfo,const D3DXMATRIX* parentMatrix)
{
	D3DXVECTOR3		scaling;
	D3DXQUATERNION	rotation;
	D3DXVECTOR3		translation;

	D3DXVECTOR3		*pScaling=NULL;
	D3DXQUATERNION	*pRotation=NULL;
	D3DXVECTOR3		*pTranslation=NULL;

	if(keyFrameCount[KGTR]>0)
	{
		if(GetTransferMatrix(&translation,animInfo))
			pTranslation=&translation;
	}

	if(keyFrameCount[KGRT]>0)
	{
		if(GetRotationMatrix(&rotation,animInfo))
			pRotation=&rotation;
	}

	if(keyFrameCount[KGSC]>0)
	{
		if(CheckHideByScale(animInfo))
		{
			pScaling=&scaling;
		}
//		if(GetScaleMatrix(&scaling,animInfo))
//			pScaling=&scaling;
	}	

	D3DXMatrixTransformation(	&transformMatrix,
								center,
								NULL,
								pScaling,
								center,
								pRotation,
								pTranslation
							);

	D3DXMatrixMultiply(&transformMatrix, &transformMatrix, parentMatrix);
	return &transformMatrix;
}


void CBone2::Read(TypePointer &inP)
{
	TypePointer p(inP);
	int iread = 0;

	while(iread < 3)
	{
		//trasfer
		if( TAG(*p.dw) == 'KGTR' ){
			++iread;
			p.dw++;
			keyFrameCount[KGTR]=*(p.dw++);
			lineType[KGTR]=*(p.dw++);
			p.dw++;
			keyFrames[KGTR]=p.p;
			//non linear trasfer
			if( lineType[KGTR] > LINEAR )
				p.c+=keyFrameCount[KGTR]*sizeof(NonLinearKeyFrame3);
			else	 
				p.c+=keyFrameCount[KGTR]*sizeof(LinearKeyFrame3);
		}
		else
		//rotation
		if(TAG(*p.dw) == 'KGRT'){
			++iread;
			p.dw++;
			keyFrameCount[KGRT]=*(p.dw++);
			lineType[KGRT]=*(p.dw++);
			p.dw++;
			keyFrames[KGRT]=p.p;
			//non linear rotaion
			if( lineType[KGRT] > LINEAR )
				p.c+=keyFrameCount[KGRT]*sizeof(NonLinearKeyFrame4);
			else
				p.c+=keyFrameCount[KGRT]*sizeof(LinearKeyFrame4);
		}
		else
		//scale
		if(TAG(*p.dw) == 'KGSC'){
			++iread;
			p.dw++;
			keyFrameCount[KGSC]=*(p.dw++);
			lineType[KGSC]=*(p.dw++);
			p.dw++;
			keyFrames[KGSC]=p.p;
			//non linear scale
			if( lineType[KGSC] > LINEAR )
				p.c+=keyFrameCount[KGSC]*sizeof(NonLinearKeyFrame3);
			else	 
				p.c+=keyFrameCount[KGSC]*sizeof(LinearKeyFrame3);
		}
		else
			break;

	
	}

	inP.p = p.p;
}








/*********************************************************************************/
/*                                                                               */
/* CEmitter definitions                                                          */
/*                                                                               */
/*********************************************************************************/

CEmitter::CEmitter(void)
{
	memset(&Particles, 0, sizeof(Particles));
	eName = NULL;
	Flags = parentID = objectID = -1;
	curWidth = curLength = curEmissionRate = curVisibility = Lifespan = Gravity = Latitude = Variation = curSpeed = 0.0f;
	FilterMode = Rows = Columns = Flag2 = -1;
	TailLength = Time = 0.0f;
	Alpha1 = Alpha2 = Alpha3 = 0;
	ScalingX = ScalingY = ScalingZ = 0.0f;
	LifeSpanUVAnim1 = LifeSpanUVAnim2 = LifeSpanUVAnim3 = -1;
	DecayUVAnim1 = DecayUVAnim2 = DecayUVAnim3 = -1;
	TailUVAnim1 = TailUVAnim2 = TailUVAnim3 = -1;
	TailDecayUVAnim1 = TailDecayUVAnim2 = TailDecayUVAnim3 = -1;
	TextureID = -1;
	Squirt = -1;
	PriorityPlane = -1;
	ReplaceableID = -1;

	Visibility = NULL;
	numVisibility = 0;

	EmissnRate = NULL;
	numEmissnRate = 0;

	Length = NULL;
	numLength = 0;

	Width = NULL;
	numWidth = 0;

	Speed = NULL;
	numSpeed = 0;
}


CEmitter::~CEmitter(void)
{
}



void CEmitter::Read(TypePointer inP,int inSize)
{

	Visibility = NULL;
	numVisibility = 0;

	EmissnRate = NULL;
	numEmissnRate = 0;

	Length = NULL;
	numLength = 0;

	Width = NULL;
	numWidth = 0;

	Speed = NULL;
	numSpeed = 0;
	
	char *dstart = inP.c;

	inP.dw+=2;	

	eName = inP.c;	inP.c+=80;
	
	objectID = *inP.dw++;
	parentID = *inP.dw++;
	Flags	 = *inP.dw++;
	
	bone.Read(inP);

	curSpeed = *inP.f++;
	Variation=*inP.f++;
	Latitude=*inP.f++;
	Gravity=*inP.f++;
	Lifespan=*inP.f++;
	curEmissionRate=*inP.f++;
	curLength=*inP.f++;
	curWidth=*inP.f++;
	
	FilterMode=*inP.dw++;
	Rows=*inP.dw++;
	Columns=*inP.dw++;
	Flag2=*inP.dw++;
	
	TailLength = *inP.f++;
	Time = *inP.f++;
	
	memcpy(&SegmentColor, inP.p, 9*sizeof(float));  inP.c+=9*sizeof(float);
	Alpha1 = (unsigned char)inP.c++;
	Alpha2 = (unsigned char)inP.c++;
	Alpha3 = (unsigned char)inP.c++;

	ScalingX = *inP.f++;
	ScalingY = *inP.f++;
	ScalingZ = *inP.f++;

	LifeSpanUVAnim1 = *inP.dw++;
	LifeSpanUVAnim2 = *inP.dw++;
	LifeSpanUVAnim3 = *inP.dw++;

	DecayUVAnim1 = *inP.dw++;
	DecayUVAnim2 = *inP.dw++;
	DecayUVAnim3 = *inP.dw++;

	TailUVAnim1 = *inP.dw++;
	TailUVAnim2 = *inP.dw++;
	TailUVAnim3 = *inP.dw++;

	
	TailDecayUVAnim1 = *inP.dw++;
	TailDecayUVAnim2 = *inP.dw++;
	TailDecayUVAnim3 = *inP.dw++;

	TextureID = *inP.dw++;
	Squirt = *inP.dw++;
	PriorityPlane = *inP.dw++;
	ReplaceableID = *inP.dw++;


	while((inP.c-dstart)<inSize)
	{
		switch(TAG(*inP.dw))
		{
			case 'KP2V':
			{
				inP.dw++;
				numVisibility = *inP.dw++;
				inP.dw+=2;
				Visibility = (TVisibility*)inP.p;
				
				if(numVisibility) inP.c+=numVisibility*sizeof(TVisibility);
			}
			break;

			case 'KP2E':
			{
				inP.dw++;
				numEmissnRate = *inP.dw++;
				inP.dw+=2;
				EmissnRate = (TVisibility*)inP.p;
				
				if(numEmissnRate) inP.c+=numEmissnRate*sizeof(TVisibility);
			}
			break;

			case 'KP2N':
			{
				inP.dw++;
				numLength = *inP.dw++;
				inP.dw+=2;
				Length = (TVisibility*)inP.p;
				
				if(numLength) inP.c+=numLength*sizeof(TVisibility);
			}
			break;

			case 'KP2W':
			{
				inP.dw++;
				numWidth = *inP.dw++;
				inP.dw+=2;
				Width = (TVisibility*)inP.p;
				
				if(numWidth) inP.c+=numWidth*sizeof(TVisibility);
			}
			break;

			case 'KP2S':
			{
				inP.dw++;
				numSpeed = *inP.dw++;
				inP.dw+=2;
				Speed = (TVisibility*)inP.p;
				
				if(numSpeed) inP.c+=numSpeed*sizeof(TVisibility);
			}
			break;

			default:
				inP.c++;
				break;
		}
	}


	int k;

	if(EmissnRate)
	{
		curEmissionRate = EmissnRate[0].f;
		float emax = 0.0f;
		for(k=0; k<numEmissnRate; ++k)
		{
			if(EmissnRate[k].f>emax) emax = EmissnRate[k].f;
		}

		numParticles = (int)emax+1;
	}
	else
		numParticles = (int)curEmissionRate+1;

	numParticles = min(numParticles, MDX_MAX_PARTICLES-1);

	if(Visibility)
		curVisibility = Visibility[0].f;

	if(Length)
		curLength = Length[0].f;
	
	if(Width)
		curWidth = Width[0].f;

	if(Speed)
		curSpeed = Speed[0].f;
	
	
	for(k=0; k<=numParticles; ++k)
	{
		Particles[k].activated=FALSE;
		Particles[k].life = -1.0f;
		Particles[k].zg = Gravity/4000.0f;

		Particles[k].xi=float((rand()%((int)curSpeed+1))-(curSpeed/2.0f))/(curSpeed/2.0f);
		Particles[k].yi=float((rand()%((int)curSpeed+1))-(curSpeed/2.0f))/(curSpeed/2.0f);
		Particles[k].zi=/*float((rand()%60)-30.0f)/30.0f;*/0.0f;
	
		Particles[k].fade=Lifespan/curEmissionRate;

		Particles[k].r=SegmentColor[k%3].B;
		Particles[k].g=SegmentColor[k%3].G;
		Particles[k].b=SegmentColor[k%3].R;
	}

	if(curVisibility == 0.0f && Visibility)
	{
		k=0;
		while(k<numVisibility && Visibility[k].f == 0.0f) ++k;
		if(k==numVisibility)
		{
			Visibility = NULL;
			curVisibility = 1.0f;
		}
	}
}



void CEmitter::SetCurrentOptions(const AnimInfo& animInfo)
{
	int k;

	if(Visibility)
	{
		k=0;
		while(k<numVisibility && animInfo.currentFrame >= Visibility[k].kframe)
			++k;

		curVisibility = Visibility[(k-1<0 ? 0 : (k-1))].f;
	}
	else
		curVisibility = 1.0f;
	
	if(EmissnRate)
	{
		k=0;
		while(k<numEmissnRate && animInfo.currentFrame >= EmissnRate[k].kframe)
			++k;

		curEmissionRate = EmissnRate[(k-1<0 ? 0 : (k-1))].f;
	}

	if(Length)
	{
		k=0;
		while(k<numLength && animInfo.currentFrame >= Length[k].kframe)
			++k;

		curLength = Length[k-1].f;
	}

	if(Width)
	{
		k=0;
		while(k<numWidth && animInfo.currentFrame >= Width[k].kframe)
			++k;

		curWidth = Width[(k-1<0 ? 0 : (k-1))].f;
	}

	if(Speed)
	{
		k=0;
		while(k<numSpeed && animInfo.currentFrame >= Speed[k].kframe)
			++k;

		curSpeed = Speed[(k-1<0 ? 0 : (k-1))].f;
	}
}



void CEmitter::Render(D3DXMATRIX *parentMatrix, D3DXVECTOR3 *center, const AnimInfo& animInfo, long texbindid, long texwidth, long texlength)
{
//	bool bVisible = TRUE;
//	
//	SetCurrentOptions(animInfo);
//
//
//	if(curWidth == 0.0f && !Width)
//		curWidth = (float)texwidth;
//
//	if(curLength == 0.0f && !Length)
//		curLength = (float)texlength;
//
//	bVisible = (curVisibility != 0.0f ? TRUE : FALSE);
//
//
////	if(strstr(eName, "Shoulder")!=NULL) bVisible=TRUE; else bVisible=FALSE;
//
//
//	D3DXMATRIX transformMatrix;
//	D3DXVECTOR3 vec, vin, vright, vup, vpos;
//
//	transformMatrix = *parentMatrix;
//
//	D3DXVec3TransformCoord(&vec,center,&transformMatrix);
//
//	float mat[16];
//	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
//	vright=D3DXVECTOR3(mat[0], mat[4], mat[8]);
//	vup=D3DXVECTOR3(mat[1], mat[5], mat[9]);
//
//
//	int loop;
//	long NumCreated=0, offset, i, j;
//	float left, top, right, bottom;
//
//	float prevcol[4];
//	glGetFloatv(GL_CURRENT_COLOR, prevcol);
//
//	glEnable(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, texbindid);
//	glEnable(GL_DEPTH_TEST); 
//	glDepthMask(false); 
//	glEnable(GL_BLEND);
//	glEnable(GL_COLOR_MATERIAL);
//
//	if(FilterMode)
//		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
//	else
//		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//	
//	
//
//	for (loop=0; loop <= numParticles; loop++)
//	{
//		if(Particles[loop].activated == FALSE && bVisible == TRUE)
//		{
//			Particles[loop].activated = TRUE;
//			Particles[loop].x=vec.x;
//			Particles[loop].y=vec.y;
//			Particles[loop].z=vec.z;
//		}
//
//		if(Particles[loop].life>=0.0f && Particles[loop].activated == TRUE)
//		{		
//			float x=Particles[loop].x;
//			float y=Particles[loop].y;
//			float z=Particles[loop].z;
//
//			glColor4f(Particles[loop].r,Particles[loop].g,Particles[loop].b,Particles[loop].life/Lifespan);
//
//			offset = (long)((1.0f-Particles[loop].life/Lifespan)*(float)(Rows*Columns));
//			i=offset%Columns;
//			j=offset/Columns;
//
//			left = (float)i/Columns;
//			top = (float)j/Rows;
//
//			right = left+1.0f/Columns;
//			bottom = top+1.0f/Rows;
//
///*
//			if(offset > 32)
//				offset=offset;
//*/
//			
//			vpos=D3DXVECTOR3(x,y,z);
//
//			glBegin(GL_TRIANGLE_STRIP);
//				glTexCoord2d(right,bottom);
//				//glVertex3f(x,y+curWidth,z+curLength);								
//				glVertex3fv(vpos +vright*curWidth +vup*curLength);
//
//				glTexCoord2d(left,bottom);
//				//glVertex3f(x,y+curWidth,z-curLength);		
//				glVertex3fv(vpos -vright*curWidth +vup*curLength);
//
//				glTexCoord2d(right,top);
//				//glVertex3f(x,y-curWidth,z+curLength);
//				glVertex3fv(vpos +vright*curWidth -vup*curLength);
//
//				glTexCoord2d(left,top);
//				//glVertex3f(x,y-curWidth,z-curLength);
//				glVertex3fv(vpos -vright*curWidth -vup*curLength);
//				
//			glEnd();
//
//			Particles[loop].x+=Particles[loop].xi;
//			Particles[loop].y+=Particles[loop].yi;
//			Particles[loop].z+=Particles[loop].zi;
//			
//			Particles[loop].xi+=Particles[loop].xg;
//			Particles[loop].yi+=Particles[loop].yg;
//			Particles[loop].zi-=Particles[loop].zg;
//			Particles[loop].life-=Particles[loop].fade;
//		}
//	
//		if (Particles[loop].life<0.0f && bVisible == TRUE && NumCreated<1)
//		{
//			Particles[loop].life=Lifespan;
//			// Particles[loop].fade=float(rand()%100)/1000.0f+0.003f;
//			Particles[loop].fade=Lifespan/curEmissionRate;
//			
//			Particles[loop].x=vec.x;
//			Particles[loop].y=vec.y;
//			Particles[loop].z=vec.z;
//
//			Particles[loop].xi=float((rand()%((int)curSpeed+1))-(curSpeed/2.0f))/(curSpeed/2.0f);
//			Particles[loop].yi=float((rand()%((int)curSpeed+1))-(curSpeed/2.0f))/(curSpeed/2.0f);
//			Particles[loop].zi=/*float((rand()%60)-30.0f)/30.0f;*/0.0f;
//
//			Particles[loop].r=SegmentColor[loop%3].B;
//			Particles[loop].g=SegmentColor[loop%3].G;
//			Particles[loop].b=SegmentColor[loop%3].R;
//			++NumCreated;
//		}
//	}
//	glDepthMask(true);
//
//	//glColor3f(1.0f, 1.0f, 1.0f);
//	glColor4fv(prevcol);
//	glDisable(GL_DEPTH_TEST);
//	glEnable(GL_TEXTURE_2D);
//	glDisable(GL_BLEND);
//	glDisable(GL_ALPHA_TEST);
////	glDisable(GL_COLOR_MATERIAL);
//	glEnable(GL_LIGHTING);

}





CEmitters::CEmitters(void)
{
	numEmitters = 0;
	nEmitterArraySize = 0;
	Emitters = NULL;
}


CEmitters::~CEmitters(void)
{
	Cleanup();
}

void CEmitters::Cleanup()
{
	if(Emitters)
	{
		delete [] Emitters;
		Emitters = NULL;
		nEmitterArraySize = 0;
		numEmitters = 0;
	}
}

void CEmitters::Read(TypePointer inP,int inSize)
{
	if(nEmitterArraySize == 0)
	{
		nEmitterArraySize = MAX_NUM_EMMITTERS;
		Emitters = new CEmitter[nEmitterArraySize];
	}

	char *dstart = inP.c;
	while(((inP.c-dstart)<inSize))
	{
		int eSize = *inP.dw;
		Emitters[numEmitters++].Read(inP, eSize);
		inP.c+=eSize;
		
		// just increase 2 times the size of MAX_NUM_EMMITTERS
		if(numEmitters == nEmitterArraySize)
		{
			CEmitter* tmp = new CEmitter[nEmitterArraySize*2];
			memcpy(tmp, Emitters, sizeof(CEmitter)*nEmitterArraySize);
			nEmitterArraySize *= 2;
			delete [] Emitters;
			Emitters = tmp;
			OutputDebugString("bones.cpp: emitters Size doubled\n");
		}
	}	
}



int FindParent(int parentID, const CSkeleton *skel)
{
	int i=0;
	for(i=0; i<(int)skel->bones.size(); ++i)
	{
		if(skel->bones[i]->objectID == parentID)
			return i;
	}

	return -1;
}



void CEmitters::Render(const AnimInfo& animInfo, const CSkeleton *skel, CTexture *curtex)
{
	int k=0, pai = 0;
	D3DXMATRIX parident, *parmat;
	D3DXMatrixIdentity(&parident);

	for(k=0; k<numEmitters; ++k)
	{
		D3DXVECTOR3 center(0,0,0);

		if(Emitters[k].parentID < 0)
		{			
			Emitters[k].bone.CalcTransformMatrix(&center,
												 animInfo,
												 &parident);
			Emitters[k].Render(Emitters[k].bone.GetFinalTransformMatrix(), &center, animInfo, (long)curtex->GetBindTexture(Emitters[k].TextureID), curtex->m_pTextures[Emitters[k].TextureID]->width, curtex->m_pTextures[Emitters[k].TextureID]->height);
		}
		else
		{
			pai = FindParent(Emitters[k].parentID, skel);
			parmat = skel->bones[pai]->GetFinalTransformMatrix();
			if(pai>=0)
			{
				Emitters[k].bone.CalcTransformMatrix(&center,
													 animInfo,
													 parmat);
				Emitters[k].Render(Emitters[k].bone.GetFinalTransformMatrix(), &center, animInfo, (long)curtex->GetBindTexture(Emitters[k].TextureID), curtex->m_pTextures[Emitters[k].TextureID]->width, curtex->m_pTextures[Emitters[k].TextureID]->height);
			}
			else
			{
				Emitters[k].bone.CalcTransformMatrix(&center,
													 animInfo,
													 &parident);
				Emitters[k].Render(Emitters[k].bone.GetFinalTransformMatrix(), &center, animInfo, (long)curtex->GetBindTexture(Emitters[k].TextureID), curtex->m_pTextures[Emitters[k].TextureID]->width, curtex->m_pTextures[Emitters[k].TextureID]->height);
			}
		}		
	}
}
