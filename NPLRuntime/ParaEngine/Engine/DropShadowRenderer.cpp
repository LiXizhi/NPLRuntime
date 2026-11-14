//-----------------------------------------------------------------------------
// Class:	Simple stencil-based cylinder shadow dropper
// Authors:	Clayman, LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2012.12.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DropShadowRenderer.h"
#include "terrain/GlobalTerrain.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockCommon.h"
#include "effect_file.h"
#include "EffectManager.h"
#include "SceneObject.h"

namespace ParaEngine
{
	using namespace ParaTerrain;

	VertexPositionIndex::VertexPositionIndex()
		: Position(0, 0, 0), InstanceIndex(0), v(0)
	{
	}

	void DropShadowRenderer::Render()
	{
		if (m_instanceCount == 0 || m_pGeometryBuffer == nullptr || m_pIndexBuffer == nullptr)
			return;

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_SIMPLE_DROPSHADOW);
		CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();
		if (pEffect != 0)
		{
			if (pEffect->begin(true))
			{
				if (pEffect->BeginPass(0))
				{
					auto pDevice = CGlobals::GetRenderDevice();
					pDevice->SetStreamSource(0, m_pGeometryBuffer, 0, sizeof(VertexPositionIndex));
					pDevice->SetIndices(m_pIndexBuffer);

					// step 1:
					pDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
					pDevice->SetRenderState(ERenderState::STENCILENABLE, TRUE);
					pDevice->SetRenderState(ERenderState::STENCILFUNC, RSV_CMP_ALWAYS);
					pDevice->SetRenderState(ERenderState::STENCILZFAIL, RSV_STENCILOP_KEEP);
					pDevice->SetRenderState(ERenderState::STENCILFAIL, RSV_STENCILOP_KEEP);
					pDevice->SetRenderState(ERenderState::STENCILPASS, RSV_STENCILOP_INCR);
					pDevice->SetRenderState(ERenderState::STENCILREF, 0x01);
					pDevice->SetRenderState(ERenderState::STENCILMASK, 0xffffffff);
					pDevice->SetRenderState(ERenderState::STENCILWRITEMASK, 0xffffffff);
					pDevice->SetRenderState(ERenderState::COLORWRITEENABLE, 0x00000000);
					DrawAllBatch(pDevice, pEffect);

					// step 2:
					pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CW);
					pDevice->SetRenderState(ERenderState::STENCILPASS, RSV_STENCILOP_DECR);
					DrawAllBatch(pDevice, pEffect);

					// step 3:
					pDevice->SetRenderState(ERenderState::COLORWRITEENABLE, 0xf);
					pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
					pDevice->SetRenderState(ERenderState::STENCILFUNC, RSV_CMP_LESSEQUAL);
					pDevice->SetRenderState(ERenderState::STENCILPASS, RSV_STENCILOP_KEEP);
					DrawAllBatch(pDevice, pEffect);

					pDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
					pDevice->SetRenderState(ERenderState::STENCILENABLE, FALSE);

					pEffect->EndPass();
				}
				pEffect->end();
			}
		}
	}

	DropShadowRenderer::DropShadowRenderer()
		: m_pGeometryBuffer(nullptr), m_pIndexBuffer(nullptr),
		m_instanceVertexCount(0), m_instanceIndexCount(0),
		m_totalVertexCount(0), m_totalIndexCount(0),
		m_instanceCount(0)
	{
	}

	DropShadowRenderer::~DropShadowRenderer()
	{
		Cleanup();
	}

	void DropShadowRenderer::DrawAllBatch(IRenderDevice* pDevice, CEffectFile* pEffect)
	{
		uint32_t batchCount = m_instanceCount / g_maxInstancePerBatch + 1;
		for (uint32_t i = 0; i < batchCount; i++)
		{
			int currentInstancCount = (i < (batchCount - 1)) ? (int)g_maxInstancePerBatch : (int)(m_instanceCount - i * g_maxInstancePerBatch);

			if (currentInstancCount > 0)
			{
				uint32_t offset = i * g_maxInstancePerBatch * 4 * 3;
				pEffect->SetRawValue("instanceData", &m_constantBuffer[offset], 0, currentInstancCount * 3 * 4 * sizeof(float));
				pEffect->CommitChanges();
				pDevice->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST, 0, 0, currentInstancCount * m_instanceVertexCount, 0, currentInstancCount * m_instanceIndexCount / 3);
			}
		}
	}

	// called at the beginning of each frame
	void DropShadowRenderer::BeginFrame()
	{
		if (m_pGeometryBuffer == nullptr || m_pIndexBuffer == nullptr)
			InitDeviceObjects();

		if (m_constantBuffer.size() < 1)
			m_constantBuffer.resize(g_maxInstanceCount * 4 * 3, 0.0f);

		m_instanceCount = 0;
	}

	void DropShadowRenderer::AddInstance(Vector3& position, Vector3& renderOffset, float size)
	{
		if (m_instanceCount >= g_maxInstanceCount)
			return;

		CGlobalTerrain* terrain = CGlobals::GetGlobalTerrain();
		float terrainElevation = terrain->GetElevation(position.x, position.z);

		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();

		if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			uint16 block_x, block_y, block_z;
			const float block_size = BlockConfig::g_blockSize;
			const float quat_block_size = block_size * 0.25f;
			BlockCommon::ConvertToBlockIndex(position.x, position.y + quat_block_size, position.z, block_x, block_y, block_z);

			if (block_y >= 1)
			{
				for (uint16 y = block_y - 1; y > 0 && y > ((int)block_y - 9); y--)
				{
					uint16_t templateId = pBlockWorldClient->GetBlockTemplateIdByIdx(block_x, y, block_z);

					if (templateId > 0)
					{
						BlockTemplate* temp = pBlockWorldClient->GetBlockTemplate(templateId);
						if (temp && temp->IsMatchAttribute(BlockTemplate::batt_solid))
						{
							float block_height = BlockCommon::ConvertToRealPosition(0, y, 0, 4).y;
							if (terrainElevation > position.y)
								terrainElevation = block_height; // use block height if under terrain
							else if (block_height > terrainElevation)
								terrainElevation = block_height; // otherwise, use the higher of the two.
						}
					}
				}
			}
		}

		float deltaHeight = position.y - terrainElevation;
		if (deltaHeight > -0.1f && deltaHeight < 10.f)
		{
			deltaHeight = (1 - 0.08f * deltaHeight) * size * 0.2f;
			if (deltaHeight < 0.1f)
				deltaHeight = 0.1f;

			// build world matrix
			int idx = m_instanceCount * 12;
			memset(&m_constantBuffer[idx], 0, 12 * sizeof(float));
			m_constantBuffer[idx] = deltaHeight;      // m11
			m_constantBuffer[idx + 5] = 1.0f;         // m22
			m_constantBuffer[idx + 10] = deltaHeight; // m33
			m_constantBuffer[idx + 3] = renderOffset.x; // m14
			m_constantBuffer[idx + 7] = terrainElevation - CGlobals::GetScene()->GetRenderOrigin().y; // m24
			m_constantBuffer[idx + 11] = renderOffset.z; // m34
			m_instanceCount++;
		}
	}

	void DropShadowRenderer::InitDeviceObjects()
	{
		CreateBuffer();
	}

	void DropShadowRenderer::InvalidateDeviceObjects()
	{
		DeleteDeviceObjects();
	}

	void DropShadowRenderer::DeleteDeviceObjects()
	{
		ReleaseRHIResource(m_pGeometryBuffer);
		ReleaseRHIResource(m_pIndexBuffer);
	}

	void DropShadowRenderer::Cleanup()
	{
		DeleteDeviceObjects();
		m_constantBuffer.clear();
	}

	void DropShadowRenderer::CreateBuffer()
	{
		DeleteDeviceObjects();

		auto* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return;

		int tesselationFactor = 16;

		m_instanceVertexCount = tesselationFactor * 2 + 2;
		std::vector<Vector3> vertices(m_instanceVertexCount);

		m_instanceIndexCount = tesselationFactor * 12;
		std::vector<uint16_t> indices(m_instanceIndexCount);

		// create data for a single geometry
		CreateCylinder(1.0f, 1.0f, tesselationFactor, vertices.data(), indices.data());

		// create vertex buffer for a batch
		std::vector<VertexPositionIndex> vbData(m_instanceVertexCount * g_maxInstancePerBatch);
		uint32_t idx = 0;
		for (uint32_t i = 0; i < g_maxInstancePerBatch; i++)
		{
			for (uint16_t j = 0; j < m_instanceVertexCount; j++)
			{
				vbData[idx].Position = vertices[j];
				vbData[idx].InstanceIndex = (float)i;
				vbData[idx].v = 0.0f;
				idx++;
			}
		}
		uint32_t vbSize = (uint32_t)(vbData.size() * sizeof(VertexPositionIndex));
		m_pGeometryBuffer = pDevice->CreateBuffer(vbSize, false, ERHIBufferUsage::Static, vbData.data());

		// create index buffer
		std::vector<uint16_t> ibData(m_instanceIndexCount * g_maxInstancePerBatch);
		idx = 0;
		for (uint32_t i = 0; i < g_maxInstancePerBatch; i++)
		{
			for (uint16_t j = 0; j < m_instanceIndexCount; j++)
			{
				ibData[idx] = (uint16_t)(indices[j] + i * m_instanceVertexCount);
				idx++;
			}
		}
		uint32_t ibSize = (uint32_t)(ibData.size() * sizeof(uint16_t));
		m_pIndexBuffer = pDevice->CreateBuffer(ibSize, true, ERHIBufferUsage::Static, ibData.data());
	}

	void DropShadowRenderer::CreateCylinder(float height, float radius, int tesselationFactor, Vector3* pVertices, uint16_t* pIndices)
	{
		if (tesselationFactor < 4)
			tesselationFactor = 4;

		// fill vertex
		float pi = atan(1.0f) * 4.0f;
		float radianSpan = pi / tesselationFactor * 2;
		float topHeight = height * 0.35f;
		float bottomHeight = -height * 0.7f;
		for (int i = 0; i < tesselationFactor; i++)
		{
			float radian = i * radianSpan;
			float x = radius * cos(radian);
			float z = radius * sin(radian);

			// vertex on top face
			Vector3& vec0 = pVertices[i];
			vec0.x = x;
			vec0.y = topHeight;
			vec0.z = z;

			// vertex on bottom face
			Vector3& vec1 = pVertices[i + tesselationFactor];
			vec1.x = x;
			vec1.y = bottomHeight;
			vec1.z = z;
		}
		uint16_t topCenterVertex = m_instanceVertexCount - 2;
		uint16_t bottomCenterVertex = m_instanceVertexCount - 1;

		Vector3& topCenterPos = pVertices[topCenterVertex];
		topCenterPos.x = 0;
		topCenterPos.y = topHeight;
		topCenterPos.z = 0;

		Vector3& bottomCenterPos = pVertices[bottomCenterVertex];
		bottomCenterPos.x = 0;
		bottomCenterPos.y = bottomHeight;
		bottomCenterPos.z = 0;

		// fill index
		uint16_t idx = 0;
		for (int i = 0; i < tesselationFactor; i++)
		{
			if (i < tesselationFactor - 1)
			{
				pIndices[idx++] = i;
				pIndices[idx++] = topCenterVertex;
				pIndices[idx++] = i + 1;

				pIndices[idx++] = i;
				pIndices[idx++] = i + 1;
				pIndices[idx++] = i + tesselationFactor;

				pIndices[idx++] = i + tesselationFactor;
				pIndices[idx++] = i + 1;
				pIndices[idx++] = i + tesselationFactor + 1;

				pIndices[idx++] = i + tesselationFactor;
				pIndices[idx++] = bottomCenterVertex;
				pIndices[idx++] = i + tesselationFactor + 1;
			}
			else
			{
				pIndices[idx++] = i;
				pIndices[idx++] = topCenterVertex;
				pIndices[idx++] = 0;

				pIndices[idx++] = i;
				pIndices[idx++] = 0;
				pIndices[idx++] = i + tesselationFactor;

				pIndices[idx++] = i + tesselationFactor;
				pIndices[idx++] = 0;
				pIndices[idx++] = tesselationFactor;

				pIndices[idx++] = i + tesselationFactor;
				pIndices[idx++] = bottomCenterVertex;
				pIndices[idx++] = tesselationFactor;
			}
		}
	}
}
