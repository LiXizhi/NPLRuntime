//-----------------------------------------------------------------------------
// Class:	CMoviePlatform
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "IMovieCodec.h"
#ifdef USE_FREEIMAGE
#include <FreeImage.h>
#endif
#include "util/CyoEncode.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <thread>
#endif
#include "VertexFVF.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "EffectManager.h"
#include "FrameRateController.h"
#include "ParaEngineSettings.h"
#include "IParaEngineApp.h"
#include "MoviePlatform.h"
#include "util/ParaTime.h"
#include "PluginManager.h"
#include "ViewportManager.h"
#include "PluginAPI.h"
#include "ParaImage.h"
#include "ZipArchive.h"
#include "AudioEngine2.h"


using namespace ParaEngine;

/** the movie codec plugin dll file name. We will try these location in order */
#ifdef _DEBUG
	const char* MOVIE_CODEC_DLL_FILE_PATHS[] = { "Mod/MovieCodecPlugin/MovieCodecPlugin_d.dll",  "Mod/MovieCodecPlugin/MovieCodecPlugin.dll", "MovieCodecPlugin_d.dll" };
#else
	const char* MOVIE_CODEC_DLL_FILE_PATHS[] = { "Mod/MovieCodecPlugin/MovieCodecPlugin.dll", "MovieCodecPlugin.dll" };
#endif

/** the MOVIE_CODEC class interface id. */
#define MovieCodec_CLASS_ID Class_ID(0x2b305a40, 0x47a409cf)
	
CMoviePlatform::CMoviePlatform(void)
	:m_bCaptureGUI(false), m_isLeftEye(true),
	m_pCaptureRT(nullptr), m_nEncodeCodec(CODEC_XVID),
	m_bRenderBorder(true), m_bAllowCodecSelection(false), m_fLastRefreshInterval(-1.f), m_pMovieCodec(NULL), m_nMovieCodecVersion(0), m_nMarginLeft(16), m_nMarginRight(16), m_nMarginTop(16), m_nMarginBottom(16), m_bUseGDI(true)
{
	m_recorderState = RECORDER_STATE_END;
	m_nScreenWidth = 640;
	m_nScreenHeight = 480;
	m_nRecordingFPS = 25;	// 25 FPS
	m_nStereoCaptureMode = MOVIE_CAPTURE_MODE_NORMAL;
	m_nStereoEyeSeparation = 0.06f;
	m_fStereoConvergenceOffset = 0.f;
}

CMoviePlatform::~CMoviePlatform(void)
{
	Cleanup();
}

void CMoviePlatform::Cleanup()
{
	InvalidateDeviceObjects();
	if (m_pMovieCodec != 0)
	{
		m_pMovieCodec->Release();
		m_pMovieCodec = NULL;
	}
}

CMoviePlatform& CMoviePlatform::GetSingleton()
{
	static CMoviePlatform g_singleton;
	return g_singleton;
}


bool CMoviePlatform::IsUseGDI() const
{
	return m_bUseGDI;
}

void CMoviePlatform::SetUseGDI(bool val)
{
	m_bUseGDI = val;
}

IMovieCodec* CMoviePlatform::GetMovieCodec(bool bCreateIfNotExist)
{
	if (m_pMovieCodec)
	{
		return m_pMovieCodec;
	}
	else
	{
		static bool s_bIsLoaded = false;
		if (s_bIsLoaded || !bCreateIfNotExist)
		{
			return m_pMovieCodec;
		}
		else
		{
			s_bIsLoaded = true;

			ParaEngine::DLLPlugInEntity* pPluginEntity = NULL;
			for (int i = 0; m_pMovieCodec == 0 && i < sizeof(MOVIE_CODEC_DLL_FILE_PATHS) / sizeof(const char*); ++i)
			{
				const char* sFilename = MOVIE_CODEC_DLL_FILE_PATHS[i];

				pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(sFilename);

				if (pPluginEntity == 0)
				{
					// load the plug-in if it has never been loaded before. 
					pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", sFilename);
				}

				if (pPluginEntity != 0 && pPluginEntity->IsValid())
				{
					m_nMovieCodecVersion = pPluginEntity->GetLibVersion();
					if (m_nMovieCodecVersion >= 3)
					{
						for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
						{
							ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

							if (pClassDesc && pClassDesc->ClassID() == MovieCodec_CLASS_ID)
							{
								m_pMovieCodec = (IMovieCodec*)pClassDesc->Create();
							}
						}
					}
					else
					{
						OUTPUT_LOG("movie codec require at least version 3 but you only have version %d\n", pPluginEntity->GetLibVersion());
						CGlobals::GetApp()->SystemMessageBox("MovieCodec plugin needs at least version 3. Please update from official website!");
					}
				}
			}
		}
		return m_pMovieCodec;
	}
}

void CMoviePlatform::SetRecordingFPS( int nFPS )
{
	if(nFPS>0 && nFPS<1000)
	{
		m_nRecordingFPS = nFPS;
	}
	else
	{
		OUTPUT_LOG("warning: setting recording FPS out of valid range. \n");
	}
}

