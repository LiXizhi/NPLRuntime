//-----------------------------------------------------------------------------
// Class:	AnimInstanceBase
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel/AnimTable.h"
#include "AnimInstanceBase.h"

using namespace ParaEngine;


ParaEngine::CAnimInstanceBase::CAnimInstanceBase() :
m_modelColor((uint32)0xff0000ff),
m_fSpeedScale(1.0f),
m_fSizeScale(1.0f), m_nRenderCount(0),
m_bEnableAnimIDMap(false), m_bEnableAnimation(true)
{

}

int ParaEngine::CAnimInstanceBase::GetRenderCount()
{
	return m_nRenderCount;
}

void ParaEngine::CAnimInstanceBase::SetRenderCount(int nCount)
{
	m_nRenderCount = nCount;
}

void ParaEngine::CAnimInstanceBase::EnableAnimIDMap(bool bEnable)
{
	m_bEnableAnimIDMap = bEnable;
}

bool ParaEngine::CAnimInstanceBase::IsAnimIDMapEnabled()
{
	return m_bEnableAnimIDMap;
}

map<int, int>* ParaEngine::CAnimInstanceBase::GetAnimIDMap()
{
	return &m_mapAnimIDs;
}

void ParaEngine::CAnimInstanceBase::ApplyLocalTransformToVec3(Vector3 *pOut, const Vector3 *pV)
{
	ParaVec3TransformCoord(pOut, pV, &m_mxLocalTransform);
}

void ParaEngine::CAnimInstanceBase::ApplyLocalTransformToVec1(float *pOut, float* pV)
{
	*pOut = (*pV) * m_mxLocalTransform._11;
}

void ParaEngine::CAnimInstanceBase::LoadAnimation(const char * sName, float * fSpeed, bool bAppend /*= false*/)
{
	LoadAnimation(CAnimTable::GetAnimIDByName(sName), fSpeed, bAppend);
}

void ParaEngine::CAnimInstanceBase::LoadAnimation(int nAnimID, float * fSpeed, bool bAppend /*= false*/)
{

}

void ParaEngine::CAnimInstanceBase::LoadDefaultStandAnim(float * fSpeed)
{
	LoadAnimation(ANIM_STAND, fSpeed);
}

void ParaEngine::CAnimInstanceBase::LoadDefaultWalkAnim(float * fSpeed)
{
	LoadAnimation(ANIM_RUN, fSpeed);
}

bool ParaEngine::CAnimInstanceBase::HasAnimId(int nAnimID)
{
	return false;
}

int ParaEngine::CAnimInstanceBase::GetCurrentAnimation()
{
	return 0;
}

void ParaEngine::CAnimInstanceBase::ResetAnimation()
{

}

void ParaEngine::CAnimInstanceBase::SetModelColor(LinearColor modelColor)
{
	m_modelColor = modelColor;
}

void ParaEngine::CAnimInstanceBase::ShowMesh(BOOL bShow, int nIndex)
{

}

void ParaEngine::CAnimInstanceBase::ShowMesh(DWORD dwBitfields)
{

}

void ParaEngine::CAnimInstanceBase::SetLocalTransform(Matrix4 mXForm)
{
	m_mxLocalTransform = mXForm;
}

void ParaEngine::CAnimInstanceBase::AdvanceTime(double dTimeDelta)
{

}

void ParaEngine::CAnimInstanceBase::Animate(double dTimeDelta, int nRenderNumber/*=0 */)
{

}

HRESULT ParaEngine::CAnimInstanceBase::Draw(SceneState * sceneState, const Matrix4* mxWorld, CParameterBlock* paramBlock /*= NULL*/)
{
	return 0;
}

void ParaEngine::CAnimInstanceBase::GetSpeedOf(const char * sName, float * fSpeed)
{

}

void ParaEngine::CAnimInstanceBase::GetCurrentSpeed(float* fSpeed)
{
	*fSpeed = 1.0f;
}

float ParaEngine::CAnimInstanceBase::GetSpeedScale()
{
	return m_fSpeedScale;
}

void ParaEngine::CAnimInstanceBase::SetSpeedScale(float fScale)
{
	m_fSpeedScale = fScale;
}

void ParaEngine::CAnimInstanceBase::GetCurrentSize(float * fWidth, float * fDepth)
{

}

void ParaEngine::CAnimInstanceBase::SetSizeScale(float fScale)
{
	m_fSizeScale = fScale;
}

float ParaEngine::CAnimInstanceBase::GetSizeScale()
{
	return m_fSizeScale;
}

void ParaEngine::CAnimInstanceBase::BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{

}

void ParaEngine::CAnimInstanceBase::SetUseGlobalTime(bool bUseGlobalTime)
{

}

bool ParaEngine::CAnimInstanceBase::IsUseGlobalTime()
{
	return false;
}

void ParaEngine::CAnimInstanceBase::SetAnimFrame(int nFrame)
{

}

int ParaEngine::CAnimInstanceBase::GetAnimFrame()
{
	return 0;
}

void ParaEngine::CAnimInstanceBase::SetBlendingFactor(float fBlendingFactor)
{

}

bool ParaEngine::CAnimInstanceBase::HasAnimation(int nAnimID)
{
	return HasAnimId(nAnimID);
}

bool ParaEngine::CAnimInstanceBase::HasAlphaBlendedObjects()
{
	return false;
}

int ParaEngine::CAnimInstanceBase::GetValidAnimID(int nAnimID)
{
	return -1;
}

void ParaEngine::CAnimInstanceBase::EnableAnimation(bool bAnimated)
{
	m_bEnableAnimation = bAnimated;
}

bool ParaEngine::CAnimInstanceBase::IsAnimationEnabled() const
{
	return m_bEnableAnimation;
}

int ParaEngine::CAnimInstanceBase::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("RenderCount", FieldType_Float, (void*)SetRenderCount_s, (void*)GetRenderCount_s, NULL, "", bOverride);
	pClass->AddField("EnableAnimIDMap", FieldType_Bool, (void*)EnableAnimIDMap_s, (void*)IsAnimIDMapEnabled_s, NULL, "", bOverride);
	pClass->AddField("ResetAnimation", FieldType_void, (void*)ResetAnimation_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("CurrentAnimation", FieldType_Int, (void*)0, (void*)GetCurrentAnimation_s, NULL, "", bOverride);
	pClass->AddField("ShowMesh", FieldType_DWORD, (void*)ShowMesh_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("SizeScale", FieldType_Float, (void*)SetSizeScale_s, (void*)GetSizeScale_s, NULL, "", bOverride);
	pClass->AddField("SpeedScale", FieldType_Float, (void*)SetSpeedScale_s, (void*)GetSpeedScale_s, NULL, "", bOverride);
	pClass->AddField("IsUseGlobalTime", FieldType_Bool, (void*)SetUseGlobalTime_s, (void*)IsUseGlobalTime_s, NULL, "", bOverride);
	pClass->AddField("AnimFrame", FieldType_Int, (void*)SetAnimFrame_s, (void*)GetAnimFrame_s, NULL, "", bOverride);
	pClass->AddField("IsAnimationEnabled", FieldType_Bool, (void*)EnableAnimation_s, (void*)IsAnimationEnabled_s, NULL, "", bOverride);
	return S_OK;
}
