//-----------------------------------------------------------------------------
// Class:	CMdxAniInstance
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.7.23
// Revised: 2004.7.23
//
/**
// MDX file based Animation instance:
// It is advised to call SetSizeScale() at least once before drawing this object.
// in most cases, SetSizeScale(1.0f) will use the size of the first animation
// defined in the mdx model file.
//
// see <scenecreationhelper> for its usage
Motion warping is added. We use linear interpolation between the last framework in the old sequence
and the first few frames in the new sequence. See the LoadAnimation() function and advanceTime() function 
for more details.
*/
//-----------------------------------------------------------------------------
#ifdef USE_MDXFILE
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "mdxfile\MDXModel.h"
#include "MdxAnimInstance.h"
using namespace ParaEngine;


extern BOOL g_showmeshes[];
const float g_fMdxGlobalScale = 0.01f;

CMdxAniInstance::CMdxAniInstance(void):
	CAnimInstanceBase(), 
	m_ppModel(NULL)
{
	SetSizeScale(1.0);
}

CMdxAniInstance::~CMdxAniInstance(void)
{
}

HRESULT CMdxAniInstance::InitObject(MDXEntity* ppModel)
{
	m_ppModel = ppModel;
	return S_OK;
}

void CMdxAniInstance::LoadDefaultStandAnim(float * fSpeed)
{
	LoadAnimation(m_ppModel->GetModel()->nDefaultStandIndex, fSpeed);
}
void CMdxAniInstance::LoadDefaultWalkAnim(float * fSpeed)
{
	LoadAnimation(m_ppModel->GetModel()->nDefaultWalkIndex, fSpeed);
}

/// call these functions at any time to start a new animation
/// one may follow a AdvanceTime() call, if he does not want to 
/// play from the beginning of the sequence. this function has no effect
/// if the new animation is the same animation as it is currently playing
/// it will continue to play the same animation after that animation stops.
/// this is useful, when you issue several non-looping animation in short succession.
/// it will not jerk to 0 animation.
void CMdxAniInstance::LoadAnimation(const char * sName, float * fSpeed,bool bAppend)
{
	/// map string to anim Index 
	DWORD nIndexAnim = m_ppModel->GetModel()->GetAnimNum(sName);
	if(nIndexAnim>=0)
		LoadAnimation(nIndexAnim, fSpeed, bAppend);
}

/// fSpeed = M_fSpeedScale* M_fSizeScale* G_fMdxGlobalScale * moveSpeed
void CMdxAniInstance::LoadAnimation(int nIndexAnim, float * fSpeed, bool bAppend)
{
	if(m_aiAnimIntance.nCurrentAnim != nIndexAnim)
	{
		/// play immediately from the beginning if the animation is different
		/// otherwise continue the last animation.
		// turn on motion warping
		m_aiAnimIntance.blendingAnim = m_aiAnimIntance.nCurrentAnim;
		m_aiAnimIntance.blendingFrame = m_aiAnimIntance.currentFrame;
		m_aiAnimIntance.blendingFactor = 1.0;
		
		m_aiAnimIntance.nCurrentAnim = nIndexAnim;
		m_aiAnimIntance.currentFrame = m_ppModel->GetModel()->sequences.sequences[nIndexAnim].startFrame;
		
		/// set the next animation to default
		m_aiAnimIntance.nNextAnim = -1;	
		
	}
	else
		m_aiAnimIntance.nNextAnim = nIndexAnim;// = m_aiAnimIntance.nCurrentAnim;

	*fSpeed = m_fSpeedScale* m_fSizeScale* g_fMdxGlobalScale * m_ppModel->GetModel()->sequences.sequences[nIndexAnim].moveSpeed;
}
/// a model might has several meshes, you can show or hide them by setting
/// the bitfield of a DWORD. the lowest bit is the first mesh.
void CMdxAniInstance::ShowMesh(BOOL bShow, int nIndex)
{
	m_aiAnimIntance.ShowMesh(bShow, nIndex);
}
void CMdxAniInstance::ShowMesh(DWORD dwBitfields)
{
	m_aiAnimIntance.dwShowMeshes = dwBitfields;
}

