//-----------------------------------------------------------------------------
// Class:	EffectHelper
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.12.12
// Revised: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EffectManager.h"
#include "EffectHelper.h"

using namespace ParaEngine;

ParaEngine::EffectTextureStateStack::EffectTextureStateStack(EffectManager* pEffectManager, bool bPointTexture) : m_pEffectManager(pEffectManager)
{
	m_dwFilter = bPointTexture ? D3DTEXF_POINT : D3DTEXF_LINEAR;
	m_nLastMinFilter = pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MINFILTER);
	m_nLastMagFilter = pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MAGFILTER);
	if (m_dwFilter != m_nLastMinFilter)
	{
		pEffectManager->SetDefaultSamplerState(0, D3DSAMP_MINFILTER, m_dwFilter);
	}
	if (m_dwFilter != m_nLastMagFilter)
	{
		pEffectManager->SetDefaultSamplerState(0, D3DSAMP_MAGFILTER, m_dwFilter);
	}
}

ParaEngine::EffectTextureStateStack::~EffectTextureStateStack()
{
	if (m_dwFilter != m_nLastMinFilter)
	{
		m_pEffectManager->SetDefaultSamplerState(0, D3DSAMP_MINFILTER, m_nLastMinFilter);
	}
	if (m_dwFilter != m_nLastMagFilter)
	{
		m_pEffectManager->SetDefaultSamplerState(0, D3DSAMP_MAGFILTER, m_nLastMinFilter);
	}
}
