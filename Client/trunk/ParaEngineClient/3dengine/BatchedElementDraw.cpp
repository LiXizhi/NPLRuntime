//-----------------------------------------------------------------------------
// Class:	CBatchedElementDraw
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.2.28, added ThickLine 2014.8.10
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ViewportManager.h"
#include "SceneObject.h"
#include "TextureEntity.h"
#include "EffectManager.h"
#include "BatchedElementDraw.h"
using namespace ParaEngine;


std::vector<char> ParaEngine::CBatchedElementDraw::s_VertexData;

ParaEngine::CBatchedElementDraw::CBatchedElementDraw()
	:m_vOffset(0,0,0), m_nDrawMode(0), m_nLineCount(0), m_vDefaultColor(1,0,0)
{
	m_lines_vertex_array.reserve(1000);
	m_lines_index_array.reserve(1000);
}

ParaEngine::CBatchedElementDraw::~CBatchedElementDraw()
{
	ClearParticles();
}

// at least implement this. 
void ParaEngine::CBatchedElementDraw::DrawLine( const PARAVECTOR3& from,const PARAVECTOR3& to,const PARAVECTOR3& color )
{
	int16 nNextIndex = (int16)(m_lines_vertex_array.size());
	LINEVERTEX v1;
	v1.p = (Vector3&)from;
	v1.p += m_vOffset;
	v1.color = (DWORD)(LinearColor(color.x, color.y, color.z, 1.f));

	LINEVERTEX v2;
	v2.p = (Vector3&)to;
	v2.p += m_vOffset;
	v2.color = v1.color;
	m_lines_vertex_array.push_back(v1);
	m_lines_vertex_array.push_back(v2);

	m_lines_index_array.push_back(nNextIndex);
	m_lines_index_array.push_back(nNextIndex + 1);

	m_nLineCount++;
}

void ParaEngine::CBatchedElementDraw::AddThickLine(const PARAVECTOR3& from, const PARAVECTOR3& to, const PARAVECTOR4& color, float fThickness /*= 0.f*/, float DepthBias)
{
	if (fThickness == 0.f)
	{
		PARAVECTOR3 color_(color.x, color.y, color.z);
		DrawLine(from, to, color_);
	}
	else
	{
		if (m_listThickLines.size() > 1000)
			return;
		BatchedThickLines thickline;
		thickline.vStart = from;
		thickline.vStart.x += m_vOffset.x;
		thickline.vStart.y += m_vOffset.y;
		thickline.vStart.z += m_vOffset.z;
		thickline.vEnd = to;
		thickline.vEnd.x += m_vOffset.x;
		thickline.vEnd.y += m_vOffset.y;
		thickline.vEnd.z += m_vOffset.z;
		thickline.Color = (DWORD)(LinearColor(color.x, color.y, color.z, color.w));
		thickline.Thickness = fThickness;
		thickline.DepthBias = DepthBias;
		m_listThickLines.push_back(thickline);
	}
}

void ParaEngine::CBatchedElementDraw::AddThickLines(const PARAVECTOR3* points, int nPointCount, const PARAVECTOR4& color, float fThickness /*= 0.f*/, float DepthBias /*= 0.f*/)
{
	for (int i = 0; i < nPointCount - 1;++i)
	{
		AddThickLine(points[i], points[i + 1], color, fThickness, DepthBias);
	}
}

void ParaEngine::CBatchedElementDraw::DrawPredefinedLines( const PARAVECTOR3* lines, int nLineCount, const PARAVECTOR3& color )
{
	static const int pIndexBuffer[] = {
		0,1,1,2,2,3,3,0, // bottom
		4,5,5,6,6,7,7,4, // top
		0,4,1,5,2,6,3,7, // sides
	};
	DWORD color_ = (DWORD)(LinearColor(color.x, color.y, color.z, 1.f));
	int nIndexCount = 0;
	int nVertexCount = 0;
	if(nLineCount == 4)
	{
		nVertexCount = 4;
		nIndexCount = 8;
	}
	else if(nLineCount == 12)
	{
		nVertexCount = 8;
		nIndexCount = 24;
	}
	int16 nNextIndex = (int16)(m_lines_vertex_array.size());
	int i;
	for (i = 0; i < nVertexCount; ++i)
	{
		LINEVERTEX v1;
		v1.p = (Vector3&)(lines[i]);
		v1.p += m_vOffset;
		v1.color = color_;
		m_lines_vertex_array.push_back(v1);
	}
	for (i = 0; i < nIndexCount; ++i)
	{
		m_lines_index_array.push_back( (nNextIndex+pIndexBuffer[i]) );
	}
	m_nLineCount += nLineCount;
}

