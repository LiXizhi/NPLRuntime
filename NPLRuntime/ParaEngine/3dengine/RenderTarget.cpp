//-----------------------------------------------------------------------------
// Class:	RenderTarget
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.2.22
// Revised: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ViewportManager.h"
#include "ParaWorldAsset.h"
#include "PaintEngine/PaintEngineGPU.h"
#include "ImageEntity.h"
#include "RenderTarget.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#include "TextureD3D9.h"
#endif

#if USE_OPENGL_RENDERER
#include "RenderDeviceOpenGL.h"
#endif

/** @def default canvas map width in pixels */
#define DEFAULT_CANVAS_MAP_WIDTH	512
/** @def default canvas map height in pixels */
#define DEFAULT_CANVAS_MAP_HEIGHT	512

#ifndef CHECK_RETURN_CODE
#define CHECK_RETURN_CODE(text, hr) if(FAILED((hr))){OUTPUT_LOG(text);return hr;}
#endif

using namespace ParaEngine;

ParaEngine::CRenderTarget::CRenderTarget()
	: engine(0),
	m_bInitialized(false), m_bIsBegin(false), m_dwClearColor(0), m_nTextureWidth(DEFAULT_CANVAS_MAP_WIDTH), m_nTextureHeight(DEFAULT_CANVAS_MAP_HEIGHT), 
	m_bActiveRendering(false), m_bIsDirty(true), m_nLifeTime(-1), m_bPersistentRenderTarget(false),
	m_depthStencilFormat(D3DFMT_D24S8)
{
	m_pDepthStencilSurface = NULL;
	m_pOldRenderTarget = NULL;
	m_pOldZBuffer = NULL;
	m_vOldRenderOrigin = Vector3::ZERO;
}

ParaEngine::CRenderTarget::~CRenderTarget()
{
	SAFE_DELETE(engine);
	Cleanup();
	if (m_pCanvasTexture && !m_bPersistentRenderTarget)
	{
		TextureEntity* texture = m_pCanvasTexture.get();
		m_pCanvasTexture->MakeInvalid();
		if (m_pCanvasTexture->GetRefCount() > 1)
		{
			m_pCanvasTexture.reset();
			TextureAssetManager::GetInstance().DeleteEntity(texture);
		}
	}
}

bool ParaEngine::CRenderTarget::IsPersistentRenderTarget() const
{
	return m_bPersistentRenderTarget;
}

void ParaEngine::CRenderTarget::SetPersistentRenderTarget(bool val)
{
	m_bPersistentRenderTarget = val;
}

TextureEntity* ParaEngine::CRenderTarget::GetTexture()
{
	return m_pCanvasTexture.get();
}

AssetEntity* ParaEngine::CRenderTarget::GetPrimaryAsset()
{
	if (!m_pCanvasTexture)
	{
		InitWithWidthAndHeight(m_nTextureWidth, m_nTextureHeight);
	}
	return GetTexture();
}

