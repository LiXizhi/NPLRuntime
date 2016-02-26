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
#include "CharacterDB.h"
#include "SceneObject.h"
#include "CustomCharModelInstance.h"
#include "CustomCharRenderTarget.h"
#include "ContentLoaderCCSFace.h"
using namespace ParaEngine;


//////////////////////////////////////////////////////////////////////////
//
// CCCSFaceLoader
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CCCSFaceLoader::CCCSFaceLoader(CharModelInstance* pCharModelInstance, CartoonFace&  faceTexture, const char* sFileName)
	: m_pCharModelInstance(pCharModelInstance), m_nRetryCount(0), m_pAssetFileEntry(NULL)
{
	faceTexture.GetFaceComponents(m_layers);
	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CCCSFaceLoader::~CCCSFaceLoader()
{

}

const char* ParaEngine::CCCSFaceLoader::GetFileName()
{
	return m_sFileName.c_str();
}

HRESULT ParaEngine::CCCSFaceLoader::Decompress(void** ppData, int* pcBytes)
{
	return S_OK;
}

HRESULT ParaEngine::CCCSFaceLoader::CleanUp()
{
	return S_OK;
}

class SCCSFaceCallBackData
{
public:
	SCCSFaceCallBackData(CharModelInstance * pAsset_) :pAsset(pAsset_){}

	CharModelInstance * pAsset;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if (nResult == 0)
		{
			if (CharModelInstance::HasPendingCartoonFace(pAsset))
			{
				CharModelInstance::AddPendingCartoonFace(pAsset);
			}
		}
	}
};

