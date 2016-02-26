//-----------------------------------------------------------------------------
// File: MultiAnimation.h
//
// Desc: Header file for the MultiAnimation library.  This contains the
//       declarations of
//
//       MultiAnimFrame              (no .cpp file)
//       MultiAnimMC                 (MultiAnimationLib.cpp)
//       CMultiAnimAllocateHierarchy (AllocHierarchy.cpp)
//       CMultiAnim                  (MultiAnimationLib.cpp)
//       CAnimInstance               (AnimationInstance.cpp)
//
// Copyright (c) Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#pragma once
#include "BaseObject.h"
#include "mdxfile/ModelUtils.h"
#include "AnimInstanceBase.h"

namespace ParaEngine
{
/** Animation instance that each mdx object owns*/
struct AnimInstance
{
public:
	int nCurrentAnim;
	/// -1 to use the default one
	int nNextAnim;		
	int currentFrame;	// an absolute MDX frame 
	int blendingAnim;	
	int blendingFrame;	// an absolute MDX frame with which the current motion is blended
	float blendingFactor; // [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame
	
	/// bit field, the lowest bits is the first mesh, default to 0xffffffff
	DWORD		dwShowMeshes;	

public:
	AnimInstance()
	{
		nCurrentAnim = 0;
		nNextAnim = -1;
		currentFrame = -1;
		blendingAnim = -1;
		blendingFrame = 0;
		blendingFactor = 0;
		/// show all by default
		dwShowMeshes = 0xffffffff; 
	};

	void ShowMesh(BOOL bShow, int nIndex)
	{
		if(bShow)
			dwShowMeshes = dwShowMeshes | (0x1<<nIndex);
		else
			dwShowMeshes = dwShowMeshes & (~(0x1<<nIndex));
	};
};

struct MDXEntity;
class ShadowVolume;
struct LightParams;
/// MDX file based Animation instance:
/// must call SetSizeScale() at least once before drawing this object.
/// in most cases, SetSizeScale(1.0f) will use the size of the first animation
/// defined in the mdx model file.
///
/// see <scenecreationhelper> for its usage
class CMdxAniInstance : public CAnimInstanceBase
{
public:
	CMdxAniInstance(void);
	~CMdxAniInstance(void);
	
	/// MDX animation instance struct
	AnimInstance m_aiAnimIntance;
	
	///-- 3d device objects
	MDXEntity* m_ppModel;
public:
	void CopyAnimInfoToParaXModel();
	void SaveAnimInfoFromParaXModel();

public:
	// -- for init and delete
	HRESULT InitObject(MDXEntity* ppModel);
	AssetEntity* GetPrimaryAsset(){return (AssetEntity*)m_ppModel;};				// used as KEY for batch rendering
	
	// -- CAnimInstanceBase virtual functions

	/**
	* Play or append the specified animation.
	* If the current animation is the same as the specified animation, it will loop on the 
	* current animation infinitely until a different animation is specified; otherwise,
	* the current animation will be stopped, the specified animation played and the next animation emptied.
	* @param fSpeed: [out] the speed of the animation after applying scaling is returned.
	* @param bAppend: if true, the new animation will be appended to the current animation according to the following rules:
	*		- if the current animation is non-looping, the new animation will be played after the end of the current animation.
	*		- if the current animation is looping, the new animation will be played immediately.
	*/
	virtual void LoadAnimation(const char * sName, float * fSpeed,bool bAppend = false);
	virtual void LoadAnimation(int nIndexAnim, float * fSpeed,bool bAppend = false);
	virtual void LoadDefaultStandAnim(float * fSpeed);
	virtual void LoadDefaultWalkAnim(float * fSpeed);


	/// a model might has several meshes, you can show or hide them by setting
	/// the bitfield of a DWORD. the lowest bit is the first mesh.
	virtual void ShowMesh(BOOL bShow, int nIndex);
	virtual void ShowMesh(DWORD dwBitfields);
	
	/// Draw() method will call this function automatically
	virtual void AdvanceTime( double dTimeDelta );
	/// simply call this function to draw and advance time. 
	/// set dTimeDelta to 0 if you want to render only static model.
	virtual HRESULT Draw( SceneState * sceneState, const Matrix4* mxWorld);
	virtual void BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

	/// get the size scale scale normally this is 1.0f
	virtual void SetSizeScale(float fScale);
	/// normally this will read the radius of the current animation
	/// and calculate the correct size after scaling
	virtual void GetCurrentRadius(float* fRadius);
	/// normally this will read the radius of the current animation
	/// and calculate the correct size after scaling
	virtual void GetCurrentSize(float * fWidth, float * fDepth);
	/// set the scale factor that will be premultiplied to the loaded model and animation
	virtual void SetSpeedScale( float fScale );
	/// normally this will read the move speed of the current animation
	/// and calculate the correct(scaled) speed
	virtual void GetCurrentSpeed (float* fSpeed);
	/// normally this will read the move speed of the specified animation
	/// and calculate the correct(scaled) speed
	virtual void GetSpeedOf(const char * sName, float * fSpeed);
};
}