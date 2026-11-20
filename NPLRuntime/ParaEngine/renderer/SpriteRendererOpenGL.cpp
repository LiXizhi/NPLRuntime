//----------------------------------------------------------------------
// Class:	2D sprite renderer
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2014.8
// Desc:  code is partially based on d3dxsprite WINE implementation.
// https://github.com/wine-mirror/wine/blob/master/dlls/d3dx9_36/sprite.c
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef USE_NULL_RENDERER
#include "TextureEntity.h"
#include "OpenGLWrapper/GLProgram.h"
#include "OpenGLWrapper/GLTexture2D.h"
//#include "CGUIRootLayer.h"
#include "EffectManager.h"
#include "2dengine/GUIRoot.h"
#include "ParaWorldAsset.h"
#include "BaseObject.h"
#include "ViewportManager.h"
#include "SpriteRendererOpenGL.h"
#include "VertexDeclarationOpenGL.h"

using namespace ParaEngine;

ParaEngine::CSpriteRendererOpenGL::CSpriteRendererOpenGL()
	:m_sprite_count(0), m_pEffectFile(0), m_bNeedUpdateShader(false), m_bIsTextMode(false), m_bIsTextModeDevice(false)
{

}

ParaEngine::CSpriteRendererOpenGL::~CSpriteRendererOpenGL()
{
	Cleanup();
}

void ParaEngine::CSpriteRendererOpenGL::Cleanup()
{
}

HRESULT ParaEngine::CSpriteRendererOpenGL::Begin(DWORD Flags)
{
	m_flags = Flags;
	m_ready = TRUE;

	ParaViewport viewport;
	CGlobals::GetViewportManager()->GetCurrentViewport(viewport);

	m_viewport = viewport;

	if (!IsUseObjectSpaceTransform())
	{
		CGlobals::GetViewMatrixStack().push(Matrix4::IDENTITY);
		Matrix4 matProj;
		if (!IsIgnoreScreenRotate() && CGlobals::GetApp()->IsRotateScreen() && CGlobals::GetViewportManager()->GetActiveViewPort()->GetRenderTarget() == NULL)
		{
			ParaMatrixOrthoOffCenterLH(&matProj, (float)(m_viewport.Y), (float)(m_viewport.Y + m_viewport.Height), (float)(m_viewport.X + m_viewport.Width), (float)(m_viewport.X), 0.f, 1.f);
				
			Matrix4 rotationZ;
			rotationZ.makeRot(Quaternion(Vector3(0, 0, 1.f), MATH_PI / 2.f), Vector3(0, 0, 0));
			matProj = matProj * rotationZ;
		}
		else
		{
			// @NOTE: directX and openGL screen space coordinate system differs by 1-y
			// we modified the projection matrix, and swapped top and bottom to make it behave like DirectX
			ParaMatrixOrthoOffCenterLH(&matProj, (float)(m_viewport.X), (float)(m_viewport.X + m_viewport.Width), (float)(m_viewport.Y + m_viewport.Height), (float)(m_viewport.Y), 0.f, 1.f);
			// the following is used by opengl
			//ParaMatrixOrthoOffCenterOpenGL(&matProj, 0.f, lastScreenSize.x, 0.f, lastScreenSize.y, 0.f, 1.f);
		}
		CGlobals::GetProjectionMatrixStack().push(matProj);
	}

	UpdateShader(true);
	return S_OK;
}

bool ParaEngine::CSpriteRendererOpenGL::BeginCustomShader()
{
	Flush();
	SetNeedUpdateShader(false);
	m_batchEffect = nullptr;
	return true;
}

void ParaEngine::CSpriteRendererOpenGL::EndCustomShader()
{
	SetNeedUpdateShader(true);
}

void ParaEngine::CSpriteRendererOpenGL::SetNeedUpdateShader(bool bNeedUpdate)
{
	m_bNeedUpdateShader = bNeedUpdate;
}

