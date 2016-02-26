//----------------------------------------------------------------------
// Class:	CSkeleton
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer

#include "dxStdAfx.h"
#include "ParaEngine.h"
#include "Skeleton.h"
#include <queue>

using namespace ParaEngine;

CSkeleton::CSkeleton(void)
{
	bones.clear();
	nFirstBoneID = 0;
}

CSkeleton::~CSkeleton(void)
{
	Cleanup();
}
void CSkeleton::Cleanup()
{
	vector <CBone1*>::iterator itCurCP, itEndCP = bones.end();

	for( itCurCP = bones.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		delete (*itCurCP);
	}
	bones.clear();
}

CBone1*		CSkeleton::CreateNewBone()
{
	CBone1 * pBone = new CBone1();
	if(!bones.empty())
		bones.back()->nextID = (int)bones.size();
	else
		nFirstBoneID = 0;
	bones.push_back(pBone);
	return pBone;
}
void CSkeleton::ReadHelpers(TypePointer inP,int inSize)
{
	TypePointer p(inP);
	while(p.c < inP.c+inSize){
		int size = *(p.dw++);
		CreateNewBone()->Read(p,size);
		p.c+=size-4;
	}
}

void CSkeleton::ReadBones(TypePointer inP,int inSize)
{
	TypePointer p(inP);

	while(p.c < inP.c+inSize){
		int size = *(p.dw++);
		CreateNewBone()->Read(p,size+4);
		p.c+=size+4;
	}
}

// we assume that all children has index(ID) larger than its parent.
void CSkeleton::SortBones()
{
	// breadth first trasversal
	int numBone = (int)bones.size();
	queue <int> myQueue;
	int oldObjID = -1;
	while(1)
	{
		for(int k=0;k<numBone;k++)
		{
			if(bones[k]->GetParentID()== oldObjID)
			{
				myQueue.push(k);
			}
		}
		
		if(myQueue.empty())
			break;
		int index = myQueue.front();
		CBone1* pBone = bones[index];
		myQueue.pop();
		if(oldObjID == -1)
		{
			nFirstBoneID = index;
		}
		else
		{
			bones[oldObjID]->nextID = index;
		}
		
		// repair wrong ObjectID, so that they are the same as the index
		if(bones[index]->GetObjectID() != index)
		{
			for(int k=0;k<numBone;k++)
			{
				if(bones[k]->GetParentID()== bones[index]->GetObjectID())
				{
					bones[k]->parentID = index;
				}
			}
			bones[index]->objectID = index;
		}
		oldObjID = bones[index]->GetObjectID();
	}
	bones[oldObjID]->nextID = -1;
}

void CSkeleton::ReadGeosetAnims(TypePointer inP,int inSize)
{
	geosetAnims.Read(inP,inSize);
}

void CSkeleton::ReadPivotPoints(TypePointer inP,int inSize)
{
	int				numPivotPoint;
	D3DXVECTOR3*	pivotPoints;

	numPivotPoint=inSize/(3*sizeof(float));
	pivotPoints = new D3DXVECTOR3[numPivotPoint];
	memcpy(pivotPoints, inP.c, inSize);

	/// set offset matrix for each bone
	/// set the animation translation keyframe for each bone
	D3DXVECTOR3 centerLast(0,0,0);
	D3DXVECTOR3 center;
	int numBone = (int)bones.size();
	for(int i=0;i<numBone;i++)
	{
		/// set the offset matrix to be a translation matrix of the negation of the pivot point
		center=pivotPoints[bones[i]->GetObjectID()];
		D3DXMatrixTranslation(&bones[i]->offsetMatrix, -center.x, -center.y, -center.z);

		/// set the translation animation keyframe so that a component of Pivot[i]-Pivot[i-1]
		/// is added to each translation key. If there is no key, we will add one. 
		if(bones[i]->GetParentID() >=0)
			centerLast = pivotPoints[bones[i]->GetParentID()];
		else
			centerLast = D3DXVECTOR3(0,0,0);
		D3DXVECTOR3 diff = (center-centerLast);

		D3DXMatrixTranslation(&bones[i]->localMatrix, diff.x, diff.y, diff.z);
		
		int		frameCount	=	bones[i]->keyFrameCount[CBone1::KGTR];
		int		type	=	bones[i]->lineType[CBone1::KGTR];
		void*	frames	=	bones[i]->keyFrames[CBone1::KGTR];
		for(int k=0; k<frameCount; k++)
		{
			if(type == CBone1::LINEAR)
			{
				LinearKeyFrame3 *p = (LinearKeyFrame3 *)frames;
				p[k].vec+=diff;
			}
			else
			{
				NonLinearKeyFrame3 *p=(NonLinearKeyFrame3 *)frames;
				p[k].vec+=diff;
			}
		}
	}
	delete [] pivotPoints;
}

/// Call this function for every time advance to update the all the bone matrix
void CSkeleton::CalcTransformMatrix(const AnimInfo& animInfo)
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	CalcTransformMatrix(animInfo,&mat,-1);
}

void CSkeleton::CalcTransformMatrix(const AnimInfo& animInfo,D3DXMATRIX *parentMatrix,int parentID)
{
	int numBone = (int)bones.size();

	int i = nFirstBoneID;
	if(numBone <= 0)
		return;
	while(i>=0)
	{
		D3DXVECTOR3 center(0,0,0);
		if(bones[i]->GetParentID()>=0)
			bones[i]->CalcTransformMatrix(&center,animInfo,bones[bones[i]->GetParentID()]->GetCombinedTransformMatrix());
		else
			bones[i]->CalcTransformMatrix(&center,animInfo,parentMatrix);

		float alpha=geosetAnims.GetAnimAlpha(bones[i]->GetGeosetAnimID(),animInfo);
		bones[i]->SetGeosetAlpha(alpha);
		i = bones[i]->nextID;
	}
}

/// call this function to get the matrix for a weighted sum of a group of bones.
/// this is for skinned animation. 
float CSkeleton::GetTransformMatrix(D3DXMATRIX &mat,float& animAlpha,int *index,int count)
{
	mat=*bones[index[0]]->GetFinalTransformMatrix();
	animAlpha=bones[index[0]]->GetGeosetAlpha();

	if(count>1)
	{
		/// it is assumed that weight of multiple bones are evenly distributed.
		for(int k=1; k<count; ++k)
		{
			mat += *bones[index[k]]->GetFinalTransformMatrix();
			animAlpha += bones[index[k]]->GetGeosetAlpha();
		}
		mat *= 1.0f /( (float)count );
		animAlpha *= 1.0f /( (float)count );
	}

	return animAlpha;
}
