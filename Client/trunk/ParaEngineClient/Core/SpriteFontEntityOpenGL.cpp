//-----------------------------------------------------------------------------
// Class:	SpriteFontEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12, 2014.8(cross platform)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_OPENGL_RENDERER
#include "platform/OpenGLWrapper.h"
#include "StringHelper.h"
#include "2dengine/GUIBase.h"
#include "2dengine/FontRendererOpenGL.h"
#include "SpriteFontEntity.h"
#include "memdebug.h"

using namespace ParaEngine;
USING_NS_CC;

SpriteFontEntityOpenGL::SpriteFontEntityOpenGL(const AssetKey& key)
:SpriteFontEntity(key), m_fontRenderer(NULL)
{
}
SpriteFontEntityOpenGL::SpriteFontEntityOpenGL()
: m_fontRenderer(NULL)
{
}

SpriteFontEntityOpenGL::~SpriteFontEntityOpenGL()
{
}

void ParaEngine::SpriteFontEntityOpenGL::Cleanup()
{
	CC_SAFE_RELEASE_NULL(m_fontRenderer);
}

HRESULT ParaEngine::SpriteFontEntityOpenGL::DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor)
{
	LoadAsset();
	if (m_fontRenderer)
		m_fontRenderer->DrawTextW(pSprite, strText, *rect, dwTextFormat, textColor);
	return S_OK;
}

HRESULT ParaEngine::SpriteFontEntityOpenGL::DrawTextA(CSpriteRenderer* pSprite, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor)
{
	LoadAsset();
	if (m_fontRenderer)
		m_fontRenderer->DrawTextA(pSprite, strText, *rect, dwTextFormat, textColor);
	return S_OK;
}

HRESULT ParaEngine::SpriteFontEntityOpenGL::InitDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;
	CC_SAFE_RELEASE(m_fontRenderer);
	m_fontRenderer = CFontRendererOpenGL::create(GetFontName(), GetFontSize());
	CC_SAFE_RETAIN(m_fontRenderer);
	return S_OK;
}

HRESULT ParaEngine::SpriteFontEntityOpenGL::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	CC_SAFE_RELEASE_NULL(m_fontRenderer);
	return S_OK;
}

HRESULT ParaEngine::SpriteFontEntityOpenGL::RendererRecreated()
{
	// the default implementation of CCLabel's FontAtlas does not seem to handle renderer creation very well
	// we just delete and create again. 
	DeleteDeviceObjects();
	InitDeviceObjects();
	return S_OK;
}

#endif