//-----------------------------------------------------------------------------
// Class:	Content loaders
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: for async asset streaming. It uses architecture proposed by the content streaming sample in DirectX 9&10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "GDIEngine.h"
#endif
#include "AssetManifest.h"
#include "AsyncLoader.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "RenderTarget.h"
#include "PaintEngine/Painter.h"
#include "CustomCharModelInstance.h"
#include "CustomCharRenderTarget.h"
#include "ContentLoaderCCSSkin.h"
using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
//
// CCCSSkinLoader
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CCCSSkinLoader::CCCSSkinLoader(CharModelInstance* pCharModelInstance, CharTexture&  charTexture, const char* sFileName)
	: m_pCharModelInstance(pCharModelInstance), m_nRetryCount(0), m_pAssetFileEntry(NULL)
{
	m_layers = charTexture.components;

	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CCCSSkinLoader::~CCCSSkinLoader()
{

}

const char* ParaEngine::CCCSSkinLoader::GetFileName()
{
	return m_sFileName.c_str();
}

HRESULT ParaEngine::CCCSSkinLoader::Decompress(void** ppData, int* pcBytes)
{
	return S_OK;
}

HRESULT ParaEngine::CCCSSkinLoader::CleanUp()
{
	return S_OK;
}


class SCCSSkinCallBackData
{
public:
	SCCSSkinCallBackData(CharModelInstance * pAsset_) :pAsset(pAsset_){}

	CharModelInstance * pAsset;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if (nResult == 0)
		{
			if (CharModelInstance::HasPendingBodyTexture(pAsset))
			{
				CharModelInstance::AddPendingBodyTexture(pAsset);
			}
		}
	}
};

