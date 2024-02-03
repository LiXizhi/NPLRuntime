#pragma once
#include "BaseObject.h"
#include "PaintEngine/PaintDevice.h"

namespace ParaEngine
{
	struct TextureEntity;
	class ImageEntity;
	class CPaintEngine;
	class CPainter;

	/** a render target scene object.
	* render target is not initialized until the first Begin() End() pair is called.
	* to update the render target, draw objects between Begin() and End().
	*/
	class CRenderTarget : public CBaseObject, public CPaintDevice
	{
	public:
		CRenderTarget();
		virtual ~CRenderTarget();

		ATTRIBUTE_DEFINE_CLASS(CRenderTarget);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CRenderTarget);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CRenderTarget, GetClearColor_s, Vector3*) { *p1 = cls->GetClearColor().ToVector3(); return S_OK; }
		ATTRIBUTE_METHOD1(CRenderTarget, SetClearColor_s, Vector3) { LinearColor c(p1.x, p1.y, p1.z, 1); cls->SetClearColor(c); return S_OK; }

		ATTRIBUTE_METHOD1(CRenderTarget, GetRenderTargetSize_s, Vector2*) { *p1 = cls->GetRenderTargetSize(); return S_OK; }
		ATTRIBUTE_METHOD1(CRenderTarget, SetRenderTargetSize_s, Vector2) { cls->SetRenderTargetSize(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CRenderTarget, IsActiveRenderingEnabled_s, bool*) { *p1 = cls->IsActiveRenderingEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CRenderTarget, EnableActiveRendering_s, bool) { cls->EnableActiveRendering(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CRenderTarget, IsDirty_s, bool*) { *p1 = cls->IsDirty(); return S_OK; }
		ATTRIBUTE_METHOD1(CRenderTarget, SetDirty_s, bool) { cls->SetDirty(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CRenderTarget, IsPersistentRenderTarget_s, bool*) { *p1 = cls->IsPersistentRenderTarget(); return S_OK; }
		ATTRIBUTE_METHOD1(CRenderTarget, SetPersistentRenderTarget_s, bool) { cls->SetPersistentRenderTarget(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CRenderTarget, SaveToFile_s, const char*) { cls->SaveToFile(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CRenderTarget, RunCommandList_s, char*) { cls->RunCommandList(p1); return S_OK; }

		/** define the On_Paint script callback for painting in 2d space. */
		DEFINE_SCRIPT_EVENT(CRenderTarget, Paint);


	public:
		virtual CPaintEngine* paintEngine() const;
		virtual int metric(PaintDeviceMetric metric) const;

		/** initializes a RenderTexture object with width and height in Points and a pixel format( only RGB and RGBA formats are valid ) and depthStencil format*/
		bool InitWithWidthAndHeight(int width, int height, D3DFORMAT format = D3DFMT_A8R8G8B8, D3DFORMAT depthStencilFormat = D3DFMT_D16);

		virtual int PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState);
		/// only for drawable objects
		virtual HRESULT Draw(SceneState* sceneState);

		/** starts rendering to texture. /sa ScopedPaintOnRenderTarget */
		virtual bool Begin();
		virtual bool Begin(int x, int y, int w, int h);

		void CheckInit();

		/** end rendering to texture, restore old render target */
		virtual void End();

		/** this is usually the first function to call after calling begin(). */
		virtual void Clear(const LinearColor& color, float depthValue = 1.f, int stencilValue = 0, DWORD flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER);

		/** if true, contents will be drawn to the current render target each frame. Default value is false. where the user can call DrawToTexture() on demand.*/
		void EnableActiveRendering(bool bEnable);

		/** if true, contents will be drawn to the current render target each frame. Default value is false. where the user can call DrawToTexture() on demand.*/
		bool IsActiveRenderingEnabled() const;

		/** init device objects.
		* the create() function must be called, before this function is called.*/
		HRESULT InitDeviceObjects();
		/** restore device object*/
		HRESULT RestoreDeviceObjects();
		/** Invalid device object*/
		HRESULT InvalidateDeviceObjects();
		/** delete device objects */
		HRESULT DeleteDeviceObjects();
		/** clean up all resource objects */
		virtual void Cleanup();
		/** invoke the On_Paint script event handler if any. Normally this is called between Begin() and End().
		* subclass can also inherit this method, to provide its own custom drawing functions.
		*/
		virtual void DoPaint(CPainter* painter = NULL);

		/** set the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		void SetClearColor(const LinearColor& bgColor);
		/** Get the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		LinearColor GetClearColor();

		/** default to false, where the render target texture will be deleted when scene object is deleted.
		* if set to true, we will not delete the render target even if the current object is deleted.
		* For example, sometimes, the render target may be used by GUI objects which lasts cross different scenes,
		* hence we need to set to true for those render target.
		*/
		bool IsPersistentRenderTarget() const;
		void SetPersistentRenderTarget(bool val);

		/**
		* the canvas texture, which can be used as any other ordinary texture on 3D or 2D object.
		* @return
		*/
		virtual TextureEntity* GetTexture();
		/** get the asset render target object. It will force init the render target if it is not. */
		virtual AssetEntity* GetPrimaryAsset();

		/**
		* save o a different texture file format and save with full mipmapping to disk.
		* @param sFileName a texture file path to save the file to.
		*	we support ".dds", ".jpg", ".png" files. If the file extension is not recognized, ".png" file is used.
		*  it can also be "output.jpg;0 0 width height" to specify srcLeft, srcTop, srcWidth, srcHeight, such as via Attribute Field
		* @param width: if this is zero, the original size is used. If it is dds, all mip map levels are saved.
		* @param dwDXTFormat: if 0, it mean automatic according to file extension. if filename is *.dds, it will be saved as DXT3.
		*		1,2,3,4,5, stands for D3DFMT_DXT1-5.
		* @param srcLeft, srcTop, srcWidth, srcHeight: which portion of the scene to save to the file. default to full rect.
		*/
		HRESULT SaveToFile(const char* filename, int width = 0, int height = 0, DWORD dwFormat = 3, UINT MipLevels = 0, int srcLeft = 0, int srcTop = 0, int srcWidth = 0, int srcHeight = 0);

		/* creates a new Image from with the texture's data.
		Caller is responsible for releasing it by calling delete.
		* @param colorKey: if 0 no color key is used. for black color key use 0xff000000
		*/
		ImageEntity* NewImage(bool bFlipImage = true, Color colorKey = 0);

		/**
		* set the canvas size in pixels
		* @param nWidth
		* @param nHeight
		*/
		void SetRenderTargetSize(int nWidth, int nHeight);
		void GetRenderTargetSize(int* nWidth, int* nHeight);
		void SetRenderTargetSize(const Vector2& size);
		Vector2 GetRenderTargetSize();

		int GetTextureWidth() const { return m_nTextureWidth; }
		int GetTextureHeight() const { return m_nTextureHeight; }

		/** whether render target is dirty and should be redraw on the next frame. */
		bool IsDirty() const;
		void SetDirty(bool val);

		int GetLifeTime() const;
		void SetLifeTime(int val);

		virtual bool IsDead();
		virtual void SetDead();

		const std::string& GetCanvasTextureName();
		void SetCanvasTextureName(const std::string& sValue);

		virtual HRESULT RendererRecreated() override;

		void RunCommandList(const char* cmd);
	protected:
		// whether device is created.
		bool		m_bInitialized;

		bool		m_bPersistentRenderTarget;
		int			m_nTextureWidth;
		int         m_nTextureHeight;
		/* -1 means alive forever. 0 means dead. any other value means number of frames to live before it is released. */
		int m_nLifeTime;

		DWORD		m_depthStencilFormat;
		/** render target*/
		asset_ptr<TextureEntity> m_pCanvasTexture;
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DSURFACE9      m_pCanvasSurface;
		LPDIRECT3DSURFACE9      m_pDepthStencilSurface;

		LPDIRECT3DSURFACE9 m_pOldRenderTarget;
		LPDIRECT3DSURFACE9 m_pOldZBuffer;
#elif defined USE_OPENGL_RENDERER
		GLuint       _FBO;
		GLuint       _depthRenderBufffer;
		GLint        _oldFBO;
		GLint		 _oldRBO;
#endif
		D3DVIEWPORT9 m_oldViewport;
		Vector3 m_vOldRenderOrigin;

		/** used when clearing background color */
		DWORD m_dwClearColor;

		/* whether we are currently inside begin() end() function pair. */
		bool m_bIsBegin;

		/** if true, contents will be drawn to the current render target each frame. Default value is false. where the user can call DrawToTexture() on demand.*/
		bool m_bActiveRendering;
		/** whether render target is dirty and should be redraw on the next frame. */
		bool m_bIsDirty;
		/** render target canvas name */
		std::string m_sCanvasTextureName;

		mutable CPaintEngine* engine;
	};

	/** helper class for painting on render target*/
	class ScopedPaintOnRenderTarget
	{
	public:
		ScopedPaintOnRenderTarget(CRenderTarget* pRendertarget) :m_pRenderTarget(pRendertarget) {
			if (m_pRenderTarget)
				m_pRenderTarget->Begin();
		}
		ScopedPaintOnRenderTarget(CRenderTarget* pRendertarget, int x, int y, int w, int h) :m_pRenderTarget(pRendertarget) {
			if (m_pRenderTarget) {
				m_pRenderTarget->Begin(x, y, w, h);
			}
		}
		~ScopedPaintOnRenderTarget() {
			if (m_pRenderTarget) {
				m_pRenderTarget->End();
			}
		};
	public:
		CRenderTarget* m_pRenderTarget;
	};
}

