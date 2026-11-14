//-----------------------------------------------------------------------------
// Class:	ParaMovie
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2005.11
// Desc: 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
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

    bool ParaMovie::TakeScreenShot4(const char* filename, int width, int height, bool isCenter)
    {
        return CGlobals::GetMoviePlatform()->TakeScreenShot(filename, width, height, isCenter);
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
		string filePath = filename ? filename : "";
		string callback_script = sCallBackScript ? sCallBackScript : "";
		std::function<void(bool, std::vector<BYTE>&)> callback = [callback_script, bEncode](bool bSuccessful, std::vector<BYTE>& base64) {
			if (!callback_script.empty())
			{
				string scode, sequence;
				string base64Str = "";
				string size = std::to_string(base64.size());
				if (bEncode)
				{
					base64Str = string(base64.begin(), base64.end());
				}
				ParaEngine::StringHelper::DevideString(callback_script, scode, sequence);
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
		CGlobals::GetMoviePlatform()->TakeScreenShot_Async(filePath, bEncode, width, height, [callback](bool bSuccessful, std::vector<BYTE>& base64) {
			callback(bSuccessful, base64);
		});
	}
	// this function is obsoleted, use ParaMovie::TakeScreenShot3 instead. 
	bool ParaMovie::RenderToTexture(const char* filename, int width, int height)
	{
		return TakeScreenShot3(filename, width, height);
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
		return CGlobals::GetMoviePlatform()->GetEncodeCodec();
	}
	void ParaMovie::SetEncodeMethod(int eType)
	{
		switch(eType) {
		case CMoviePlatform::CODEC_XVID:
		case CMoviePlatform::CODEC_WMV:
			CGlobals::GetMoviePlatform()->EnableCodecSelection(false);
			CGlobals::GetMoviePlatform()->SetEncodeCodec(eType);
			break;
		default:
			CGlobals::GetMoviePlatform()->EnableCodecSelection(true);
			return;
		}
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