void CMoviePlatform::SetStereoCaptureMode( MOVIE_CAPTURE_MODE nMode )
{

	m_nStereoCaptureMode = nMode;
	if (nMode == MOVIE_CAPTURE_MODE_STEREO_LEFT_RIGHT)
	{
		CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_LEFT_RIGHT);
	}
	else if (nMode == MOVIE_CAPTURE_MODE_STEREO_RED_BLUE)
	{
		CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_RED_BLUE);
	}
	else if (nMode == MOVIE_CAPTURE_MODE_STEREO_OMNI)
	{
		CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_OMNI);
	}
	else if (nMode == MOVIE_CAPTURE_MODE_STEREO_OMNI_SINGLE_EYE)
	{
		CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE);
	}
	else if (nMode == MOVIE_CAPTURE_MODE_STEREO_OMNI_SINGLE_EYE_1)
	{
		CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE_1);
	}
	else
	{
		CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_DEFAULT);
	}
}
	
MOVIE_CAPTURE_MODE CMoviePlatform::GetStereoCaptureMode()
{
	return m_nStereoCaptureMode;
}

int CMoviePlatform::GetRecordingFPS()
{
	return m_nRecordingFPS;
}

void CMoviePlatform::SetStereoEyeSeparation( float fDist )
{
	m_nStereoEyeSeparation = fDist;
}

float CMoviePlatform::GetStereoEyeSeparation()
{
	return m_nStereoEyeSeparation;
}

void ParaEngine::CMoviePlatform::SetStereoConvergenceOffset(float fDist)
{
	m_fStereoConvergenceOffset = fDist;
}

float ParaEngine::CMoviePlatform::GetStereoConvergenceOffset()
{
	return m_fStereoConvergenceOffset;
}

void CMoviePlatform::GetMovieScreenSize(int* nWidth, int* nHeight)
{
	if(nWidth!=0)
		*nWidth = m_nScreenWidth;
	if(nHeight!=0)
		*nHeight = m_nScreenHeight;
}

string CMoviePlatform::GetMovieFileName()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
	{
		// use ffmpeg external dll
		return pMovieCodec->GetFileName();
	}
	return m_sMovieName;
}

bool CMoviePlatform::GetImageInfo(const string& filename, int* width, int* height, int* nFileSize)
{
	CParaFile file;
	if (!file.OpenFile(filename.c_str()))
	{
		OUTPUT_LOG("warning:unable to open file when doing GetImageInfo, %s\n", filename.c_str());
		return false;
	}

	ParaImage img;
	if (!img.initWithImageData((unsigned char*)file.getBuffer(), file.getSize()))
	{
		OUTPUT_LOG("warning:unable to load image when doing GetImageInfo, %s\n", filename.c_str());
		return false;
	}
	if (nFileSize)
		*nFileSize = (int)(file.getSize());
	if (width)
		*width = (int)img.getWidth();
	if (height)
		*height = (int)img.getHeight();
	return true;
}

bool CMoviePlatform::ResizeImage(const string& filename, int width, int height, const string& destFilename)
{
	CParaFile file;
	if (!file.OpenFile(filename.c_str()))
	{
		OUTPUT_LOG("warning:unable to open file when doing ResizeImage, %s\n", filename.c_str());
		return false;
	}

	ParaImage img;
	if (!img.initWithImageData((unsigned char*)file.getBuffer(), file.getSize()))
	{
		OUTPUT_LOG("warning:unable to load image when doing ResizeImage, %s\n", filename.c_str());
		return false;
	}

	if (!img.resize(width, height))
	{
		OUTPUT_LOG("warning:unable to resize image, %s\n", filename.c_str());
		return false;
	}

	std::string dest = destFilename;
	if (!CParaFile::IsAbsolutePath(dest))
	{
		dest = CParaFile::GetWritablePath() + dest;
	}
	return img.saveToFile(dest);
}

