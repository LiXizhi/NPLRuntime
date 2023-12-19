//-----------------------------------------------------------------------------
// Class:	CMoviePlatform
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "IMovieCodec.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#include "ScreenShotSystem.h"
#ifdef USE_FREEIMAGE
#include <FreeImage.h>
#endif
#include "util/CyoEncode.h"
using namespace ScreenShot;
#endif
#include <thread>
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
#include "AudioEngine2.h"
#include "StringHelper.h"

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
	:m_bCaptureGUI(false), m_isLeftEye(true), m_nMovieCodecVersion(0),
#ifdef USE_DIRECTX_RENDERER
m_pCaptureTexture(NULL), m_pCaptureSurface(NULL), m_pBackBufferSurface(NULL), m_pDepthStencilSurface(NULL), m_pOldDepthStencilSurface(NULL), m_pOffScreenSurface(NULL), m_CompatibleHDC(NULL), m_BitmapHandle(NULL),
#endif
m_bRenderBorder(true), m_bAllowCodecSelection(false), m_fLastRefreshInterval(-1.f), m_pMovieCodec(NULL), m_nMarginLeft(16), m_nMarginRight(16), m_nMarginTop(16), m_nMarginBottom(16), m_bUseGDI(true)
{
	m_recorderState = RECORDER_STATE_END;
	m_nScreenWidth = 640;
	m_nScreenHeight = 480;
	m_nRecordingFPS = 25;	// 25 FPS
	m_nStereoCaptureMode = MOVIE_CAPTURE_MODE_NORMAL;
	m_nStereoEyeSeparation = 0.06f;
}

CMoviePlatform::~CMoviePlatform(void)
{
	Cleanup();
}

void CMoviePlatform::Cleanup()
{
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
					if (m_nMovieCodecVersion >= 9)
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
						OUTPUT_LOG("movie codec require at least version 9 but you only have version %d\n", m_nMovieCodecVersion);
						CGlobals::GetApp()->SystemMessageBox("MovieCodec plugin needs at least version 9. Please update from official website!");
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
	else
	{
#ifdef USE_DIRECTX_RENDERER
		return GSSHOTSYSTEM->GetVideoFileName();
#endif
	}
	return "";
}

bool CMoviePlatform::GetImageInfo(const string& filename, int* width, int* height, int* nFileSize)
{
#ifdef USE_DIRECTX_RENDERER
	CParaFile file(filename.c_str());
	if(!file.isEof())
	{
		D3DXIMAGE_INFO SrcInfo;
		
		if(SUCCEEDED(D3DXGetImageInfoFromFileInMemory(file.getBuffer(), (int)file.getSize(), &SrcInfo)))
		{
			if(nFileSize){
				*nFileSize = (int)(file.getSize());
			}
			if(width){
				*width = SrcInfo.Width;
			}
			if(height){
				*height = SrcInfo.Height;
			}
			return true;
		}
	}
#endif
	return false;
}

bool CMoviePlatform::ResizeImage(const string& filename, int width, int height, const string& destFilename)
{
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DTEXTURE9 pTexture = NULL;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	std::wstring filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
	HRESULT hr = D3DXCreateTextureFromFileExW(CGlobals::GetRenderDevice(), filename16.c_str(), width, height, D3DX_FROM_FILE, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, (LPDIRECT3DTEXTURE9*)(&pTexture));
#else
	HRESULT hr = D3DXCreateTextureFromFileEx(CGlobals::GetRenderDevice(), filename.c_str(), width, height, D3DX_FROM_FILE, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, (LPDIRECT3DTEXTURE9*)(&pTexture));
#endif
	
	if(SUCCEEDED(hr))
	{
		string sExt = CParaFile::GetFileExtension(destFilename);

		D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;

		if(sExt == "dds")
		{
			FileFormat = D3DXIFF_DDS;
		}
		else if(sExt == "jpg")
		{
			FileFormat = D3DXIFF_JPG;
		}
		else if(sExt == "bmp")
		{
			FileFormat = D3DXIFF_BMP;
		}
		else if(sExt == "tga")
		{
			FileFormat = D3DXIFF_TGA;
		}
		else // if(sExt == "png")
		{
			FileFormat = D3DXIFF_PNG;
		}
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
		std::wstring destFilename16 = StringHelper::MultiByteToWideChar(destFilename.c_str(), DEFAULT_FILE_ENCODING);
		D3DXSaveTextureToFileW(destFilename16.c_str(), FileFormat, pTexture, NULL);
#else 
		D3DXSaveTextureToFile(destFilename.c_str(), FileFormat, pTexture, NULL);
#endif
		SAFE_RELEASE(pTexture);
		return true;
	}
	else
	{
		OUTPUT_LOG("warning:unable to create texture when doing ResizeImage %s to dimension %d, %d\n", filename.c_str(), width, height);
		return false;
	}
#else
	return false;
#endif
}

