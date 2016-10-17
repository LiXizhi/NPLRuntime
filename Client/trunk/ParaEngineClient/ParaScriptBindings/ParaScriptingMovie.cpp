//-----------------------------------------------------------------------------
// Class:	ParaMovie
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2005.11
// Desc: 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#include "ScreenShotSystem.h"
using namespace ScreenShot;
#endif

#include "util/StringHelper.h"
#include "AISimulator.h"
#include "MoviePlatform.h"
#include "ParaScriptingMovie.h"

using namespace ParaEngine;

namespace ParaScripting
{

	//////////////////////////////////////////////////////////////////////////
	//
	// screen capture and movie setting
	//
	//////////////////////////////////////////////////////////////////////////

	void ParaMovie::SetCaptureGUI(bool bGUI)
	{
		CGlobals::GetMoviePlatform()->SetCaptureGUI(bGUI);
	}

	bool ParaMovie::CaptureGUI()
	{
		return CGlobals::GetMoviePlatform()->CaptureGUI();
	}

	void ParaMovie::SetMovieScreenSize(int nWidth, int nHeight)
	{
		CGlobals::GetMoviePlatform()->SetMovieScreenSize(nWidth, nHeight);
	}

	bool ParaMovie::BeginCapture(string sFileName)
	{
		return CGlobals::GetMoviePlatform()->BeginCapture(sFileName);
	}
	bool ParaMovie::EndCapture()
	{
		return CGlobals::GetMoviePlatform()->EndCapture();
	}
	void ParaMovie::PauseCapture()
	{
		CGlobals::GetMoviePlatform()->PauseCapture();
	}
	void ParaMovie::ResumeCapture()
	{
		CGlobals::GetMoviePlatform()->ResumeCapture();
	}
	bool ParaMovie::IsInCaptureSession()
	{
		return CGlobals::GetMoviePlatform()->IsInCaptureSession();
	}
	bool ParaMovie::IsRecording()
	{
		return CGlobals::GetMoviePlatform()->IsRecording();
	}

	bool ParaMovie::TakeScreenShot(const char* filename)
	{
		return CGlobals::GetMoviePlatform()->TakeScreenShot(filename);
	}

	bool ParaMovie::TakeScreenShot3(const char* filename, int width, int height)
	{
		return CGlobals::GetMoviePlatform()->TakeScreenShot(filename, width, height);
	}