bool CMoviePlatform::TakeScreenShot(const string& filename, int width, int height, bool isCenter)
{
	auto* pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice)
		return false;

	int backWidth = static_cast<int>(pRenderDevice->GetBackBufferWidth());
	int backHeight = static_cast<int>(pRenderDevice->GetBackBufferHeight());
	if ((backWidth <= 0 || backHeight <= 0) && CGlobals::GetApp() && CGlobals::GetApp()->GetRenderWindow())
	{
		backWidth = static_cast<int>(CGlobals::GetApp()->GetRenderWindow()->GetWidth());
		backHeight = static_cast<int>(CGlobals::GetApp()->GetRenderWindow()->GetHeight());
	}
	if (backWidth <= 0 || backHeight <= 0)
		return false;

	int captureWidth = (width <= 0) ? backWidth : Math::Min(width, backWidth);
	int captureHeight = (height <= 0) ? backHeight : Math::Min(height, backHeight);

	std::string outFilename = filename;
	if (outFilename.empty())
	{
		char validFilename[256] = {};
		std::string date_str = ParaEngine::GetDateFormat("MMM dd yy");
		snprintf(validFilename, 255, "Screen Shots\\ParaEngine_%s_%s.jpg", date_str.c_str(), ParaEngine::GetTimeFormat("hh'H'mm'M'ss tt").c_str());
		outFilename = validFilename;
	}

	if (!CParaFile::IsAbsolutePath(outFilename))
	{
		outFilename = CParaFile::GetWritablePath() + outFilename;
	}

	if (!CParaFile::CreateDirectory(outFilename.c_str()))
		return false;

	int startX = isCenter ? Math::Max(0, (backWidth - captureWidth) / 2) : 0;
	int startY = isCenter ? Math::Max(0, (backHeight - captureHeight) / 2) : 0;

	std::vector<uint32_t> pixels(static_cast<size_t>(captureWidth) * static_cast<size_t>(captureHeight), 0u);
	if (!pRenderDevice->ReadPixels(startX, startY, captureWidth, captureHeight, pixels.data()))
		return false;

	const uint32_t* rawPixels = pixels.data();
	std::vector<uint32_t> flippedPixels;
	if (!pRenderDevice->GetCaps().topLeftScreenOrigin && captureHeight > 1)
	{
		flippedPixels.resize(pixels.size());
		for (int row = 0; row < captureHeight; ++row)
		{
			memcpy(&flippedPixels[static_cast<size_t>(captureWidth) * row],
				&pixels[static_cast<size_t>(captureWidth) * (captureHeight - row - 1)],
				static_cast<size_t>(captureWidth) * sizeof(uint32_t));
		}
		rawPixels = flippedPixels.data();
	}

	ParaImage img;
	if (!img.initWithRawData(reinterpret_cast<const unsigned char*>(rawPixels), pixels.size() * sizeof(uint32_t), captureWidth, captureHeight, 32))
		return false;

	return img.saveToFile(outFilename);
}

bool CMoviePlatform::TakeScreenShot(const string& filename, int width, int height)
{
	if (CMoviePlatform::TakeScreenShot(filename))
	{
		if (width == 0 || height == 0)
			return true;
		return ResizeImage(filename, width, height, filename);
	}
	return false;
}

bool CMoviePlatform::TakeScreenShot(const string& filename)
{
	return TakeScreenShot(filename, 0, 0, false);
}

void CMoviePlatform::TakeScreenShot_Async(const string& filename, bool bEncode, int width, int height, screenshot_callback callback)
{
#if defined(WIN32) && !defined(EMSCRIPTEN_SINGLE_THREAD)
	std::thread worker([=]() {
		std::vector<BYTE> buffers;
		bool result = TakeScreenShot_FromGDI(filename, buffers, bEncode, width, height);
		if (callback != nullptr)
		{
			callback(result, buffers);
		}
	});
	worker.detach();
#else
	std::vector<BYTE> buffers;
	bool result = TakeScreenShot(filename, width > 0 ? width : 0, height > 0 ? height : 0);
	if (result && bEncode)
	{
		CParaFile file;
		if (file.OpenFile(filename.c_str()))
		{
			size_t nBufferSize = CyoEncode::Base64EncodeGetLength(file.getSize());
			buffers.resize(nBufferSize);
			CyoEncode::Base64Encode(buffers.data(), file.getBuffer(), file.getSize());
		}
	}
	if (callback != nullptr)
	{
		callback(result, buffers);
	}
#endif
}