bool CMoviePlatform::TakeScreenShot(const string& filename, int width, int height)
{
	if(CMoviePlatform::TakeScreenShot(filename))
	{
		if(width == 0 || height ==0)
		{
			return true;
		}
		else
		{
			return ResizeImage(filename, width, height, filename);
		}
	}
	else
		return false;
}
bool CMoviePlatform::TakeScreenShot(const string& filename)
{
#ifdef USE_DIRECTX_RENDERER
#define SCREENSHOT_FROM_BACKBUFFER
#ifdef SCREENSHOT_FROM_BACKBUFFER
	LPDIRECT3DSURFACE9  pBackBuffer = CGlobals::GetDirectXEngine().GetRenderTarget();
	if(pBackBuffer)
	{
		string Filename = filename;
		
		if(filename.empty())
		{
			char ValidFilename[256];
			ZeroMemory(ValidFilename, sizeof(ValidFilename));

			std::string date_str = ParaEngine::GetDateFormat("MMM dd yy");
			snprintf(ValidFilename, 255, "Screen Shots\\ParaEngine_%s_%s.jpg", date_str.c_str(), ParaEngine::GetTimeFormat("hh'H'mm'M'ss tt").c_str());
			Filename = ValidFilename;
		}

		string sExt = CParaFile::GetFileExtension(Filename);

		D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;

		if(sExt == "dds")
		{
			FileFormat = D3DXIFF_DDS;
		}
		else if(sExt == "jpg")
		{
			FileFormat = D3DXIFF_JPG;
		}
		else if(sExt == "bmp")
		{
			FileFormat = D3DXIFF_BMP;
		}
		else if(sExt == "tga")
		{
			FileFormat = D3DXIFF_TGA;
		}
		else // if(sExt == "png")
		{
			FileFormat = D3DXIFF_PNG;
		}

		// this will create the directory is not ready.
		if(CParaFile::CreateDirectory(Filename.c_str()))
		{
			// save texture
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
			LPCWSTR Filename16 = StringHelper::MultiByteToWideChar(Filename.c_str(), DEFAULT_FILE_ENCODING);
			if (SUCCEEDED(D3DXSaveSurfaceToFileW(Filename16, FileFormat, pBackBuffer, NULL, NULL)))
#else 
			if (SUCCEEDED(D3DXSaveSurfaceToFile(Filename.c_str(), FileFormat, pBackBuffer, NULL, NULL)))
#endif
			{
				return true;
			}
		}
	}
#else
	if(filename.empty())
		GSSHOTSYSTEM->TakeScreenShot(NULL);
	else
		GSSHOTSYSTEM->TakeScreenShot(filename.c_str());
#endif
#elif defined(PARAENGINE_MOBILE)
	string Filename = filename;

	if (filename.empty())
	{
		char ValidFilename[256];
		ZeroMemory(ValidFilename, sizeof(ValidFilename));

		std::string date_str = ParaEngine::GetDateFormat("MMM dd yy");
		snprintf(ValidFilename, 255, "Screen Shots\\ParaEngine_%s_%s.jpg", date_str.c_str(), ParaEngine::GetTimeFormat("hh'H'mm'M'ss tt").c_str());
		Filename = ValidFilename;
	}
	Filename = CParaFile::GetWritablePath() + Filename;
	if (CParaFile::CreateDirectory(Filename.c_str()))
	{
		int width=cocos2d::Director::getInstance()->getOpenGLView()->getFrameSize().width;
		int height = cocos2d::Director::getInstance()->getOpenGLView()->getFrameSize().height;
		std::vector<unsigned int> pixels;
		pixels.resize(width*height);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
		CHECK_GL_ERROR_DEBUG();
		std::vector<unsigned int> img_pixels;
		img_pixels.resize(pixels.size());
		for (int row = 0; row < height; ++row)
		{
			memcpy(&img_pixels[width*row], &pixels[width*(height - row - 1)], width*sizeof(unsigned int));
		}
		cocos2d::CCImage img;
		img.initWithRawData(reinterpret_cast<const unsigned char*>(&img_pixels[0]), img_pixels.size()*sizeof(unsigned int), width, height, 32);
		img.saveToFile(Filename);
		return true;
	}
#endif
	return false;
}