void ParaEngine::CSpriteRendererOpenGL::UpdateShader(bool bForceUpdate)
{
	if (m_bNeedUpdateShader || bForceUpdate)
	{
		SetNeedUpdateShader(false);
		CGlobals::GetEffectManager()->BeginEffect(TECH_GUI, &m_pEffectFile);

		m_batchEffect = nullptr;
#ifdef PARA_ENABLE_OPENGL
		auto* device = CGlobals::GetRenderDevice();
		static const bool enabled = strcmp(CGlobals::GetApp()->GetAppCommandLineByParam("guimultitexture", "true"), "false") != 0;
		if (enabled && device->GetBackendType() == ERHIBackend::OpenGL && device->GetCaps().featureLevel >= 300 &&
			device->GetMaxSimultaneousTextures() >= 4 && m_pEffectFile && m_pEffectFile->GetFileName() == ":IDR_FX_GUI")
		{
			auto* effect = CGlobals::GetAssetManager()->LoadEffectFile("guiBatch", ":IDR_FX_GUI_BATCH");
			if (effect)
			{
				effect->LoadAsset();
				if (effect->IsValid()) m_batchEffect = effect;
			}
		}
#endif
		if (m_pEffectFile && m_pEffectFile->begin())
		{
			m_pEffectFile->BeginPass(0);
			m_pEffectFile->SetBool("k_bBoolean0", !m_bIsTextMode);
			m_bIsTextModeDevice = m_bIsTextMode;
		}
	}
}


void ParaEngine::CSpriteRendererOpenGL::End()
{
	if (!m_ready){
		OUTPUT_LOG("error:CSpriteRendererOpenGL::End() and Begin() are not called in pairs\n");
		return;
	}
	Flush();
	m_ready = FALSE;

	// Not needed now, this will force update text mode uniform after next begin() function.
	// SetTextMode(false);
	// PrepareDraw();

	if (m_pEffectFile)
	{
		m_pEffectFile->EndPass();
		m_pEffectFile->end();
	}

	if (!IsUseObjectSpaceTransform())
	{
		CGlobals::GetViewMatrixStack().pop();
		CGlobals::GetProjectionMatrixStack().pop();
	}
}

HRESULT ParaEngine::CSpriteRendererOpenGL::DrawRect(const RECT* pRect, Color color, float depth)
{
	auto pWhiteTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0);
	RECT rcTexture = { 0, 0, pRect->right - pRect->left, pRect->bottom - pRect->top };
	Vector3 vPos((float)pRect->left, (float)pRect->top, depth);
	return DrawQuad(pWhiteTexture, &rcTexture, NULL, &vPos, color);
}

HRESULT ParaEngine::CSpriteRendererOpenGL::DrawQuad(TextureEntity* pTexture, const RECT* rect, const Vector3* center, const Vector3* position, Color color)
{
	if (pTexture == NULL || !m_ready)
		return E_FAIL;

	if (m_batchEffect && !m_bNeedUpdateShader && m_pEffectFile == CGlobals::GetEffectManager()->GetCurrentEffectFile())
	{
		// Preserve primitive order across triangles/lines. Quads carry their
		// own glyph mode, so changing from a label to its neighbor can wait.
		if (m_thickline_count || m_triangles_count) Flush();
	}
	else
		PrepareDraw();

	if (m_sprites.size() == 0)
	{
		m_sprites.resize(32);
	}
	else if ((int)m_sprites.size() <= m_sprite_count)
	{
		m_sprites.resize(m_sprites.size() + m_sprites.size() /2);
	}
	auto texture = pTexture->GetTexture();
	SpriteQuad& newSprite = m_sprites[m_sprite_count];
	newSprite.texture = texture;

	int texWidth=0, texHeight = 0;

	/* Reuse the texture desc if possible */
	if (m_sprite_count== 0 || m_sprites[m_sprite_count - 1].texture != texture) {
		texWidth = pTexture->GetWidth();
		texHeight = pTexture->GetHeight();
	}
	else {
		texWidth = m_sprites[m_sprite_count - 1].texw;
		texHeight = m_sprites[m_sprite_count - 1].texh;
	}

	newSprite.texw = texWidth;
	newSprite.texh = texHeight;

	if (rect == NULL) {
		newSprite.rect.left = 0;
		newSprite.rect.top = 0;
		newSprite.rect.right = texWidth;
		newSprite.rect.bottom = texHeight;
	}
	else
		newSprite.rect = *rect;

	if (pTexture->IsFlipY())
	{
		int nTop = texHeight - newSprite.rect.bottom;
		int nBottom = texHeight - newSprite.rect.top;
		newSprite.rect.top = nBottom;
		newSprite.rect.bottom = nTop;
	}

	if (center == NULL) {
		newSprite.center.x = 0.0f;
		newSprite.center.y = 0.0f;
		newSprite.center.z = 0.0f;
	}
	else
		newSprite.center = *center;

	if (position == NULL) {
		newSprite.pos.x = 0.0f;
		newSprite.pos.y = 0.0f;
		newSprite.pos.z = 0.0f;
	}
	else
		newSprite.pos = *position;

	newSprite.color = color;
	newSprite.transform = m_transform;
	if (m_bIsTextMode)
	{
		newSprite.bPointFilter = false;
		newSprite.bClampAddress = true;
	}
	else
	{
		DWORD curMinFilter = D3DTEXF_LINEAR;
		CGlobals::GetEffectManager()->GetSamplerState(0, ESamplerStateType::MINFILTER, &curMinFilter);
		newSprite.bPointFilter = pTexture->IsSamplerStateBlocky()
			|| (curMinFilter == D3DTEXF_POINT)
			|| (CGUIRoot::GetInstance() && CGUIRoot::GetInstance()->GetUsePointTextureFiltering());
		newSprite.bClampAddress = false;
	}
	m_sprite_count++;

	return S_OK;
}