bool CMoviePlatform::TakeScreenShot_FromGDI(const string& filename, std::vector<BYTE>& outBase64Buffers, bool bEncode, int width, int height)
{
#if defined(WIN32)
	auto* pRenderDevice = CGlobals::GetRenderDevice();
	int wndWidth = pRenderDevice ? static_cast<int>(pRenderDevice->GetBackBufferWidth()) : 0;
	int wndHeight = pRenderDevice ? static_cast<int>(pRenderDevice->GetBackBufferHeight()) : 0;
	if ((wndWidth <= 0 || wndHeight <= 0) && CGlobals::GetAppHWND())
	{
		RECT rc = {};
		if (::GetClientRect(CGlobals::GetAppHWND(), &rc))
		{
			wndWidth = rc.right - rc.left;
			wndHeight = rc.bottom - rc.top;
		}
	}
	if (wndWidth <= 0 || wndHeight <= 0)
		return false;

	if (width <= 0 || height <= 0)
	{
		width = wndWidth;
		height = wndHeight;
	}

	string outFilename = filename;
	if (outFilename.empty())
	{
		char validFilename[256] = {};
		std::string date_str = ParaEngine::GetDateFormat("MMM dd yy");
		snprintf(validFilename, 255, "Screen Shots\\ParaEngine_%s_%s.jpg", date_str.c_str(), ParaEngine::GetTimeFormat("hh'H'mm'M'ss tt").c_str());
		outFilename = validFilename;
	}

	if (!CParaFile::IsAbsolutePath(outFilename))
	{
		outFilename = CParaFile::GetWritablePath() + outFilename;
	}

	std::vector<BYTE> buffers;
	int fileSize = 0;
	int infoSize = 0;
	if (CaptureBitmapBuffer(CGlobals::GetAppHWND(), fileSize, infoSize, buffers, false, 0, 0, wndWidth, wndHeight) != 0 || buffers.empty())
		return false;

#ifdef USE_FREEIMAGE
	string sExt = CParaFile::GetFileExtension(outFilename);
	FREE_IMAGE_FORMAT FileFormat = FIF_PNG;
	if (sExt == "dds")
		FileFormat = FIF_DDS;
	else if (sExt == "jpg")
		FileFormat = FIF_JPEG;
	else if (sExt == "bmp")
		FileFormat = FIF_BMP;
	else if (sExt == "tga")
		FileFormat = FIF_TARGA;

	FIMEMORY* sourceMemBuff = FreeImage_OpenMemory(buffers.data(), static_cast<DWORD>(buffers.size()));
	FIBITMAP* sourceImage = sourceMemBuff ? FreeImage_LoadFromMemory(FIF_BMP, sourceMemBuff) : nullptr;
	if (!sourceImage)
	{
		if (sourceMemBuff)
			FreeImage_CloseMemory(sourceMemBuff);
		return false;
	}
	if (width != wndWidth || height != wndHeight)
	{
		FIBITMAP* rescaled = FreeImage_Rescale(sourceImage, width, height, FILTER_BOX);
		FreeImage_Unload(sourceImage);
		sourceImage = rescaled;
		if (!sourceImage)
		{
			FreeImage_CloseMemory(sourceMemBuff);
			return false;
		}
	}
	FIMEMORY* destMemBuff = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FileFormat, sourceImage, destMemBuff);
	FreeImage_Unload(sourceImage);

	BYTE* mem_buffer = NULL;
	DWORD size_in_bytes = 0;
	FreeImage_AcquireMemory(destMemBuff, &mem_buffer, &size_in_bytes);

	if (bEncode && mem_buffer && size_in_bytes > 0)
	{
		size_t nBufferSize = CyoEncode::Base64EncodeGetLength(size_in_bytes);
		outBase64Buffers.resize(nBufferSize);
		CyoEncode::Base64Encode(outBase64Buffers.data(), mem_buffer, size_in_bytes);
	}

	bool bSaved = false;
	CParaFile::CreateDirectory(outFilename.c_str());
	CParaFile file;
	if (mem_buffer && size_in_bytes > 0 && file.CreateNewFile(outFilename.c_str()))
	{
		file.write(mem_buffer, size_in_bytes);
		file.close();
		bSaved = true;
	}
	FreeImage_CloseMemory(sourceMemBuff);
	FreeImage_CloseMemory(destMemBuff);
	return bSaved;
#else
	ParaImage img;
	if (!img.initWithImageData(buffers.data(), buffers.size()))
		return false;
	if ((width != wndWidth || height != wndHeight) && !img.resize(width, height))
		return false;
	if (!CParaFile::CreateDirectory(outFilename.c_str()) || !img.saveToFile(outFilename))
		return false;
	if (bEncode)
	{
		CParaFile savedFile;
		if (savedFile.OpenFile(outFilename.c_str()))
		{
			size_t nBufferSize = CyoEncode::Base64EncodeGetLength(savedFile.getSize());
			outBase64Buffers.resize(nBufferSize);
			CyoEncode::Base64Encode(outBase64Buffers.data(), savedFile.getBuffer(), savedFile.getSize());
		}
	}
	return true;
#endif
#else
	return false;
#endif
}