class TakeScreenShotTask
{
public: 
	std::string m_filename;
	bool m_encode;
	int m_width;
	int m_height;
	std::function<void(bool, std::vector<BYTE>& base64)> m_callback;
};

void CMoviePlatform::TakeScreenShot_Async(const string& filename, bool bEncode, int width, int height, screenshot_callback callback)
{
#ifdef USE_DIRECTX_RENDERER
	static std::shared_ptr<std::mutex> s_mutex = std::make_shared<std::mutex>();
	static std::shared_ptr<std::deque<std::shared_ptr<TakeScreenShotTask>>> s_deque = std::make_shared<std::deque<std::shared_ptr<TakeScreenShotTask>>>();
	auto task = std::make_shared<TakeScreenShotTask>();
	task->m_filename = filename;
	task->m_encode = bEncode;
	task->m_width = width;
	task->m_height = height;
	task->m_callback = callback;
	s_mutex->lock();
	s_deque->push_back(task);
	if (s_deque->size() > 10) s_deque->pop_front();
	s_mutex->unlock();
	std::thread thread([=]() {
		int max_wait_count = 100; // 最大等待次数
		int wait_count = 0;       // 最大等待次数
		while (wait_count < max_wait_count)
		{
			s_mutex->lock();
			int size = s_deque->size(); 
			s_mutex->unlock();
			if (size == 0)
			{
				wait_count++;
				std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 等待0.5秒  50秒内无新请求直接退出线程
				continue;
			}
			wait_count = 0;   // 有新的任务重置等待次数
			s_mutex->lock();
			auto task = s_deque->front();
			s_deque->pop_front();
			s_mutex->unlock();

			std::vector<BYTE> buffers;
			bool result = TakeScreenShot_FromGDI(task->m_filename, buffers, task->m_encode, task->m_width, task->m_height);
			if (task->m_callback != nullptr)
			{
				(task->m_callback)(result,buffers);
			}
		}
	});
	thread.detach();
#endif
}
bool CMoviePlatform::TakeScreenShot_FromGDI(const string& filename, std::vector<BYTE>& outBase64Buffers, bool bEncode, int width, int height)
{
#if defined(USE_DIRECTX_RENDERER) && defined(USE_FREEIMAGE)
	// force same resolution as current back buffer.  
	LPDIRECT3DSURFACE9 pFromSurface = CGlobals::GetDirectXEngine().GetRenderTarget(0);
	D3DSURFACE_DESC desc;
	if (!pFromSurface)
	{
		return false;
	}
	int wndWidth = 0;
	int wndHeight = 0;
	if (SUCCEEDED(pFromSurface->GetDesc(&desc)))
	{
		wndWidth = desc.Width;
		wndHeight = desc.Height;
	}
	if (width <= 0 || height <= 0)
	{
		width = wndWidth;
		height = wndHeight;
	}
	string Filename = filename;

	if (filename.empty())
	{
		char ValidFilename[256];
		ZeroMemory(ValidFilename, sizeof(ValidFilename));

		std::string date_str = ParaEngine::GetDateFormat("MMM dd yy");
		snprintf(ValidFilename, 255, "Screen Shots\\ParaEngine_%s_%s.jpg", date_str.c_str(), ParaEngine::GetTimeFormat("hh'H'mm'M'ss tt").c_str());
		Filename = ValidFilename;
	}

	string sExt = CParaFile::GetFileExtension(Filename);

	FREE_IMAGE_FORMAT FileFormat = FIF_PNG;

	if (sExt == "dds")
	{
		FileFormat = FIF_DDS;
	}
	else if (sExt == "jpg")
	{
		FileFormat = FIF_JPEG;
	}
	else if (sExt == "bmp")
	{
		FileFormat = FIF_BMP;
	}
	else if (sExt == "tga")
	{
		FileFormat = FIF_TARGA;
	}
	else // if(sExt == "png")
	{
		FileFormat = FIF_PNG;
	}
	std::vector<BYTE> buffers;
	int fileSize = 0;
	int infoSize = 0;

	if (CaptureBitmapBuffer(CGlobals::GetAppHWND(), fileSize, infoSize, buffers, false, 0, 0, wndWidth, wndHeight) != 0)
	{
		return false;
	}

	FIMEMORY* sourceMemBuff = FreeImage_OpenMemory(&(buffers[0]), buffers.size());;
	FIBITMAP* sourceImage = FreeImage_LoadFromMemory(FIF_BMP, sourceMemBuff);
	if (width != wndWidth || height != wndHeight)
	{
		sourceImage = FreeImage_Rescale(sourceImage, width, height, FILTER_BOX);
	}
	FIMEMORY* destMemBuff = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FileFormat, sourceImage, destMemBuff);

	BYTE *mem_buffer = NULL;
	DWORD size_in_bytes = 0;
	FreeImage_AcquireMemory(destMemBuff, &mem_buffer, &size_in_bytes);

	if (bEncode)
	{
		int nBufferSize = CyoEncode::Base64EncodeGetLength(size_in_bytes);
		outBase64Buffers.resize(nBufferSize);
		CyoEncode::Base64Encode(&(outBase64Buffers[0]), mem_buffer, size_in_bytes);
	}

	CParaFile file;
	if (file.CreateNewFile(filename.c_str()))
	{
		file.write(mem_buffer, size_in_bytes);
		file.close();
		FreeImage_CloseMemory(sourceMemBuff);
		FreeImage_CloseMemory(destMemBuff);
		return true;
	}
