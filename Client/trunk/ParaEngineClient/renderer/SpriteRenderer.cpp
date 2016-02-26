//----------------------------------------------------------------------
// Class:	2D sprite renderer
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2014.8
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "SpriteRenderer.h"
#include "TextureEntity.h"

#ifdef USE_DIRECTX_RENDERER
#include "SpriteRendererDirectX.h"
#endif
#ifdef USE_OPENGL_RENDERER
#include "SpriteRendererOpenGL.h"
#endif

using namespace ParaEngine;

CSpriteRenderer* ParaEngine::CSpriteRenderer::Create()
{
#ifdef USE_DIRECTX_RENDERER
	return new CSpriteRendererDirectX();
#elif defined USE_OPENGL_RENDERER
	return new CSpriteRendererOpenGL();
#else
	return new CSpriteRenderer();
#endif
}

ParaEngine::CSpriteRenderer::CSpriteRenderer()
	: m_transform(Matrix4::IDENTITY), m_thickline_count(0), m_triangles_count(0), m_flags(0)
{
}


HRESULT ParaEngine::CSpriteRenderer::Begin(DWORD Flags)
{
	return E_FAIL;
}

void ParaEngine::CSpriteRenderer::End()
{

}

HRESULT ParaEngine::CSpriteRenderer::SetTransform(const Matrix4& mat)
{
	m_transform = mat;
	return S_OK;
}

void ParaEngine::CSpriteRenderer::GetTransform(Matrix4& outMat)
{
	outMat = m_transform;
}


const Matrix4& ParaEngine::CSpriteRenderer::GetTransform() const
{
	return m_transform;
}

HRESULT ParaEngine::CSpriteRenderer::DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c)
{
	return S_OK;
}

HRESULT ParaEngine::CSpriteRenderer::DrawRect(const RECT* pRect, Color color, float depth)
{
	return S_OK;
}

void ParaEngine::CSpriteRenderer::Flush()
{
	FlushQuads();
	FlushThickLines();
	FlushTriangles();
}

float ParaEngine::CSpriteRenderer::GetFontScaling() const
{
	return 1.f;
}


HRESULT ParaEngine::CSpriteRenderer::DrawTriangleList(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* pVertices, int nTriangleCount, Color color)
{
	if (pTexture == NULL)
		return E_FAIL;
	PrepareDraw();
	if (m_triangles.size() == 0)
	{
		m_triangles.resize(nTriangleCount+3);
	}
	else if ((int)m_triangles.size() <= (m_triangles_count + nTriangleCount))
	{
		m_triangles.resize(m_triangles_count + nTriangleCount + 3);
	}

	auto texture = pTexture->GetTexture();

	SpriteTriangle& newSprite = m_triangles[m_triangles_count];
	newSprite.texture = texture;

	int texWidth = 0, texHeight = 0;
	/* Reuse the texture desc if possible */
	if (m_triangles_count == 0 || m_triangles[m_triangles_count - 1].texture != texture) {
		texWidth = pTexture->GetWidth();
		texHeight = pTexture->GetHeight();
	}
	else {
		texWidth = m_triangles[m_triangles_count - 1].texw;
		texHeight = m_triangles[m_triangles_count - 1].texh;
	}
	newSprite.texw = texWidth;
	newSprite.texh = texHeight;

	if (pSrcRect == NULL) {
		newSprite.rect.left = 0;
		newSprite.rect.top = 0;
		newSprite.rect.right = texWidth;
		newSprite.rect.bottom = texHeight;
	}
	else
		newSprite.rect = *pSrcRect;
	newSprite.color = color;
	GetTransform(newSprite.transform);

	newSprite.vertices[0] = pVertices[0];
	newSprite.vertices[1] = pVertices[1];
	newSprite.vertices[2] = pVertices[2];
	m_triangles_count++;

	if (nTriangleCount > 1)
	{
		for (int i = 1; i < nTriangleCount; ++i)
		{
			pVertices = pVertices + 3;
			SpriteTriangle& newSprite_ = m_triangles[m_triangles_count];
			newSprite_ = newSprite;
			newSprite_.vertices[0] = pVertices[0];
			newSprite_.vertices[1] = pVertices[1];
			newSprite_.vertices[2] = pVertices[2];
			m_triangles_count++;
		}
	}
	return S_OK;
}

