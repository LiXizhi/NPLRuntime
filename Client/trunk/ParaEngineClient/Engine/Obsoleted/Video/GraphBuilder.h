#pragma once
#include "DShowTextures.h"

namespace ParaEngine
{
	/**
	* Usage:
	* LoadVideoFile before you want to play any video file. If you want to load other file, call LoadVideoFile again. 
	* When the device gets invalidate or restore, call RestoreDeviceObjects and InvalidateDeviceObjects respectively. 
	* The loaded video file will not get lost in this process. So you don't need to reload the file again.
	*/
	class CGraphBuilder
	{
	public:
		CGraphBuilder();
		~CGraphBuilder();
		/**
		* Loads a video file from disk.
		* @param wFileName: the input file name, in Unicode
		* @param bAudioOn: whether to allow audio output.
		* @param pTexMutex: for sharing the texture. 
		* @return: return S_OK if success, error code otherwise.
		* @remark: this function will also creates the render texture for the loaded video file. 
		*/
		HRESULT LoadVideoFile ( LPCWSTR wFileName, bool bAudioOn);
		/**
		* Start playing the video file
		* This will start playing a stopped video file or a paused video file.
		* @return: return S_OK if success, error code otherwise.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return E_FAIL or other error codes.
		*/
		HRESULT RunGraph ();
		/**
		* Pause a video file 
		* This will pause a playing video file.
		* @return: return S_OK if success, error code otherwise.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return E_FAIL or other error codes.
		*/
		HRESULT PauseGraph ();
		/**
		* Stop a video file 
		* This will stop a playing video file or a paused video file. The video position is reset to the beginning. 
		* @return: return S_OK if success, error code otherwise.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return E_FAIL or other error codes.
		*/
		HRESULT StopGraph ();
		/**
		* Seek a video file to a given percentage 
		* This will not affect the current state of the video file.
		* @param seekvalue: the percentage of the video file you want to seek to. The range is 0 to 100.
		* @return: return S_OK if success, error code otherwise.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return E_FAIL or other error codes.
		*/
		HRESULT Seek(double seekvalue);
		HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);
		HRESULT InvalidateDeviceObjects();
		/**
		* Get the current position of the video file in second.
		* This will not affect the current state of the video file.
		* @return: return the current second if success, 0 otherwise.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return 0.
		*/
		double GetCurrPos();
		/**
		* Get the length in second of the video file.
		* This will not affect the current state of the video file.
		* @return: return the current second if success, 0 otherwise.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return 0.
		*/
		double GetDuration();

		/**
		* Check if certain event such as a complete event happens.
		* This will not affect the current state of the video file.
		* @param bIsRepeat: whether to repeat the file from the beginning if it is completed.
		* @return: return the event code if success. If there is no event or an error occurs, this function returns 0;
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return 0.
		*  If bIsRepeat is set true, this function will automatically reset the playing position to 0.
		*/
		long CheckMovieStatus(bool bIsRepeat);
		void CleanupDShow(void);
		/**
		* Get the width of frame size of the video file
		* @return: return the width if success. If an error occurs, this function returns 0;
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return 0.
		*/
		int GetWidth();
		/**
		* Get the height of frame size of the video file
		* @return: return the height if success. If an error occurs, this function returns 0;
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return 0.
		*/
		int GetHeight();
		/**
		* Update and get the texture of the current frame. 
		* @return: return a pointer to the texture. If an error occurs, this function returns NULL.
		* @remark: make sure you have successfully loaded a video file. Otherwise, the function will return NULL.
		*/
		LPDIRECT3DTEXTURE9 GetTexture();

		/**
		* Check if the texture has been successfully created. If not, load the video file again.
		* @return: true if the texture is created, false if not.
		*/
		bool IsNeedCreateTexture(){return m_pTexture==NULL?true:false;}

	protected:
		HRESULT ConnectWMVFile( LPCWSTR wFileName );
		HRESULT ConnectOtherFile( LPCWSTR wFileName );
		LPDIRECT3DTEXTURE9 m_pTexture;
		CComPtr<IGraphBuilder> m_pGB;
		CComPtr<IMediaControl> m_pMC;	
		CComPtr<IMediaPosition> m_pMP;
		CComPtr<IMediaEvent> m_pME;
		IFileSourceFilter* m_pFileSource;
		CComPtr<IPin> m_pPinOut1;//for video
		CComPtr<IPin> m_pPinOut0;//for audio
		CComPtr<IBaseFilter> m_pRenderer1;
		CTextureRenderer *m_pRenderer;
		bool m_bLocked;
		bool m_bAudioOn;
	};
}