void ParaEngine::CBatchedElementDraw::DrawContactPoint( const PARAVECTOR3& PointOnB,const PARAVECTOR3& normalOnB,float distance,int lifeTime,const PARAVECTOR3& color )
{
	
}

void ParaEngine::CBatchedElementDraw::ReportErrorWarning( const char* warningString )
{
	// output log message
	OUTPUT_LOG("DebugDrawWarning: %s\n", warningString);
}

void ParaEngine::CBatchedElementDraw::Draw3dText( const PARAVECTOR3& location,const char* textString )
{

}

void ParaEngine::CBatchedElementDraw::SetRenderOffset( const PARAVECTOR3& vOffset )
{
	m_vOffset = (const Vector3&) vOffset;
}

void ParaEngine::CBatchedElementDraw::SetDebugDrawMode( int debugMode )
{
	m_nDrawMode = debugMode;
}

int ParaEngine::CBatchedElementDraw::GetDebugDrawMode()
{
	return m_nDrawMode;
}

void ParaEngine::CBatchedElementDraw::ClearLines()
{
	m_lines_vertex_array.clear();
	m_lines_index_array.clear();
	m_nLineCount = 0;
}

int ParaEngine::CBatchedElementDraw::GetLineCount()
{
	return m_nLineCount;
}

bool ParaEngine::CBatchedElementDraw::IsEmpty()
{
	return !(GetLineCount() > 0 || m_listThickLines.size() > 0 || m_listParticles.size()>0);
}

void ParaEngine::CBatchedElementDraw::DrawAll(bool bClear /*= true*/)
{
	DrawBatchedLines(bClear);
	DrawBatchedThickLines(bClear);
	DrawBatchedParticles(bClear);
}

IParaDebugDraw* ParaEngine::CBatchedElementDraw::GetDebugDrawInterface()
{
	return (IParaDebugDraw*)this;
}

void ParaEngine::CBatchedElementDraw::DrawBatchedLines(bool bClear)
{
	if(m_nLineCount == 0)
		return;
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice)
		return;
#ifdef USE_OPENGL_RENDERER
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	CEffectFile* pEffect = NULL;
	pEffectManager->BeginEffect(TECH_SINGLE_COLOR, &pEffect);
	if (pEffect == 0 || !pEffect->begin() || !pEffect->BeginPass(0))
	{
		if (bClear)
			ClearLines();
	}
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
#elif defined(USE_DIRECTX_RENDERER)
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	pEffectManager->BeginEffect(TECH_NONE, &(CGlobals::GetSceneState()->m_pCurrentEffect));
#endif
	// set render state
	{
		// pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetTexture(0, 0);

		pd3dDevice->SetTransform(D3DTS_WORLD, CGlobals::GetIdentityMatrix()->GetConstPointer());
		pd3dDevice->SetFVF(LINEVERTEX::FVF);

		RenderDevice::DrawIndexedPrimitiveUP(CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN,D3DPT_LINELIST, 0, 
			(int)m_lines_vertex_array.size(), m_nLineCount, &(m_lines_index_array[0]), D3DFMT_INDEX16,
			&(m_lines_vertex_array[0]), sizeof(LINEVERTEX));
	}
#ifdef USE_OPENGL_RENDERER
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
	pEffect->EndPass();
	pEffect->end();
#endif
	if(bClear)
		ClearLines();
}