#endif
	return false;
}
int CMoviePlatform::CaptureBitmapBuffer(HWND nHwnd, int& outFileHeaderSize, int& outInfoHeaderSize, std::vector<BYTE>& outBuffers, bool bCaptureMouse, int nLeft /*= 0*/, int nTop /*= 0*/, int width /*= 0*/, int height /*= 0*/)
{
#ifdef USE_DIRECTX_RENDERER
	//--Get the interface device context
	HDC SourceHDC = GetDC(nHwnd);
	//--Create a compatible device context from the interface context
	HDC compatibleHDC = CreateCompatibleDC(SourceHDC);

	BITMAP Bitmap;

	// Create a compatible bitmap
	HBITMAP bitmapHandle = CreateCompatibleBitmap(SourceHDC, width, height);
	if (bitmapHandle == 0 || !SelectObject(compatibleHDC, bitmapHandle))
	{
		OUTPUT_LOG("error: can not select bitmap in MovieCodec::TakeScreenshot()\n");
		return -1;
	}
	// copy the bitmap to the memory device context
	// SRCCOPY | CAPTUREBLT: Includes any windows that are layered on top of your window in the resulting image. By default, the image only contains your window.
	if (!BitBlt(compatibleHDC, 0, 0, width, height, SourceHDC, nLeft, nTop, SRCCOPY))
	{
		OUTPUT_LOG("error: bitblt failed with code %d in MovieCodec::TakeScreenshot()\n", GetLastError());
		return -1;
	}

	// copy the mouse cursor to the bitmap
	if (bCaptureMouse) {
		HCURSOR hc = ::GetCursor();
		CURSORINFO cursorinfo;
		ICONINFO iconinfo;
		cursorinfo.cbSize = sizeof(CURSORINFO);
		::GetCursorInfo(&cursorinfo);
		::GetIconInfo(cursorinfo.hCursor, &iconinfo);
		::ScreenToClient(nHwnd, &cursorinfo.ptScreenPos);
		::DrawIcon(m_CompatibleHDC, cursorinfo.ptScreenPos.x - iconinfo.xHotspot, cursorinfo.ptScreenPos.y - iconinfo.yHotspot, cursorinfo.hCursor);
	}

	//--Get the bitmap
	GetObject(bitmapHandle, sizeof(BITMAP), &Bitmap);

	outFileHeaderSize = sizeof(BITMAPFILEHEADER);
	outInfoHeaderSize = sizeof(BITMAPINFOHEADER);
	//--Compute the bitmap size
	unsigned long BitmapSize = outFileHeaderSize + outInfoHeaderSize + (Bitmap.bmWidth  * Bitmap.bmHeight * 3);

	//--Allocate a memory block for the bitmap
	//allocate the output buffer;
	outBuffers.resize(BitmapSize);
	memset(&(outBuffers[0]), 0, BitmapSize);

	BYTE* MemoryHandle = &(outBuffers[0]);

	LPBITMAPFILEHEADER fileHeader = (LPBITMAPFILEHEADER)(MemoryHandle);
	fileHeader->bfType = 0x4d42;
	fileHeader->bfSize = 0;
	fileHeader->bfReserved1 = 0;
	fileHeader->bfReserved2 = 0;
	fileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//--Setup the bitmap data
	LPBITMAPINFOHEADER pBmpInfo = (LPBITMAPINFOHEADER)(&(outBuffers[outFileHeaderSize]));
	pBmpInfo->biSizeImage = 0;//BitmapSize - sizeof(BITMAPINFOHEADER);
	pBmpInfo->biSize = sizeof(BITMAPINFOHEADER);
	pBmpInfo->biHeight = Bitmap.bmHeight;
	pBmpInfo->biWidth = Bitmap.bmWidth;
	pBmpInfo->biCompression = BI_RGB;
	pBmpInfo->biBitCount = 24;
	pBmpInfo->biPlanes = 1;
	pBmpInfo->biXPelsPerMeter = 0;
	pBmpInfo->biYPelsPerMeter = 0;
	pBmpInfo->biClrUsed = 0;
	pBmpInfo->biClrImportant = 0;

	//--Get the bitmap data from memory
	GetDIBits(compatibleHDC, bitmapHandle, 0, Bitmap.bmHeight, (unsigned char*)(&(outBuffers[outFileHeaderSize + outInfoHeaderSize])), (LPBITMAPINFO)pBmpInfo, DIB_RGB_COLORS);

	::DeleteObject(bitmapHandle);
	::DeleteDC(compatibleHDC);
	::ReleaseDC(nHwnd, SourceHDC);
	::ReleaseDC(nHwnd, compatibleHDC);
#endif
	return 0;
}
HRESULT CMoviePlatform::InvalidateDeviceObjects()
{
	IMovieCodec* pMovieCodec = GetMovieCodec(false);
	if (pMovieCodec)
	{
#ifdef USE_DIRECTX_RENDERER
		SAFE_RELEASE(m_pOffScreenSurface);
#endif
	}
	else
	{
#ifdef USE_DIRECTX_RENDERER
		SAFE_RELEASE(m_pCaptureSurface);
		SAFE_RELEASE(m_pCaptureTexture);
		SAFE_RELEASE(m_pDepthStencilSurface);
		GSSHOTSYSTEM->SetCaptureTexture(NULL);
#endif
	}
	return S_OK;
}


