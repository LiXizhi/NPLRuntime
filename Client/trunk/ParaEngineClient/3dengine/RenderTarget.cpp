//-----------------------------------------------------------------------------
// Class:	RenderTarget
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.2.22
// Revised: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#endif
#include "SceneObject.h"
#include "ViewportManager.h"
#include "ParaWorldAsset.h"
#include "PaintEngine/PaintEngineGPU.h"
#include "ImageEntity.h"
#include "RenderTarget.h"
#include "StringHelper.h"

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
#ifdef USE_DIRECTX_RENDERER
	m_pDepthStencilSurface = NULL;
	m_pCanvasSurface = NULL;
	m_pOldRenderTarget = NULL;
	m_pOldZBuffer = NULL;
#elif defined(USE_OPENGL_RENDERER)
	_FBO = 0;
	_depthRenderBufffer = 0;
	_oldFBO = 0;
	_oldRBO = 0;
#endif
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

	RenderDevicePtr pD3dDevice = CGlobals::GetRenderDevice();

	int nWidth = m_nTextureWidth;
	int nHeight = m_nTextureHeight;

#ifdef USE_DIRECTX_RENDERER
	HRESULT hr;
	
	/*hr = pD3dDevice->CreateTexture(nWidth, 	nHeight,
	1, D3DUSAGE_RENDERTARGET,
	D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pCanvasTexture, NULL);
	CHECK_RETURN_CODE("CreateTexture Canvas Texture", hr);

	hr = m_pCanvasTexture->GetSurfaceLevel(0, &m_pCanvasSurface);
	CHECK_RETURN_CODE("GetSurfaceLevel Canvas Surface", hr);*/

	hr = m_pCanvasTexture->RestoreDeviceObjects();
	CHECK_RETURN_CODE("CreateTexture Canvas Texture", hr);

	hr = m_pCanvasTexture->GetTexture()->GetSurfaceLevel(0, &m_pCanvasSurface);
	CHECK_RETURN_CODE("GetSurfaceLevel Canvas Surface", hr);

	hr = pD3dDevice->CreateDepthStencilSurface(nWidth, nHeight, D3DFMT_D16,
		D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthStencilSurface, NULL);
	CHECK_RETURN_CODE("failed creating depth stencil buffer", hr);
#elif defined(USE_OPENGL_RENDERER)
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);

	glGetIntegerv(GL_RENDERBUFFER_BINDING, &_oldRBO);

	// generate FBO
	glGenFramebuffers(1, &_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

	// associate texture with FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pCanvasTexture->GetTexture(), 0);

	if (m_depthStencilFormat != 0)
	{
		GLuint depthStencilFormat = GL_DEPTH24_STENCIL8;
		//create and attach depth buffer
		glGenRenderbuffers(1, &_depthRenderBufffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBufffer);
		glRenderbufferStorage(GL_RENDERBUFFER, depthStencilFormat, (GLsizei)nWidth, (GLsizei)nHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBufffer);

		// if depth format is the one with stencil part, bind same render buffer as stencil attachment
		if (depthStencilFormat == GL_DEPTH24_STENCIL8)
		{
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBufffer);
		}
	}
	// check if it worked (probably worth doing :) )
	PE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	((TextureEntityOpenGL*)m_pCanvasTexture.get())->SetAliasTexParameters();

	glBindRenderbuffer(GL_RENDERBUFFER, _oldRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);

#endif
	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::InvalidateDeviceObjects()
{
	if (m_pCanvasTexture == 0)
		return S_OK;
	m_bInitialized = false;
#ifdef USE_DIRECTX_RENDERER
	SAFE_RELEASE(m_pDepthStencilSurface);
	SAFE_RELEASE(m_pCanvasSurface);
	m_pCanvasTexture->InvalidateDeviceObjects();
#elif defined(USE_OPENGL_RENDERER)
	glDeleteFramebuffers(1, &_FBO);
	_FBO = 0;
	if (_depthRenderBufffer)
	{
		glDeleteRenderbuffers(1, &_depthRenderBufffer);
		_depthRenderBufffer = 0;
	}
#endif
	SetDirty(true);
	return S_OK;
}