int CMoviePlatform::CaptureBitmapBuffer(HWND nHwnd, int& outFileHeaderSize, int& outInfoHeaderSize, std::vector<BYTE>& outBuffers, bool bCaptureMouse, int nLeft /*= 0*/, int nTop /*= 0*/, int width /*= 0*/, int height /*= 0*/)
{
#ifdef WIN32
	if (!nHwnd || width <= 0 || height <= 0)
		return -1;

	HDC SourceHDC = ::GetDC(nHwnd);
	if (!SourceHDC)
		return -1;
	HDC compatibleHDC = ::CreateCompatibleDC(SourceHDC);
	if (!compatibleHDC)
	{
		::ReleaseDC(nHwnd, SourceHDC);
		return -1;
	}

	BITMAP Bitmap = {};
	HBITMAP bitmapHandle = ::CreateCompatibleBitmap(SourceHDC, width, height);
	HGDIOBJ oldObj = bitmapHandle ? ::SelectObject(compatibleHDC, bitmapHandle) : NULL;
	if (bitmapHandle == 0 || !oldObj)
	{
		OUTPUT_LOG("error: can not select bitmap in MovieCodec::TakeScreenshot()\n");
		if (bitmapHandle)
			::DeleteObject(bitmapHandle);
		::DeleteDC(compatibleHDC);
		::ReleaseDC(nHwnd, SourceHDC);
		return -1;
	}

	if (!::BitBlt(compatibleHDC, 0, 0, width, height, SourceHDC, nLeft, nTop, SRCCOPY))
	{
		OUTPUT_LOG("error: bitblt failed with code %d in MovieCodec::TakeScreenshot()\n", GetLastError());
		::SelectObject(compatibleHDC, oldObj);
		::DeleteObject(bitmapHandle);
		::DeleteDC(compatibleHDC);
		::ReleaseDC(nHwnd, SourceHDC);
		return -1;
	}

	if (bCaptureMouse)
	{
		CURSORINFO cursorinfo = {};
		cursorinfo.cbSize = sizeof(CURSORINFO);
		if (::GetCursorInfo(&cursorinfo) && cursorinfo.hCursor)
		{
			ICONINFO iconinfo = {};
			if (::GetIconInfo(cursorinfo.hCursor, &iconinfo))
			{
				::ScreenToClient(nHwnd, &cursorinfo.ptScreenPos);
				::DrawIcon(compatibleHDC, cursorinfo.ptScreenPos.x - iconinfo.xHotspot - nLeft, cursorinfo.ptScreenPos.y - iconinfo.yHotspot - nTop, cursorinfo.hCursor);
				if (iconinfo.hbmColor)
					::DeleteObject(iconinfo.hbmColor);
				if (iconinfo.hbmMask)
					::DeleteObject(iconinfo.hbmMask);
			}
		}
	}

	::GetObject(bitmapHandle, sizeof(BITMAP), &Bitmap);

	outFileHeaderSize = sizeof(BITMAPFILEHEADER);
	outInfoHeaderSize = sizeof(BITMAPINFOHEADER);
	int rowPitch = ((Bitmap.bmWidth * 3 + 3) & ~3);
	unsigned long BitmapSize = outFileHeaderSize + outInfoHeaderSize + (rowPitch * Bitmap.bmHeight);

	outBuffers.assign(BitmapSize, 0);
	BYTE* MemoryHandle = outBuffers.data();

	LPBITMAPFILEHEADER fileHeader = reinterpret_cast<LPBITMAPFILEHEADER>(MemoryHandle);
	fileHeader->bfType = 0x4d42;
	fileHeader->bfSize = BitmapSize;
	fileHeader->bfReserved1 = 0;
	fileHeader->bfReserved2 = 0;
	fileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	LPBITMAPINFOHEADER pBmpInfo = reinterpret_cast<LPBITMAPINFOHEADER>(&outBuffers[outFileHeaderSize]);
	pBmpInfo->biSize = sizeof(BITMAPINFOHEADER);
	pBmpInfo->biWidth = Bitmap.bmWidth;
	pBmpInfo->biHeight = Bitmap.bmHeight;
	pBmpInfo->biPlanes = 1;
	pBmpInfo->biBitCount = 24;
	pBmpInfo->biCompression = BI_RGB;
	pBmpInfo->biSizeImage = rowPitch * Bitmap.bmHeight;

	::GetDIBits(compatibleHDC, bitmapHandle, 0, Bitmap.bmHeight, &outBuffers[outFileHeaderSize + outInfoHeaderSize], reinterpret_cast<LPBITMAPINFO>(pBmpInfo), DIB_RGB_COLORS);

	::SelectObject(compatibleHDC, oldObj);
	::DeleteObject(bitmapHandle);
	::DeleteDC(compatibleHDC);
	::ReleaseDC(nHwnd, SourceHDC);
	return 0;
#else
	return -1;
#endif
}

HRESULT CMoviePlatform::InvalidateDeviceObjects()
{
	ReleaseRHIResource(m_pCaptureRT);
	m_captureBuffer.clear();
	return S_OK;
}

HRESULT CMoviePlatform::RestoreDeviceObjects()
{
	if (IsInCaptureSession())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (!pMovieCodec)
		{
			auto* pRenderDevice = CGlobals::GetRenderDevice();
			if (pRenderDevice && GetScreenWidth() > 0 && GetScreenHeight() > 0)
			{
				ReleaseRHIResource(m_pCaptureRT);
				m_pCaptureRT = pRenderDevice->CreateRenderTarget(
					static_cast<uint32_t>(GetScreenWidth()),
					static_cast<uint32_t>(GetScreenHeight()),
					PixelFormat::A8R8G8B8,
					PixelFormat::D24S8);
				if (!m_pCaptureRT)
					return E_FAIL;
			}
		}
	}
	return S_OK;
}

bool CMoviePlatform::SetCaptureTarget()
{
	if (IsInCaptureSession())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
			return false;
		auto* pRenderDevice = CGlobals::GetRenderDevice();
		if (pRenderDevice && m_pCaptureRT)
		{
			if (!pRenderDevice->PushRenderTarget(m_pCaptureRT))
				return false;
			pRenderDevice->BeginRenderTarget(m_pCaptureRT);
			return true;
		}
	}
	return false;
}

bool CMoviePlatform::UnsetCaptureTarget()
{
	auto* pRenderDevice = CGlobals::GetRenderDevice();
	if (IsInCaptureSession() && pRenderDevice && m_pCaptureRT)
	{
		pRenderDevice->EndRenderTarget();
		return pRenderDevice->PopRenderTarget();
	}
	return false;
}