HRESULT CMoviePlatform::RestoreDeviceObjects()
{
	if(IsInCaptureSession())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			int captureTexWidth = GetScreenWidth();
			int captureTexHeight = GetScreenHeight();

			if (m_bUseGDI)
			{
			}
			else
			{
#ifdef USE_DIRECTX_RENDERER
				if (FAILED(CGlobals::GetRenderDevice()->CreateOffscreenPlainSurface(captureTexWidth, captureTexHeight, D3DFMT_X8R8G8B8,
					D3DPOOL_SYSTEMMEM, &m_pOffScreenSurface, NULL)))
				{
					m_pOffScreenSurface = NULL;
				}
#endif
			}
		}
		else
		{
#ifdef USE_DIRECTX_RENDERER
			D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;

			int captureTexWidth = GetScreenWidth();
			int captureTexHeight = GetScreenHeight();

			if (FAILED(CGlobals::GetRenderDevice()->CreateTexture(captureTexWidth, captureTexHeight, 1, D3DUSAGE_RENDERTARGET, colorFormat,
				D3DPOOL_DEFAULT, &m_pCaptureTexture, NULL)))
				return E_FAIL;

			if (FAILED(CGlobals::GetRenderDevice()->CreateDepthStencilSurface(captureTexWidth, captureTexHeight, D3DFMT_D24S8,
				D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthStencilSurface, NULL)))
			{
				OUTPUT_LOG("failed creating depth stencil buffer for Movie Platform\r\n");
				return E_FAIL;
			}

			// Retrieve top-level surfaces of our capture buffer (need these for use with SetRenderTarget)
			if (FAILED(m_pCaptureTexture->GetSurfaceLevel(0, &m_pCaptureSurface)))
				return E_FAIL;

			GSSHOTSYSTEM->SetCaptureTexture(m_pCaptureTexture);
#endif
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
		{
			// we do not need to modify the capture target using this capture method. 
			return false;
		}
		else
		{
#ifdef USE_DIRECTX_RENDERER
			if (m_pCaptureSurface != NULL)
			{
				// save old render target
				m_pBackBufferSurface = CGlobals::GetDirectXEngine().GetRenderTarget(0);
				if (FAILED(CGlobals::GetRenderDevice()->GetDepthStencilSurface(&m_pOldDepthStencilSurface)))
					return false;

				// set new render target
				CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pCaptureSurface);
				CGlobals::GetRenderDevice()->SetDepthStencilSurface(m_pDepthStencilSurface);

				return true;
			}
#endif
		}
	}

	return false;
}

bool CMoviePlatform::UnsetCaptureTarget()
{
#ifdef USE_DIRECTX_RENDERER
	if(IsInCaptureSession() && m_pCaptureSurface!=NULL)
	{
		// restore old render target
		CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pBackBufferSurface);
		CGlobals::GetRenderDevice()->SetDepthStencilSurface( m_pOldDepthStencilSurface );
		SAFE_RELEASE(m_pOldDepthStencilSurface);

		return true;
	}