void ParaEngine::CBatchedElementDraw::DrawBatchedThickLines(bool bClear /*= true*/)
{
	if (m_listThickLines.size() == 0)
		return;
#ifdef USE_OPENGL_RENDERER
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	CEffectFile* pEffect = NULL;
	pEffectManager->BeginEffect(TECH_SINGLE_COLOR, &pEffect);
	if (pEffect == 0 || !pEffect->begin() || !pEffect->BeginPass(0))
	{
		if (bClear)
			m_listThickLines.clear();
	}
#elif defined(USE_DIRECTX_RENDERER)
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	pEffectManager->BeginEffect(TECH_NONE, &(CGlobals::GetSceneState()->m_pCurrentEffect));
#endif
	// set render state
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	if (pd3dDevice)
	{
		float ViewportSizeX = (float)CGlobals::GetViewportManager()->GetActiveViewPort()->GetWidth();
		Matrix4 viewProjMatrix = CGlobals::GetViewMatrixStack().SafeGetTop() * CGlobals::GetProjectionMatrixStack().SafeGetTop();
		Matrix4 InverseViewProjMatrix;
		InverseViewProjMatrix = viewProjMatrix.inverse();
		Vector3 CameraX, CameraY, CameraEye;
		ParaVec3TransformCoord(&CameraX, &Vector3::UNIT_X, &InverseViewProjMatrix);
		ParaVec3TransformCoord(&CameraY, &Vector3::UNIT_Y, &InverseViewProjMatrix);
		ParaVec3TransformCoord(&CameraEye, &Vector3::ZERO, &InverseViewProjMatrix);
		CameraX -= CameraEye;
		CameraY -= CameraEye;
		CameraX.normalise();
		CameraY.normalise();


		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pd3dDevice->SetTexture(0, 0);

		pd3dDevice->SetTransform(D3DTS_WORLD, CGlobals::GetIdentityMatrix()->GetConstPointer());
		pd3dDevice->SetFVF(LINEVERTEX::FVF);
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		float OrthoZoomFactor = 1.f; //  tan(fAspectRatio*0.5f);

		const Matrix4& matProj = CGlobals::GetProjectionMatrixStack().SafeGetTop();
		const bool bIsPerspective = matProj._44 < 1.0f ? true : false;
		if (!bIsPerspective)
		{
			OrthoZoomFactor = 1.0f / matProj._11;
		}

		int32 LineIndex = 0;
		const int32 MaxLinesPerBatch = 512;
		while (LineIndex < (int)m_listThickLines.size())
		{
			int32 FirstLineThisBatch = LineIndex;
			float DepthBiasThisBatch = m_listThickLines[LineIndex].DepthBias;
			while (++LineIndex < (int)m_listThickLines.size())
			{
				if ((m_listThickLines[LineIndex].DepthBias != DepthBiasThisBatch)
					|| ((LineIndex - FirstLineThisBatch) >= MaxLinesPerBatch))
				{
					break;
				}
			}
			int32 NumLinesThisBatch = LineIndex - FirstLineThisBatch;

			const bool bEnableMSAA = true;
			const bool bEnableLineAA = false;

			s_VertexData.resize(8 * 3 * sizeof(LINEVERTEX) * NumLinesThisBatch);

			LINEVERTEX* ThickVertices = (LINEVERTEX*)(&s_VertexData[0]);

			for (int i = 0; i < NumLinesThisBatch; ++i)
			{
				const BatchedThickLines& Line = m_listThickLines[FirstLineThisBatch + i];
				const float Thickness = Math::Abs(Line.Thickness);

				float StartW = 1.f;
				float EndW = 1.f;
				if (Line.IsScreenSpace())
				{
					Vector4 vStart, vEnd;
					vStart = Vector4(Line.vStart.x, Line.vStart.y, Line.vStart.z, 1.f) * viewProjMatrix;
					vEnd = Vector4(Line.vEnd.x, Line.vEnd.y, Line.vEnd.z, 1.f) * viewProjMatrix;
					StartW = vStart.w;
					EndW = vEnd.w;
				}
				const float ScalingStart = Line.IsScreenSpace() ? StartW / ViewportSizeX : 1.0f;
				const float ScalingEnd = Line.IsScreenSpace() ? EndW / ViewportSizeX : 1.0f;

				OrthoZoomFactor = Line.IsScreenSpace() ? OrthoZoomFactor : 1.0f;

				const float ScreenSpaceScaling = Line.IsScreenSpace() ? 2.0f : 1.0f;

				const float StartThickness = Thickness * ScreenSpaceScaling * OrthoZoomFactor * ScalingStart;
				const float EndThickness = Thickness * ScreenSpaceScaling * OrthoZoomFactor * ScalingEnd;

				const Vector3 WorldPointXS = CameraX * StartThickness * 0.5f;
				const Vector3 WorldPointYS = CameraY * StartThickness * 0.5f;

				const Vector3 WorldPointXE = CameraX * EndThickness * 0.5f;
				const Vector3 WorldPointYE = CameraY * EndThickness * 0.5f;

				// Generate vertices for the point such that the post-transform point size is constant.

				Vector3 vStart(Line.vStart.x, Line.vStart.y, Line.vStart.z);
				Vector3 vEnd(Line.vEnd.x, Line.vEnd.y, Line.vEnd.z);
				DWORD color = Line.Color;

				// Begin point
				ThickVertices[0].p = vStart + WorldPointXS - WorldPointYS; ThickVertices[0].color = color; // 0S
				ThickVertices[1].p = vStart + WorldPointXS + WorldPointYS; ThickVertices[1].color = color;  // 1S
				ThickVertices[2].p = vStart - WorldPointXS - WorldPointYS; ThickVertices[2].color = color;  // 2S

				ThickVertices[3].p = vStart + WorldPointXS + WorldPointYS; ThickVertices[3].color = color;  // 1S
				ThickVertices[4].p = vStart - WorldPointXS - WorldPointYS; ThickVertices[4].color = color;  // 2S
				ThickVertices[5].p = vStart - WorldPointXS + WorldPointYS; ThickVertices[5].color = color;  // 3S

				// Ending point
				ThickVertices[0 + 6].p = vEnd + WorldPointXE - WorldPointYE; ThickVertices[6].color = color; // 0E
				ThickVertices[1 + 6].p = vEnd + WorldPointXE + WorldPointYE; ThickVertices[7].color = color; // 1E
				ThickVertices[2 + 6].p = vEnd - WorldPointXE - WorldPointYE; ThickVertices[8].color = color; // 2E

				ThickVertices[3 + 6].p = vEnd + WorldPointXE + WorldPointYE; ThickVertices[9].color = color; // 1E
				ThickVertices[4 + 6].p = vEnd - WorldPointXE - WorldPointYE; ThickVertices[10].color = color; // 2E
				ThickVertices[5 + 6].p = vEnd - WorldPointXE + WorldPointYE; ThickVertices[11].color = color; // 3E

				// First part of line
				ThickVertices[0 + 12].p = vStart - WorldPointXS - WorldPointYS; ThickVertices[12].color = color; // 2S
				ThickVertices[1 + 12].p = vStart + WorldPointXS + WorldPointYS; ThickVertices[13].color = color; // 1S
				ThickVertices[2 + 12].p = vEnd - WorldPointXE - WorldPointYE; ThickVertices[14].color = color; // 2E

				ThickVertices[3 + 12].p = vStart + WorldPointXS + WorldPointYS; ThickVertices[15].color = color; // 1S
				ThickVertices[4 + 12].p = vEnd + WorldPointXE + WorldPointYE; ThickVertices[16].color = color; // 1E
				ThickVertices[5 + 12].p = vEnd - WorldPointXE - WorldPointYE; ThickVertices[17].color = color; // 2E

				// Second part of line
				ThickVertices[0 + 18].p = vStart - WorldPointXS + WorldPointYS; ThickVertices[18].color = color; // 3S
				ThickVertices[1 + 18].p = vStart + WorldPointXS - WorldPointYS; ThickVertices[19].color = color; // 0S
				ThickVertices[2 + 18].p = vEnd - WorldPointXE + WorldPointYE; ThickVertices[20].color = color; // 3E

				ThickVertices[3 + 18].p = vStart + WorldPointXS - WorldPointYS; ThickVertices[21].color = color; // 0S
				ThickVertices[4 + 18].p = vEnd + WorldPointXE - WorldPointYE; ThickVertices[22].color = color; // 0E
				ThickVertices[5 + 18].p = vEnd - WorldPointXE + WorldPointYE; ThickVertices[23].color = color; // 3E

				ThickVertices += 24;
			}
			pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&DepthBiasThisBatch);
			RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLELIST,
				8 * NumLinesThisBatch, (LINEVERTEX*)(&s_VertexData[0]), sizeof(LINEVERTEX));
		}
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
	}