void CMoviePlatform::RenderCaptured()
{
	auto* pd3dDevice = CGlobals::GetRenderDevice();
	if (pd3dDevice && IsInCaptureSession() && m_pCaptureRT && m_pCaptureRT->GetColorTexture())
	{
		CGlobals::GetEffectManager()->BeginEffect(TECH_GUI);
		float sx, sy;
		if (CaptureGUI())
		{
			sx = (float)GetScreenWidth();
			sy = (float)GetScreenHeight();
		}
		else
		{
			ParaViewport curViewport = pd3dDevice->GetViewport();
			sx = (float)curViewport.Width;
			sy = (float)curViewport.Height;
		}

		DXUT_SCREEN_VERTEX v[10];
		v[0].x = 0;  v[0].y = sy; v[0].tu = 0;   v[0].tv = 1.0f;
		v[1].x = 0;  v[1].y = 0;  v[1].tu = 0;   v[1].tv = 0.0f;
		v[2].x = sx; v[2].y = sy; v[2].tu = 1.f; v[2].tv = 1.0f;
		v[3].x = sx; v[3].y = 0;  v[3].tv = 0.f; v[3].tu = 1.0f;
		if (!pd3dDevice->GetCaps().topLeftScreenOrigin)
		{
			v[0].tv = 0.0f; v[1].tv = 1.0f; v[2].tv = 0.0f; v[3].tv = 1.0f;
		}

		DWORD dwColor = LinearColor(1.f, 1.f, 1.f, 1.f);
		for (int i = 0; i < 4; ++i)
		{
			v[i].color = dwColor;
			v[i].z = 0;
			v[i].h = 1.0f;
		}

		pd3dDevice->SetFVF(DXUT_SCREEN_VERTEX::FVF);
		pd3dDevice->SetTexture(0, m_pCaptureRT->GetColorTexture());
		pd3dDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, v, sizeof(DXUT_SCREEN_VERTEX));

		if (m_bRenderBorder)
		{
			float bBorderSize = 7.f;
			v[0].x = 0;              v[0].y = 0;
			v[1].x = bBorderSize;    v[1].y = bBorderSize;
			v[2].x = sx;             v[2].y = 0;
			v[3].x = sx - bBorderSize; v[3].y = bBorderSize;
			v[4].x = sx;             v[4].y = sy;
			v[5].x = sx - bBorderSize; v[5].y = sy - bBorderSize;
			v[6].x = 0;              v[6].y = sy;
			v[7].x = bBorderSize;    v[7].y = sy - bBorderSize;
			v[8].x = v[0].x;         v[8].y = v[0].y;
			v[9].x = v[1].x;         v[9].y = v[1].y;

			dwColor = IsRecording() ? LinearColor(1.0f, 0.f, 0.0f, 1.f) : LinearColor(0.f, 1.f, 1.0f, 1.f);
			for (int i = 0; i < 10; ++i)
			{
				v[i].tu = 0; v[i].tv = 0;
				v[i].color = dwColor;
				v[i].z = 0;
				v[i].h = 1.0f;
			}
			pd3dDevice->SetTexture(0, nullptr);
			CGlobals::GetEffectManager()->SetCullingMode(false);
			pd3dDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 8, v, sizeof(DXUT_SCREEN_VERTEX));
			CGlobals::GetEffectManager()->SetCullingMode(true);
		}
		CGlobals::GetEffectManager()->EndEffect();
	}
}

void CMoviePlatform::CleanUp()
{
}

void CMoviePlatform::SetMovieScreenSize(int nWidth, int nHeight)
{
	if(!IsInCaptureSession())
	{
		// must be multiple of 4
		m_nScreenWidth = nWidth - nWidth%4;
		m_nScreenHeight = nHeight - nHeight%4;
	}
}

bool CMoviePlatform::IsInCaptureSession()
{
	return (m_recorderState != RECORDER_STATE_END);
}

bool CMoviePlatform::IsRecording()
{
	return (m_recorderState == RECORDER_STATE_BEGIN);
}

void CMoviePlatform::PauseCapture()
{
	if(IsInCaptureSession())
	{
		m_recorderState = RECORDER_STATE_PAUSED;
	}
}
void CMoviePlatform::ResumeCapture()
{
	if(IsInCaptureSession())
	{
		m_recorderState = RECORDER_STATE_BEGIN;
	}
}

int CMoviePlatform::GetCurrentFrameNumber()
{
	if (IsInCaptureSession())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			return pMovieCodec->GetCurrentFrameNumber();
		}
	}
	return 0;
}