	void ParaMovie::TakeScreenShot_Async(const char* filename, const char* sCallBackScript)
	{
		TakeScreenShot_Async_Internal(filename, false, -1, -1, sCallBackScript);
	}
	void ParaMovie::TakeScreenShot2_Async(const char* filename, bool bEncode, const char* sCallBackScript)
	{
		TakeScreenShot_Async_Internal(filename, bEncode, -1, -1, sCallBackScript);
	}
	void ParaMovie::TakeScreenShot3_Async(const char* filename, bool bEncode, int width, int height, const char* sCallBackScript)
	{
		TakeScreenShot_Async_Internal(filename, bEncode, width, height, sCallBackScript);
	}
	void ParaMovie::TakeScreenShot_Async_Internal(const char* filename, bool bEncode, int width, int height, const char* sCallBackScript)
	{
#ifdef WIN32
		string callback_script = sCallBackScript;
		std::function<void(bool, std::vector<BYTE>&)> callback = [=](bool bSuccessful, std::vector<BYTE>& base64) {
			if (!callback_script.empty())
			{
				string scode, sequence;
				string base64Str = "";
				string size = std::to_string(base64.size());
				if (bEncode)
				{
					base64Str = string(base64.begin(), base64.end());
				}
				ParaEngine::StringHelper::DevideString(sCallBackScript, scode, sequence);
				if (bSuccessful)
				{
					scode = "msg={res = 0, base64 = \"" + base64Str + "\" , size = " + size + ", s = " + sequence + "};" + scode;
				}
				else
				{
					scode = "msg={res = -1, base64 = \"" + base64Str + "\" , size = " + size + ", s = " + sequence + "};" + scode;
				}
				ParaEngine::CGlobals::GetAISim()->NPLActivate("", scode.c_str(), (int)(scode.size()));
			}
		};
		CGlobals::GetMoviePlatform()->TakeScreenShot_Async(filename, bEncode, width, height, [=](bool bSuccessful, std::vector<BYTE>& base64) {
			callback(bSuccessful, base64);
		});
#endif
	}
	// this function is obsoleted, use ParaMovie::TakeScreenShot3 instead. 
	bool ParaMovie::RenderToTexture(const char* filename, int width, int height)
	{
// code no longer used. it just provides an example of calling render to texture.
#if 0
		//#include "AutoCamera.h"
		//#include "SceneObject.h"
		//#include "FrameRateController.h"

		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		if(pd3dDevice == 0)
			return false;
		CSceneObject* pScene = CGlobals::GetScene();

		LPDIRECT3DTEXTURE9 m_pRenderTarget = NULL;
		LPDIRECT3DSURFACE9 m_pRenderTargetSurface = NULL;
		LPDIRECT3DSURFACE9 m_pDepthSurface = NULL;
		D3DVIEWPORT9 oldViewport;

		D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
		D3DFORMAT zFormat = D3DFMT_D24S8;

		
		// render the scene
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			if(FAILED(pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, colorFormat, 
				D3DPOOL_DEFAULT, &m_pRenderTarget, NULL)))
				return false;
			if(FAILED(m_pRenderTarget->GetSurfaceLevel(0, &m_pRenderTargetSurface)))
				return false;
			if(FAILED(pd3dDevice->CreateDepthStencilSurface(width, height, D3DFMT_D16, 
					D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthSurface, NULL)))
				return false;
		
			//////////////////////////////////////////////////////////////////////////
			// Render to the reflection map
			LPDIRECT3DSURFACE9 pOldRenderTarget =  CGlobals::GetDirectXEngine().GetRenderTarget();
			CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pRenderTargetSurface);
			CBaseCamera* pCamera = pScene->GetCurrentCamera();
			float fOldAspectRatio = pCamera->GetAspectRatio();
			pCamera->SetAspectRatio((float)width/(float)height);
			pd3dDevice->GetViewport(&oldViewport);
			D3DVIEWPORT9 newViewport;
			newViewport.X = 0;
			newViewport.Y = 0;
			newViewport.Width  = width;
			newViewport.Height = height;
			
			newViewport.MinZ = 0.0f;
			newViewport.MaxZ = 1.0f;
			pd3dDevice->SetViewport(&newViewport);

			// set depth surface
			LPDIRECT3DSURFACE9 pOldZBuffer = NULL;
			if(FAILED(pd3dDevice->GetDepthStencilSurface(&pOldZBuffer)))
			{
				OUTPUT_LOG("GetDepthStencilSurface failed\r\n");
				return false;
			}
			pd3dDevice->SetDepthStencilSurface( m_pDepthSurface );

			/////////////////////////////////////////////////////////////////////////
			/// render
			/// clear to scene
			pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
				0x00000000, 1.0f, 0L );

			if(pScene->IsSceneEnabled())
			{
				///-- set up effects parameters
				((CAutoCamera*)pCamera)->UpdateViewProjMatrix();
				pScene->AdvanceScene(0);
			}

			//////////////////////////////////////////////////////////////////////////
			// Restore to old settings
			D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_JPG;
			D3DXSaveTextureToFile(filename, FileFormat, m_pRenderTarget, NULL);

			// restore old view port
			pd3dDevice->SetViewport(&oldViewport);
			pCamera->SetAspectRatio(fOldAspectRatio);
			
			pd3dDevice->SetDepthStencilSurface( pOldZBuffer);
			SAFE_RELEASE(pOldZBuffer);
			CGlobals::GetDirectXEngine().SetRenderTarget(0, pOldRenderTarget);

			SAFE_RELEASE(m_pRenderTargetSurface);
			SAFE_RELEASE(m_pDepthSurface);
			SAFE_RELEASE(m_pRenderTarget);

			pd3dDevice->EndScene();
		}
