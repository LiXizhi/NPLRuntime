//-----------------------------------------------------------------------------
// Class:	CadContentLoader
// Authors:	Clayman@paraengine.com
// Date:	2007.11
// Revised: 2007.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <string>
#include "ICadModelImporter.h"
#include "CadContentLoader.h"
#include "CadImporterFactory.h"

namespace ParaEngine
{
	//////////////////////////////////////////////////////////////////////////
	// CadModelLoader
	//////////////////////////////////////////////////////////////////////////
	CadModelLoader::CadModelLoader(asset_ptr<CadModel>& pAsset, const char* fileName)
	{
		m_asset = pAsset;
		if (fileName)
			m_fileName = fileName;
	}

	CadModelLoader::~CadModelLoader()
	{
		CleanUp();
	}

	const char* CadModelLoader::GetFileName()
	{
		if (!m_fileName.empty())
		{
			return m_fileName.c_str();
		}
		else if (m_asset.get() != 0)
		{
			return m_asset->GetLocalFileName().c_str();
		}
		return NULL;
	}

	HRESULT CadModelLoader::Decompress(void** ppData, int* pcBytes)
	{
		return S_OK;
	}

	HRESULT CadModelLoader::CleanUp()
	{
		return S_OK;
	}

	HRESULT CadModelLoader::Destroy()
	{
		CleanUp();
		return S_OK;
	}

