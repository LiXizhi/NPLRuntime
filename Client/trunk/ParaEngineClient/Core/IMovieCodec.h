#pragma once

namespace ParaEngine
{
	/** Video capture mode	*/
	enum MOVIE_CAPTURE_MODE
	{
		MOVIE_CAPTURE_MODE_NORMAL = 0,
		MOVIE_CAPTURE_MODE_STEREO_LINE_INTERLACED,
		MOVIE_CAPTURE_MODE_STEREO_LEFT_RIGHT,
		MOVIE_CAPTURE_MODE_STEREO_ABOVE_BELOW,
		MOVIE_CAPTURE_MODE_STEREO_FRAME_INTERLACED,
	};

	/** for movie capturing */
	class IMovieCodec
	{
	public:
		virtual void Release() = 0;

		/**encode a random dummy video to the given filename with given codec. */
		virtual void video_encode_example(const char *filename, int codec_id) = 0;

		/** set preferred codec */
		virtual void SetCodec(int nCodec) = 0;

		/** get preferred codec */
		virtual int GetCodec() = 0;

		/** begin recording to a given file. 
		* Please note that the height and width may be different. since it is internally converted to multiples of 4. 
		*/
		virtual int BeginCapture(const char *filename, HWND nHwnd, int nLeft = 0, int nTop = 0, int width = 0, int height = 0, int m_nFPS = 0, int codec_id = 0) = 0;

		/** capture the current frame. 
		* @param pData: raw RGB array. 3 bytes per pixels
		* @param nDataSize: data size in bytes. it must be width*height*3
		*/
		virtual int FrameCapture(const BYTE* pData, int nDataSize = 0, int* pnFrameCount = 0) = 0;

		/** end recording for the current file. */
		virtual int EndCapture() = 0;

		/** if we are recording */
		virtual bool IsRecording() = 0;

		/** get current filename */
		virtual const char* GetFileName() = 0;

		/** get current frame number */
		virtual int GetCurrentFrameNumber() = 0;

		/** get current width, rounded to multiple of 4. */
		virtual int GetWidth() = 0;

		/** get current height, rounded to multiple of 4. */
		virtual int GetHeight() = 0;

		virtual void SetVideoBitRate(int nRate) = 0;

		virtual int GetVideoBitRate() = 0;

		virtual void SetCaptureAudio(bool bEnable) = 0;
		virtual bool IsCaptureAudio() = 0;

		virtual void SetCaptureMic(bool bEnable) = 0;
		virtual bool IsCaptureMic() = 0;

		/**
		* set the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player.
		*  - 0 for disable stereo capture(default);
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo;
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image;
		*/
		virtual void SetStereoCaptureMode(MOVIE_CAPTURE_MODE nMode = MOVIE_CAPTURE_MODE_NORMAL) = 0;

		/**
		* Get the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player.
		*  - 0 for disable stereo capture(default);
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo;
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image;
		*/
		virtual MOVIE_CAPTURE_MODE GetStereoCaptureMode() = 0;
	};
}