#endif
	return false;
}

void CMoviePlatform::RenderCaptured()
{
#ifdef USE_DIRECTX_RENDERER
	if(IsInCaptureSession())
	{
		//////////////////////////////////////////////////////////////////////////
		//
		// Render quad effect
		//
		//////////////////////////////////////////////////////////////////////////
		CGlobals::GetEffectManager()->BeginEffect(TECH_GUI);

		/** draw a square in front of the screen. */
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// fixed programming pipeline

			//////////////////////////////////////////////////////////////////////////
			// draw the captured video image to screen. If GUI is being captured, the video is drawn with its original size
			// if not, the video is stretched to the entire screen. 
			float sx,sy;
			if(CaptureGUI())
			{
				sx = (float)GetScreenWidth();
				sy = (float)GetScreenHeight();
			}
			else
			{
				D3DVIEWPORT9 curViewport;
				CGlobals::GetRenderDevice()->GetViewport(&curViewport);
				sx = (float)curViewport.Width;
				sy = (float)curViewport.Height;
			}
			
			DXUT_SCREEN_VERTEX v[10];
			v[0].x = 0;  v[0].y = sy; v[0].tu = 0;  v[0].tv = 1.0f;
			v[1].x = 0;  v[1].y = 0;  v[1].tu = 0;  v[1].tv = 0;
			v[2].x = sx; v[2].y = sy; v[2].tu = 1.f; v[2].tv = 1.f;
			v[3].x = sx; v[3].y = 0;  v[3].tu = 1.f; v[3].tv = 0;
			
			DWORD dwColor = LinearColor(1.f,1.f,1.f,1.f);
			int i;
			for(i=0;i<4;i++)
			{
				v[i].color = dwColor;
				v[i].z = 0;
				v[i].h = 1.0f;
			}

			CGlobals::GetRenderDevice()->SetTexture(0, m_pCaptureTexture);
			RenderDevice::DrawPrimitiveUP( CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UI, D3DPT_TRIANGLESTRIP, 2, v, sizeof(DXUT_SCREEN_VERTEX) );

			//////////////////////////////////////////////////////////////////////////
			// render a border, indicating whether the screen is being recorded or not.
			if(m_bRenderBorder)
			{
				/*
				0------------------------------------2
				8|\                                 / |
				| 1-------------------------------3  |
				| |9                              |  |
				| |                               |  |
				| |                               |  |
				| 7-------------------------------5  |
				|/                                 \ |
				6------------------------------------4
				*/
				// border width in pixels
				float bBorderSize = 7.f; 
				v[0].x = 0;				v[0].y = 0;
				v[1].x = bBorderSize;	v[1].y = bBorderSize;
				v[2].x = sx;			v[2].y = 0;
				v[3].x = sx-bBorderSize;v[3].y = bBorderSize;
				v[4].x = sx;			v[4].y = sy;
				v[5].x = sx-bBorderSize;v[5].y = sy-bBorderSize;
				v[6].x = 0;				v[6].y = sy;
				v[7].x = bBorderSize;	v[7].y = sy - bBorderSize;
				v[8].x = v[0].x;		v[8].y = v[0].y;
				v[9].x = v[1].x;		v[9].y = v[1].y;

				if(IsRecording())
				{
					// the following code will animate (flash) the border color, which is annoying after I implemented it
					/*static float fAnim = 1.0f;
					const int FLASH_FREQUENCY = 1;
					fAnim += (float)CGlobals::GetSceneState()->dTimeDelta*FLASH_FREQUENCY;
					if(fAnim >= 2.0f)
					fAnim = 0;
					dwColor = LinearColor(fAnim>1.f?(2.f-fAnim):fAnim,0,0,1.f);*/

					// just use a static red color to indicate recording state
					dwColor = LinearColor(1.0f,0.f,0.0f,1.f);
				}
				else
					dwColor = LinearColor(0,1.f,1.0f,1.f);
				for(i=0;i<10;i++)
				{
					v[i].tu = 0;v[i].tv = 0;
					v[i].color = dwColor;
					v[i].z = 0;
					v[i].h = 1.0f;
				}
				CGlobals::GetRenderDevice()->SetTexture(0, NULL);
				CGlobals::GetEffectManager()->SetCullingMode(false);
				RenderDevice::DrawPrimitiveUP( CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UI, D3DPT_TRIANGLESTRIP, 8, v, sizeof(DXUT_SCREEN_VERTEX) );
				CGlobals::GetEffectManager()->SetCullingMode(true);
			}
		}
	}
