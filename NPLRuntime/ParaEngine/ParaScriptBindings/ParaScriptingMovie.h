//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.11
// Description:	API for AVI based  movie. 
//-----------------------------------------------------------------------------
#pragma once
#include "ParaScriptingCommon.h"

#include <string>
namespace ParaScripting
{
	using namespace std;
	/** 
	* @ingroup ParaMovie
	movie making and screen capture functions  */
	class PE_CORE_DECL ParaMovie
	{
	public:
		/** get the attribute object associated with an object. */
		static ParaAttributeObject GetAttributeObject();

		/** set the movie screen size in pixel. 
		@param nWidth: in pixels, must be multiple of 4. 
		@param nHeight: in pixels, must be multiple of 4. */
		static void SetMovieScreenSize(int nWidth, int nHeight);

		/** set whether GUI is captured.*/
		static void SetCaptureGUI(bool bGUI);
		/** return true if GUI is also captured.*/
		static bool CaptureGUI();

		/** set the recording FPS, the default is 20 FPS. Some may prefer 30FPS. */
		static void SetRecordingFPS(int nFPS);

		/** Get the recording FPS, the default is 20 FPS. Some may prefer 30FPS. */
		static int GetRecordingFPS();

		/** 
		* set the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player. 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image; 
		*/
		static void SetStereoCaptureMode(int nMode);

		/** 
		* Get the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player. 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for iPod mode, where the odd frame is the left eye and even frame is the right image; 
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for interlaced stereo.
		*/
		static int GetStereoCaptureMode();


		/** the distance in meters between the left and right eye when generating the stereo image. 
		* some common values are in range [0.03, 0.1]. This is also related to the rendering unit that we used in games. 
		* since ParaEngine games usually use meter as its rendering unit, the value is such near the real eye separation distance.*/
		static void SetStereoEyeSeparation(float fDist);

		/** the distance in meters between the left and right eye when generating the stereo image. 
		* some common values are in range [0.03, 0.1]. This is also related to the rendering unit that we used in games. 
		* since ParaEngine games usually use meter as its rendering unit, the value is such near the real eye separation distance.*/
		static float  GetStereoEyeSeparation();

		/** starting capturing a screen movie 
		@param sFileName: the movie file name, which can be "". If it is "", a default name is used.*/
		static bool BeginCapture(string sFileName);
		/** end capturing a screen movie and save movie to file. */
		static bool EndCapture();
		/** pause the capturing. */
		static void PauseCapture();
		/** resume capturing */
		static void ResumeCapture();
		/** whether we are doing screen capture. I.e. true between BeginCapture() and EndCapture()
		However, it may be in the recording or paused state. */
		static bool IsInCaptureSession();
		/** whether it is recording.*/
		static bool IsRecording();
		/** capture a given frame*/
		static bool FrameCapture();

		/** get movie screen size. In script. Use like this : local x,y = ParaMovie.GetMovieScreenSize(); */
		static void GetMovieScreenSize(int* nWidth, int* nHeight);
		/** get the movie file name */
		static string GetMovieFileName();

		/** display a dialog which allows the user to set the codec to be used. */
		static void SelectCodecOptions();

		/** set the code
		@param eType: 0 for XVID; 1 for WMV; -1 for user select codec.*/
		static void SetEncodeMethod(int eType);
		/** @see SetEncodeMethod()*/
		static int GetEncodeMethod();

		/** 
		* we will automatically take screen shot according to the file extensions,
		* supported file extensions are "jpg","dds","bmp","tga",
		* @note: "jpg" has small file size; where "bmp" and "tga" is lossless.
		* @param filename: this is the file name.If this is NULL or "", it will be automatically named as jpg file under the "screen shots" directory.
		*/
		static bool TakeScreenShot(const char* filename);


		/** 
		* render the current scene to texture, UI are disabled by default. Aspect ratio are changed according to width/height. 
		* supported file extensions are "jpg","dds","bmp","tga",
		* @note: "jpg" has small file size; where "bmp" and "tga" is lossless.
		* @param filename: this is the file name.If this is NULL or "", it will be automatically named as jpg file under the "screen shots" directory.
		* @param width; in pixel, if 0 it will be the screen size
		* @param height; in pixel, if 0 it will be the screen size
		*/
		static bool TakeScreenShot3(const char* filename, int width, int height);

		// Use a new thread to take a screenshot.
		static void TakeScreenShot_Async(const char* filename, const char* sCallBackScript);
		static void TakeScreenShot2_Async(const char* filename, bool bEncode, const char* sCallBackScript);
		static void TakeScreenShot3_Async(const char* filename, bool bEncode, int width, int height, const char* sCallBackScript);
		static void TakeScreenShot_Async_Internal(const char* filename, bool bEncode, int width, int height, const char* sCallBackScript);

		static bool RenderToTexture(const char* filename, int width, int height);
		
		/**
		* resize the given image. It can also be used to change the file format
		* @param filename: source file name
		* @param width; in pixel
		* @param height; in pixel
		* @param destFilename: destination file name. If nil or "", it will be the same as input. It can also be used to change the file format
		*/
		static bool ResizeImage(const string& filename, int width, int height, const char* destFilename);

		/**
		* get the given image info: i.e. size
		* e.g. local width, height, filesize = ParaMovie.GetImageInfo("abc.jpg")
		* @param width; out in pixel
		* @param height; out in pixel
		* @param nFileSize: out in size in bytes
		*/
		static void GetImageInfo(const string& filename, int* width, int* height, int* nFileSize);

	};

}
