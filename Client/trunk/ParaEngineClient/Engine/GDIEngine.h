#pragma once
#include <gdiplus.h>
#include <map>
#include "util/intrusive_ptr.h"

namespace Gdiplus
{
	class Image;
	class Bitmap;
	class Graphics;
	class Matrix;
}

namespace ParaEngine
{
	struct DXUT_SCREEN_VERTEX;

	/** a gdi render target is a Gdiplus::Bitmap and Gdiplus::Graphics object. */
	class CGDIRenderTarget : public ParaEngine::intrusive_ptr_single_thread_base
	{
	public:
		/** @param nImageSize: render target size.  if provided, it will be initialized. */
		CGDIRenderTarget(int nImageSize = 0);
		virtual ~CGDIRenderTarget();

		/** call this function to init render target to a new size */
		void Init(int nImageSize);

		/** called to invalidate all interface */
		void Cleanup();

		/** if interface is initialized and valid. */
		bool IsValid();

		/** get the render target size. */
		int GetRenderTargetSize(){return m_nRenderTargetSize;}

		/* get the graphics interface */
		Gdiplus::Graphics* GetGraphics() {return m_pGraphics;}

		/* get the bitmap interface */
		Gdiplus::Bitmap* GetBitmapSurface(){return m_pBitmapSurface;}

		/** the current render target. */
		Gdiplus::Bitmap* m_pBitmapSurface;

		/** the current render target size, default to 256 pixels. */
		int m_nRenderTargetSize;

		/** the default graphics for the render target. */
		Gdiplus::Graphics* m_pGraphics;
	};
	typedef ParaIntrusivePtr<CGDIRenderTarget> CGDIRenderTarget_ptr;

	/** this allows us to create another device, such as in a worker thread to 
	* perform some background work while the main directX device has other work to do. */
	class CGDIEngine
	{
	public:
		typedef std::map<std::string, Gdiplus::Bitmap*> TextureAsset_Map_Type;
		typedef std::map<std::string, CGDIRenderTarget_ptr> RenderTarget_Map_Type;

		CGDIEngine();
		~CGDIEngine(void);
	public:
		virtual HRESULT Create();
		virtual HRESULT Destroy();
		
	public:
		//some GUI functions here 
		HRESULT Clear(const Color &color);
		
		/** set transform */
		void SetTransform(const Gdiplus::Matrix * matTransform);

		// actually no need to be called. since we never present to windows. We only render to render target. 
		bool Begin();

		/** draw image at given position. */
		bool DrawImage(Gdiplus::Image *image, float x, float y,float width, float height);

		/** draw image and pre-multiply it with color. This function uses ImageAttributes for color transformation, 
		so it can be very very slow. Use it sparingly. */
		bool DrawImage(Gdiplus::Image *image, float x, float y,float width, float height, DWORD dwColor);

		void End();

		void TestMe();

		/** load a texture. it will cache the file in memory. 
		* @param filename: if filename is "256", it is render target
		*/
		Gdiplus::Bitmap* LoadTexture(const string& filename);

		/** load a texture. it will cache the file in memory. 
		* @param filename: if filename is "256", it is render target
		* @param nFileFormat: one of the FREE_IMAGE_FORMAT. 
		*/
		Gdiplus::Bitmap* LoadTexture(const string& filename, int nFileFormat);

		/**
		* same as above, except that file format is determined by fileextension. 
		*/
		Gdiplus::Bitmap* LoadTexture(const string& filename, const string& fileextension);

		/** create get a render target. 
		* if a previously create render target exist, the second parameter nRenderTargetSize will be ignored. 
		*/
		CGDIRenderTarget_ptr CreateGetRenderTarget(const std::string& sName, int nRenderTargetSize = 256);

		/** create get a render target whose name is tostring(size).  For example, if size is 256, then a render target of "256" will be created and returned. 
		* this is a handy function to the above CreateGetRenderTarget() function. 
		*/
		CGDIRenderTarget_ptr CreateGetRenderTargetBySize(int nRenderTargetSize = 256);
		
		/** set render target to a given texture. 
		*/
		bool SetRenderTarget(CGDIRenderTarget_ptr pRenderTarget);

		/** save the current render target to file. 
		* @param colorKey: if 0 no color key is used. for black color key use 0xff000000
		*/
		bool SaveRenderTarget(const string& filename, int nWidth=256, int nHeight=256, bool bHasAlpha=true, DWORD colorKey = 0);
	private:
		/** if device objects are all valid. */
		bool IsValid();

		/** GDI+ for side-by-side texture generation in the IO thread.*/
		HDC m_hDC;

		/** all previously loaded textures. */
		TextureAsset_Map_Type m_textures;

		/** all render target created. */
		RenderTarget_Map_Type m_render_targets;

		/** the current render target. */
		CGDIRenderTarget_ptr m_pRenderTarget;

		/** the default render target */
		CGDIRenderTarget_ptr m_pDefaultRenderTarget;
		
		/** the current render target size, default to 256 pixels. */
		int m_nRenderTargetSize;

		/** the default graphics for the render target. */
		Gdiplus::Graphics* m_pGraphics;
	};
}