HRESULT ParaEngine::CRenderTarget::InitDeviceObjects()
{
	if (m_pCanvasTexture)
		m_pCanvasTexture->InitDeviceObjects();
	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::RestoreDeviceObjects()
{
	if (m_pCanvasTexture == 0)
		return S_OK;
	m_bInitialized = true;

	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();

	int nWidth = m_nTextureWidth;
	int nHeight = m_nTextureHeight;

	HRESULT hr;

	hr = m_pCanvasTexture->RestoreDeviceObjects();
	CHECK_RETURN_CODE("CreateTexture Canvas Texture", hr);

	m_pDepthStencilSurface = CGlobals::GetRenderDevice()->CreateTexture(nWidth, nHeight, EPixelFormat::D24S8, ETextureUsage::DepthStencil);
	if (!m_pDepthStencilSurface)
	{
		OUTPUT_LOG("failed creating depth stencil buffer");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::InvalidateDeviceObjects()
{
	if (m_pCanvasTexture == 0)
		return S_OK;
	m_bInitialized = false;
	SAFE_RELEASE(m_pDepthStencilSurface);
	m_pCanvasTexture->InvalidateDeviceObjects();
	SetDirty(true);
	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::DeleteDeviceObjects()
{
	if (m_pCanvasTexture == 0)
		return S_OK;
	m_pCanvasTexture->DeleteDeviceObjects();
	// Bug fixed: we should specify the texture size, otherwise the back buffer size is used. 
	m_pCanvasTexture->SetTextureInfo(TextureEntity::TextureInfo(m_nTextureWidth, m_nTextureHeight, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
	SetDirty(true);
	return S_OK;
}

void ParaEngine::CRenderTarget::Cleanup()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

HRESULT ParaEngine::CRenderTarget::SaveToFile(const char* sFileName, int nImageWidth /*= 0*/, int nImageHeight /*= 0*/, EPixelFormat dwFormat /*= 3*/, UINT MipLevels /*= 0*/, int srcLeft /*= 0*/, int srcTop /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/)
{
	if (m_pCanvasTexture == 0)
		return E_FAIL;
#ifdef USE_DIRECTX_RENDERER
	string sFile = sFileName;
	string sExt = CParaFile::GetFileExtension(sFileName);

	D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;

	if (sExt == "dds")
	{
		FileFormat = D3DXIFF_DDS;
	}
	else if (sExt == "jpg")
	{
		FileFormat = D3DXIFF_JPG;
	}
	else if (sExt == "tga")
	{
		FileFormat = D3DXIFF_TGA;
	}
	else // if(sExt == "png")
	{
		sFile = CParaFile::ChangeFileExtension(sFile, "png");
	}

	if ((FileFormat != D3DXIFF_DDS) && (nImageWidth <= 0 || nImageWidth >= m_nTextureWidth) && srcWidth == 0)
	{
		HRESULT hr = D3DXSaveTextureToFile(sFile.c_str(), FileFormat, GetD3DTex(m_pCanvasTexture->GetTexture()), NULL);
		if (SUCCEEDED(hr))
		{
			OUTPUT_LOG("miniscenegraph portrait %d taken for %s", m_nTextureWidth, sFile.c_str());
		}
	}
	else if (FileFormat == D3DXIFF_DDS  && srcWidth == 0)
	{
		if ((int)dwFormat == 1)
			dwFormat = EPixelFormat::DXT1;
		else if ((int)dwFormat == 2)
			dwFormat = EPixelFormat::DXT2;
		else if ((int)dwFormat == 3)
			dwFormat = EPixelFormat::DXT3;
		else if ((int)dwFormat == 4)
			dwFormat = EPixelFormat::DXT4;
		else if ((int)dwFormat == 5)
			dwFormat = EPixelFormat::DXT5;
		else
			dwFormat = EPixelFormat::DXT3;

		if (m_pCanvasTexture->GetTexture())
		{
			if (nImageWidth <= 0)
				nImageWidth = m_nTextureWidth;
			if (nImageHeight <= 0)
			{
				nImageHeight = (m_nTextureHeight * nImageWidth) / m_nTextureWidth;
			}
			m_pCanvasTexture->SaveToFile(sFileName,dwFormat, nImageWidth, nImageHeight, 0);
		}
	}
	else
	{
		// For none-dds texture

		// if the size is somewhere in the middle.
		/*int nMipLevel = 0;
		int nWidth = m_nTextureWidth;
		while (nImageSize <nWidth)
		{
		nMipLevel++;
		nWidth = nWidth / 2;
		}*/

		if (nImageWidth <= 0)
			nImageWidth = m_nTextureWidth;
		if (nImageHeight <= 0)
		{
			nImageHeight = (m_nTextureHeight * nImageWidth) / m_nTextureWidth;
		}

		LPDIRECT3DTEXTURE9 pTex = GetD3DTex(m_pCanvasTexture->GetTexture());
		if (pTex)
		{
			LPDIRECT3DSURFACE9 pSur = NULL;
			pTex->GetSurfaceLevel(0, &pSur);

			if (pSur)
			{
				auto pRenderDevice = CGlobals::GetRenderDevice();
				D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
				LPDIRECT3DTEXTURE9 pTextureDest = NULL;
				LPDIRECT3DSURFACE9 pSurDest = NULL;
				RECT srcRect;
				srcRect.left = srcLeft; srcRect.top = srcTop; srcRect.right = srcLeft + srcWidth; srcRect.bottom = srcTop + srcHeight;

				if (FAILED(GETD3D(CGlobals::GetRenderDevice())->CreateTexture((int)nImageWidth, (int)nImageHeight, 1, D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &pTextureDest, NULL)))
				{
					SAFE_RELEASE(pSur);
					return E_FAIL;
				}
				if (FAILED(pTextureDest->GetSurfaceLevel(0, &pSurDest)))
				{
					SAFE_RELEASE(pTextureDest);
					SAFE_RELEASE(pSur);
					return E_FAIL;
				}

				// Copy scene to render target texture
				if (SUCCEEDED(GETD3D(CGlobals::GetRenderDevice())->StretchRect(pSur, (srcWidth == 0) ? NULL : (&srcRect), pSurDest, NULL, D3DTEXF_LINEAR)))
				{
					if (SUCCEEDED(D3DXSaveSurfaceToFile(sFile.c_str(), FileFormat, pSurDest, NULL, NULL)))
					{
						OUTPUT_LOG("miniscenegraph portrait (%d X %d) taken for %s", nImageWidth, nImageHeight, sFile.c_str());
					}
				}
				SAFE_RELEASE(pSur);
				SAFE_RELEASE(pSurDest);
				SAFE_RELEASE(pTextureDest);
			}
		}
	}
#elif defined(USE_OPENGL_RENDERER)
	ImageEntity* image = NewImage(true);
	if (image)
	{
		if (image->IsValid())
		{
			image->SaveToFile(sFileName);
		}
		SAFE_DELETE(image);
	}
#endif
	return S_OK;
}


ImageEntity* ParaEngine::CRenderTarget::NewImage(bool bFlipImage, Color colorKey)
{
	return nullptr;
}


void ParaEngine::CRenderTarget::SetRenderTargetSize(int nWidth, int nHeight)
{
	if (m_nTextureWidth != nWidth || m_nTextureHeight != nHeight)
	{
		m_nTextureWidth = nWidth;
		m_nTextureHeight = nHeight;
		SetDirty(true);
		Cleanup();
	}
}

void ParaEngine::CRenderTarget::SetRenderTargetSize(const Vector2& size)
{
	SetRenderTargetSize((int)size.x, (int)size.y);
}

void ParaEngine::CRenderTarget::GetRenderTargetSize(int* nWidth, int* nHeight)
{
	*nWidth = m_nTextureWidth;
	*nHeight = m_nTextureHeight;
}

ParaEngine::Vector2 ParaEngine::CRenderTarget::GetRenderTargetSize()
{
	int nWidth, nHeight;
	GetRenderTargetSize(&nWidth, &nHeight);
	return Vector2((float)nWidth, (float)nHeight);
}

const std::string& ParaEngine::CRenderTarget::GetCanvasTextureName()
{
	if (m_sCanvasTextureName.empty()){
		m_sCanvasTextureName = "_miniscenegraph_";
		m_sCanvasTextureName += GetIdentifier();
	}
	return m_sCanvasTextureName;
}

HRESULT ParaEngine::CRenderTarget::RendererRecreated()
{
	m_bInitialized = false;
	return CBaseObject::RendererRecreated();
}

void ParaEngine::CRenderTarget::SetCanvasTextureName(const std::string& sValue)
{
	if (m_sCanvasTextureName != sValue){
		m_sCanvasTextureName = sValue;
	}
}

bool ParaEngine::CRenderTarget::InitWithWidthAndHeight(int width, int height, D3DFORMAT format, D3DFORMAT depthStencilFormat /*= D3DFMT_D16*/)
{
	const std::string& sKey = GetCanvasTextureName();

	SetRenderTargetSize(width, height);

	// create the canvas texture
	m_pCanvasTexture = CGlobals::GetAssetManager()->LoadTexture(sKey, sKey, TextureEntity::RenderTarget);
	m_pCanvasTexture->UnloadAsset();
	m_pCanvasTexture->SurfaceType = TextureEntity::RenderTarget;
	m_pCanvasTexture->SetTextureInfo(TextureEntity::TextureInfo(m_nTextureWidth, m_nTextureHeight, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
	OUTPUT_LOG("CRenderTarget: %s render target renewed (%d %d)\n", sKey.c_str(), m_nTextureWidth, m_nTextureHeight);

	return true;
}

bool ParaEngine::CRenderTarget::Begin()
{
	auto pRenderDevice = CGlobals::GetRenderDevice();

	CheckInit();

	// save old viewport
	m_oldViewport = CGlobals::GetRenderDevice()->GetViewport();

	// save old render origin
	m_vOldRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

	m_pOldRenderTarget = NULL;
	m_pOldZBuffer = NULL;
	if (m_pCanvasTexture == 0 || m_pDepthStencilSurface == 0)
		return false;
	if (m_pCanvasTexture)
	{
		m_pCanvasTexture->SetHitCount(0);
	}

	m_pOldRenderTarget = pRenderDevice->GetRenderTarget(0);
	
	pRenderDevice->SetRenderTarget(0, m_pCanvasTexture->GetTexture());

	pRenderDevice->SetRenderTarget(1, NULL);
	pRenderDevice->SetRenderTarget(2, NULL);
	pRenderDevice->SetRenderTarget(3, NULL);

	m_pOldZBuffer = pRenderDevice->GetDepthStencil();
	// set depth surface
	if (m_pOldZBuffer == nullptr)
	{
		OUTPUT_LOG("error: can not get GetDepthStencilSurface in miniscene graph.\n");
		return false;
	}
	CGlobals::GetRenderDevice()->SetDepthStencil(m_pDepthStencilSurface);
	m_bIsBegin = true;
	return true;
}

void ParaEngine::CRenderTarget::End()
{
	if (m_bIsBegin)
		m_bIsBegin = false;
	else
	{
		OUTPUT_LOG("warning:calling end() without calling begin in CRenderTarget. \n");
		return;
	}
	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();
	// restore render origin
	CGlobals::GetScene()->RegenerateRenderOrigin(m_vOldRenderOrigin);
	
	// restore old depth surface
	CGlobals::GetRenderDevice()->SetDepthStencil(m_pOldZBuffer);
	SAFE_RELEASE(m_pOldZBuffer);
	// Restore the old render target: i.e. the backbuffer
	bool ret  = pRenderDevice->SetRenderTarget(0, m_pOldRenderTarget);
	PE_ASSERT(ret);
	m_pOldRenderTarget = NULL;
	// restore viewport
	pRenderDevice->SetViewport(m_oldViewport);
}


void CRenderTarget::SetClearColor(const LinearColor& bgColor)
{
	m_dwClearColor = bgColor;
}

LinearColor CRenderTarget::GetClearColor()
{
	LinearColor color(m_dwClearColor);
	return color;
}

void ParaEngine::CRenderTarget::Clear(const LinearColor& color, float depthValue /*= 1.f*/, int stencilValue /*= 0*/, DWORD flags /*= D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER*/)
{
	if (m_bIsBegin)
	{
		RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();
		pRenderDevice->SetClearColor(Color4f(color.r,color.g,color.b,color.a));
		pRenderDevice->SetClearDepth(depthValue);
		pRenderDevice->SetClearStencil(stencilValue);
		pRenderDevice->Clear(true, true, true);
	}
	else
	{
		OUTPUT_LOG("warning:calling clear outside begin/end is not allowed in CRenderTarget. \n");
	}
}

CPaintEngine * ParaEngine::CRenderTarget::paintEngine() const
{
	if (engine)
		return engine;

	CPaintEngine *engine_ = CPaintEngineGPU::GetInstance();
	if (engine_->isActive() && engine_->paintDevice() != this) {
		engine_ = new CPaintEngineGPU();
		return engine_;
	}
	return engine_;
}

void ParaEngine::CRenderTarget::DoPaint(CPainter* painter)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_Paint);
	if (pCallback)
	{
		if (painter)
		{
			pCallback->ActivateLocalNow(pCallback->GetCode());
		}
		else
		{
			CPainter painter(this);
			pCallback->ActivateLocalNow(pCallback->GetCode());
		}
	}
}

int ParaEngine::CRenderTarget::PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState)
{
	if (m_nLifeTime > 0)
		m_nLifeTime--;
	if (IsDirty() && pSceneState && IsVisible())
	{
		pSceneState->listOwnerDrawObjs.push_back(GetWeakReference());
	}
	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::Draw(SceneState * sceneState)
{
	if (IsDirty())
	{
		SetDirty(false);

		ScopedPaintOnRenderTarget paint(this);
		DoPaint();
	}
	return S_OK;
}

bool ParaEngine::CRenderTarget::IsDirty() const
{
	return m_bIsDirty || IsActiveRenderingEnabled();
}

void ParaEngine::CRenderTarget::SetDirty(bool val)
{
	m_bIsDirty = val;
}

bool ParaEngine::CRenderTarget::IsActiveRenderingEnabled() const
{
	return m_bActiveRendering;
}

void ParaEngine::CRenderTarget::EnableActiveRendering(bool bEnable)
{
	m_bActiveRendering = bEnable;
}

void ParaEngine::CRenderTarget::CheckInit()
{
	if (!m_bInitialized)
	{
		InitDeviceObjects();
		RestoreDeviceObjects();
	}
}

int ParaEngine::CRenderTarget::metric(PaintDeviceMetric metric) const
{
	if (metric == CPaintDevice::PdmWidth)
		return m_nTextureWidth;
	else if (metric == CPaintDevice::PdmHeight)
		return m_nTextureHeight;
	else
		return CPaintDevice::metric(metric);
}

bool ParaEngine::CRenderTarget::IsDead()
{
	return m_nLifeTime == 0;
}

int ParaEngine::CRenderTarget::GetLifeTime() const
{
	return m_nLifeTime;
}

void ParaEngine::CRenderTarget::SetLifeTime(int val)
{
	m_nLifeTime = val;
}

void ParaEngine::CRenderTarget::SetDead()
{
	m_nLifeTime = 0;
}

int ParaEngine::CRenderTarget::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);
	pClass->AddField("ClearColor", FieldType_Vector3, (void*)SetClearColor_s, (void*)GetClearColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);
	pClass->AddField("On_Paint", FieldType_String, (void*)SetPaint_s, (void*)GetPaint_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("EnableActiveRendering", FieldType_Bool, (void*)EnableActiveRendering_s, (void*)IsActiveRenderingEnabled_s, NULL, "", bOverride);
	pClass->AddField("RenderTargetSize", FieldType_Vector2, (void*)SetRenderTargetSize_s, (void*)GetRenderTargetSize_s, NULL, "", bOverride);
	pClass->AddField("Dirty", FieldType_Bool, (void*)SetDirty_s, (void*)IsDirty_s, NULL, "", bOverride);
	pClass->AddField("IsPersistentRenderTarget", FieldType_Bool, (void*)SetPersistentRenderTarget_s, (void*)IsPersistentRenderTarget_s, NULL, "", bOverride);
	return S_OK;
}
