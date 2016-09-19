//////////////////////////////////////////////////////////////////////////////-
// Class:	block piece particle
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.1.17
//////////////////////////////////////////////////////////////////////////////-
#include "ParaEngine.h"
#include "SceneState.h"
#include "BatchedElementDraw.h"
#include "ParaWorldAsset.h"
#include "FastRandom.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockPieceParticle.h"

using namespace ParaEngine;

/////////////////////////////////////////////////
//
// CBlockPieceParticleElement
//
/////////////////////////////////////////////////
ParaEngine::CBlockPieceParticleElement::CBlockPieceParticleElement(CBlockPieceParticle* pParent) : m_pParent(pParent)
{
	if (m_pParent)
		m_pParent->addref();
}

ParaEngine::CBlockPieceParticleElement::~CBlockPieceParticleElement()
{
	SAFE_RELEASE(m_pParent);
}

int ParaEngine::CBlockPieceParticleElement::RenderParticle(SPRITEVERTEX** pVertexBuffer, SceneState* pSceneState)
{
	return m_pParent ? m_pParent->RenderParticle(pVertexBuffer, pSceneState) : 0;
}

TextureEntity* ParaEngine::CBlockPieceParticleElement::GetTexture()
{
	return m_pParent ? m_pParent->GetTexture() : NULL;
}


/////////////////////////////////////////////////
//
// BlockPieceParticle
//
/////////////////////////////////////////////////
ParaEngine::CBlockPieceParticle::CBlockPieceParticle()
	:m_particle_element(new CBlockPieceParticleElement(this))
{
	GenerateParticleUV();
}

ParaEngine::CBlockPieceParticle::~CBlockPieceParticle()
{

}

void ParaEngine::CBlockPieceParticle::GenerateParticleUV()
{
	// the triangle winding order of the quad: whether it is CCW(-1) or CW(0). It does not matter if particle is transparent.
	int order = -1;
	static FastRandom r;
	ParticleRectUV& tc = m_texUVs;
	int Cols = 4;
	int Rows = 4;
	Vector2 otc[4];
	Vector2 a, b;
	int x = r.random(0, Cols - 1);
	int y = r.random(0, Rows - 1);
	a.x = x * (1.0f / Cols);
	a.y = y * (1.0f / Rows);
	b.x = (x + 1) * (1.0f / Cols);
	b.y = (y + 1) * (1.0f / Rows);

	otc[0] = a;
	otc[2] = b;
	otc[1].x = b.x;
	otc[1].y = a.y;
	otc[3].x = a.x;
	otc[3].y = b.y;

	for (int i = 0; i < 4; i++) {
		tc.tc[(i + 4 - order) & 3] = otc[i];
	}
}


HRESULT ParaEngine::CBlockPieceParticle::Draw(SceneState * sceneState)
{
	if (IsDead())
		return S_OK;

	sceneState->GetBatchedElementDrawer()->AddParticle(m_particle_element.get());
	return S_OK;
}

int ParaEngine::CBlockPieceParticle::PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState)
{
	if (TestCollision(pCamera))
	{
		Draw(pSceneState);
	}
	return 0;
}

void ParaEngine::CBlockPieceParticle::SetTextureFilename(const std::string& texture)
{
	m_texture = CGlobals::GetAssetManager()->LoadTexture(texture, texture, TextureEntity::StaticTexture);
}

float ParaEngine::CBlockPieceParticle::CalculateOpacity()
{
	if (m_fade_out_time > 0.f && m_lifetime < m_fade_out_time)
	{
		float opacity = m_lifetime / m_fade_out_time;
		return opacity;
	}
	return 1.f;
}

TextureEntity* ParaEngine::CBlockPieceParticle::GetTexture()
{
	return m_texture.get();
}

int ParaEngine::CBlockPieceParticle::RenderParticle(SPRITEVERTEX** ppVertexBuffer, SceneState* pSceneState)
{
	// the number of particles generated (1 particle has 2 triangles with 6 vertices).
	int nNumOfParticles = 0;

	float height = GetRadius();
	float width = height;
	Vector3 vPos = GetRenderOffset();
	float vX = vPos.x;
	float vY = vPos.y + height;
	float vZ = vPos.z;
	ParticleRectUV& uv = m_texUVs;

	uint8 brightness;
	BlockWorldClient::GetInstance()->GetBlockBrightness(m_vBlockPos, &brightness, 1, -1);
	brightness *= 16;
	DWORD color = Color(brightness, brightness, brightness, (uint8)(255*CalculateOpacity()));

	SPRITEVERTEX* pVertexBuffer = *ppVertexBuffer;
	// 2 triangle list with 6 vertices
	Vector3 v;

	BillBoardViewInfo& bbInfo = pSceneState->BillBoardInfo();
	v.x = -width; v.y = -height; v.z = 0;
	v = bbInfo.TransformVertexWithoutY(v);
	CParticleElement::SetParticleVertex(pVertexBuffer[0], vX + v.x, vY + v.y, vZ + v.z, uv.tc[0].x, uv.tc[0].y, color);
	v.x = -width; v.y = height; v.z = 0;
	v = bbInfo.TransformVertexWithoutY(v);
	CParticleElement::SetParticleVertex(pVertexBuffer[1], vX + v.x, vY + v.y, vZ + v.z, uv.tc[1].x, uv.tc[1].y, color);
	v.x = width; v.y = height; v.z = 0;
	v = bbInfo.TransformVertexWithoutY(v);
	CParticleElement::SetParticleVertex(pVertexBuffer[2], vX + v.x, vY + v.y, vZ + v.z, uv.tc[2].x, uv.tc[2].y, color);
	v.x = width; v.y = -height; v.z = 0;
	v = bbInfo.TransformVertexWithoutY(v);
	CParticleElement::SetParticleVertex(pVertexBuffer[3], vX + v.x, vY + v.y, vZ + v.z, uv.tc[3].x, uv.tc[3].y, color);
		
	pVertexBuffer[4] = pVertexBuffer[0];
	pVertexBuffer[5] = pVertexBuffer[2];
	nNumOfParticles = 1;
	(*ppVertexBuffer) += 6;
	return nNumOfParticles;
}


int ParaEngine::CBlockPieceParticle::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBlockDynamicObject::InstallFields(pClass, bOverride);

	pClass->AddField("TextureFilename", FieldType_String, (void*)SetTextureFilename_s, (void*)NULL, NULL, NULL, bOverride);
	return S_OK;
}


