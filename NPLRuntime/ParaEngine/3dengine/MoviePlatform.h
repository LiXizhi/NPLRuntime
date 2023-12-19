#pragma once
#include <vector>
#include "IAttributeFields.h"
#include "IMovieCodec.h"
#include <mutex>
namespace ParaEngine
{
	using namespace std;

	enum MOVIE_RECORDING_STATE
	{
		/// recording
		RECORDER_STATE_BEGIN=0,
		/// recording started but paused
		RECORDER_STATE_PAUSED,
		/// recording stopped
		RECORDER_STATE_END
	};

	/** For managing and rendering the movie. A movie may be comprised of series of clips*/
	class CMoviePlatform : public IAttributeFields
	{
	public:
		CMoviePlatform(void);
		virtual ~CMoviePlatform(void);
		static CMoviePlatform& GetSingleton();

		void Cleanup();

		/** try get the movie codec. this function may return NULL if dll does not exist. */
		IMovieCodec* GetMovieCodec(bool bCreateIfNotExist = true);

	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_MoviePlatform; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "MoviePlatform"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CMoviePlatform, video_encode_example_s, const char*)	{ cls->video_encode_example(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CMoviePlatform, GetCodec_s, int*)	{ *p1 = cls->GetCodec(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetCodec_s, int)	{ cls->SetCodec(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, GetMarginLeft_s, int*)	{ *p1 = cls->MarginLeft(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetMarginLeft_s, int)	{ cls->MarginLeft(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CMoviePlatform, GetMarginTop_s, int*)	{ *p1 = cls->MarginTop(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetMarginTop_s, int)	{ cls->MarginTop(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CMoviePlatform, GetMarginRight_s, int*)	{ *p1 = cls->MarginRight(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetMarginRight_s, int)	{ cls->MarginRight(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CMoviePlatform, GetMarginBottom_s, int*)	{ *p1 = cls->MarginBottom(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetMarginBottom_s, int)	{ cls->MarginBottom(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, HasMoviePlugin_s, bool*)	{ *p1 = cls->HasMoviePlugin(); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, GetRecordingFPS_s, int*)	{ *p1 = cls->GetRecordingFPS(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetRecordingFPS_s, int)	{ cls->SetRecordingFPS(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, GetVideoBitRate_s, int*)	{ *p1 = cls->GetVideoBitRate(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetVideoBitRate_s, int)	{ cls->SetVideoBitRate(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, IsCaptureMic_s, bool*)	{ *p1 = cls->IsCaptureMic(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetCaptureMic_s, bool)	{ cls->SetCaptureMic(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, IsCaptureAudio_s, bool*)	{ *p1 = cls->IsCaptureAudio(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetCaptureAudio_s, bool)	{ cls->SetCaptureAudio(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, GetStereoCaptureMode_s, int*)	{ *p1 = cls->GetStereoCaptureMode(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetStereoCaptureMode_s, int)	{ cls->SetStereoCaptureMode((MOVIE_CAPTURE_MODE)p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, GetStereoEyeSeparation_s, float*)	{ *p1 = cls->GetStereoEyeSeparation(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetStereoEyeSeparation_s, float)	{ cls->SetStereoEyeSeparation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMoviePlatform, GetStereoConvergenceOffset_s, float*) { *p1 = cls->GetStereoConvergenceOffset(); return S_OK; }
		ATTRIBUTE_METHOD1(CMoviePlatform, SetStereoConvergenceOffset_s, float) { cls->SetStereoConvergenceOffset(p1); return S_OK; }

	public:
		/**encode a random dummy video to the given filename with given codec. */
		void video_encode_example(const char *filename, int codec_id = 0);

		/** set preferred codec */
		virtual void SetCodec(int nCodec);

		/** get preferred codec */
		virtual int GetCodec();
	public:
		typedef std::function<void(bool, std::vector<BYTE>& base64)>			screenshot_callback;

	public:
		/** clear all clips*/
		void CleanUp();

		/** set whether GUI is captured.*/
		void SetCaptureGUI(bool bGUI){m_bCaptureGUI = bGUI;}
		/** return true if GUI is also captured.*/
		bool CaptureGUI(){return m_bCaptureGUI;};

		/** set the movie name */
		void SetMovieName(string name){m_sMovieName = name;}
		/** Get the movie name */
		string GetMovieName(){return m_sMovieName;}

		/** get the movie screen width and height*/
		int GetScreenWidth(){return m_nScreenWidth;}
		/** clip height in pixel*/
		int GetScreenHeight(){return m_nScreenHeight;}

		void SetVideoBitRate(int nRate);
		int GetVideoBitRate();

		/** set the movie screen size in pixel. 
		@param nWidth: in pixels, must be multiple of 4. 
		@param nHeight: in pixels, must be multiple of 4. */
		void SetMovieScreenSize(int nWidth, int nHeight);
		/** get movie screen size*/
		void GetMovieScreenSize(int* nWidth, int* nHeight);

		/** get the movie file name */
		string GetMovieFileName();

		/** call this function just outside BeginScene and EndScene */
		void BeginCaptureFrame();
		void EndCaptureFrame();
		//////////////////////////////////////////////////////////////////////////
		//
		// screen capture and movie setting
		//
		//////////////////////////////////////////////////////////////////////////
		/** starting capturing a screen movie 
		@param sFileName: the movie file name, which can be "". If it is "", a default name is used.*/
		bool BeginCapture(const string& sFileName);
		/** end capturing a screen movie and save movie to file. */
		bool EndCapture();
		/** pause the capturing. */
		void PauseCapture();
		/** resume capturing */
		void ResumeCapture();
		/** whether we are doing screen capture. I.e. true between BeginCapture() and EndCapture()
		However, it may be in the recording or paused state. */
		bool IsInCaptureSession();
		/** whether it is recording.*/
		bool IsRecording();

		/** set the current render target, so that everything that is rendered subsequently will be recorded 
		* One must call UnsetCaptureTarget(), in a render frame.*/
		bool SetCaptureTarget();
		/** restore the render target to what it is when SetCaptureTarget() is called. 
		This usually restore render target to the back buffer */
		bool UnsetCaptureTarget();
		/** render (usually stretch) the capture target to the current render target at the specified location 
		* it will by default render to fill the entire render target.
		*/
		void RenderCaptured();

		/** save the captured frame to file. 
		* @param nMode: 0 for normal capture; 2 for capture only even lines, 3 for capture only odd lines.
		*/
		void SaveCapturedFrame(int nMode=0);

		/** by default it save the screen shot as JPG file in the ./screenshot directory. 
		@param filename: this is the file name without extension. It can be "".*/
		bool TakeScreenShot(const string& filename);

		/** Use a new thread to take a screenshot.
		* @param filename; Its included file path and file name.
		* @param bEncode;If true Enable Base64Encode.
		* @param width; The out value of Image's width, if width = -1 or height = -1, the out value same as the width of Windows.
		* @param height; The out value of Image's height, if width = -1 or height = -1, the out value same as the height of Windows.
		* @param screenshot_callback; It will be actived after take a screenshot.
		*/
		void TakeScreenShot_Async(const string& filename, bool bEncode = false, int width = -1, int height = -1, screenshot_callback callback = nullptr);
		/** Captures a bitmap buffer and uses FreeImage library to change it into a Image.
		*   the format of Image defined by the extension of filename.
		*   default format is PNG,also supported DDS JPG BMP TGA.
		* @param filename; Its included file path and file name.
		* @param outBase64Buffers; The result of Base64Encode.
		* @param bEncode; If true Enable Base64Encode.
		* @param width; The out value of Image's width, if width = -1 or height = -1, the out value same as the width of Windows.
		* @param height; The out value of Image's height, if width = -1 or height = -1, the out value same as the height of Windows.
		*/
		bool TakeScreenShot_FromGDI(const string& filename, std::vector<BYTE>& outBase64Buffers, bool bEncode = false, int width = -1, int height = -1);
		/** Captures a bitmap buffer through Windows GDI.
		* @param nHwnd; A windows handle.
		* @param outFileHeaderSize; Out the size of BITMAPFILEHEADER.
		* @param outInfoHeaderSize; Out the size of BITMAPINFOHEADER
		* @param outBuffers; Out the completely buffer of a Bitmap includes BITMAPFILEHEADER and BITMAPINFOHEADER.
		* @param bCaptureMouse;True capture the mouse,False otherwise.
		* @param nLeft; The margin left of screen start to be captured.
		* @param nTop; The margin top of screen start to be captured.
		* @param width; The width of screen to be captured.
		* @param height; The height of screen to be captured.
		*/
		int CaptureBitmapBuffer(HWND nHwnd, int& outFileHeaderSize, int& outInfoHeaderSize, std::vector<BYTE>& outBuffers, bool bCaptureMouse = false, int nLeft = 0, int nTop = 0, int width = 0, int height = 0);

		/** by default it save the screen shot as JPG file in the ./screenshot directory. 
		* @param filename: this is the file name without extension. It can be "".
		* @param width; in pixel, if 0 it will be the screen size
		* @param height; in pixel, if 0 it will be the screen size
		*/
        bool TakeScreenShot(const string& filename, int width, int height, bool isCenter);
		bool TakeScreenShot(const string& filename, int width, int height);

		/**
		* resize the given image
		* @param width; in pixel
		* @param height; in pixel
		* @param destFilename: destination file name
		*/
		bool ResizeImage(const string& filename, int width, int height, const string& destFilename);

		/**
		* get the given image info: i.e. size
		* @param width; out in pixel
		* @param height; out in pixel
		* @param nFileSize: out in size in bytes
		*/
		bool GetImageInfo(const string& filename, int* width, int* height, int* nFileSize);

		HRESULT InvalidateDeviceObjects(); // called just before device is Reset
		HRESULT RestoreDeviceObjects();    // called when device is restored

		/** if true, a dialog will be displayed for the user to select the code and settings to use; 
		otherwise the default codec will be used*/
		void EnableCodecSelection(bool bEnable);
		bool IsCodecSelectionEnabled();


		/** set the recording FPS, the default is 20 FPS. Some may prefer 30FPS. */
		void SetRecordingFPS(int nFPS);

		/** Get the recording FPS, the default is 20 FPS. Some may prefer 30FPS. */
		int GetRecordingFPS();

		/** 
		* set the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player. 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image; 
		*/
		void SetStereoCaptureMode(MOVIE_CAPTURE_MODE nMode = MOVIE_CAPTURE_MODE_NORMAL);

		/** 
		* Get the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player. 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image; 
		*/
		MOVIE_CAPTURE_MODE GetStereoCaptureMode();

		/** the distance in meters between the left and right eye when generating the stereo image. 
		* some common values are in range [0.03, 0.1]. This is also related to the rendering unit that we used in games. 
		* since ParaEngine games usually use meter as its rendering unit, the value is such near the real eye separation distance.*/
		void SetStereoEyeSeparation(float fDist);

		/** the distance in meters between the left and right eye when generating the stereo image. 
		* some common values are in range [0.03, 0.1]. This is also related to the rendering unit that we used in games. 
		* since ParaEngine games usually use meter as its rendering unit, the value is such near the real eye separation distance.*/
		float  GetStereoEyeSeparation();

		/** offset along the eye look at position. this will make the stereo scene pop out of paralex convergence plane */
		void SetStereoConvergenceOffset(float fDist);
		float GetStereoConvergenceOffset();

		/** get the current frame number to be recorded. This is usually used for stereo vision capture mode, when the even frame number is left eye, and odd is for right eye.
		* if capture has not begun, it is always 0.
		*/
		int GetCurrentFrameNumber();

		/** force capture the current frame using just the movie codec plugin. */
		void FrameCaptureFFMPEG();

		void FrameCaptureDX(IMovieCodec* pMovieCodec);

		/** whether we have the moviecodec plugin which uses ffmpeg internally. */
		bool HasMoviePlugin();

		void SetCaptureAudio(bool bEnable);
		bool IsCaptureAudio();

		void SetCaptureMic(bool bEnable);
		bool IsCaptureMic();

		bool IsUseGDI() const;
		void SetUseGDI(bool val);

	private:
		IMovieCodec* m_pMovieCodec;
		string m_sMovieName;
		string m_beginningStr;
		string m_endingStr;
		float m_fLastRefreshInterval;

		std::vector<std::function<void()>> m_functionsToPerform;
		std::mutex m_performMutex;
		
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DTEXTURE9 m_pCaptureTexture;
		LPDIRECT3DSURFACE9 m_pCaptureSurface;
		LPDIRECT3DSURFACE9 m_pBackBufferSurface;
		LPDIRECT3DSURFACE9 m_pOldDepthStencilSurface, m_pDepthStencilSurface; 
		LPDIRECT3DSURFACE9 m_pOffScreenSurface;
		HDC m_CompatibleHDC;
		HBITMAP m_BitmapHandle;
#endif

		bool m_bUseGDI;
		/** current recorder state */
		MOVIE_RECORDING_STATE m_recorderState;
		/** whether GUI is captured*/
		bool m_bCaptureGUI;
		/** render border when in capture mode*/
		bool m_bRenderBorder;
		/** whether to use a water mark, when capturing the movie*/
		bool m_bUseWaterMark;
		/** clip width in pixel*/
		int m_nScreenWidth;
		/** clip height in pixel*/
		int m_nScreenHeight;

		int m_nMarginLeft;
		int MarginLeft() const { return m_nMarginLeft; }
		void MarginLeft(int val) { m_nMarginLeft = val; }
		int m_nMarginRight;
		int MarginRight() const { return m_nMarginRight; }
		void MarginRight(int val) { m_nMarginRight = val; }
		int m_nMarginTop;
		int MarginTop() const { return m_nMarginTop; }
		void MarginTop(int val) { m_nMarginTop = val; }
		int m_nMarginBottom;
		int MarginBottom() const { return m_nMarginBottom; }
		void MarginBottom(int val) { m_nMarginBottom = val; }

		/** fps, default is 20*/
		int m_nRecordingFPS;
		/** if true, a dialog will be displayed for the user to select the code and settings to use; 
		otherwise the default codec will be used*/
		bool m_bAllowCodecSelection;

		/** Video capture mode	*/
		MOVIE_CAPTURE_MODE m_nStereoCaptureMode;

		/** the distance in meters between the left and right eye when generating the stereo image. 
		* some common values are in range [0.03, 0.1]. This is also related to the rendering unit that we used in games. 
		* since ParaEngine games usually use meter as its rendering unit, the value is such near the real eye separation distance.*/
		float m_nStereoEyeSeparation;

		/** offset along the eye look at position. this will make the stereo scene pop out of paralex convergence plane */
		float m_fStereoConvergenceOffset;

		bool m_isLeftEye;
	};

}
