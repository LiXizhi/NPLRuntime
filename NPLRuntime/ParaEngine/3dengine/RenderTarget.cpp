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
#include "TextureEntity.h"
#include "util/StringHelper.h"
#include "RenderTarget.h"

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
	m_depthStencilFormat(D3DFMT_D24S8),
	m_rhiRenderTarget(nullptr)
{
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

	if (m_rhiRenderTarget)
	{
		m_pCanvasTexture->AttachRHITexture(nullptr, nWidth, nHeight);
		m_rhiRenderTarget->ReleaseRHIResource();
		m_rhiRenderTarget = nullptr;
	}

	if (pRenderDevice)
	{
		PixelFormat depthFmt = (m_depthStencilFormat != 0) ? PixelFormat::D24S8 : PixelFormat::Unknown;
		m_rhiRenderTarget = pRenderDevice->CreateRenderTarget(static_cast<uint32_t>(nWidth), static_cast<uint32_t>(nHeight), PixelFormat::A8R8G8B8, depthFmt, 1, 0);
	}

	if (m_rhiRenderTarget && m_rhiRenderTarget->GetColorTexture(0))
	{
		m_pCanvasTexture->AttachRHITexture(m_rhiRenderTarget->GetColorTexture(0), nWidth, nHeight);
		m_pCanvasTexture->SetAliasTexParameters();
	}
	else
	{
		m_pCanvasTexture->RestoreDeviceObjects();
	}

	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::InvalidateDeviceObjects()
{
	if (m_pCanvasTexture == 0 && m_rhiRenderTarget == nullptr)
		return S_OK;
	m_bInitialized = false;
	if (m_pCanvasTexture)
	{
		m_pCanvasTexture->AttachRHITexture(nullptr, m_nTextureWidth, m_nTextureHeight);
		m_pCanvasTexture->InvalidateDeviceObjects();
	}
	if (m_rhiRenderTarget)
	{
		m_rhiRenderTarget->ReleaseRHIResource();
		m_rhiRenderTarget = nullptr;
	}
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

HRESULT ParaEngine::CRenderTarget::SaveToFile(const char* sFileName, int nImageWidth /*= 0*/, int nImageHeight /*= 0*/, PixelFormat dwFormat /*= 3*/, UINT MipLevels /*= 0*/, int srcLeft /*= 0*/, int srcTop /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/)
{
	if (m_pCanvasTexture == 0 || sFileName == nullptr || sFileName[0] == '\0')
		return E_FAIL;

	RECT rect;
	std::string filename = sFileName;
	if (srcWidth == 0 && ParaEngine::StringHelper::GetImageAndRect(sFileName, filename, &rect) != 0)
	{
		srcLeft = rect.left;
		srcTop = rect.top;
		srcWidth = rect.right - rect.left;
		srcHeight = rect.bottom - rect.top;
		if (nImageWidth == 0)
		{
			nImageWidth = srcWidth;
			nImageHeight = srcHeight;
		}
	}
	if (filename.empty())
		filename = sFileName;

	std::string sFile = filename;
	std::string sExt = CParaFile::GetFileExtension(sFile);
	if (sExt == "dds" && srcWidth == 0)
	{
		if ((int)dwFormat == 1)
			dwFormat = PixelFormat::DXT1;
		else if ((int)dwFormat == 2)
			dwFormat = PixelFormat::DXT2;
		else if ((int)dwFormat == 3)
			dwFormat = PixelFormat::DXT3;
		else if ((int)dwFormat == 4)
			dwFormat = PixelFormat::DXT4;
		else if ((int)dwFormat == 5)
			dwFormat = PixelFormat::DXT5;
		else
			dwFormat = PixelFormat::DXT3;

		if (nImageWidth <= 0)
			nImageWidth = m_nTextureWidth;
		if (nImageHeight <= 0 && m_nTextureWidth > 0)
			nImageHeight = (m_nTextureHeight * nImageWidth) / m_nTextureWidth;
		return m_pCanvasTexture->SaveToFile(sFile.c_str(), dwFormat, nImageWidth, nImageHeight, MipLevels) ? S_OK : E_FAIL;
	}

	if (sExt != "jpg" && sExt != "jpeg" && sExt != "png" && sExt != "tga")
	{
		sFile = CParaFile::ChangeFileExtension(sFile, "png");
		sExt = "png";
	}

	ImageEntity* image = NewImage(true, 0, srcLeft, srcTop, srcWidth, srcHeight);
	if (!image)
		return E_FAIL;

	HRESULT hr = E_FAIL;
	if (image->IsValid())
	{
		const int curW = image->getWidth();
		const int curH = image->getHeight();
		if (nImageWidth > 0 && m_nTextureWidth > 0 && nImageHeight <= 0)
			nImageHeight = (curH * nImageWidth) / (curW > 0 ? curW : m_nTextureWidth);

		if (nImageWidth > 0 && nImageHeight > 0 && (nImageWidth != curW || nImageHeight != curH) && curW > 0 && curH > 0)
		{
			std::vector<unsigned char> scaled(static_cast<size_t>(nImageWidth) * static_cast<size_t>(nImageHeight) * 4u);
			const unsigned char* srcData = image->getData();
			for (int dy = 0; dy < nImageHeight; ++dy)
			{
				const int sy = (std::min)(curH - 1, (dy * curH) / nImageHeight);
				for (int dx = 0; dx < nImageWidth; ++dx)
				{
					const int sx = (std::min)(curW - 1, (dx * curW) / nImageWidth);
					memcpy(&scaled[(static_cast<size_t>(dy) * nImageWidth + dx) * 4u],
						&srcData[(static_cast<size_t>(sy) * curW + sx) * 4u], 4u);
				}
			}
			ImageEntity scaledImg;
			if (scaledImg.LoadFromRawData(scaled.data(), scaled.size(), nImageWidth, nImageHeight, 8))
			{
				const bool isToRGB = (sExt == "jpg" || sExt == "jpeg");
				if (scaledImg.SaveToFile(sFile, isToRGB))
					hr = S_OK;
			}
		}
		else
		{
			const bool isToRGB = (sExt == "jpg" || sExt == "jpeg");
			if (image->SaveToFile(sFile, isToRGB))
				hr = S_OK;
		}
	}
	SAFE_DELETE(image);
	return hr;
}


ImageEntity* ParaEngine::CRenderTarget::NewImage(bool bFlipImage, Color colorKey, int srcLeft, int srcTop, int srcWidth, int srcHeight)
{
	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();
	if (nullptr == GetTexture() || !pRenderDevice)
	{
		return nullptr;
	}

	CheckInit();
	if (!m_rhiRenderTarget && pRenderDevice->GetBackendType() != ERHIBackend::Null)
	{
		return nullptr;
	}

	Vector2 s = GetRenderTargetSize();
	int savedBufferWidth = (int)s.x;
	int savedBufferHeight = (int)s.y;
	if (savedBufferWidth <= 0 || savedBufferHeight <= 0)
		return nullptr;

	if (srcLeft < 0)
		srcLeft = 0;
	if (srcTop < 0)
		srcTop = 0;
	if (srcWidth <= 0)
		srcWidth = savedBufferWidth - srcLeft;
	else if (srcLeft + srcWidth > savedBufferWidth)
		srcWidth = savedBufferWidth - srcLeft;

	if (srcHeight <= 0)
		srcHeight = savedBufferHeight - srcTop;
	else if (srcTop + srcHeight > savedBufferHeight)
		srcHeight = savedBufferHeight - srcTop;

	if (srcWidth <= 0 || srcHeight <= 0)
		return nullptr;

	uint8* buffer = new (std::nothrow) uint8[static_cast<size_t>(srcWidth) * static_cast<size_t>(srcHeight) * 4u];
	if (!buffer)
		return nullptr;

	ImageEntity* image = new ImageEntity();
	do
	{
		const bool needTempBind = !m_bIsBegin && (m_rhiRenderTarget != nullptr);
		if (needTempBind)
			pRenderDevice->PushRenderTarget(m_rhiRenderTarget);

		bool readOk = false;
		if (pRenderDevice->GetBackendType() == ERHIBackend::Null)
		{
			memset(buffer, 0, static_cast<size_t>(srcWidth) * static_cast<size_t>(srcHeight) * 4u);
			readOk = true;
		}
		else
		{
			readOk = pRenderDevice->ReadPixels(srcLeft, srcTop, srcWidth, srcHeight, buffer);
		}

		if (needTempBind)
			pRenderDevice->PopRenderTarget();

		if (!readOk)
			break;

		if ((DWORD)colorKey != 0)
		{
			int nSize = srcWidth * srcHeight;
			DWORD* pPixelData = reinterpret_cast<DWORD*>(buffer);
			DWORD dwTransparentColorKey = colorKey;
			for (int i = 0; i < nSize; ++i, ++pPixelData)
			{
				if (*pPixelData == dwTransparentColorKey)
					*pPixelData = 0;
			}
		}

		// OpenGL ReadPixels returns bottom-to-top rows (topLeftScreenOrigin == false),
		// whereas D3D11/modern APIs return top-to-bottom rows (topLeftScreenOrigin == true).
		const bool needRowFlip = (bFlipImage != pRenderDevice->GetCaps().topLeftScreenOrigin);
		if (needRowFlip)
		{
			uint8* tempData = new (std::nothrow) uint8[static_cast<size_t>(srcWidth) * static_cast<size_t>(srcHeight) * 4u];
			if (!tempData)
				break;

			for (int i = 0; i < srcHeight; ++i)
			{
				memcpy(&tempData[static_cast<size_t>(i) * srcWidth * 4u],
					&buffer[static_cast<size_t>(srcHeight - i - 1) * srcWidth * 4u],
					static_cast<size_t>(srcWidth) * 4u);
			}
			image->LoadFromRawData(tempData, static_cast<size_t>(srcWidth) * static_cast<size_t>(srcHeight) * 4u, srcWidth, srcHeight, 8);
			SAFE_DELETE_ARRAY(tempData);
		}
		else
		{
			image->LoadFromRawData(buffer, static_cast<size_t>(srcWidth) * static_cast<size_t>(srcHeight) * 4u, srcWidth, srcHeight, 8);
		}
	} while (0);

	SAFE_DELETE_ARRAY(buffer);
	return image;
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
	if (m_pCanvasTexture)
	{
		m_pCanvasTexture->AttachRHITexture(nullptr, m_nTextureWidth, m_nTextureHeight);
	}
	if (m_rhiRenderTarget)
	{
		m_rhiRenderTarget->ReleaseRHIResource();
		m_rhiRenderTarget = nullptr;
	}
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

bool ParaEngine::CRenderTarget::Begin(int x, int y, int w, int h)
{
	bool ret = this->Begin();
	if (!ret)
		return false;

	m_oldViewport.X = x;
	m_oldViewport.Y = y;
	m_oldViewport.Width = w;
	m_oldViewport.Height = h;

	return true;
}


bool ParaEngine::CRenderTarget::Begin()
{
	auto pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice)
		return false;

	CheckInit();
	if (!m_rhiRenderTarget && pRenderDevice->GetBackendType() != ERHIBackend::Null)
		return false;

	// save old viewport
	m_oldViewport = pRenderDevice->GetViewport();

	// save old render origin
	m_vOldRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

	if (m_pCanvasTexture)
		m_pCanvasTexture->SetHitCount(0);

	pRenderDevice->PushRenderTarget(m_rhiRenderTarget);
	pRenderDevice->BeginRenderTarget(static_cast<uint32_t>(m_nTextureWidth), static_cast<uint32_t>(m_nTextureHeight));

	ParaViewport vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = GetTextureWidth();
	vp.Height = GetTextureHeight();
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	pRenderDevice->SetViewport(vp);

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
	if (pRenderDevice)
	{
		pRenderDevice->EndRenderTarget();
		pRenderDevice->PopRenderTarget();
		pRenderDevice->SetViewport(m_oldViewport);
	}
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
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);

		/*pRenderDevice->SetClearStencil(1);
		pRenderDevice->Clear(true, true, true);
		pRenderDevice->SetRenderState(ERenderState::SCISSORTESTENABLE, FALSE);*/
		pRenderDevice->Clear(true, true, false);
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
		if (m_bIsBegin)
		{
			DoPaint();
		}
	}
	return S_OK;
}

bool ParaEngine::CRenderTarget::IsDirty() const
{
	if (m_bIsDirty)
		return true;
	else if (IsActiveRenderingEnabled())
	{
		if (m_pCanvasTexture)
			return m_pCanvasTexture->GetHitCount() > 0;
		else
			return true;
	}
	return false;
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


void ParaEngine::CRenderTarget::RunCommandList(const char* cmd)
{
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
	pClass->AddField("SaveToFile", FieldType_String, (void*)SaveToFile_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("RunCommandList", FieldType_String, (void*)RunCommandList_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("run", FieldType_String, (void*)RunCommandList_s, (void*)0, NULL, NULL, bOverride);
	return S_OK;
}