HRESULT ParaEngine::CCCSSkinLoader::Destroy()
{
	if (m_pAssetFileEntry != 0)
	{
		// we need to download from the web server. 
		return m_pAssetFileEntry->SyncFile_Async(SCCSSkinCallBackData(m_pCharModelInstance));
	}

	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CCCSSkinLoader::Load()
{
	if (CParaFile::DoesFileExist(m_sFileName.c_str(), false))
		return S_OK;

	// check if all textures involved are loaded. 
	int nSize = (int)m_layers.size();
	for (int i = 0; i < nSize; ++i)
	{
		CharTextureComponent &component = m_layers[i];
		if (component.name.empty())
			continue;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(component.name);
		if (pEntry)
		{
			if (!pEntry->DoesFileExist())
			{
				m_pAssetFileEntry = pEntry;
				return E_PENDING;
			}
		}
	}
#ifdef USE_DIRECTX_RENDERER
	return ComposeWithGDIPlus();
#else
	return S_OK;
#endif
}

HRESULT ParaEngine::CCCSSkinLoader::ComposeWithGDIPlus()
{
	HRESULT hr = S_OK;
#ifdef USE_DIRECTX_RENDERER
	// DO the texture composition here
	CGDIEngine* pEngine = CAsyncLoader::GetSingleton().GetGDIEngine();
	if (pEngine == NULL)
		return E_FAIL;
	
	pEngine->SetRenderTarget(pEngine->CreateGetRenderTargetBySize(CCharCustomizeSysSetting::CharTexSize));

	pEngine->Begin();
	// Note: LiXizhi. this will clear the render buffer. However, if the first component is always opaque, this step is not necessary. 
	pEngine->Clear(0x00000000);

	int nSize = (int)m_layers.size();
	for (int i = 0; i < nSize; ++i)
	{
		CharTextureComponent &component = m_layers[i];
		const CharRegionCoords &coords = CCharCustomizeSysSetting::regions[component.region];

		// load the component texture
		if (component.name.empty())
			continue;
		string componentfilename = component.name;
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(componentfilename);
		if (pEntry)
			componentfilename = pEntry->GetLocalFileName();

		Color color = component.GetColor();
		pEngine->DrawImage(pEngine->LoadTexture(componentfilename, component.name), (float)coords.xpos, (float)coords.ypos, (float)coords.xsize, (float)coords.ysize, color);
	}
	CParaFile::CreateDirectory(m_sFileName.c_str());
	hr = (pEngine->SaveRenderTarget(m_sFileName, CCharCustomizeSysSetting::CharTexSize, CCharCustomizeSysSetting::CharTexSize, true, COLOR_XRGB(0, 0, 0))) ? S_OK : E_FAIL;
	pEngine->End();
#endif
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//
// CCCSSkinProcessor
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CCCSSkinProcessor::CCCSSkinProcessor(CharModelInstance* pCharModelInstance, const char* sFileName)
{
	m_pCharModelInstance = pCharModelInstance;
	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CCCSSkinProcessor::~CCCSSkinProcessor()
{

}

HRESULT ParaEngine::CCCSSkinProcessor::LockDeviceObject()
{
	return S_OK;
}

RenderDevicePtr ParaEngine::CCCSSkinProcessor::GetRenderDevice()
{
	return (m_pDevice != 0) ? m_pDevice : NULL;
}

HRESULT ParaEngine::CCCSSkinProcessor::UnLockDeviceObject()
{
	if (CharModelInstance::RemovePendingBodyTexture(m_pCharModelInstance))
	{
		// assign the texture (m_sFileName) to m_pCharModelInstance
		if (m_pCharModelInstance->m_bodyTexture && m_pCharModelInstance->m_bodyTexture->GetCacheFileName() == m_sFileName)
		{
#ifdef USE_DIRECTX_RENDERER
			m_pCharModelInstance->m_textures[CharModelInstance::CHAR_TEX] = CGlobals::GetAssetManager()->LoadTexture("", m_sFileName.c_str(), TextureEntity::StaticTexture);
#else
			m_pCharModelInstance->m_textures[CharModelInstance::CHAR_TEX] = ComposeRenderTarget();
#endif
		}
		else
		{
			// if the texture is changed when the texture is ready at this time, we will send another request to the queue.  
			m_pCharModelInstance->AddPendingBodyTexture(m_pCharModelInstance);
		}
	}
	return S_OK;
}

HRESULT ParaEngine::CCCSSkinProcessor::CleanUp()
{
	return S_OK;
}

HRESULT ParaEngine::CCCSSkinProcessor::Destroy()
{
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CCCSSkinProcessor::Process(void* pData, int cBytes)
{
	return S_OK;
}

HRESULT ParaEngine::CCCSSkinProcessor::CopyToResource()
{
	return S_OK;
}

void ParaEngine::CCCSSkinProcessor::SetResourceError()
{
	std::list <CharModelInstance*>::iterator itCur, itEnd = CharModelInstance::g_listPendingBodyTextures.end();
	for (itCur = CharModelInstance::g_listPendingBodyTextures.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == m_pCharModelInstance)
		{
			// TODO: shall we use a error texture here?
			// m_pCharModelInstance->m_textures[CharModelInstance::CHAR_TEX] = CGlobals::GetAssetManager()->LoadTexture("", m_sFileName.c_str(), TextureEntity::StaticTexture);
			break;
		}
	}
}

TextureEntity* ParaEngine::CCCSSkinProcessor::ComposeRenderTarget()
{
	if (m_pCharModelInstance && m_pCharModelInstance->m_bodyTexture)
	{
		const char* sName = "SkinComposer";
		TextureComposeRenderTarget* pRenderTarget = static_cast<TextureComposeRenderTarget*>(CGlobals::GetScene()->FindObjectByNameAndType(sName, "TextureComposeRenderTarget"));
		if (!pRenderTarget)
		{
			pRenderTarget = new TextureComposeRenderTarget();
			pRenderTarget->SetIdentifier(sName);
			/** black color as the transparent color*/
			pRenderTarget->SetColorKey(COLOR_XRGB(0, 0, 0));
			pRenderTarget->InitWithWidthAndHeight(CCharCustomizeSysSetting::CharTexSize, CCharCustomizeSysSetting::CharTexSize);
			CGlobals::GetScene()->AttachObject(pRenderTarget);
		}
		if (pRenderTarget)
		{
			auto pComposer = new CSkinLayers(m_sFileName, m_pCharModelInstance->m_bodyTexture->components);
			pRenderTarget->AddTask(pComposer);
			return pComposer->GetTexture();
		}
		// return CGlobals::GetAssetManager()->GetDefaultTexture(0);
	}
	return NULL;
}