HRESULT ParaEngine::CSpriteRenderer::DrawLine(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3& vStart, const Vector3& vEnd, float thickness, Color color)
{
	if (pTexture == NULL)
		return E_FAIL;
	PrepareDraw();
	if (m_thickLines.size() == 0)
	{
		m_thickLines.resize(32);
	}
	else if ((int)m_thickLines.size() <= m_thickline_count)
	{
		m_thickLines.resize(m_thickLines.size() + m_thickLines.size() / 2);
	}

	auto texture = pTexture->GetTexture();
	
	SpriteThickLine& newSprite = m_thickLines[m_thickline_count];
	newSprite.texture = texture;

	int texWidth = 0, texHeight = 0;
	/* Reuse the texture desc if possible */
	if (m_thickline_count == 0 || m_thickLines[m_thickline_count - 1].texture != texture) {
		texWidth = pTexture->GetWidth();
		texHeight = pTexture->GetHeight();
	}
	else {
		texWidth = m_thickLines[m_thickline_count - 1].texw;
		texHeight = m_thickLines[m_thickline_count - 1].texh;
	}
	newSprite.texw = texWidth;
	newSprite.texh = texHeight;

	if (pSrcRect == NULL) {
		newSprite.rect.left = 0;
		newSprite.rect.top = 0;
		newSprite.rect.right = texWidth;
		newSprite.rect.bottom = texHeight;
	}
	else
		newSprite.rect = *pSrcRect;

	newSprite.vStart = vStart;
	newSprite.vEnd = vEnd;
	newSprite.color = color;
	newSprite.Thickness = thickness;
	GetTransform(newSprite.transform);
	m_thickline_count++;
	return S_OK;
}

void ParaEngine::CSpriteRenderer::FlushTriangles()
{
	if (m_triangles_count<= 0) return;
	bool bIsObjectSpace = IsUseObjectSpaceTransform();
	int verticesPerTriangle = 3;
	m_vertices.resize(verticesPerTriangle * m_triangles_count);
	int count = 0;

	for (int start = 0; start < m_triangles_count; start += count, count = 0)
	{
		int i = start;
		while (i < m_triangles_count &&
			(count == 0 || m_triangles[i].texture == m_triangles[i - 1].texture))
		{
			SpriteTriangle& triangle = m_triangles[i];
			float spritewidth = (float)triangle.rect.right - (float)triangle.rect.left;
			float spriteheight = (float)triangle.rect.bottom - (float)triangle.rect.top;
			int nBaseIndex = verticesPerTriangle * i;

			DWORD color = triangle.color;

			sprite_vertex* vertices = &m_vertices[nBaseIndex];
			// convert to Y downward coordinate
			vertices[0].pos = triangle.vertices[0].InvertYCopy(); vertices[0].col = color;
			vertices[1].pos = triangle.vertices[1].InvertYCopy(); vertices[1].col = color;
			vertices[2].pos = triangle.vertices[2].InvertYCopy();	vertices[2].col = color;
				
			ParaVec3TransformCoordArray(&vertices->pos, sizeof(sprite_vertex), &vertices->pos, sizeof(sprite_vertex), &triangle.transform, 3);

			count++;
			i++;
		}

		// do the batched rendering 
		{
			RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
			pd3dDevice->SetTexture(0, m_triangles[start].texture);
			DrawTriangles(&m_vertices[verticesPerTriangle * start], count * verticesPerTriangle / 3);
		}
	}
	m_vertices.clear();
	m_triangles_count = 0;
}


