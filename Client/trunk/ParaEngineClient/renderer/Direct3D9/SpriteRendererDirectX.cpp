//----------------------------------------------------------------------
// Class:	2D sprite renderer
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2014.8
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "TextureEntity.h"
#include "ParaWorldAsset.h"
#include "2dengine/GUIRoot.h"
#include "SpriteRendererDirectX.h"

using namespace ParaEngine;


ParaEngine::CSpriteRendererDirectX::CSpriteRendererDirectX()
	:m_pSprite(NULL), m_pLastDecl(NULL)
{

}

ParaEngine::CSpriteRendererDirectX::~CSpriteRendererDirectX()
{
	SAFE_RELEASE(m_pSprite);
}

HRESULT ParaEngine::CSpriteRendererDirectX::Begin(DWORD Flags)
{
	m_flags = Flags;
	if (m_pSprite)
		return m_pSprite->Begin(Flags);
	else
		return E_FAIL;
}

void ParaEngine::CSpriteRendererDirectX::End()
{
	if (m_pSprite)
		m_pSprite->End();
}

void ParaEngine::CSpriteRendererDirectX::FlushQuads()
{
	if (m_pSprite)
		m_pSprite->Flush();
}

HRESULT ParaEngine::CSpriteRendererDirectX::DrawRect(const RECT* pRect, Color color, float depth)
{
	auto pWhiteTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0);
	RECT rcTexture = { 0, 0, pRect->right - pRect->left, pRect->bottom - pRect->top };
	Vector3 vPos((float)pRect->left, (float)pRect->top, depth);
	return DrawQuad(pWhiteTexture, &rcTexture, NULL, &vPos, color);
}

HRESULT ParaEngine::CSpriteRendererDirectX::DrawQuad(TextureEntity* pTexture_, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color color)
{
	TextureEntityDirectX* pTexture = ((TextureEntityDirectX*)pTexture_);
	if (pTexture && m_pSprite)
	{
		auto pTex = pTexture->GetTexture();
		if (pTex != 0)
		{
			return m_pSprite->Draw(pTex, pSrcRect, (const DeviceVector3*)vCenter, (const DeviceVector3*)pPosition, (DWORD)color);
		}
	}
	return S_OK;
}

HRESULT ParaEngine::CSpriteRendererDirectX::SetTransform(const Matrix4& mat)
{
	m_transform = mat;
	if (m_pSprite)
	{
		m_pSprite->SetTransform(mat.GetConstPointer());
	}
	return E_FAIL;
}

void ParaEngine::CSpriteRendererDirectX::InitDeviceObjects()
{
	SAFE_RELEASE(m_pSprite);
	HRESULT hr = D3DXCreateSprite(CGlobals::GetRenderDevice(), &m_pSprite);
	if (FAILED(hr))
	{
		OUTPUT_LOG("error: D3DXCreateSprite in CSpriteRendererDirectX::InitDeviceObjects\n");
	}
}

void ParaEngine::CSpriteRendererDirectX::RestoreDeviceObjects()
{
	if (m_pSprite)
		m_pSprite->OnResetDevice();
}

void ParaEngine::CSpriteRendererDirectX::InvalidateDeviceObjects()
{
	if (m_pSprite)
		m_pSprite->OnLostDevice();
}

void ParaEngine::CSpriteRendererDirectX::DeleteDeviceObjects()
{
	SAFE_RELEASE(m_pSprite);
}

#endif