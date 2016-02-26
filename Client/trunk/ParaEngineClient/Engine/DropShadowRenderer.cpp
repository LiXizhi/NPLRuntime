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

	
	VertexPositionIndex::VertexPositionIndex():Position(0,0,0),InstanceIndex(0){}

	void DropShadowRenderer::Render()
	{
		//for all shadow cast 
		//1. find shadow position on terrain
		//2. for max_batch size: fill constant buffer
		//2.1 disable frame & depth buffer
		//2.2 draw front face, if depth pass,increase stencil
		//2.3 draw back face, if depth pass,decrease stencil value

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_SIMPLE_DROPSHADOW);
		CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();
		if(pEffect != 0)
		{
			if(pEffect->begin(true))
			{
				if(pEffect->BeginPass(0))
				{
					IDirect3DDevice9* pDevice = CGlobals::GetRenderDevice();
					pDevice->SetStreamSource(0,m_pGeometryBuffer,0,sizeof(VertexPositionIndex));
					pDevice->SetIndices(m_pIndexBuffer);

					//step 1:
					pDevice->SetRenderState(D3DRS_ZWRITEENABLE,false);
					pDevice->SetRenderState(D3DRS_STENCILENABLE, true);
					pDevice->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_ALWAYS);
					pDevice->SetRenderState(D3DRS_STENCILZFAIL,D3DSTENCILOP_KEEP);
					pDevice->SetRenderState(D3DRS_STENCILFAIL,D3DSTENCILOP_KEEP);
					pDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
					pDevice->SetRenderState( D3DRS_STENCILREF, 0x01 );
					pDevice->SetRenderState( D3DRS_STENCILMASK, 0xffffffff );
					pDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
					pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x00000000 );
					DrawAllBatch(pDevice);

					//step 2;
					pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
					pDevice->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_DECR);
					DrawAllBatch(pDevice);

					//step 3:
					pDevice->SetRenderState(D3DRS_COLORWRITEENABLE,0xf);
					pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
					pDevice->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_LESSEQUAL);
					pDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
					DrawAllBatch(pDevice);

					pDevice->SetRenderState(D3DRS_ZWRITEENABLE,true);
					pDevice->SetRenderState(D3DRS_STENCILENABLE,false);

					pEffect->EndPass();
				}
				pEffect->end();
			}
		}
	}

	DropShadowRenderer::DropShadowRenderer()
		:m_pGeometryBuffer(NULL),m_pIndexBuffer(NULL),
		m_instanceCount(0),m_instanceVertexCount(0),m_instanceIndexCount(0),
		m_totalVertexCount(0),m_totalIndexCount(0)
	{
	}

	DropShadowRenderer::~DropShadowRenderer()
	{
		Cleanup();
	}

	void DropShadowRenderer::DrawAllBatch(IDirect3DDevice9* pDevice)
	{
		uint32_t batchCount = m_instanceCount / g_maxInstancePerBatch + 1;
		for(uint32_t i=0;i<batchCount;i++)
		{
			int currentInstancCount = (i<(batchCount-1))?g_maxInstancePerBatch:(m_instanceCount - i*g_maxInstancePerBatch);
			
			if(currentInstancCount > 0)
			{
				uint32_t offset = i * g_maxInstancePerBatch * 4 * 3;
				pDevice->SetVertexShaderConstantF(20,&m_constantBuffer[offset] ,currentInstancCount*3);
				pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,currentInstancCount*m_instanceVertexCount,0,currentInstancCount*m_instanceIndexCount/3);
			}
		}
	}

	//called at the beginning of each frame
	void DropShadowRenderer::BeginFrame()
	{
		if(m_pGeometryBuffer == NULL || m_pIndexBuffer == NULL )
			InitDeviceObjects();

		if(m_constantBuffer.size() < 1)
			m_constantBuffer.resize(g_maxInstanceCount * 4 * 3);

		m_instanceCount = 0;
	}

	void DropShadowRenderer::AddInstance(Vector3& position,Vector3& renderOffset,float size)
	{
		if(m_instanceCount >= g_maxInstanceCount)
			return;

		CGlobalTerrain* terrain = CGlobals::GetGlobalTerrain();
		float terrainElevation =  terrain->GetElevation(position.x,position.z);

		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();

		if(pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			uint16 block_x, block_y, block_z;
			const float block_size = BlockConfig::g_blockSize;
			const float quat_block_size = block_size*0.25f;
			BlockCommon::ConvertToBlockIndex(position.x, position.y+quat_block_size, position.z, block_x, block_y, block_z);

			if(block_y>=1)
			{
				for(uint16 y = block_y - 1; y > 0 && y >((int)block_y - 9); y --)
				{
					uint16_t templateId = pBlockWorldClient->GetBlockTemplateIdByIdx(block_x, y, block_z);

					if(templateId > 0)
					{
						BlockTemplate* temp = pBlockWorldClient->GetBlockTemplate(templateId);
						if(temp && temp->IsMatchAttribute(BlockTemplate::batt_solid))
						{
							float block_height = BlockCommon::ConvertToRealPosition(0, y, 0, 4).y;
							if(terrainElevation > position.y)
								terrainElevation = block_height; // use block height if under terrain
							else if(block_height > terrainElevation)
								terrainElevation = block_height; // otherwise, use the higher of the two. 
						}
					}
				}
			}
		}

		float deltaHeight = position.y - terrainElevation;
		if(deltaHeight > -0.1f && deltaHeight < 10.f)
		{
			deltaHeight = (1 - 0.08f * deltaHeight) * size * 0.2f;
			if(deltaHeight < 0.1) deltaHeight = 0.1f;

			//build world matrix
			int idx = m_instanceCount*12;
			m_constantBuffer[idx] = deltaHeight;					//m11
			m_constantBuffer[idx+5] = 1;					//m21
			m_constantBuffer[idx+10] = deltaHeight;				//m33
			m_constantBuffer[idx+3] = renderOffset.x;		//m14
			m_constantBuffer[idx+7] = terrainElevation - CGlobals::GetScene()->GetRenderOrigin().y;	//m24
			m_constantBuffer[idx+11] = renderOffset.z;		//m34
			m_instanceCount++;
		}
	}

	void DropShadowRenderer::InitDeviceObjects()
	{
		CreateBuffer();
	}

	void DropShadowRenderer::DeleteDeviceObjects()
	{
		SAFE_RELEASE(m_pGeometryBuffer);
		SAFE_RELEASE(m_pIndexBuffer);
	}

	void DropShadowRenderer::Cleanup()
	{
		DeleteDeviceObjects();
		m_constantBuffer.clear();
	}

	void DropShadowRenderer::CreateBuffer()
	{
		DeleteDeviceObjects();

		int tesselationFactor = 16;

		m_instanceVertexCount = tesselationFactor * 2 + 2;
		Vector3* pVertices = new Vector3[m_instanceVertexCount];

		m_instanceIndexCount = tesselationFactor * 12;
		uint16_t* pIndices = new uint16_t[m_instanceIndexCount];

		//create data for a single geometry
		CreateCylinder(1,1,tesselationFactor,pVertices,pIndices);

		//create vertex buffer for a batch
		IDirect3DDevice9* pDevice = CGlobals::GetRenderDevice();
		uint32_t vbSize = m_instanceVertexCount * g_maxInstancePerBatch * sizeof(VertexPositionIndex);
		pDevice->CreateVertexBuffer(vbSize,D3DUSAGE_WRITEONLY,0,D3DPOOL_MANAGED,&m_pGeometryBuffer,NULL);

		VertexPositionIndex* pVBData;
		m_pGeometryBuffer->Lock(0,0,(void**)&pVBData,0);

		uint32_t idx = 0;
		for(int i=0;i<g_maxInstancePerBatch;i++)
		{
			for(int j=0;j<m_instanceVertexCount;j++)
			{
				pVBData[idx].Position = pVertices[j];
				pVBData[idx].InstanceIndex = (float) i;
				idx++;
			}
		}
		m_pGeometryBuffer->Unlock();
		delete[] pVertices;

		//create index buffer 
		uint32_t ibSize = m_instanceIndexCount * g_maxInstancePerBatch * sizeof(uint16_t);
		pDevice->CreateIndexBuffer(ibSize,D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_pIndexBuffer,NULL);
		
		uint16_t* pIBData;
		m_pIndexBuffer->Lock(0,0,(void**)&pIBData, 0);
		idx = 0;
		for(int i=0;i<g_maxInstancePerBatch;i++)
		{
			for(int j=0;j<m_instanceIndexCount;j++)
			{
				pIBData[idx] = pIndices[j] + i * m_instanceVertexCount;
				idx++;
			}
		}
		m_pIndexBuffer->Unlock();
		delete[] pIndices;
	}
	
	void DropShadowRenderer::CreateCylinder(float height,float radius,int tesselationFactor,Vector3* pVertices,uint16_t* pIndices)
	{
		if(tesselationFactor < 4)
			tesselationFactor = 4;

		//fill vertex
		float pi = atan(1.0f)*4.0f;
		float radianSpan = pi / tesselationFactor * 2;
		float topHeight = height * 0.35f;
		float bottomHeight = -height * 0.7f;
		for(int i=0;i<tesselationFactor; i++)
		{
			float radian = i * radianSpan;
			float x = radius * cos(radian);
			float z = radius * sin(radian);

			//vertex on top face
			Vector3& vec0 = pVertices[i];
			vec0.x = x;
			vec0.y = topHeight;
			vec0.z = z;

			//vertex on bottom face
			Vector3& vec1 = pVertices[i+tesselationFactor];
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

		//fill index
		uint16_t idx = 0;
		for(int i=0;i<tesselationFactor;i++)
		{
			if(i < tesselationFactor - 1)
			{	
				pIndices[idx++] = i;
				pIndices[idx++] = topCenterVertex;
				pIndices[idx++] = i+1;

				pIndices[idx++] = i;
				pIndices[idx++] = i+1;
				pIndices[idx++] = i + tesselationFactor;

				pIndices[idx++] = i + tesselationFactor;
				pIndices[idx++] = i+1;
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