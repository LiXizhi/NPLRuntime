#pragma once
#include "mutex.h"
#include <streams.h>
#include <vector>

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;
//#define REGISTER_FILTERGRAPH

namespace ParaEngine
{
	using namespace std;

	class CGraphBuilder;
	/**
	* D3DFMT_X8R8G8B8 and RGB24 renderer. Supporting WMV and any video format codec available in the system. 
	*/
	class CTextureRenderer:public CBaseVideoRenderer
	{
	public:
		CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
		~CTextureRenderer();

		//void SetPinConnectState(BOOL bStatus);

		//////////////////////////////////////////////////////////////////////////
		//
		// the following function just override the base renderer interface. They are called in the codec thread. 
		//
		//////////////////////////////////////////////////////////////////////////

		/// Format acceptable
		HRESULT CheckMediaType(const CMediaType *pmt); 

		/** Video format notification, only by this time we know the size of the video, 
		* so that we can create our texture surface accordingly. 
		*/
		HRESULT SetMediaType(const CMediaType *pmt );       
		HRESULT GetConnectedMediaType(D3DFORMAT *uiTexFmt, LONG *uiVidBpp);
		
		/**
		* this is most important function. Called when a new sample has been rendered. 
		* we just keep it in a temporary buffer. 
		*/
		HRESULT DoRenderSample(IMediaSample *pMediaSample); 

		HRESULT GetDDMediaSample(IMediaSample *pSample);
#ifdef REGISTER_FILTERGRAPH
		HRESULT AddToROT(IUnknown *pUnkGraph);
		void RemoveFromROT(void);
#endif
	
	public:
		//////////////////////////////////////////////////////////////////////////
		//
		// the following functions provide interface for the game thread to retrieve video data.
		//
		//////////////////////////////////////////////////////////////////////////

		/** one can retrieve texture data by lock this. 
		* No longer to use this function if one uses UpdateTexture() to retrieve data directly into a d3d texture created by CreateTexture().
		*/
		HRESULT lock(byte** data, int* nSize);
		/** lock and unlock must be paired*/
		HRESULT unlock();
		
		/** this function will return when the video is initialized and its size is known. 
		* Internally it will wait for the codec to be ready. 
		* @note: there is a timeout of 100 milliseconds, if the codec still has no idea of the media type, it will return 0.*/
		int GetWidth();
		/** this function will return when the video is initialized and its size is known. 
		* Internally it will wait for the codec to be ready. 
		* @note: there is a timeout of 100 milliseconds, if the codec still has no idea of the media type, it will return 0.*/
		int GetHeight();

		/** this function will return when the video is initialized and its size is known. 
		* Internally it will wait for the codec to be ready. 
		* @note: there is a timeout of 100 milliseconds, if the codec still has no idea of the media type, it will return E_UNEXPECTED.*/
		HRESULT CreateTexture(LPDIRECT3DTEXTURE9 *pTexture);
		
		/** update the texture with the latest content. It is safe to call this function at higher frequency than the movie frame rate. 
		* it will internally detect if the current frame has changed since the last invocation of this function. */
		HRESULT UpdateTexture(LPDIRECT3DTEXTURE9 pTexture);

	protected:
		D3DFORMAT m_TextureFormat;
		LONG      m_VideoBpp;

		VIDEOINFO m_viBmp;
		bool m_bUseDynamicTextures;
		LONG m_lVidWidth,m_lVidHeight,m_lVidPitch;
		bool m_bUpSideDown;
		HANDLE m_hInitDone;

		Mutex mMutex;
		vector <byte> m_textureData;
		bool m_bFrameChanged;
	};
}