bool CMoviePlatform::BeginCapture(const string& sFileName)
{
	if (IsInCaptureSession())
		return true;
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (!pMovieCodec)
		return false;

	auto* pRenderDevice = CGlobals::GetRenderDevice();
	if (pRenderDevice && pRenderDevice->GetBackBufferWidth() > 0 && pRenderDevice->GetBackBufferHeight() > 0)
	{
		m_nScreenWidth = static_cast<int>(pRenderDevice->GetBackBufferWidth());
		m_nScreenHeight = static_cast<int>(pRenderDevice->GetBackBufferHeight());
	}

	InvalidateDeviceObjects();
	m_recorderState = RECORDER_STATE_BEGIN;
	RestoreDeviceObjects();

	bool bAllowCodecSelection = IsCodecSelectionEnabled();
	if (bAllowCodecSelection)
	{
		CGlobals::GetSettings().SetWindowedMode(true);
	}

	((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->EnableStereoVision(GetStereoCaptureMode() != MOVIE_CAPTURE_MODE_NORMAL);

	string fileName;
	if (sFileName.empty())
		fileName = "paraengine.avi";
	else if (sFileName.size() > 4 && sFileName[sFileName.size() - 4] == '.')
		fileName = sFileName;
	else
		fileName = sFileName + ".avi";
	m_sMovieName = fileName;

	int captureTexWidth = GetScreenWidth();
	int captureTexHeight = GetScreenHeight();
#ifdef PLATFORM_WINDOWS
	if (pMovieCodec->BeginCapture(sFileName.c_str(), CGlobals::GetAppHWND(), MarginLeft(), MarginTop(), captureTexWidth - MarginLeft() - MarginRight(), captureTexHeight - MarginTop() - MarginBottom(), m_nRecordingFPS) != 0)
	{
		m_recorderState = RECORDER_STATE_END;
		InvalidateDeviceObjects();
		return false;
	}
	ParaEngine::CAudioEngine2::GetInstance()->SetHistoryWithCaptureBegin();
#endif

	m_fLastRefreshInterval = CGlobals::GetApp()->GetRefreshTimer();
	int nRefreshFPS = m_nRecordingFPS;
	CGlobals::GetApp()->SetRefreshTimer(1.f / nRefreshFPS);

	return true;
}

void CMoviePlatform::FrameCaptureDX(IMovieCodec* pMovieCodec)
{
	auto* pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice || !pMovieCodec)
		return;

	int captureTexWidth = GetScreenWidth();
	int captureTexHeight = GetScreenHeight();
	int nMarginLeft = MarginLeft();
	int nMarginTop = MarginTop();
	int nWidth = pMovieCodec->GetWidth();
	int nHeight = pMovieCodec->GetHeight();
	if (nWidth <= 0 || nHeight <= 0 || (nMarginLeft + nWidth) > captureTexWidth || (nMarginTop + nHeight) > captureTexHeight)
	{
		EndCapture();
		return;
	}

	size_t rgbaSize = static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 4u;
	size_t bgrSize = static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 3u;
	if (m_captureBuffer.size() < rgbaSize + bgrSize)
		m_captureBuffer.resize(rgbaSize + bgrSize);

	uint8_t* rgbaBuf = m_captureBuffer.data();
	uint8_t* bgrBuf = m_captureBuffer.data() + rgbaSize;

	if (!pRenderDevice->ReadPixels(nMarginLeft, nMarginTop, nWidth, nHeight, rgbaBuf))
		return;

	const bool flipY = !pRenderDevice->GetCaps().topLeftScreenOrigin;
	for (int row = 0; row < nHeight; ++row)
	{
		int srcRow = flipY ? (nHeight - 1 - row) : row;
		const uint8_t* pSrc = rgbaBuf + static_cast<size_t>(srcRow) * static_cast<size_t>(nWidth) * 4u;
		uint8_t* pDst = bgrBuf + static_cast<size_t>(row) * static_cast<size_t>(nWidth) * 3u;
		for (int col = 0; col < nWidth; ++col)
		{
			pDst[0] = pSrc[2]; // B
			pDst[1] = pSrc[1]; // G
			pDst[2] = pSrc[0]; // R
			pSrc += 4;
			pDst += 3;
		}
	}

	pMovieCodec->FrameCapture(bgrBuf, static_cast<int>(bgrSize));
}

void CMoviePlatform::FrameCaptureFFMPEG()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (!pMovieCodec)
	{
		EndCapture();
		return;
	}
	if (!IsUseGDI())
	{
		FrameCaptureDX(pMovieCodec);
	}
}

void CMoviePlatform::SaveCapturedFrame(int nMode)
{
	if (IsRecording())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			FrameCaptureFFMPEG();
		}
	}
}