void ParaEngine::CSpriteRendererOpenGL::InitDeviceObjects()
{
}

void ParaEngine::CSpriteRendererOpenGL::RestoreDeviceObjects()
{
}

void ParaEngine::CSpriteRendererOpenGL::InvalidateDeviceObjects()
{
}

void ParaEngine::CSpriteRendererOpenGL::DeleteDeviceObjects()
{

}


void ParaEngine::CSpriteRendererOpenGL::FlushQuads()
{
	if (!m_ready) return;
	if (m_sprite_count<=0) return;

	UpdateShader();

	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();

	DWORD savedMinFilter = D3DTEXF_LINEAR;
	DWORD savedMagFilter = D3DTEXF_LINEAR;
	DWORD savedAddressU = D3DTADDRESS_WRAP;
	DWORD savedAddressV = D3DTADDRESS_WRAP;
	auto* pEffectMgr = CGlobals::GetEffectManager();
	pEffectMgr->GetSamplerState(0, ESamplerStateType::MINFILTER, &savedMinFilter);
	pEffectMgr->GetSamplerState(0, ESamplerStateType::MAGFILTER, &savedMagFilter);
	pEffectMgr->GetSamplerState(0, ESamplerStateType::ADDRESSU, &savedAddressU);
	pEffectMgr->GetSamplerState(0, ESamplerStateType::ADDRESSV, &savedAddressV);

#ifdef EMSCRIPTEN_SINGLE_THREAD
	if (m_sprite_count > 1 && pRenderDevice->GetBackendType() == ERHIBackend::OpenGL &&
		m_pEffectFile && pEffectMgr->GetCurrentEffectFile() == m_pEffectFile &&
		m_pEffectFile->GetFileName() == ":IDR_FX_GUI")
	{
		// Resolve at flush so uploads, edits and device loss invalidate a queued
		// image before it is used. Offsetting both edges preserves quad geometry.
		for (int i = 0; i < m_sprite_count; ++i)
		{
			auto& sprite = m_sprites[i];
			if (sprite.bClampAddress) continue; // Glyphs already have their own atlas.
			auto* image = dynamic_cast<GLTexture2D*>(sprite.texture);
			if (image)
			{
				if (auto* page = image->getGuiAtlasRegion(sprite.rect))
				{
					sprite.texture = page;
					sprite.texw = page->GetWidth();
					sprite.texh = page->GetHeight();
				}
			}
		}
	}
#endif
	auto writeQuad = [](const SpriteQuad& curSprite, auto vertex, size_t stride)
	{
		const float width = (float)Math::Abs(curSprite.rect.right - curSprite.rect.left);
		const float height = (float)Math::Abs(curSprite.rect.bottom - curSprite.rect.top);
		const Vector3 origin = curSprite.pos - curSprite.center;
		const float right = width + curSprite.pos.x - curSprite.center.x;
		const float bottom = height + curSprite.pos.y - curSprite.center.y;
		vertex(0).pos = origin;
		vertex(1).pos = Vector3(right, origin.y, origin.z);
		vertex(2).pos = Vector3(right, bottom, origin.z);
		vertex(3).pos = Vector3(origin.x, bottom, origin.z);

		const float u0 = (float)curSprite.rect.left / curSprite.texw;
		const float v0 = (float)curSprite.rect.top / curSprite.texh;
		const float u1 = (float)curSprite.rect.right / curSprite.texw;
		const float v1 = (float)curSprite.rect.bottom / curSprite.texh;
		vertex(0).tex = Vector2(u0, v0);
		vertex(1).tex = Vector2(u1, v0);
		vertex(2).tex = Vector2(u1, v1);
		vertex(3).tex = Vector2(u0, v1);
		for (int j = 0; j < 4; ++j)
			vertex(j).col = curSprite.color;
		const auto& m = curSprite.transform;
		if (m._12 == 0 && m._13 == 0 && m._14 == 0 && m._21 == 0 && m._23 == 0 && m._24 == 0 &&
			m._31 == 0 && m._32 == 0 && m._34 == 0 && m._44 == 1)
		{
			// Axis-aligned HUD images and glyphs share two x and two y coordinates.
			const float x0 = origin.x * m._11 + m._41, x1 = right * m._11 + m._41;
			const float y0 = origin.y * m._22 + m._42, y1 = bottom * m._22 + m._42;
			const float z = origin.z * m._33 + m._43;
			vertex(0).pos = Vector3(x0, y0, z);
			vertex(1).pos = Vector3(x1, y0, z);
			vertex(2).pos = Vector3(x1, y1, z);
			vertex(3).pos = Vector3(x0, y1, z);
		}
		else
			ParaVec3TransformCoordArray(&vertex(0).pos, stride, &vertex(0).pos, stride, &m, 4);
		vertex(4) = vertex(0);
		vertex(5) = vertex(2);
	};

	// Keep translucent quads in submission order, including text between images.
	constexpr int maxSlots = 8;
	const int textureSlots = pRenderDevice->GetMaxSimultaneousTextures() >= maxSlots ? maxSlots : 4;
	const bool batch = m_batchEffect && m_pEffectFile == pEffectMgr->GetCurrentEffectFile();
	for (int start = 0, end; start < m_sprite_count; start = end)
	{
		DeviceTexturePtr_type textures[maxSlots] = {m_sprites[start].texture};
		int used = 1;
		const auto& first = m_sprites[start];
		for (end = start + 1; end < m_sprite_count; ++end)
		{
			const auto& next = m_sprites[end];
			if (next.bPointFilter != first.bPointFilter) break;
			if (next.texture == first.texture && next.bClampAddress == first.bClampAddress) continue;
			if (!batch || !next.texture || !first.texture) break;
			auto* found = std::find(textures, textures + used, next.texture);
			if (found == textures + used)
			{
				if (used == textureSlots) break;
				textures[used++] = next.texture;
			}
		}

		const DWORD filter = first.bPointFilter ? D3DTEXF_POINT : D3DTEXF_LINEAR;
		pRenderDevice->SetSamplerState(0, ESamplerStateType::MINFILTER, filter);
		pRenderDevice->SetSamplerState(0, ESamplerStateType::MAGFILTER, filter);
		pRenderDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, first.bClampAddress ? D3DTADDRESS_CLAMP : savedAddressU);
		pRenderDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, first.bClampAddress ? D3DTADDRESS_CLAMP : savedAddressV);
		pRenderDevice->SetTexture(0, first.texture);