#endif
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
	if(IsInCaptureSession())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			// use ffmpeg external dll
			return pMovieCodec->GetCurrentFrameNumber();
		}
		else
		{
#ifdef USE_DIRECTX_RENDERER
			// use buildin system
			return GSSHOTSYSTEM->GetCurrentFrameNumber();
#endif
		}
		
	}
	else
		return 0;
	return 0;
}

bool CMoviePlatform::BeginCapture(const string& sFileName)
{
	if(IsInCaptureSession())
		return true;

	IMovieCodec* pMovieCodec = GetMovieCodec();

	if (pMovieCodec)
	{
#ifdef USE_DIRECTX_RENDERER
		// force same resolution as current back buffer.  
		LPDIRECT3DSURFACE9 pFromSurface = CGlobals::GetDirectXEngine().GetRenderTarget(0);
		D3DSURFACE_DESC desc;
		if (SUCCEEDED(pFromSurface->GetDesc(&desc)))
		{
			m_nScreenWidth = desc.Width;
			m_nScreenHeight = desc.Height;
		}
#endif
	}

	InvalidateDeviceObjects();
	m_recorderState = RECORDER_STATE_BEGIN;
	RestoreDeviceObjects();

	if (!pMovieCodec)
	{
#ifdef USE_DIRECTX_RENDERER
		if (m_pCaptureTexture == 0)
		{
			m_recorderState = RECORDER_STATE_END;
			return false;
		}
#endif
	}
	
	bool bAllowCodecSelection = IsCodecSelectionEnabled();
	if (bAllowCodecSelection)
	{
		// Enforce windowed mode, when a windows dialog is displayed for displaying the text.
		CGlobals::GetSettings().SetWindowedMode(true);
	}

	((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->EnableStereoVision(GetStereoCaptureMode() != MOVIE_CAPTURE_MODE_NORMAL);

	string fileName;
	if (sFileName == "")
		fileName = "paraengine.avi";
	else
	{
		// only added extension if there is no file extension specified. 
		if (sFileName.size() > 4 && sFileName[sFileName.size() - 4] == '.')
			fileName = sFileName;
		else
			fileName = sFileName + ".avi";
	}

	if (pMovieCodec)
	{


		// use ffmpeg external dll
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
	}
	else
	{
#ifdef USE_DIRECTX_RENDERER
		// use buildin system
		if (!GSSHOTSYSTEM->BeginMovieCapture(fileName.c_str(), m_nRecordingFPS, bAllowCodecSelection))
		{
			m_recorderState = RECORDER_STATE_END;
			InvalidateDeviceObjects();
			return false;
		}
#endif
	}
	
	m_fLastRefreshInterval = CGlobals::GetApp()->GetRefreshTimer();
	int nRefreshFPS = m_nRecordingFPS;
	if (GetStereoCaptureMode() == MOVIE_CAPTURE_MODE_STEREO_LEFT_RIGHT || GetStereoCaptureMode() == MOVIE_CAPTURE_MODE_STEREO_ABOVE_BELOW || GetStereoCaptureMode() == MOVIE_CAPTURE_MODE_STEREO_LINE_INTERLACED)
	{
		// double the FPS for special stereo mode, since we will render one frame for the left eye and one frame for the right eye, and the output video is half the actual FPS. 
		// nRefreshFPS *= 2;
	}
	CGlobals::GetApp()->SetRefreshTimer(1.f/nRefreshFPS);
	if (!pMovieCodec)
	{
		// since, we are capturing audio, we should use real time frame rate instead of constant time FRC. 
		CFrameRateController::LoadFRCCapture(nRefreshFPS);
	}
		
	return true;
}

// obsoleted:
void CMoviePlatform::FrameCaptureDX(IMovieCodec* pMovieCodec)
{
#ifdef USE_DIRECTX_RENDERER
	if (m_pOffScreenSurface)
	{
		// capture using DirectX surface: this is slow when reading from video memory to system memory for large image
		LPDIRECT3DSURFACE9 pFromSurface = CGlobals::GetDirectXEngine().GetRenderTarget(0);
		if (SUCCEEDED(D3DXLoadSurfaceFromSurface(m_pOffScreenSurface, NULL, NULL, pFromSurface, NULL, NULL, D3DX_FILTER_NONE, 0)))
		{
			// Lock the Texture
			BYTE  *pBmpBuffer = NULL;
			int captureTexWidth = GetScreenWidth();
			int captureTexHeight = GetScreenHeight();
			int nMarginLeft = MarginLeft();
			int nMarginTop = MarginTop();
			int nWidth = pMovieCodec->GetWidth();
			int nHeight = pMovieCodec->GetHeight();
			if ((nMarginLeft + nWidth) > captureTexWidth || (nMarginTop + nHeight) > captureTexHeight)
			{
				EndCapture();
				return;
			}
			size_t bmpsize = nWidth*nHeight * 3;

			D3DLOCKED_RECT d3dlr;
			if (SUCCEEDED(m_pOffScreenSurface->LockRect(&d3dlr, 0, D3DLOCK_READONLY)))
			{
				// Get the texture buffer & pitch
				BYTE  * pbD = NULL;
				DWORD * pdwS = NULL;
				DWORD * pdwD = NULL;

				BYTE  * pTxtBuffer = static_cast<BYTE *>(d3dlr.pBits);
				LONG lTxtPitch = d3dlr.Pitch;

				//calculate the bitmap pitch
				LONG lVidPitch = nWidth * 3;

				//allocate the output buffer;
				static vector<BYTE> g_buffer;

				if (g_buffer.size() != bmpsize)
					g_buffer.resize(bmpsize);
				pBmpBuffer = &(g_buffer[0]);


				for (int row = 0; row < nHeight; row++)
				{
					pdwS = (DWORD*)(pTxtBuffer + (row + nMarginTop)*lTxtPitch);
					pdwS += nMarginLeft;

					pbD = pBmpBuffer;

					for (int col = 0; col < nWidth; col++)
					{
						pbD[0] = (BYTE)(pdwS[0] & 0x000000ff);
						pbD[1] = (BYTE)((pdwS[0] & 0x0000ff00) >> 8);
						pbD[2] = (BYTE)((pdwS[0] & 0x00ff0000) >> 16);
						pdwS++;
						pbD += 3;
					}
					pBmpBuffer += lVidPitch;
				}

				pBmpBuffer = &(g_buffer[0]);

				m_pOffScreenSurface->UnlockRect();
			}

			if (pBmpBuffer)
			{
				// capture from directX back buffer 
				pMovieCodec->FrameCapture(pBmpBuffer, (int)bmpsize);
			}
		}
	}
#endif
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
		// GDI is captured in another thread, so we do nothing here. This is the default mode. 
	}
	else
	{
		// obsoleted:
		FrameCaptureDX(pMovieCodec);
	}
		
	return;
}

void CMoviePlatform::SaveCapturedFrame(int nMode)
{
	if(IsRecording())
	{
		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			FrameCaptureFFMPEG();
		}
		else
		{
#ifdef USE_DIRECTX_RENDERER
			// use build in system
			GSSHOTSYSTEM->UpdateMovieCapture(0, nMode);
#endif
		}
	}
}