/** Copy animation status of this MDX instance object to the MDX model */
void CMdxAniInstance::CopyAnimInfoToParaXModel()
{
	CMDXModel *pMdxModel =  (m_ppModel)->GetModel();

	pMdxModel->UpdateAnim(m_aiAnimIntance.nCurrentAnim);
	if(m_aiAnimIntance.currentFrame >=0 )
		pMdxModel->currentAnimInfo.currentFrame = m_aiAnimIntance.currentFrame;
	pMdxModel->currentAnimInfo.blendingFrame = m_aiAnimIntance.blendingFrame;
	pMdxModel->currentAnimInfo.blendingFactor = m_aiAnimIntance.blendingFactor;
	if((m_aiAnimIntance.blendingFrame >=0) && (m_aiAnimIntance.blendingFactor>0))
	{
		SequenceAnim *anim = pMdxModel->sequences.GetSequence(m_aiAnimIntance.blendingAnim);
		if(anim)
		{
			pMdxModel->currentAnimInfo.blendingStartFrame = anim->startFrame;
			pMdxModel->currentAnimInfo.blendingEndFrame = anim->endFrame;
		}
	}

	/// set the mesh group visibility
	int nNumMeshes = pMdxModel->geosets.GetNumChunks();
	DWORD dwShowMesh = m_aiAnimIntance.dwShowMeshes;

	for(int i=0; i<nNumMeshes;i++)
	{
		if(dwShowMesh & 0x1)
			g_showmeshes[i] = TRUE;
		else
			g_showmeshes[i] = FALSE;

		dwShowMesh = dwShowMesh>>1;
	}
}

/** 
1.Copy animation status from MDX model back to this MDX instance object 
2.It also tests to see if see whether the end of animation has been reached
when the animation reaches its end,it will by default loop from the beginning or start
play the index=0 animation, according to its bLooping property.
however, if the user has already queued an animation, we will play that one
instead of the default action defined internally in the pMdxModel.
*/
void CMdxAniInstance::SaveAnimInfoFromParaXModel()
{
	CMDXModel *pMdxModel =  (m_ppModel)->GetModel();

	/** Copy animation status from MDX model back to this MDX instance object */
	m_aiAnimIntance.nCurrentAnim = pMdxModel->currentAnimInfo.currentAnim;
	m_aiAnimIntance.currentFrame = pMdxModel->currentAnimInfo.currentFrame;
	m_aiAnimIntance.blendingFrame = pMdxModel->currentAnimInfo.blendingFrame;
	m_aiAnimIntance.blendingFactor= pMdxModel->currentAnimInfo.blendingFactor;

	// test the following expression to see whether the end of animation has been reached
	if(pMdxModel->currentAnimInfo.currentFrame == pMdxModel->currentAnimInfo.startFrame)
	{ 
		/// when the animation reaches its end,it will by default loop from the beginning or start
		/// play the index=0 animation, according to its bLooping property.
		/// however, if the user has already queued an animation, we will play that one
		/// instead of the default action defined internally in the pMdxModel.
		if(m_aiAnimIntance.nNextAnim >= 0)
		{
			/// LoadAnimation() is reused  to set the current animation, however 
			/// the animation queue is emptied.
			float fSpeed;
			m_aiAnimIntance.currentFrame = pMdxModel->currentAnimInfo.endFrame;
			LoadAnimation(m_aiAnimIntance.nNextAnim,&fSpeed);
			m_aiAnimIntance.currentFrame = pMdxModel->currentAnimInfo.startFrame;

			/// empty the queue
			m_aiAnimIntance.nNextAnim = -1;
		}
	}
}
// ----------------------------------------------------------------------
/// desc: advance animation without rendering
/// call this function at every frame move
// ----------------------------------------------------------------------
void CMdxAniInstance::AdvanceTime( double dTimeDelta )
{
	CMDXModel *pMdxModel =  (m_ppModel)->GetModel();

	/** Copy animation status of this MDX instance object to the MDX model */
	CopyAnimInfoToParaXModel();

	/** Advance the current animation */
	/// Speed up the animation rendering by a factor of m_fSpeedScale
	pMdxModel->AdvanceTime(dTimeDelta*m_fSpeedScale);

	/** Copy animation status from MDX model back to this MDX instance object */
	SaveAnimInfoFromParaXModel();
}

// ----------------------------------------------------------------------
/// render and advance animation
/// simply call this function to draw the model at its current animation frame. 
// ----------------------------------------------------------------------
HRESULT CMdxAniInstance::Draw( SceneState * sceneState, const Matrix4* mxWorld)
{
	/** advance time */
	CopyAnimInfoToParaXModel();

	/** render model */
	LPDIRECT3DDEVICE9 pd3dDevice = sceneState->m_pd3dDevice;

	/// Copy animation status to the MDX model
	/// currentAnimInfo should always be given a valid data before calling Render() method of the mdx model
	CMDXModel *pMdxModel =  (m_ppModel)->GetModel();
	
	/// set the model color
	pMdxModel->SetModelColor(m_modelColor);

	/// set the fixed function shader for drawing the floor
    pd3dDevice->SetVertexShader( NULL );
	
	/// Render state
	pd3dDevice->SetRenderState(D3DRS_LIGHTING,sceneState->bEnableLighting);
	CGlobals::GetEffectManager()->SetCullingMode(true);

	/// get world transform matrix
	Matrix4 mx;
    ParaMatrixMultiply( & mx, & m_mxLocalTransform, mxWorld );
    pd3dDevice->SetTransform( D3DTS_WORLD, & mx );
	
	/// render
	pMdxModel->Render(sceneState);
	
	return S_OK;
}