#ifdef PARA_ENABLE_OPENGL
		if (used > 1 && m_batchEffect->begin() && m_batchEffect->BeginPass(0))
		{
			static const VertexElement layout[] = {
				{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
				{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
				{0, 16, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
				D3DDECL_END()
			};
			static CVertexDeclaration declaration(layout);
			const size_t vertexCount = 6 * (end - start);
			if (m_batchVertices.size() < vertexCount)
				m_batchVertices.resize(vertexCount);
			for (int i = start; i < end; ++i)
			{
				const int unit = static_cast<int>(std::find(textures, textures + used, m_sprites[i].texture) - textures);
				const float slot = float(unit + 1) * (m_sprites[i].bClampAddress ? -1.f : 1.f);
				auto* quad = &m_batchVertices[6 * (i - start)];
				writeQuad(m_sprites[i], [quad](int j) -> sprite_vertex& { return quad[j].sprite; }, sizeof(BatchVertex));
				for (int j = 0; j < 6; ++j)
					quad[j].slot = slot;
			}
			GLuint previous[maxSlots] = {};
			// Native GL samples only slots referenced by the queued vertices.
			// WebGL also validates texture completeness for inactive branches.
#ifdef __EMSCRIPTEN__
			const int boundSlots = textureSlots;
#else
			const int boundSlots = used;
#endif
			for (int unit = 1; unit < boundSlots; ++unit)
			{
				auto* texture = unit < used ? textures[unit] : first.texture;
				previous[unit] = GL::bindTexture2DN(unit, static_cast<GLuint>(reinterpret_cast<uintptr_t>(texture->GetNativeHandle())));
			}
			pRenderDevice->SetVertexDeclaration(&declaration);
			pRenderDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLELIST, 2 * (end - start), m_batchVertices.data(), sizeof(BatchVertex));
			for (int unit = 1; unit < boundSlots; ++unit) GL::bindTexture2DN(unit, previous[unit]);
			pRenderDevice->SetTexture(0, first.texture);
			pRenderDevice->SetVertexDeclaration(pEffectMgr->GetVertexDeclaration(EffectManager::S0_POS_TEX0_COLOR));
			m_batchEffect->EndPass();
			m_batchEffect->end();
			m_pEffectFile->BeginPass(0);
			continue;
		}
#endif
		// The original effect also handles unavailable batch shaders and single-texture runs.
		for (int i = start, j; i < end; i = j)
		{
			for (j = i + 1; j < end && m_sprites[j].texture == m_sprites[i].texture &&
				m_sprites[j].bClampAddress == m_sprites[i].bClampAddress; ++j) {}
			if (batch && m_bIsTextModeDevice != m_sprites[i].bClampAddress)
			{
				m_bIsTextModeDevice = m_sprites[i].bClampAddress;
				m_pEffectFile->SetBool("k_bBoolean0", !m_bIsTextModeDevice);
			}
			if (batch) m_pEffectFile->CommitChanges();
			if (i != start) pRenderDevice->SetTexture(0, m_sprites[i].texture);
			const size_t vertexCount = 6 * (j - i);
			if (m_vertices.size() < vertexCount)
				m_vertices.resize(vertexCount);
			for (int k = i; k < j; ++k)
			{
				auto* quad = &m_vertices[6 * (k - i)];
				writeQuad(m_sprites[k], [quad](int n) -> sprite_vertex& { return quad[n]; }, sizeof(sprite_vertex));
			}
			DrawTriangles(m_vertices.data(), 2 * (j - i));
		}
	}

	pRenderDevice->SetSamplerState(0, ESamplerStateType::MINFILTER, savedMinFilter);
	pRenderDevice->SetSamplerState(0, ESamplerStateType::MAGFILTER, savedMagFilter);
	pRenderDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, savedAddressU);
	pRenderDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, savedAddressV);
	/*if (!(m_flags & D3DXSPRITE_DO_NOT_ADDREF_TEXTURE))
	for (int i = 0; i < m_sprite_count; i++)
		IDirect3DTexture9_Release(m_sprites[i].texture);*/
	m_sprite_count = 0;
	/* Flush may be called more than once, so we don't reset ready here */
}

void ParaEngine::CSpriteRendererOpenGL::DrawTriangles(const sprite_vertex* pVertices, int nTriangleCount)
{
	CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLELIST, nTriangleCount, pVertices, sizeof(sprite_vertex));
}

void ParaEngine::CSpriteRendererOpenGL::SetTextMode(bool bIsTextMode /*= true*/)
{
	if (m_pEffectFile)
	{
		m_bIsTextMode = bIsTextMode;
	}
}

void ParaEngine::CSpriteRendererOpenGL::PrepareDraw()
{
	if (!m_pEffectFile)
		return;

	if (m_batchEffect && m_sprite_count) Flush(); // Non-quad commands consume the previous quad run.
	if (m_bIsTextMode != m_bIsTextModeDevice)
	{
		Flush();
		if (m_pEffectFile)
			m_pEffectFile->SetBool("k_bBoolean0", !m_bIsTextMode);
		m_bIsTextModeDevice = m_bIsTextMode;
	}

	if (m_pEffectFile)
		m_pEffectFile->CommitChanges();
}


#endif