#ifdef USE_OPENGL_RENDERER
	pEffect->EndPass();
	pEffect->end();
#endif

	if (bClear)
		m_listThickLines.clear();
}


void ParaEngine::CBatchedElementDraw::ClearParticles()
{
	m_listParticles.clear();
	m_ParticleRenderPasses.clear();
}

void ParaEngine::CBatchedElementDraw::ClearAll()
{
	ClearParticles();
	ClearLines();
}

void ParaEngine::CBatchedElementDraw::AddParticle(CParticleElement* particle)
{
	if (particle)
	{
		int32 nIndex = (int32)m_listParticles.size();
		m_listParticles.push_back(particle);
		for (ParticleRenderPass& renderPass : m_ParticleRenderPasses)
		{
			if (renderPass.CheckParticle(particle))
			{
				renderPass.AddParticle(nIndex);
				return;
			}
		}
		ParticleRenderPass renderPass(particle);
		renderPass.AddParticle(nIndex);
		m_ParticleRenderPasses.push_back(renderPass);
	}
}

void ParaEngine::CBatchedElementDraw::DrawBatchedParticles(bool bClear /*= true*/)
{
	if (m_listParticles.size() == 0)
		return;

	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	CEffectFile* pEffect = NULL;
	pEffectManager->BeginEffect(TECH_PARTICLES, &pEffect);
	if (pEffect && pEffect->begin() && pEffect->BeginPass(0))
	{
		
		const int32 MaxParticlesPerBatch = Math::Min((int)m_listParticles.size(), 512);
		s_VertexData.resize(2 * 3 * sizeof(SPRITEVERTEX) * (MaxParticlesPerBatch+10));
		
		RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
		pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pEffectManager->SetCullingMode(false);

		// get camera billboarding rotations
		SceneState* pSceneState = CGlobals::GetSceneState();

		// render by texture
		ParticleRenderPass* pLastRenderPass = NULL;
		for (ParticleRenderPass& renderPass : m_ParticleRenderPasses)
		{
			if (!renderPass.GetTexture() || !(renderPass.GetTexture()->IsLoaded()))
				continue;

			int nParticleIndex = renderPass.GetMinIndex();
			pEffect->EnableAlphaTesting(renderPass.IsAlphaTested());
			if (!pLastRenderPass || pLastRenderPass->IsPointTexture() != renderPass.IsPointTexture())
			{
				if (renderPass.IsPointTexture())
				{
					pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
				}
				else
				{
					pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
					pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
					pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
				}
			}
			
			pLastRenderPass = &renderPass;

			while (nParticleIndex <= (int)renderPass.GetMaxIndex())
			{
				int NumParticlesThisBatch = 0;
				SPRITEVERTEX* pVertices = (SPRITEVERTEX*)(&s_VertexData[0]);

				for (; NumParticlesThisBatch < MaxParticlesPerBatch && nParticleIndex <= (int)renderPass.GetMaxIndex();)
				{
					CParticleElement* particle = m_listParticles.at(nParticleIndex);
					if (renderPass.CheckParticle(particle))
					{
						NumParticlesThisBatch += particle->RenderParticle(&pVertices, pSceneState);
					}
					++nParticleIndex;
				}
				pEffect->setTexture(0, renderPass.GetTexture());
				pEffect->CommitChanges();
				RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLELIST,
					2 * NumParticlesThisBatch, (SPRITEVERTEX*)(&s_VertexData[0]), sizeof(SPRITEVERTEX));
			}
		}

		pEffect->EndPass();
		pEffect->end();
	}

	if (bClear)
		ClearParticles();
}

void ParaEngine::CBatchedElementDraw::SortParticles()
{
}