bool CMoviePlatform::EndCapture()
{
	if (m_recorderState != RECORDER_STATE_END)
	{
		((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->EnableStereoVision(false);

		m_recorderState = RECORDER_STATE_END;

		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			std::string audioMapString;
			ParaEngine::CAudioEngine2::CAudioPlaybackHistory& playbackHistory = ParaEngine::CAudioEngine2::GetInstance()->SetHistoryWithCaptureEnd();
			const ParaEngine::CAudioEngine2::CAudioPlaybackHistory::Records& records = playbackHistory.GetRecords();
			for (auto iter = records.begin(); iter != records.end(); ++iter)
			{
				char record[512];
				snprintf(record, sizeof(record), "%s,%d,%d,%d,%d,%d,",
					iter->m_WaveFileName.c_str(),
					iter->m_nStartTime,
					iter->m_nEndTime,
					iter->m_nSeekPos,
					(int)iter->m_bIsLoop,
					iter->m_mTotalTime);
				audioMapString += record;
			}
			if (!audioMapString.empty())
				audioMapString.pop_back();

			if (m_nMovieCodecVersion >= 10)
				pMovieCodec->EndCapture2(audioMapString.c_str());
			else
				pMovieCodec->EndCapture(audioMapString);

			playbackHistory.Clear();
			playbackHistory.SetEnable(false);
		}

		InvalidateDeviceObjects();
		CGlobals::GetApp()->SetRefreshTimer(m_fLastRefreshInterval);
	}
	return true;
}

void CMoviePlatform::EnableCodecSelection(bool bEnable)
{
	m_bAllowCodecSelection = bEnable;
}

bool CMoviePlatform::IsCodecSelectionEnabled()
{
	return m_bAllowCodecSelection;
}

void CMoviePlatform::video_encode_example(const char *filename, int codec_id)
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
	{
		pMovieCodec->video_encode_example(filename, codec_id != 0 ? codec_id : pMovieCodec->GetCodec());
	}
}

void CMoviePlatform::SetCodec(int nCodec)
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
	{
		pMovieCodec->SetCodec(nCodec);
	}
}

int CMoviePlatform::GetCodec()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
	{
		return pMovieCodec->GetCodec();
	}
	return 0;
}

bool CMoviePlatform::HasMoviePlugin()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	return (pMovieCodec != 0);
}

int CMoviePlatform::GetVideoBitRate()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
		return pMovieCodec->GetVideoBitRate();
	else
		return 0;
}

void CMoviePlatform::SetVideoBitRate(int nRate)
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
		pMovieCodec->SetVideoBitRate(nRate);
}

void CMoviePlatform::SetCaptureAudio(bool bEnable)
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
		pMovieCodec->SetCaptureAudio(bEnable);
}

bool CMoviePlatform::IsCaptureAudio()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
		return pMovieCodec->IsCaptureAudio();
	else
		return false;
}

void CMoviePlatform::SetCaptureMic(bool bEnable)
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
		pMovieCodec->SetCaptureMic(bEnable);
}

bool CMoviePlatform::IsCaptureMic()
{
	IMovieCodec* pMovieCodec = GetMovieCodec();
	if (pMovieCodec)
		return pMovieCodec->IsCaptureMic();
	else
		return false;
}

void CMoviePlatform::BeginCaptureFrame()
{
}

void CMoviePlatform::EndCaptureFrame()
{
	if (IsInCaptureSession() && IsRecording())
	{
		SaveCapturedFrame();
	}
}


int CMoviePlatform::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	pClass->AddField("codec", FieldType_Int, (void*)SetCodec_s, (void*)GetCodec_s, NULL, NULL, bOverride);
	pClass->AddField("video_encode_example", FieldType_String, (void*)video_encode_example_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("HasMoviePlugin", FieldType_Bool, NULL, (void*)HasMoviePlugin_s, NULL, NULL, bOverride);

	pClass->AddField("MarginLeft", FieldType_Int, (void*)SetMarginLeft_s, (void*)GetMarginLeft_s, NULL, NULL, bOverride);
	pClass->AddField("MarginTop", FieldType_Int, (void*)SetMarginTop_s, (void*)GetMarginTop_s, NULL, NULL, bOverride);
	pClass->AddField("MarginRight", FieldType_Int, (void*)SetMarginRight_s, (void*)GetMarginRight_s, NULL, NULL, bOverride);
	pClass->AddField("MarginBottom", FieldType_Int, (void*)SetMarginBottom_s, (void*)GetMarginBottom_s, NULL, NULL, bOverride);

	pClass->AddField("RecordingFPS", FieldType_Int, (void*)SetRecordingFPS_s,(void*)GetRecordingFPS_s, NULL, NULL, bOverride);
	pClass->AddField("VideoBitRate", FieldType_Int, (void*)SetVideoBitRate_s,(void*)GetVideoBitRate_s, NULL, NULL, bOverride);

	pClass->AddField("CaptureMic", FieldType_Bool, (void*)SetCaptureMic_s, (void*)IsCaptureMic_s, NULL, NULL, bOverride);
	pClass->AddField("CaptureAudio", FieldType_Bool, (void*)SetCaptureAudio_s, (void*)IsCaptureAudio_s, NULL, NULL, bOverride);

	pClass->AddField("StereoCaptureMode", FieldType_Int, (void*)SetStereoCaptureMode_s, (void*)GetStereoCaptureMode_s, NULL, NULL, bOverride);
	pClass->AddField("StereoEyeSeparation", FieldType_Float, (void*)SetStereoEyeSeparation_s, (void*)GetStereoEyeSeparation_s, NULL, NULL, bOverride);
	pClass->AddField("StereoConvergenceOffset", FieldType_Float, (void*)SetStereoConvergenceOffset_s, (void*)GetStereoConvergenceOffset_s, NULL, NULL, bOverride);

	return S_OK;
}

