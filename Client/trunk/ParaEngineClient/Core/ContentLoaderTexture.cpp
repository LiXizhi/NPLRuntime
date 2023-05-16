//-----------------------------------------------------------------------------
// Class:	Content loaders
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: for async asset streaming. It uses architecture proposed by the content streaming sample in DirectX 9&10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AssetManifest.h"
#include "ContentLoaderTexture.h"

using namespace ParaEngine;

ParaEngine::CTextureLoader::CTextureLoader(asset_ptr<TextureEntity>& pAsset, const char* sFileName)
	:m_pData(NULL), m_cBytes(0), m_pAssetFileEntry(NULL), m_ppTexture(NULL), m_dwTextureFormat(D3DFMT_UNKNOWN), m_nMipLevels(D3DX_DEFAULT), m_dwColorKey(0)
{
	m_asset = pAsset;
	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CTextureLoader::~CTextureLoader()
{

}

const char* ParaEngine::CTextureLoader::GetFileName()
{
	if (!m_sFileName.empty())
	{
		return m_sFileName.c_str();
	}
	else if (m_asset.get() != 0)
	{
		return m_asset->GetLocalFileName().c_str();
	}
	return NULL;
}

const char* ParaEngine::CTextureLoader::GetKeyName()
{
	if (m_asset.get() != 0)
	{
		return m_asset->GetKey().c_str();
	}
	return NULL;
}

HRESULT ParaEngine::CTextureLoader::Decompress(void** ppData, int* pcBytes)
{
	if (!m_file.isEof() && m_file.Decompress())
	{
		*ppData = (void*)m_file.getBuffer();
		*pcBytes = m_file.getSize();
	}
	return S_OK;
}


HRESULT ParaEngine::CTextureLoader::CleanUp()
{
	m_file.close();
	return S_OK;
}

class STextureCallBackData
{
public:
	STextureCallBackData(TextureEntity * pAsset_) :pAsset(pAsset_),
		m_ppTexture(NULL), m_dwTextureFormat(D3DFMT_UNKNOWN), m_nMipLevels(D3DX_DEFAULT), m_dwColorKey(0) {}

	TextureEntity * pAsset;

	/// the format of the texture, default to D3DFMT_UNKNOWN
	D3DFORMAT m_dwTextureFormat;
	///  Mip levels, default to D3DX_DEFAULT
	UINT m_nMipLevels;
	// the color key(default to 0):  Color value to replace with transparent black, or 0 to disable the color key. This is always a 32-bit ARGB color, 
	// independent of the source image format. Alpha is significant and should usually be set to FF for opaque color keys. 
	// Thus, for opaque black, the value would be equal to 0xFF000000. COLOR_XRGB(0,0,0)
	Color m_dwColorKey;

	/// if this is NULL, the m_asset->m_pTexture is sued. 
	void** m_ppTexture;

	std::string m_sFileName;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if (nResult == 0)
		{
			pAsset->SetState(AssetEntity::ASSET_STATE_NORMAL);

			// this fixed a bug, where E_PENDING will make async loaded texture invalid, we will make them valid again when downloaded. 
			if (!pAsset->m_bIsValid)
				pAsset->m_bIsValid = true;

			//pAsset->m_bIsInitialized = false;
			//pAsset->InitDeviceObjects();

			// this fixed a bug for texture sequence
			pAsset->CreateTextureFromFile_Async(NULL, NULL, m_sFileName.c_str(), m_ppTexture, m_dwTextureFormat, m_nMipLevels, m_dwColorKey);
		}
	}
};

HRESULT ParaEngine::CTextureLoader::Destroy()
{
	if (m_pAssetFileEntry != 0)
	{
		m_asset->SetState(AssetEntity::ASSET_STATE_SYNC_START);

		STextureCallBackData pData(m_asset.get());
		pData.m_dwTextureFormat = m_dwTextureFormat;
		pData.m_nMipLevels = m_nMipLevels;
		pData.m_ppTexture = m_ppTexture;
		pData.m_dwColorKey = m_dwColorKey;
		pData.m_sFileName = m_sFileName;

		return m_pAssetFileEntry->SyncFile_Async(pData);
	}
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CTextureLoader::Load()
{
	// let us only get raw compressed data from local zip file. and let the worker thread to decompress it. 
	if (m_asset.get() != 0)
	{
		const string& sTextureFileName = m_sFileName.empty() ? m_asset->GetLocalFileName() : m_sFileName;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(sTextureFileName);
		if (pEntry)
		{
			if (pEntry->DoesFileExist())
			{
				// we already downloaded the file, so load it. 
				if (m_file.OpenFile(pEntry->GetLocalFileName().c_str(), true, NULL, true))
				{
					return S_OK;
				}
			}
			else
			{
				m_pAssetFileEntry = pEntry;
				return E_PENDING;
			}
		}
		else if (m_file.OpenFile(sTextureFileName.c_str(), true, NULL, true))
		{
			// if no entry in the manifest, we will load locally in the normal way. 
			return S_OK;
		}
		// for texture sequence, we will not set it to failed to load unless it is the last texture.
		if (m_asset->SurfaceType != TextureEntity::TextureSequence || sTextureFileName == m_asset->GetLocalFileName())
			m_asset->SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
//
// CTextureProcessor
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CTextureProcessor::CTextureProcessor(asset_ptr<TextureEntity>& pAsset)
	:m_cBytes(0), m_pData(NULL), m_pDevice(NULL), m_ppTexture(NULL), m_dwTextureFormat(D3DFMT_UNKNOWN), m_nMipLevels(D3DX_DEFAULT), m_dwColorKey(0)
{
	m_asset = pAsset;
}

ParaEngine::CTextureProcessor::~CTextureProcessor()
{
	CleanUp();
}

bool ParaEngine::CTextureProcessor::PopulateTexture()
{
	return true;
}

HRESULT ParaEngine::CTextureProcessor::LockDeviceObject()
{
	if (m_asset.get() != 0 && m_asset->Lock())
	{
		return S_OK;
	}
	return E_FAIL;
}

RenderDevicePtr ParaEngine::CTextureProcessor::GetRenderDevice()
{
	return (m_pDevice != 0) ? m_pDevice : CGlobals::GetRenderDevice();
}

HRESULT ParaEngine::CTextureProcessor::UnLockDeviceObject()
{
	if (m_asset.get() != 0 && m_pData && m_cBytes > 0)
	{
		if (GetRenderDevice() == 0)
			return E_FAIL;
		m_asset->LoadFromMemory(m_pData, m_cBytes, m_nMipLevels, m_dwTextureFormat, m_ppTexture);

		// Please note that for TextureEntity::TextureSequence, the texture may be locked multiple times. 
		m_asset->UnLock();
	}
	return S_OK;
}

HRESULT ParaEngine::CTextureProcessor::CleanUp()
{
	return S_OK;
}

HRESULT ParaEngine::CTextureProcessor::Destroy()
{
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CTextureProcessor::Process(void* pData, int cBytes)
{
	m_pData = (char*)pData;
	m_cBytes = cBytes;
	return S_OK;
}

HRESULT ParaEngine::CTextureProcessor::CopyToResource()
{
	return S_OK;
}

void ParaEngine::CTextureProcessor::SetResourceError()
{
	if (m_asset.get() != 0)
	{
		m_asset->m_bIsValid = false;
	}
}



