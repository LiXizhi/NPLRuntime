//-----------------------------------------------------------------------------
// Class:	CadModel
// Authors:	Clayman@paraengine.com
// Date:	2007.11
// Revised: 2007.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CadModel.h"
#include "ContentLoaders.h"
#include "AsyncLoader.h"
#include "CadContentLoader.h"

namespace ParaEngine
{
	const VertexElement VertexPositionNormal::g_VertexDesc[3] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		D3DDECL_END()
	};

	VertexPositionNormal::VertexPositionNormal() : Position(0, 0, 0), Normal(0, 1, 0) {}

	VertexPositionNormal::VertexPositionNormal(const Vector3& pos, const Vector3& normal)
		: Position(pos), Normal(normal) {}

	VertexPositionNormal::VertexPositionNormal(float pX, float pY, float pZ, float nX, float nY, float nZ)
		: Position(pX, pY, pZ), Normal(nX, nY, nZ) {}

	const VertexElement VertexPosition::g_VertexDesc[2] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};

	VertexPosition::VertexPosition() : Position(0, 0, 0) {}

	VertexPosition::VertexPosition(const Vector3& pos)
		: Position(pos) {}

	VertexPosition::VertexPosition(float pX, float pY, float pZ)
		: Position(pX, pY, pZ) {}

	/////////////////////////////////////////////////////////
	/// CadModelPart
	/////////////////////////////////////////////////////////
	CadModelPart::CadModelPart(CadModel* pParent, IRHIBuffer* vb, IRHIBuffer* ib,
		VertexDeclarationPtr layout, bool sharedBuffer, EPrimitiveType primitiveType,
		uint16_t vertexStride, uint32_t vertexOfs, uint32_t indexOfs, uint32_t primitiveCount, uint32_t vertexCount)
		: m_name(nullptr)
		, m_pParent(pParent)
		, m_useSharedBuffer(sharedBuffer)
		, m_paramBlockIndex(0)
		, m_vertexStride(vertexStride)
		, m_vertexOffset(vertexOfs)
		, m_indexOffset(indexOfs)
		, m_primitiveCount(primitiveCount)
		, m_vertexCount(vertexCount)
		, m_nTechniqueHandle(-1)
		, m_effectPassId(0)
		, m_diffuseColor(1.0f, 141.0f / 255.0f, 25.0f / 255.0f, 1.0f)
		, m_primitiveType(primitiveType)
		, m_pVertexLayout(layout)
		, m_pVertexBuffer(vb)
		, m_pIndexBuffer(ib)
	{
	}

	CadModelPart::~CadModelPart()
	{
		if (!m_useSharedBuffer)
		{
			ReleaseRHIResource(m_pVertexBuffer);
			ReleaseRHIResource(m_pIndexBuffer);
			SAFE_RELEASE(m_pVertexLayout);
		}
	}

	void CadModelPart::SubmitGeometry(IRenderDevice* pDevice)
	{
		if (!pDevice || !m_pVertexBuffer || !m_pIndexBuffer)
			return;
		pDevice->SetVertexDeclaration(m_pVertexLayout);
		pDevice->SetStreamSource(0, m_pVertexBuffer, m_vertexOffset * m_vertexStride, m_vertexStride);
		pDevice->SetIndices(m_pIndexBuffer);
		pDevice->DrawIndexedPrimitive(m_primitiveType, 0, 0, m_vertexCount, m_indexOffset, m_primitiveCount);
	}

	//////////////////////////////////////////////////////////////////////////
	/// CadModel
	//////////////////////////////////////////////////////////////////////////
	const std::string CadModel::g_gridModelName("ReferenceGrid.iges");

	CParameterBlock* CadModel::GetParamBlockByIndex_(int16_t index)
	{
		if (m_paramBlocks.empty())
			return nullptr;
		if (index < 0 || index >= (int)(m_paramBlocks.size()))
		{
			return &m_paramBlocks[0];
		}
		else
		{
			return &m_paramBlocks[index];
		}
	}

	HRESULT CadModel::InitDeviceObjects()
	{
		if (m_bIsInitialized)
			return S_OK;

		m_bIsInitialized = true;
		CreateFromFile_Async(NULL, NULL, NULL);
		return S_OK;
	}

	HRESULT CadModel::RestoreDeviceObjects()
	{
		return S_OK;
	}

	HRESULT CadModel::InvalidateDeviceObjects()
	{
		return S_OK;
	}

	HRESULT CadModel::DeleteDeviceObjects()
	{
		ReleaseRHIResource(m_pSharedVertexBuffer);
		ReleaseRHIResource(m_pSharedIndexBuffer);
		SAFE_RELEASE(m_pVertexLayout);
		return S_OK;
	}

	void CadModel::Refresh(const char* fileName/* =NULL */, bool lazyLoad/* =false */)
	{
		m_bIsValid = true;
		if (fileName != NULL && fileName[0] != '\0')
		{
			SetLocalFileName(fileName);
		}

		UnloadAsset();
		Cleanup();
		Init();
		if (!lazyLoad)
			LoadAsset();
	}

	HRESULT CadModel::CreateFromFile_Async(void* pContext, IRenderDevice* pDev, const char* sFileName)
	{
		asset_ptr<CadModel> asset(this);
		if (pContext == 0)
			pContext = &(CAsyncLoader::GetSingleton());
		CAsyncLoader* pAsyncLoader = (CAsyncLoader*)pContext;
		if (pAsyncLoader)
		{
			CadModelLoader* pLoader = new CadModelLoader(asset, sFileName);
			CadModelProcessor* pProcessor = new CadModelProcessor(asset);
			pProcessor->m_pDevice = pDev;
			pAsyncLoader->AddWorkItem(pLoader, pProcessor, NULL, NULL);
		}
		return S_OK;
	}

	void CadModel::Init(const char* fileName)
	{
		if (fileName)
			SetLocalFileName(fileName);
	}

	void CadModel::Cleanup()
	{
		for (CadModelPart* part : m_modelParts)
		{
			delete part;
		}
		m_modelParts.clear();
		DeleteDeviceObjects();
	}

	bool CadModel::IsLoaded()
	{
		return m_modelParts.size() > 0;
	}

	const std::string& CadModel::GetFileName()
	{
		static std::string empty;
		return empty;
	}

	CParameterBlock* CadModel::GetParamBlock(bool bCreateIfNotExist /* = false */)
	{
		return NULL;
	}

	CadModelPart* CadModel::CreateCadModelPart(IRHIBuffer* sharedVB, IRHIBuffer* sharedIB, VertexDeclarationPtr sharedLayout,
		EPrimitiveType primitiveType, uint16_t vertexStride, uint32_t vertexOfs, uint32_t indexOfs, uint32_t primitiveCount, uint32_t vertexCount)
	{
		bool sharedBuffer = (sharedVB != nullptr);
		CadModelPart* part = new CadModelPart(this, sharedVB, sharedIB, sharedLayout, sharedBuffer, primitiveType, vertexStride, vertexOfs, indexOfs, primitiveCount, vertexCount);
		m_modelParts.push_back(part);
		return part;
	}

	bool CadModel::IsIgesFile(const char* fileName)
	{
		if (fileName != NULL)
		{
			int len = (int)strlen(fileName);
			if ((len >= 4 && fileName[len - 4] == 'i' && fileName[len - 3] == 'g' && fileName[len - 2] == 'e' && fileName[len - 1] == 's') ||
				(len >= 3 && fileName[len - 3] == 'i' && fileName[len - 2] == 'g' && fileName[len - 1] == 's'))
			{
				return true;
			}
		}
		return false;
	}

	bool CadModel::IsStepFile(const char* fileName)
	{
		if (fileName != NULL)
		{
			int len = (int)strlen(fileName);
			if ((len >= 4 && fileName[len - 4] == 's' && fileName[len - 3] == 't' && fileName[len - 2] == 'e' && fileName[len - 1] == 'p') ||
				(len >= 3 && fileName[len - 3] == 's' && fileName[len - 2] == 't' && fileName[len - 1] == 'p'))
			{
				return true;
			}
		}
		return false;
	}
}