void ParaEngine::CSpriteRenderer::FlushThickLines()
{
	if (m_thickline_count <= 0) return;
	bool bIsObjectSpace = IsUseObjectSpaceTransform();
	int verticesPerLine = bIsObjectSpace ? 24 : 6;
	m_vertices.resize(verticesPerLine * m_thickline_count);
	int count = 0;
	
	for (int start = 0; start < m_thickline_count; start += count, count = 0)
	{
		int i = start;
		while (i < m_thickline_count &&
			(count == 0 || m_thickLines[i].texture == m_thickLines[i - 1].texture))
		{
			SpriteThickLine& Line = m_thickLines[i];
			float spritewidth = (float)Line.rect.right - (float)Line.rect.left;
			float spriteheight = (float)Line.rect.bottom - (float)Line.rect.top;
			int nBaseIndex = verticesPerLine * i;

			DWORD color = Line.color;

			// see CBatchedElementDraw::DrawBatchedThickLines for advanced 3d lines, here we will only draw non-scaled 2d lines. 

			Vector3 vStart = Line.vStart;
			Vector3 vEnd = Line.vEnd;
			if (bIsObjectSpace)
			{
				// 8 triangles on 3d. 
				Vector3 vLineDir = (vStart - vEnd).normalisedCopy();
				Vector3 vAnyVector = (Math::Abs(vLineDir.y) >0.5 || Math::Abs(vLineDir.z) >0.5) ? Vector3(1.f, 0, 0) : Vector3(0, 1.f, 0);
				Vector3 CameraX = vAnyVector.crossProduct(vLineDir).normalisedCopy();
				Vector3 CameraY = CameraX.crossProduct(vLineDir);
				const Vector3 WorldPointXS = CameraX * Line.Thickness * 0.5f;
				const Vector3 WorldPointYS = CameraY * Line.Thickness * 0.5f;
				const Vector3 WorldPointXE = WorldPointXS;
				const Vector3 WorldPointYE = WorldPointYS;

				sprite_vertex* ThickVertices = &m_vertices[nBaseIndex];
				// need to convert to Y downward coordinate
				// Begin point
				ThickVertices[0].pos = (vStart + WorldPointXS - WorldPointYS).InvertYCopy(); ThickVertices[0].col = color; // 0S
				ThickVertices[1].pos = (vStart + WorldPointXS + WorldPointYS).InvertYCopy(); ThickVertices[1].col = color;  // 1S
				ThickVertices[2].pos = (vStart - WorldPointXS - WorldPointYS).InvertYCopy(); ThickVertices[2].col = color;  // 2S
				ParaVec3TransformCoordArray(&ThickVertices->pos, sizeof(sprite_vertex),
					&ThickVertices->pos, sizeof(sprite_vertex), &Line.transform, 3);
				ThickVertices[3] = ThickVertices[1];
				ThickVertices[4] = ThickVertices[2];
				ThickVertices[5].pos = (vStart - WorldPointXS + WorldPointYS).InvertYCopy(); ThickVertices[5].col = color;  // 3S
				ParaVec3TransformCoord(&ThickVertices[5].pos, &ThickVertices[5].pos, &Line.transform);

				// Ending point
				ThickVertices[6].pos = (vEnd + WorldPointXE - WorldPointYE).InvertYCopy(); ThickVertices[6].col = color; // 0E
				ThickVertices[7].pos = (vEnd + WorldPointXE + WorldPointYE).InvertYCopy(); ThickVertices[7].col = color; // 1E
				ThickVertices[8].pos = (vEnd - WorldPointXE - WorldPointYE).InvertYCopy(); ThickVertices[8].col = color; // 2E
				ParaVec3TransformCoordArray(&ThickVertices[6].pos, sizeof(sprite_vertex),
					&ThickVertices[6].pos, sizeof(sprite_vertex), &Line.transform, 3);

				ThickVertices[9] = ThickVertices[7];
				ThickVertices[10] = ThickVertices[8]; 
				ThickVertices[11].pos = (vEnd - WorldPointXE + WorldPointYE).InvertYCopy(); ThickVertices[11].col = color; // 3E
				ParaVec3TransformCoord(&ThickVertices[11].pos, &ThickVertices[11].pos, &Line.transform);

				// First part of line
				ThickVertices[0 + 12] = ThickVertices[2];
				ThickVertices[1 + 12] = ThickVertices[1];
				ThickVertices[2 + 12] = ThickVertices[8];

				ThickVertices[3 + 12] = ThickVertices[1];
				ThickVertices[4 + 12] = ThickVertices[7];
				ThickVertices[5 + 12] = ThickVertices[8];

				// Second part of line
				ThickVertices[0 + 18] = ThickVertices[5];
				ThickVertices[1 + 18] = ThickVertices[0];
				ThickVertices[2 + 18] = ThickVertices[11];

				ThickVertices[3 + 18] = ThickVertices[0];
				ThickVertices[4 + 18] = ThickVertices[6];
				ThickVertices[5 + 18] = ThickVertices[11];

			}
			else
			{
				// 2 triangles on 2d screen
				Vector3 vNormal = Line.normalVector();

				sprite_vertex* ThickVertices = &m_vertices[nBaseIndex];
				ThickVertices[0].pos = vStart + vNormal; ThickVertices[0].col = color; 
				ThickVertices[1].pos = vStart - vNormal; ThickVertices[1].col = color;
				ThickVertices[2].pos = vEnd + vNormal; ThickVertices[2].col = color;
				ThickVertices[3].pos = vEnd - vNormal; ThickVertices[3].col = color;

				ParaVec3TransformCoordArray(&ThickVertices->pos, sizeof(sprite_vertex),
					&ThickVertices->pos, sizeof(sprite_vertex), &Line.transform, 4);

				ThickVertices[4] = ThickVertices[2];
				ThickVertices[5] = ThickVertices[1];

			}
			
			count++;
			i++;
		}

		// do the rendering with opengl
		{
			RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
			pd3dDevice->SetTexture(0, m_thickLines[start].texture);
			DrawTriangles(&m_vertices[verticesPerLine * start], count * verticesPerLine / 3);
		}
	}
	m_vertices.clear();
	m_thickline_count = 0;
}