void CMdxAniInstance::BuildShadowVolume(SceneState * sceneState,ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{
	CMDXModel *pMdxModel =  (m_ppModel)->GetModel();

	/** Copy animation status of this MDX instance object to the MDX model */
	CopyAnimInfoToParaXModel();

	/** Call the ParaX model's build shadow volume function */
	// update the world transform
	Matrix4 mx;
    ParaMatrixMultiply( & mx, & m_mxLocalTransform, mxWorld );
	pMdxModel->BuildShadowVolume(pShadowVolume, pLight, & mx);
}

/// get the size scale scale normally this is 1.0f
/// M_localTransform = ScaleMatrix(M_fSizeScale* G_fMdxGlobalScale)
void CMdxAniInstance::SetSizeScale(float fScale)
{
	m_fSizeScale = fScale;
	fScale *= g_fMdxGlobalScale;
	Matrix4 mx;
	ParaMatrixScaling(&mx, fScale, fScale, fScale);
	ParaMatrixRotationX(&m_mxLocalTransform, -3.1415926f/2);
	ParaMatrixMultiply( & m_mxLocalTransform, & mx, & m_mxLocalTransform );
}
/// normally this will read the radius of the current animation
/// and calculate the correct size after scaling
void CMdxAniInstance::GetCurrentRadius(float* fRadius)
{
	///fRadius = M_fSizeScale* G_fMdxGlobalScale *boundsRadius
	float boundsRadius = m_ppModel->GetModel()->sequences.sequences[m_aiAnimIntance.nCurrentAnim].boundsRadius;
	*fRadius = m_fSizeScale* g_fMdxGlobalScale *boundsRadius;
}
/// normally this will read the radius of the current animation
/// and calculate the correct size after scaling
void CMdxAniInstance::GetCurrentSize(float * fWidth, float * fDepth)
{
	///fRadius = M_fSizeScale* G_fMdxGlobalScale *boundsRadius
	Vector3	box = m_ppModel->GetModel()->sequences.sequences[m_aiAnimIntance.nCurrentAnim].maxs
		-m_ppModel->GetModel()->sequences.sequences[m_aiAnimIntance.nCurrentAnim].mins;

	/// TODO: since mdx model should be rotated 90 degrees along the neg x axis,
	/// the width and depth are also changed from x,z to x,y
	* fWidth = fabs(m_fSizeScale* g_fMdxGlobalScale * box.x);
	* fDepth = fabs(m_fSizeScale* g_fMdxGlobalScale * box.y);
}

void CMdxAniInstance::SetSpeedScale( float fScale )
{
	CAnimInstanceBase::SetSpeedScale(fScale);
}
/// normally this will read the move speed of the current animation
/// and calculate the correct(scaled) speed
void CMdxAniInstance::GetCurrentSpeed (float* fSpeed)
{
	/// fSpeed = M_fSpeedScale* M_fSizeScale* G_fMdxGlobalScale * moveSpeed
	float moveSpeed = m_ppModel->GetModel()->sequences.sequences[m_aiAnimIntance.nCurrentAnim].moveSpeed;
	*fSpeed = m_fSpeedScale* m_fSizeScale* g_fMdxGlobalScale * moveSpeed;
}
/// normally this will read the move speed of the specified animation
/// and calculate the correct(scaled) speed
void CMdxAniInstance::GetSpeedOf(const char * sName, float * fSpeed)
{
	/// map string to anim Index 
	DWORD nIndexAnim = m_ppModel->GetModel()->GetAnimNum(sName);
	if(nIndexAnim>=0 && (int)nIndexAnim<m_ppModel->GetModel()->sequences.numSequences)
	{
		/// fSpeed = M_fSpeedScale* M_fSizeScale* G_fMdxGlobalScale * moveSpeed
		float moveSpeed = m_ppModel->GetModel()->sequences.sequences[nIndexAnim].moveSpeed;
		*fSpeed = m_fSpeedScale* m_fSizeScale* g_fMdxGlobalScale * moveSpeed;
	}
}
#endif