#endif		
		return true;
	}

	bool ParaMovie::ResizeImage( const string& filename, int width, int height, const char* destFilename )
	{
		string sDestFileName;
		if(destFilename == NULL || destFilename[0] == '\0')
			sDestFileName = filename;
		else
			sDestFileName = destFilename;
		return CGlobals::GetMoviePlatform()->ResizeImage(filename, width, height, sDestFileName);
	}

	void ParaMovie::GetImageInfo(const string& filename, int* width, int* height, int* nFileSize)
	{
		if( ! CGlobals::GetMoviePlatform()->GetImageInfo(filename, width, height, nFileSize))
		{
			*width = 0;
			*height = 0;
			*nFileSize = 0;
		}
	}

	int ParaMovie::GetEncodeMethod()
	{
		if(CGlobals::GetMoviePlatform()->IsCodecSelectionEnabled())
			return -1;
		else
		{
#ifdef USE_DIRECTX_RENDERER
			return GSSHOTSYSTEM->GetCodec();
#endif
		}
		return -1;
	}
	void ParaMovie::SetEncodeMethod(int eType)
	{
#ifdef USE_DIRECTX_RENDERER
		switch(eType) {
		case SCREENSHOTSYSTEM::CODEC_XVID:
		case SCREENSHOTSYSTEM::CODEC_WMV:
			CGlobals::GetMoviePlatform()->EnableCodecSelection(false);
			GSSHOTSYSTEM->SetUsingCodec(eType);
			break;
		default:
			CGlobals::GetMoviePlatform()->EnableCodecSelection(true);
			return;
		}
#endif
	}
	void ParaMovie::GetMovieScreenSize(int* nWidth, int* nHeight)
	{
		CGlobals::GetMoviePlatform()->GetMovieScreenSize(nWidth,nHeight);
	}
	string ParaMovie::GetMovieFileName()
	{
		return CGlobals::GetMoviePlatform()->GetMovieFileName();
	}

	void ParaMovie::SelectCodecOptions()
	{
#ifdef USE_DIRECTX_RENDERER
		GSSHOTSYSTEM->SelectCodecOptions();
#endif
	}

	void ParaMovie::SetRecordingFPS( int nFPS )
	{
		CGlobals::GetMoviePlatform()->SetRecordingFPS(nFPS);
	}

	int ParaMovie::GetRecordingFPS()
	{
		return CGlobals::GetMoviePlatform()->GetRecordingFPS();
	}

	void ParaMovie::SetStereoCaptureMode( int nMode )
	{
		CGlobals::GetMoviePlatform()->SetStereoCaptureMode((MOVIE_CAPTURE_MODE)nMode);
	}

	int ParaMovie::GetStereoCaptureMode()
	{
		return (int)(CGlobals::GetMoviePlatform()->GetStereoCaptureMode());
	}

	void ParaMovie::SetStereoEyeSeparation( float fDist )
	{
		CGlobals::GetMoviePlatform()->SetStereoEyeSeparation(fDist);
	}

	float ParaMovie::GetStereoEyeSeparation()
	{
		return CGlobals::GetMoviePlatform()->GetStereoEyeSeparation();
	}

	ParaScripting::ParaAttributeObject ParaMovie::GetAttributeObject()
	{
		return ParaAttributeObject(CGlobals::GetMoviePlatform());
	}

	bool ParaMovie::FrameCapture()
	{
		CGlobals::GetMoviePlatform()->FrameCaptureFFMPEG();
		return true;
	}
}