	HRESULT CadModelLoader::Load()
	{
		if (m_asset.get() == 0)
			return E_FAIL;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CadModelProcessor
	//////////////////////////////////////////////////////////////////////////
	CadModelProcessor::CadModelProcessor(asset_ptr<CadModel>& pAsset)
		: m_pDevice(NULL), m_pData(NULL), m_bytes(0), m_pAssetFileEntry(NULL), m_modelData(NULL)
	{
		m_asset = pAsset;
	}

	CadModelProcessor::~CadModelProcessor()
	{
		CleanUp();
	}

	HRESULT CadModelProcessor::LockDeviceObject()
	{
		if (m_asset.get() != 0 && m_asset->Lock())
		{
			return S_OK;
		}
		return E_FAIL;
	}

	IRenderDevice* CadModelProcessor::GetRenderDevice()
	{
		return (m_pDevice != 0) ? m_pDevice : CGlobals::GetRenderDevice();
	}

	HRESULT CadModelProcessor::UnLockDeviceObject()
	{
		if (m_asset.get() != 0)
		{
			std::string fileName = m_asset->GetLocalFileName();
			if (fileName.compare(CadModel::g_gridModelName) == 0)
			{
				CreateGridData();
				return S_OK;
			}

			auto pDevice = GetRenderDevice();
			if (pDevice == 0 || m_modelData == nullptr)
			{
				m_asset->UnLock();
				return E_FAIL;
			}

			int32_t vertexCount = 0;
			int32_t indexCount = 0;
			for (size_t i = 0; i < m_modelData->SubPartInfo.size(); i++)
			{
				vertexCount += m_modelData->SubPartInfo[i].vertexCount;
				indexCount += m_modelData->SubPartInfo[i].primitiveCount * 3;
			}

			if (vertexCount > 0 && indexCount > 0)
			{
				uint32_t vbSize = (uint32_t)(vertexCount * sizeof(VertexPositionNormal));
				m_asset->m_pSharedVertexBuffer = pDevice->CreateBuffer(vbSize, false, ERHIBufferUsage::Static, m_modelData->VertexData.data());

				// Convert per-subpart 0-based int32 indices to uint16 indices for universal RHI compatibility
				std::vector<uint16_t> indices16((size_t)indexCount);
				for (int32_t idx = 0; idx < indexCount; ++idx)
				{
					indices16[(size_t)idx] = (uint16_t)(m_modelData->IndexData[(size_t)idx] & 0xFFFF);
				}
				uint32_t ibSize = (uint32_t)(indices16.size() * sizeof(uint16_t));
				m_asset->m_pSharedIndexBuffer = pDevice->CreateBuffer(ibSize, true, ERHIBufferUsage::Static, indices16.data());

				VertexDeclarationPtr pVertexLayout = nullptr;
				pDevice->CreateVertexDeclaration(const_cast<VertexElement*>(VertexPositionNormal::g_VertexDesc), &pVertexLayout);
				m_asset->m_pVertexLayout = pVertexLayout;

				for (size_t i = 0; i < m_modelData->SubPartInfo.size(); i++)
				{
					SubPartInfo& info = m_modelData->SubPartInfo[i];
					m_asset->CreateCadModelPart(
						m_asset->m_pSharedVertexBuffer,
						m_asset->m_pSharedIndexBuffer,
						pVertexLayout,
						EPrimitiveType::TRIANGLELIST,
						sizeof(VertexPositionNormal),
						info.vertexOffset,
						info.indexOffset,
						info.primitiveCount,
						info.vertexCount);
				}
			}

			m_asset->m_vMax.x = m_modelData->AABBMax.x;
			m_asset->m_vMax.y = m_modelData->AABBMax.y;
			m_asset->m_vMax.z = m_modelData->AABBMax.z;

			m_asset->m_vMin.x = m_modelData->AABBMin.x;
			m_asset->m_vMin.y = m_modelData->AABBMin.y;
			m_asset->m_vMin.z = m_modelData->AABBMin.z;

			m_asset->m_nTechniqueHandle = TECH_SIMPLE_CAD_MODEL;
		}

		m_asset->UnLock();
		m_asset->RestoreDeviceObjects();
		return S_OK;
	}

	HRESULT CadModelProcessor::CleanUp()
	{
		if (m_modelData != NULL)
		{
			ICadModelImporter* pImporter = CadImporterFactory::GetImporter();
			if (pImporter)
				pImporter->ReleaseModelData(m_modelData);
			m_modelData = NULL;
		}
		return S_OK;
	}

	class CadModelCallBackData
	{
	public:
		CadModelCallBackData(CadModel* pAsset_) : pAsset(pAsset_) {}

		CadModel* pAsset;

		void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
		{
			if (nResult == 0)
			{
				pAsset->SetState(AssetEntity::ASSET_STATE_NORMAL);
				pAsset->m_bIsInitialized = false;
				pAsset->InitDeviceObjects();
			}
		}
	};

	HRESULT CadModelProcessor::Destroy()
	{
		if (m_pAssetFileEntry != 0)
		{
			m_asset->SetState(AssetEntity::ASSET_STATE_SYNC_START);
			return m_pAssetFileEntry->SyncFile_Async(CadModelCallBackData(m_asset.get()));
		}
		CleanUp();
		return S_OK;
	}

	HRESULT CadModelProcessor::Process(void* pData, int bytes)
	{
		m_pData = (char*)pData;
		m_bytes = bytes;
		return S_OK;
	}

	HRESULT CadModelProcessor::CopyToResource()
	{
		if (m_asset.get() == 0)
			return E_FAIL;

		std::string fileName = m_asset->GetLocalFileName();
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(fileName);
		if (pEntry)
		{
			if (pEntry->DoesFileExist())
			{
				fileName = pEntry->GetLocalFileName();
			}
			else
			{
				m_pAssetFileEntry = pEntry;
				return E_PENDING;
			}
		}

		if (fileName.compare(CadModel::g_gridModelName) == 0)
			return S_OK;

		ICadModelImporter* pImporter = CadImporterFactory::GetImporter();
		if (pImporter == NULL)
			return E_FAIL;

		if (pImporter->LoadFromFile(fileName.c_str(), 30, &m_modelData))
			return S_OK;

		return E_FAIL;
	}

	void CadModelProcessor::SetResourceError()
	{
		if (m_asset.get() != 0)
		{
			m_asset->m_bIsValid = false;
		}
	}

	HRESULT CadModelProcessor::CreateGridData()
	{
		if (m_asset.get() != 0)
		{
			uint32_t gridCount = 24;
			float gridSpace = 0.5f;

			auto* pDevice = GetRenderDevice();
			if (pDevice == 0)
			{
				m_asset->UnLock();
				return E_FAIL;
			}

			int32_t vertexCount = (gridCount + 1) * 4;
			int32_t indexCount = (gridCount + 1) * 4;

			std::vector<VertexPosition> verts((size_t)vertexCount);
			std::vector<uint16_t> indices((size_t)indexCount);

			float halfGridWidth = gridCount * gridSpace * 0.5f;
			int32_t idx = 0;
			for (uint32_t i = 0; i < (gridCount + 1); i++)
			{
				Vector3& pos0 = verts[(size_t)idx].Position;
				pos0.x = halfGridWidth;
				pos0.y = 0;
				pos0.z = halfGridWidth - gridSpace * i;
				indices[(size_t)idx] = (uint16_t)idx;
				idx++;

				Vector3& pos1 = verts[(size_t)idx].Position;
				pos1.x = -halfGridWidth;
				pos1.y = 0;
				pos1.z = halfGridWidth - gridSpace * i;
				indices[(size_t)idx] = (uint16_t)idx;
				idx++;

				Vector3& pos2 = verts[(size_t)idx].Position;
				pos2.x = halfGridWidth - gridSpace * i;
				pos2.y = 0;
				pos2.z = halfGridWidth;
				indices[(size_t)idx] = (uint16_t)idx;
				idx++;

				Vector3& pos3 = verts[(size_t)idx].Position;
				pos3.x = halfGridWidth - gridSpace * i;
				pos3.y = 0;
				pos3.z = -halfGridWidth;
				indices[(size_t)idx] = (uint16_t)idx;
				idx++;
			}

			uint32_t vbSize = (uint32_t)(verts.size() * sizeof(VertexPosition));
			m_asset->m_pSharedVertexBuffer = pDevice->CreateBuffer(vbSize, false, ERHIBufferUsage::Static, verts.data());

			uint32_t ibSize = (uint32_t)(indices.size() * sizeof(uint16_t));
			m_asset->m_pSharedIndexBuffer = pDevice->CreateBuffer(ibSize, true, ERHIBufferUsage::Static, indices.data());

			VertexDeclarationPtr pVertexLayout = nullptr;
			pDevice->CreateVertexDeclaration(const_cast<VertexElement*>(VertexPosition::g_VertexDesc), &pVertexLayout);
			m_asset->m_pVertexLayout = pVertexLayout;

			CadModelPart* pModelPart = m_asset->CreateCadModelPart(
				m_asset->m_pSharedVertexBuffer,
				m_asset->m_pSharedIndexBuffer,
				pVertexLayout,
				EPrimitiveType::LINELIST,
				sizeof(VertexPosition),
				0,
				0,
				(gridCount + 1) * 2,
				(uint32_t)vertexCount);

			pModelPart->m_diffuseColor = LinearColor(0.5f, 0.5f, 0.5f, 1.0f);
			pModelPart->m_effectPassId = 1;

			m_asset->m_vMax.x = halfGridWidth;
			m_asset->m_vMax.y = 0;
			m_asset->m_vMax.z = halfGridWidth;

			m_asset->m_vMin.x = -halfGridWidth;
			m_asset->m_vMin.y = 0;
			m_asset->m_vMin.z = -halfGridWidth;
			m_asset->m_nTechniqueHandle = TECH_SIMPLE_CAD_MODEL;
		}

		m_asset->UnLock();
		m_asset->RestoreDeviceObjects();
		return S_OK;
	}
}