bool CMoviePlatform::EndCapture()
{
	if(m_recorderState != RECORDER_STATE_END)
	{
		((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->EnableStereoVision(false);

		m_recorderState = RECORDER_STATE_END;

		IMovieCodec* pMovieCodec = GetMovieCodec();
		if (pMovieCodec)
		{
			// use ffmpeg external dll
			std::string audioMapString;
			ParaEngine::CAudioEngine2::CAudioPlaybackHistory& playbackHistory = ParaEngine::CAudioEngine2::GetInstance()->SetHistoryWithCaptureEnd();
			const ParaEngine::CAudioEngine2::CAudioPlaybackHistory::Records& records = playbackHistory.GetRecords();
			ParaEngine::CAudioEngine2::CAudioPlaybackHistory::Records::const_iterator iter = records.begin();
			for (; iter != records.end(); ++iter)
			{
				// simply encode audio record to a string
				char record[512]; 
				std::sprintf(record, "%s,%d,%d,%d,%d,%d,",
					iter->m_WaveFileName.c_str(),
					iter->m_nStartTime,
					iter->m_nEndTime,
					iter->m_nSeekPos,
					(int)iter->m_bIsLoop,
					iter->m_mTotalTime
					);
				audioMapString += record;
				
			}
			if (audioMapString.size() > 0)
				audioMapString.resize(audioMapString.size()-1); // remove the last ","
			
			if( m_nMovieCodecVersion >= 10)
				pMovieCodec->EndCapture2(audioMapString.c_str());
			else
				pMovieCodec->EndCapture(audioMapString);

			playbackHistory.Clear();
			playbackHistory.SetEnable(false);
		}
		else
		{
#ifdef USE_DIRECTX_RENDERER
			// use buildin system
			GSSHOTSYSTEM->EndMovieCapture();
#endif
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
	return m_bAllowCodecSelection 
#ifdef USE_DIRECTX_RENDERER
		|| !GSSHOTSYSTEM->HasLastCodec()
#endif
		;
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