HRESULT ParaEngine::CRenderTarget::DeleteDeviceObjects()
{
#ifdef USE_DIRECTX_RENDERER
	if (m_pCanvasTexture == 0)
		return S_OK;
	m_pCanvasTexture->DeleteDeviceObjects();
	// Bug fixed: we should specify the texture size, otherwise the back buffer size is used. 
	m_pCanvasTexture->SetTextureInfo(TextureEntity::TextureInfo(m_nTextureWidth, m_nTextureHeight, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
#endif
	SetDirty(true);
	return S_OK;
}

void ParaEngine::CRenderTarget::Cleanup()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

HRESULT ParaEngine::CRenderTarget::SaveToFile(const char* sFileName, int nImageWidth /*= 0*/, int nImageHeight /*= 0*/, DWORD dwFormat /*= 3*/, UINT MipLevels /*= 0*/, int srcLeft /*= 0*/, int srcTop /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/)
{
	if (m_pCanvasTexture == 0)
		return E_FAIL;

	RECT rect;
	std::string filename;
	if (srcWidth == 0 && ParaEngine::StringHelper::GetImageAndRect(sFileName, filename, &rect) != 0)
	{
		srcLeft = rect.left;
		srcTop = rect.top;
		srcWidth = rect.right - rect.left;
		srcHeight = rect.bottom - rect.top;
		if (nImageWidth == 0) {
			nImageWidth = srcWidth;
			nImageHeight = srcHeight;
		}
	}
	sFileName = filename.c_str();

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
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
		std::wstring sFile16 = StringHelper::MultiByteToWideChar(sFile.c_str(), DEFAULT_FILE_ENCODING);
		if (SUCCEEDED(D3DXSaveTextureToFileW(sFile16.c_str(), FileFormat, m_pCanvasTexture->GetTexture(), NULL)))
#else 
		if (SUCCEEDED(D3DXSaveTextureToFile(sFile.c_str(), FileFormat, m_pCanvasTexture->GetTexture(), NULL)))
#endif
		{
			OUTPUT_LOG("miniscenegraph portrait %d taken for %s", m_nTextureWidth, sFile.c_str());
		}
	}
	else if (FileFormat == D3DXIFF_DDS  && srcWidth == 0)
	{
		if (dwFormat == 1)
			dwFormat = D3DFMT_DXT1;
		else if (dwFormat == 2)
			dwFormat = D3DFMT_DXT2;
		else if (dwFormat == 3)
			dwFormat = D3DFMT_DXT3;
		else if (dwFormat == 4)
			dwFormat = D3DFMT_DXT4;
		else if (dwFormat == 5)
			dwFormat = D3DFMT_DXT5;
		else
			dwFormat = D3DFMT_DXT3;

		if (m_pCanvasTexture->GetTexture())
		{
			if (nImageWidth <= 0)
				nImageWidth = m_nTextureWidth;
			if (nImageHeight <= 0)
			{
				nImageHeight = (m_nTextureHeight * nImageWidth) / m_nTextureWidth;
			}
			m_pCanvasTexture->SaveToFile(sFileName, (D3DFORMAT)dwFormat, nImageWidth, nImageHeight, 0);
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

		LPDIRECT3DTEXTURE9 pTex = m_pCanvasTexture->GetTexture();
		if (pTex)
		{
			LPDIRECT3DSURFACE9 pSur = NULL;
			pTex->GetSurfaceLevel(0, &pSur);

			if (pSur)
			{
				LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
				D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
				LPDIRECT3DTEXTURE9 pTextureDest = NULL;
				LPDIRECT3DSURFACE9 pSurDest = NULL;
				RECT srcRect;
				srcRect.left = srcLeft; srcRect.top = srcTop; srcRect.right = srcLeft + srcWidth; srcRect.bottom = srcTop + srcHeight;

				if (FAILED(pd3dDevice->CreateTexture((int)nImageWidth, (int)nImageHeight, 1, D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &pTextureDest, NULL)))
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
				if (SUCCEEDED(pd3dDevice->StretchRect(pSur, (srcWidth == 0) ? NULL : (&srcRect), pSurDest, NULL, D3DTEXF_LINEAR)))
				{
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
					LPCWSTR sFile16 = StringHelper::MultiByteToWideChar(sFile.c_str(), DEFAULT_FILE_ENCODING);
					if (SUCCEEDED(D3DXSaveSurfaceToFileW(sFile16, FileFormat, pSurDest, NULL, NULL)))
#else 
					if (SUCCEEDED(D3DXSaveSurfaceToFile(sFile.c_str(), FileFormat, pSurDest, NULL, NULL)))
#endif
					
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
#ifdef USE_OPENGL_RENDERER
	if (nullptr == GetTexture())
	{
		return nullptr;
	}
	// auto _texture = ((TextureEntityOpenGL*)GetTexture())->m_texture;
	Vector2 s = GetRenderTargetSize();

	// to get the image size to save
	//        if the saving image domain exceeds the buffer texture domain,
	//        it should be cut
	int savedBufferWidth = (int)s.x;
	int savedBufferHeight = (int)s.y;

	GLubyte *buffer = nullptr;
	
	ImageEntity* image = new ImageEntity();
	do
	{
		if(!(buffer = new (std::nothrow) GLubyte[savedBufferWidth * savedBufferHeight * 4]))
			break;

		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, savedBufferWidth, savedBufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);

		if ((DWORD)colorKey != 0)
		{
			// make colorKey transparent
			int nSize = savedBufferWidth * savedBufferHeight;
			DWORD* pPixelData = (DWORD*)(buffer);
			DWORD dwTransparentColorKey = colorKey;
			for (int i = 0; i < nSize; ++i, ++pPixelData)
			{
				if (*pPixelData == dwTransparentColorKey)
					*pPixelData = 0;
			}
		}

		// flip is required when saving image to file. 
		if (bFlipImage) 
		{
			GLubyte *tempData = nullptr;
			if (!(tempData = new (std::nothrow) GLubyte[savedBufferWidth * savedBufferHeight * 4]))
			{
				delete[] buffer;
				buffer = nullptr;
				break;
			}

			// to get the actual texture data
			// #640 the image read from render target is dirty
			for (int i = 0; i < savedBufferHeight; ++i)
			{
				memcpy(&tempData[i * savedBufferWidth * 4],
					&buffer[(savedBufferHeight - i - 1) * savedBufferWidth * 4],
					savedBufferWidth * 4);
			}
			image->LoadFromRawData(tempData, savedBufferWidth * savedBufferHeight * 4, savedBufferWidth, savedBufferHeight, 8);
			SAFE_DELETE_ARRAY(tempData);
		}
		else
		{
			image->LoadFromRawData(buffer, savedBufferWidth * savedBufferHeight * 4, savedBufferWidth, savedBufferHeight, 8);
		}

	} while (0);
	
	SAFE_DELETE_ARRAY(buffer);

	return image;
#else
	return nullptr;
#endif
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

#ifdef USE_DIRECTX_RENDERER

#elif defined(USE_OPENGL_RENDERER)
#endif
	return true;
}

bool ParaEngine::CRenderTarget::Begin(int x, int y, int w, int h)
{
	bool ret = this->Begin();

	m_oldViewport.X = x;
	m_oldViewport.Y = y;
	m_oldViewport.Width = w;
	m_oldViewport.Height = h;

	return true;
}

bool ParaEngine::CRenderTarget::Begin()
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	CheckInit();

	// save old viewport
	pd3dDevice->GetViewport(&m_oldViewport);
	// save old render origin
	m_vOldRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

#ifdef USE_DIRECTX_RENDERER
	m_pOldRenderTarget = NULL;
	m_pOldZBuffer = NULL;
	if (m_pCanvasSurface == 0 || m_pDepthStencilSurface == 0)
		return false;
	if (m_pCanvasTexture)
	{
		m_pCanvasTexture->SetHitCount(0);
	}

	m_pOldRenderTarget = CGlobals::GetDirectXEngine().GetRenderTarget();
	CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pCanvasSurface);

	pd3dDevice->SetRenderTarget(1, NULL);
	pd3dDevice->SetRenderTarget(2, NULL);
	pd3dDevice->SetRenderTarget(3, NULL);

	// set depth surface
	if (FAILED(pd3dDevice->GetDepthStencilSurface(&m_pOldZBuffer)))
	{
		OUTPUT_LOG("error: can not get GetDepthStencilSurface in miniscene graph.\n");
		return false;
	}
	pd3dDevice->SetDepthStencilSurface(m_pDepthStencilSurface);

	
#elif defined(USE_OPENGL_RENDERER)
	if (m_pCanvasTexture)
		m_pCanvasTexture->SetHitCount(0);

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);
	
	glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
	// no need to bind depth buffer, since it is automatically bind by opengl when frame buffer is bind. 
	
	pd3dDevice->BeginRenderTarget(m_nTextureWidth, m_nTextureHeight);

	//calculate viewport
	{
		ParaViewport myViewport;
		myViewport.X = 0;
		myViewport.Y = 0;
		myViewport.Width = GetTextureWidth();
		myViewport.Height = GetTextureHeight();
		pd3dDevice->SetViewport((D3DVIEWPORT9*)&myViewport);
	}
#endif

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
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	// restore render origin
	CGlobals::GetScene()->RegenerateRenderOrigin(m_vOldRenderOrigin);
#ifdef USE_DIRECTX_RENDERER
	
	// restore old depth surface
	pd3dDevice->SetDepthStencilSurface(m_pOldZBuffer);
	SAFE_RELEASE(m_pOldZBuffer);
	// Restore the old render target: i.e. the backbuffer
	HRESULT hr = CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pOldRenderTarget);
	PE_ASSERT(hr == D3D_OK);
	m_pOldRenderTarget = NULL;

#elif defined(USE_OPENGL_RENDERER)
	glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);
	pd3dDevice->EndRenderTarget();
	
	// TODO: this should be removed, when we handle glClear() by ourselves instead of cocos. 
	LinearColor color(CGlobals::GetScene()->GetClearColor());
	glClearColor(color.r, color.g, color.b, color.a);
#endif
	// restore viewport
	pd3dDevice->SetViewport(&m_oldViewport);
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
		RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
		pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, (DWORD)color, depthValue, stencilValue);
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
	return S_OK;
}