void ParaEngine::CSpriteRenderer::FlushQuads()
{
}

void ParaEngine::CSpriteRenderer::DrawTriangles(const sprite_vertex* pVertices, int nTriangleCount)
{
	RenderDevice::DrawPrimitiveUP(CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UI, D3DPT_TRIANGLELIST, nTriangleCount, pVertices, sizeof(sprite_vertex));
}

bool ParaEngine::CSpriteRenderer::IsUseObjectSpaceTransform()
{
	return (m_flags & D3DXSPRITE_OBJECTSPACE) > 0;
}

void ParaEngine::SpriteThickLine::normalVector(float x1, float y1, float x2, float y2, float *nx, float *ny, float fLineWidth)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	if (dx != 0 || dy != 0)
	{
		float pw;
		if (dx == 0)
			pw = fLineWidth / Math::Abs(dy);
		else if (dy == 0)
			pw = fLineWidth / Math::Abs(dx);
		else
			pw = fLineWidth / Math::Sqrt(dx*dx + dy*dy);

		*nx = -dy * pw;
		*ny = dx * pw;
	}
	else
	{
		*nx = 0.f;
		*ny = 0.f;
	}
}

ParaEngine::Vector3 ParaEngine::SpriteThickLine::normalVector()
{
	Vector3 vNormal(0.f, 0.f, 0.f);
	normalVector(vStart.x, vStart.y, vEnd.x, vEnd.y, &(vNormal.x), &(vNormal.y), Thickness*0.5f);
	return vNormal;
}