HRESULT ParaEngine::CCCSFaceLoader::Destroy()
{
	if (m_pAssetFileEntry != 0)
	{
		// we need to download from the web server. 
		return m_pAssetFileEntry->SyncFile_Async(SCCSFaceCallBackData(m_pCharModelInstance));
	}

	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CCCSFaceLoader::Load()
{
	if (CParaFile::DoesFileExist(m_sFileName.c_str(), false))
		return S_OK;

	// check if all textures involved are loaded. 
	for (int i = 0; i < CFS_TOTAL_NUM; ++i)
	{
		FaceTextureComponent& component = m_layers[i];
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


HRESULT ParaEngine::CCCSFaceLoader::ComposeWithGDIPlus()
{
	HRESULT hr = S_OK;
#ifdef USE_DIRECTX_RENDERER
	// DO the texture composition here
	CGDIEngine* pEngine = CAsyncLoader::GetSingleton().GetGDIEngine();
	if (pEngine == NULL)
		return E_FAIL;
	pEngine->SetRenderTarget(pEngine->CreateGetRenderTargetBySize(CCharCustomizeSysSetting::FaceTexSize));
	pEngine->Begin();

	for (int i = 0; i<CFS_TOTAL_NUM; ++i)
	{
		const CharRegionCoords &coords = CCharCustomizeSysSetting::regions[CR_FACE_BASE + i];
		const FaceTextureComponent& component = m_layers[i];

		// load the component texture
		if (component.name.empty())
			continue;
		string componentfilename = component.name;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(componentfilename);
		if (pEntry)
			componentfilename = pEntry->GetLocalFileName();

		// compute the transform matrix
		Gdiplus::Matrix transformMatrix;
		// scale around the center
		float fScale = component.GetScaling();
		if (fabs(fScale)>0.01f)
		{
			transformMatrix.Scale(fScale + 1.f, fScale + 1.f);
		}

		// rotate around the center
		float fRotation = component.GetRotation();
		if (fabs(fRotation) > 0.01f)
		{
			transformMatrix.Rotate(fRotation);
		}

		// translation
		int x, y;
		component.GetPosition(&x, &y);
		transformMatrix.Translate(coords.xpos + (float)x, coords.ypos + (float)y);
		pEngine->SetTransform(&transformMatrix);

		Color color = component.GetColor();
		pEngine->DrawImage(pEngine->LoadTexture(componentfilename, component.name), (float)(-coords.xsize / 2), (float)(-coords.ysize / 2), (float)coords.xsize, (float)coords.ysize, color);

		// for eye and eye bow, there should be a mirrored image, around the center of the render target
		if (i == CFS_EYE || i == CFS_EYEBROW)
		{
			Gdiplus::Matrix reflectMat(-1.f, 0.f, 0.f, 1.f, CCharCustomizeSysSetting::FaceTexSize - (coords.xpos + (float)x) * 2, 0.f);
			transformMatrix.Multiply(&reflectMat);
			pEngine->SetTransform(&transformMatrix);
			pEngine->DrawImage(pEngine->LoadTexture(componentfilename, component.name), (float)(-coords.xsize / 2), (float)(-coords.ysize / 2), (float)coords.xsize, (float)coords.ysize);
		}
	}
	CParaFile::CreateDirectory(m_sFileName.c_str());
	hr = (pEngine->SaveRenderTarget(m_sFileName, CCharCustomizeSysSetting::FaceTexSize, CCharCustomizeSysSetting::FaceTexSize, false, 0)) ? S_OK : E_FAIL;
	pEngine->End();
#endif
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//
// CCCSFaceProcessor
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CCCSFaceProcessor::CCCSFaceProcessor(CharModelInstance* pCharModelInstance, const char* sFileName)
{
	m_pCharModelInstance = pCharModelInstance;
	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CCCSFaceProcessor::~CCCSFaceProcessor()
{

}

HRESULT ParaEngine::CCCSFaceProcessor::LockDeviceObject()
{
	return S_OK;
}

RenderDevicePtr ParaEngine::CCCSFaceProcessor::GetRenderDevice()
{
	// TODO: use the IO thread's render device. 
	// TODO: Create device using DirectXEngine.h.
	return (m_pDevice != 0) ? m_pDevice : NULL;
}

HRESULT ParaEngine::CCCSFaceProcessor::UnLockDeviceObject()
{
	if (CharModelInstance::RemovePendingCartoonFace(m_pCharModelInstance))
	{
		// assign the texture (m_sFileName) to m_pCharModelInstance
		if (m_pCharModelInstance->m_cartoonFace && m_pCharModelInstance->m_cartoonFace->GetCacheFileName() == m_sFileName)
		{
#ifdef USE_DIRECTX_RENDERER
			m_pCharModelInstance->m_textures[CharModelInstance::CARTOON_FACE_TEX] = CGlobals::GetAssetManager()->LoadTexture("", m_sFileName.c_str(), TextureEntity::StaticTexture);
#else
			m_pCharModelInstance->m_textures[CharModelInstance::CARTOON_FACE_TEX] = ComposeRenderTarget();
#endif
		}
		else
		{
			// if the texture is changed when the texture is ready at this time, we will send another request to the queue.  
			m_pCharModelInstance->AddPendingCartoonFace(m_pCharModelInstance);
		}
	}
	return S_OK;
}


HRESULT ParaEngine::CCCSFaceProcessor::CleanUp()
{
	return S_OK;
}

HRESULT ParaEngine::CCCSFaceProcessor::Destroy()
{
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CCCSFaceProcessor::Process(void* pData, int cBytes)
{
	return S_OK;
}

HRESULT ParaEngine::CCCSFaceProcessor::CopyToResource()
{
	return S_OK;
}

void ParaEngine::CCCSFaceProcessor::SetResourceError()
{
	std::list <CharModelInstance*>::iterator itCur, itEnd = CharModelInstance::g_listPendingCartoonFaces.end();
	for (itCur = CharModelInstance::g_listPendingCartoonFaces.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == m_pCharModelInstance)
		{
			// TODO: shall we use a error texture here?
			// m_pCharModelInstance->m_textures[CharModelInstance::CARTOON_FACE_TEX] = CGlobals::GetAssetManager()->LoadTexture("", m_sFileName.c_str(), TextureEntity::StaticTexture);
			break;
		}
	}
}

TextureEntity* ParaEngine::CCCSFaceProcessor::ComposeRenderTarget()
{
	if (m_pCharModelInstance && m_pCharModelInstance->m_cartoonFace)
	{
		const char* sName = "FaceComposer";
		TextureComposeRenderTarget* pRenderTarget = static_cast<TextureComposeRenderTarget*>(CGlobals::GetScene()->FindObjectByNameAndType(sName, "TextureComposeRenderTarget"));
		if (!pRenderTarget)
		{
			pRenderTarget = new TextureComposeRenderTarget();
			pRenderTarget->SetIdentifier(sName);
			pRenderTarget->InitWithWidthAndHeight(CCharCustomizeSysSetting::FaceTexSize, CCharCustomizeSysSetting::FaceTexSize);
			CGlobals::GetScene()->AttachObject(pRenderTarget);
		}
		if (pRenderTarget)
		{
			auto pComposer = new CFaceLayers(m_sFileName, *(m_pCharModelInstance->m_cartoonFace));
			pRenderTarget->AddTask(pComposer);
			return pComposer->GetTexture();
		}
		// return CGlobals::GetAssetManager()->GetDefaultTexture(0);
	}
	return NULL;
}

