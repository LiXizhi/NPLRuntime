//----------------------------------------------------------------------
// Class:	2D sprite renderer
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2014.8
// Desc:  code is partially based on d3dxsprite WINE implementation.
// https://github.com/wine-mirror/wine/blob/master/dlls/d3dx9_36/sprite.c
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_OPENGL_RENDERER
#include "RenderDeviceOpenGL.h"
#include "TextureEntity.h"
#include "OpenGLWrapper/GLProgram.h"
#include "OpenGLWrapper/GLTexture2D.h"
//#include "CGUIRootLayer.h"
#include "EffectManager.h"
#include "ParaWorldAsset.h"
#include "BaseObject.h"
#include "ViewportManager.h"
#include "SpriteRendererOpenGL.h"

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
		if (CGlobals::GetApp()->IsRotateScreen())
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

		if (m_pEffectFile && m_pEffectFile->begin())
		{
			m_pEffectFile->BeginPass(0);
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

	m_vertices.resize(6 * m_sprite_count);

	int count = 0;
	for (int start = 0; start < m_sprite_count; start += count, count = 0)
	{
		int i = start;
		while (i < m_sprite_count &&
			(count == 0 || m_sprites[i].texture == m_sprites[i - 1].texture))
		{
			SpriteQuad& curSprite = m_sprites[i];
			float spritewidth = (float)Math::Abs(curSprite.rect.right - curSprite.rect.left);
			float spriteheight = (float)Math::Abs(curSprite.rect.bottom - curSprite.rect.top);
			int nBaseIndex = 6 * i;
			m_vertices[nBaseIndex].pos.x = curSprite.pos.x - curSprite.center.x;
			m_vertices[nBaseIndex].pos.y = curSprite.pos.y - curSprite.center.y;
			m_vertices[nBaseIndex].pos.z = curSprite.pos.z - curSprite.center.z;
			m_vertices[nBaseIndex + 1].pos.x = spritewidth + curSprite.pos.x - curSprite.center.x;
			m_vertices[nBaseIndex + 1].pos.y = curSprite.pos.y - curSprite.center.y;
			m_vertices[nBaseIndex + 1].pos.z = curSprite.pos.z - curSprite.center.z;
			m_vertices[nBaseIndex + 2].pos.x = spritewidth + curSprite.pos.x - curSprite.center.x;
			m_vertices[nBaseIndex + 2].pos.y = spriteheight + curSprite.pos.y - curSprite.center.y;
			m_vertices[nBaseIndex + 2].pos.z = curSprite.pos.z - curSprite.center.z;
			m_vertices[nBaseIndex + 3].pos.x = curSprite.pos.x - curSprite.center.x;
			m_vertices[nBaseIndex + 3].pos.y = spriteheight + curSprite.pos.y - curSprite.center.y;
			m_vertices[nBaseIndex + 3].pos.z = curSprite.pos.z - curSprite.center.z;
			m_vertices[nBaseIndex].col = curSprite.color;
			m_vertices[nBaseIndex + 1].col = curSprite.color;
			m_vertices[nBaseIndex + 2].col = curSprite.color;
			m_vertices[nBaseIndex + 3].col = curSprite.color;
			m_vertices[nBaseIndex].tex.x = (float)curSprite.rect.left / (float)curSprite.texw;
			m_vertices[nBaseIndex].tex.y = (float)curSprite.rect.top / (float)curSprite.texh;
			m_vertices[nBaseIndex + 1].tex.x = (float)curSprite.rect.right / (float)curSprite.texw;
			m_vertices[nBaseIndex + 1].tex.y = (float)curSprite.rect.top / (float)curSprite.texh;
			m_vertices[nBaseIndex + 2].tex.x = (float)curSprite.rect.right / (float)curSprite.texw;
			m_vertices[nBaseIndex + 2].tex.y = (float)curSprite.rect.bottom / (float)curSprite.texh;
			m_vertices[nBaseIndex + 3].tex.x = (float)curSprite.rect.left / (float)curSprite.texw;
			m_vertices[nBaseIndex + 3].tex.y = (float)curSprite.rect.bottom / (float)curSprite.texh;

			ParaVec3TransformCoordArray(&m_vertices[nBaseIndex].pos, sizeof(sprite_vertex),
				&m_vertices[nBaseIndex].pos, sizeof(sprite_vertex), &curSprite.transform, 4);


			m_vertices[nBaseIndex + 4] = m_vertices[nBaseIndex];
			m_vertices[nBaseIndex + 5] = m_vertices[nBaseIndex + 2];

			count++;
			i++;
		}

		// do the rendering with opengl
		{
			pRenderDevice->SetTexture(0, m_sprites[start].texture);
			DrawTriangles(&m_vertices[6 * start], count * 2);
		}
	}
	m_vertices.clear();
	/*if (!(m_flags & D3DXSPRITE_DO_NOT_ADDREF_TEXTURE))
	for (int i = 0; i < m_sprite_count; i++)
		IDirect3DTexture9_Release(m_sprites[i].texture);*/
	m_sprite_count = 0;
	/* Flush may be called more than once, so we don't reset ready here */
}

void ParaEngine::CSpriteRendererOpenGL::DrawTriangles(const sprite_vertex* pVertices, int nTriangleCount)
{
#define USE_USER_POINTER_VBO
#ifdef USE_USER_POINTER_VBO
	CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLELIST, nTriangleCount, pVertices, sizeof(sprite_vertex));
#else
#define kQuadSize sizeof(sprite_vertex)
	// vertices
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*)&(pVertices->pos));
	// colors
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*)&(pVertices->col));
	// tex coords
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*)&(pVertices->tex));

	CGlobals::GetRenderDevice()->DrawPrimitive(EPrimitiveType::TRIANGLELIST, 0, nTriangleCount);
#endif
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
	if (m_bIsTextMode != m_bIsTextModeDevice)
	{
		Flush();
		m_pEffectFile->SetBool("k_bBoolean0", !m_bIsTextMode);
		m_bIsTextModeDevice = m_bIsTextMode;
	}

	m_pEffectFile->CommitChanges();
}


#endif