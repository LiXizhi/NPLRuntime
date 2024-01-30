#pragma once
#include "AssetEntity.h"

namespace ParaEngine
{
	class ImageEntity;
	class ParaImage;

	struct ImageExtendInfo
	{
		ImageExtendInfo()
			: FocalLength(0.0f)
		{
		}

		// lens focal length, unit is mm
		double FocalLength;
	};



	/** 
	Which DXT Compression to Use?
	Obviously, there are some trade-offs between the different formats which make them better or worse for different types of images. Some general rules of thumb for good use of DXT textures are as follows:
	If your image has no alpha, use DXT1 compression. Using DXT3/5 will double your image size over DXT1 and not gain anything.
	If your image has 1-bit (on or off) alpha information, use DXT1 with one-bit alpha. If the DXT1 image quality is too low and you don't mind doubling image size, use DXT3 or DXT5 (which one doesn't matter, they'll give the same results).
	If your image has smooth gradations of alpha (fading in/out slowly), DXT5 is almost certainly your best bet, as it will give you the most accurate transparency representation.
	If your image has sharp transitions between multiple alpha levels (one pixel is 100%, the next one is 50%, and another neighbor is 12%), DXT3 is probably your best bet. You may want to compare the alpha results in DXT1, DXT3 and DXT5 compression, however, to make sure.
	*/
	struct TextureEntity : public AssetEntity
	{
	private:
		friend class CTextureProcessor;
	public:
		/** default texture to replaced downloading one. */
		static const std::string DEFAULT_STATIC_TEXTURE;

		/** Most detailed level-of-detail value to set for the mipmap chain. Default value is 0
		More specifically, if the texture was created with the dimensions of 256x256, setting the most detailed level to 0
		indicates that 256 x 256 is the largest mipmap available, setting the most detailed level to 1 indicates that 128 x 128
		is the largest mipmap available, and so on, up to the most detailed mip level (the smallest texture size) for the chain.
		*/
		static int g_nTextureLOD;

		/** whether we will load files whose name ends with _32bits as 32 bits textures.
		* disabling this will reduce memory usage by 6 times, but the rendered image will not be as sharp. since DXT5 or DXT3 compression is used instead. */
		static bool g_bEnable32bitsTexture;

		/* get image format by filename 
		* @return : -1 unknown, 24 dds, 13 png, 2 jpg, 17 tga
		*/
		static int GetFormatByFileName(const std::string& filename);
	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::texture; };

		enum _SurfaceType
		{
			/// render target, call SetTextureInfo() to specify size. if SetTextureInfo()
			/// is never called, the back buffer size is used.
			RenderTarget = 0,
			/// normal managed texture, with all mip-mapping level created
			StaticTexture,
			/// a group of textures, such as xxx_a001.jpg, ..., xxx_a009.jpg
			TextureSequence,
			/// texture in memory
			SysMemoryTexture,
			/// BLP textures
			BlpTexture,
			/// detailed terrain texture
			TerrainHighResTexture,
			/// cube texture for environment mapping, etc. 
			CubeTexture,
			/// flash textures, such as swf and flv streaming video files, etc. 
			FlashTexture,
			/// HTML renderer textures, <html>name:initial page url 
			HTMLTexture,
			/// a pure texture surface
			TextureSurface,
			/// depth stencil surface 
			DEPTHSTENCIL,
		} SurfaceType;

		/** basic texture information */
		struct TextureInfo
		{
			static const TextureInfo Empty;
			int m_width, m_height;
			
			enum TEXTURE_FORMAT{
				FMT_A8R8G8B8,
				FMT_ALPHA8,
				FMT_X8R8G8B8,
				FMT_UNKNOWN
			}m_format;
			
			enum SUB_TYPE{
				TYPE_SCREENSIZE,
				TYPE_UNKNOWN
			}m_subtype;
			
		public:
			TextureInfo(int width, int height, TEXTURE_FORMAT format, SUB_TYPE subtype)
				: m_width(width), m_height(height), m_format(format), m_subtype(subtype){};
			TextureInfo()
				: m_width(0), m_height(0), m_format(FMT_UNKNOWN), m_subtype(TYPE_UNKNOWN){};
			TextureInfo(const TextureInfo& t)
				: m_width(t.m_width), m_height(t.m_height), m_format(t.m_format), m_subtype(t.m_subtype){};
			virtual ~TextureInfo(){};
			
			TEXTURE_FORMAT GetFormat() const { return m_format; }
			int GetHeight() const { return m_height; }
			int GetWidth() const { return m_width; }
		};

		/** for animated texture*/
		struct AnimatedTextureInfo : TextureInfo{
			/// default value is 15
			float  m_fFPS;
			int m_nFrameCount;
			int m_nCurrentFrameIndex;
			/// if true, the animation texture will be automatically animated.
			bool m_bAutoAnimation;
		public:
			AnimatedTextureInfo() :m_fFPS(15.f), m_nFrameCount(0), m_nCurrentFrameIndex(0), m_bAutoAnimation(true){};
			virtual ~AnimatedTextureInfo(){};
		};

		/// it may be different struct for different surface type, but all must inherit from TextureInfo virtual base
		union
		{
			/// currently all surface types except the animated texture use this struct
			TextureInfo* m_pTextureInfo;
			/// only animated texture use this type.
			AnimatedTextureInfo* m_pAnimatedTextureInfo;
		};

		/** raw data */
		char* m_pRawData;
		/** raw data size */
		int32 m_nRawDataSize;

		// this value will be increased by one every time GetTexture() or GetSurface() is called. 
		int32 m_nHitCount;
		// the color key(default to 0):  Color value to replace with transparent black, or 0 to disable the color key. This is always a 32-bit ARGB color, 
		// independent of the source image format. Alpha is significant and should usually be set to FF for opaque color keys. 
		// Thus, for opaque black, the value would be equal to 0xFF000000. COLOR_XRGB(0,0,0)
		Color m_dwColorKey;

		/** whether to async loading the texture. this is enabled by default. */
		bool m_bAsyncLoad;

		bool m_bEmbeddedTexture;
	public:
		TextureEntity(const AssetKey& key);
		TextureEntity();
		virtual ~TextureEntity();

		// TODO: remove this function from base class after refactoring. 
		// the caller needs to cast to implementation class to use this function. 
		/** Get the texture for rendering */
		virtual DeviceTexturePtr_type GetTexture() { return 0; };

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		/** load from memory buffer.
		* @param ppTexture: if NULL, we will save to current asset, if not we will save to this object.
		*/
		virtual HRESULT LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN, void** ppTexture = NULL) { return E_FAIL; };

		
		/**  Initializes a texture from an ImageEntity object.
		* NOTE: It will not convert the pvr image file.
		* @param dwTextureFormat: if 0, we will use the image file format. 
		*/
		virtual bool LoadFromImage(ImageEntity * image, D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN, UINT nMipLevels = 0, void** ppTexture = NULL);


		/** this function is mostly used internally.
		* this function will return immediately. It will append the texture request to AsyncLoaders's IO queue.
		* @param pContext: this should be a pointer to CAsyncLoader
		* @param pDev: if NULL, the default render device is used
		* @param sFileName: if NULL, m_asset->GetLocalFileName() is used.
		* @param ppTexture: if this is NULL, the m_asset->m_pTexture is sued.
		* @param dwTextureFormat: the format of the texture, default to D3DFMT_UNKNOWN
		* @param nMipLevels: Mip levels, default to D3DX_DEFAULT
		* @param dwColorKey: color key. default to 0(disabled). Use COLOR_XRGB(0,0,0) if blank is treated transparent.
		*/
		virtual HRESULT CreateTextureFromFile_Async(void* pContext, RenderDevicePtr pDev = NULL, const char* sFileName = NULL, void** ppTexture = NULL, D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN, UINT nMipLevels = D3DX_DEFAULT, Color dwColorKey = 0);

		/** whether to async loading the texture. this is enabled by default. */
		bool IsAsyncLoad() const;
		void SetAsyncLoad(bool val);

		// make this texture render target. 
		virtual bool SetRenderTarget(int nIndex = 0) { return false; };
		
		/** whether we will use blocky or non-blocky (anti-aliased) texture */
		virtual void SetSamplerStateBlocky(bool bIsBlocky);
		virtual bool IsSamplerStateBlocky();

		// the color key(default to 0):  Color value to replace with transparent black, or 0 to disable the color key. This is always a 32-bit ARGB color, 
		// independent of the source image format. Alpha is significant and should usually be set to FF for opaque color keys. 
		// Thus, for opaque black, the value would be equal to 0xFF000000. COLOR_XRGB(0,0,0)
		void SetColorKey(Color colorKey);

		Color GetColorKey();

		/**
		* most assets are loaded asynchronously. This allows us to check if an asset is loaded.
		* For example, we can LoadAsset() for a number of assets that need preloading. and then use a timer to check if they are initialized and remove from the uninialized list.
		*/
		virtual bool IsLoaded();

		/** whether the asset is being loaded. */
		bool IsPending();
		void SetEmbeddedTexture(bool bEmbeddedTexture) { m_bEmbeddedTexture = bEmbeddedTexture; }
		bool IsEmbeddedTexture() { return m_bEmbeddedTexture; }

		/** set image from which to load the texture. image ownership is transfered to this entity. the caller should never delete the image. instead
		this entity will delete the image. */
		void SetImage(ParaImage* pImage);
		/* set raw texture data form which to load the texture, data will load by ParaImage */
		bool SetRawDataForImage(const char* pData, int nSize, bool bDeleteData = true);
		bool SetRawDataForImage(const unsigned char* pixel, size_t datalen, int width, int height, int bitsPerComponent, bool preMulti, bool bDeleteData = true);

		/* get image */
		const ParaImage* GetImage() const;
		/* */
		void SwapImage(TextureEntity* other);

		/** set raw texture data from which to load the texture. data ownership is transfered to this entity. the caller should never delete the data. instead
		this entity will delete the data. */
		void SetRawData(char* pData, int nSize);
		/** get raw data */
		char* GetRawData();
		/** get raw data size */
		int GetRawDataSize();
		/** give up raw data ownership and reset raw data to empty*/
		bool GiveupRawDataOwnership();

		/** reset the texture info. One do not need to set the texture info if they intend to load texture from file
		* if the texture is already initialized, calling this function will cause the texture be recreated using the new
		* texture information. If the texture is not initialized, calling this function will set the creation parameters
		* E.g. one can call this function to create a render target of arbitrary size.
		*/
		virtual void SetTextureInfo(const TextureInfo& tInfo);
		/** get texture info.
		* return NULL if the texture has never been set before.
		*/
		virtual const TextureInfo* GetTextureInfo();

		virtual int32 GetWidth();
		virtual int32 GetHeight();

		/** in most cases this is false, where the image origin is at left, top. 
		* however, in opengl frame buffer, the frame buffer's origin is at left, bottom. 
		*/
		virtual bool IsFlipY();

		/* this value will be increased by one every time GetTexture() or GetSurface() is called.
		* normally we never needs to care about this. Except in some rare cases, such as the mini scene graph needs to know if its being used by others in the last frame. */
		int GetHitCount(){ return m_nHitCount; }

		/* this value will be increased by one every time GetTexture() or GetSurface() is called.
		* normally we never needs to care about this. Except in some rare cases, such as the mini scene graph needs to know if its being used by others in the last frame. */
		void SetHitCount(int nHitCount){ m_nHitCount = nHitCount; }

		/** make the current texture an invalid one. It will release all resources and change the texture type to static and reference an empty texture path.
		After calling this function, one will no longer intend to use this entity any more. and other object reference this texture will render a blank image. */
		void MakeInvalid();

		/**
		* refresh this texture surface with a local file.
		* @param sFilename: if NULL or empty the old texture file(sTextureFileName) will be used.
		* @param bLazyLoad if true it will be lazy loaded.
		*/
		void Refresh(const char* sFilename = NULL, bool bLazyLoad = false);

		/**
		* get the animated texture information struct. this only return valid pointer if surface type is animated texture.
		* @return NULL if not succeeds.
		*/
		AnimatedTextureInfo* GetAnimatedTextureInfo();

		/**
		* For animated textures. set the FPS for animation textures. this provides a short cut to animated textures
		* @param nFPS frames per seconds. default value is 15 FPS
		*/
		void SetTextureFPS(float FPS);
		/**
		* For animated textures. whether to enable texture animation. this provides a short cut to animated textures
		* @param bEnable default value is true. Set this to false, if one wants to manually animate the texture, such as controlling from the scripting interface.
		*/
		void EnableTextureAutoAnimation(bool bEnable);
		/**
		* For animated textures. set the current frame number. this provides a short cut to animated textures
		* @param nFrame
		*/
		void SetCurrentFrameNumber(int nFrame);
		/**
		* For animated textures. Get the current frame number. this provides a short cut to animated textures
		* @return frame number is returned
		*/
		int GetCurrentFrameNumber();

		/**
		* For animated textures. Get the total frames in the animated texture. this provides a short cut to animated textures
		* @return frame number is returned
		*/
		int GetFrameCount();
		
		/**
		* save any texture to a different texture file format and save with full mipmapping to disk.
		*/
		virtual bool SaveToFile(const char* filename, D3DFORMAT dwFormat, int width, int height, UINT MipLevels = 1, DWORD Filter = D3DX_DEFAULT, Color ColorKey = 0);;

		/** load image of any format to buffer.
		* @param sBufMemFile: the memory file buffer.
		* @param sizeBuf: size of the memory file buffer
		* @param width: [out]
		* @param height: [out]
		* @param ppBuffer: buffer containing the read data. it may be 4 bytes per pixel or 3 bytes per pixel.
		*		this function will allocate buffer using new [] operator, the caller is responsible to delete it using delete [] operator.
		* @param bAlpha: if (bAlpha) D3DFMT_A8R8G8B8 4 bytes per pixel else D3DFMT_R8G8B8 3 bytes per pixel. */
		static void LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, bool bAlpha);
		/** this function uses FreeImage lib and support more formats. and width, height can be any number instead of square of 2.
		* @param sTextureFileName: only used for file format deduction.
		* @param nFormat: default to 32bits ARGB. PixelFormat24bppRGB, PixelFormat16bppGrayScale, PixelFormat8bppIndexed, PixelFormat32bppARGB, etc
		*/
		static bool LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel = NULL, int nFormat = -1);
		static bool LoadImageOfFormatEx(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel = NULL, int nFormat = -1, ImageExtendInfo *info = nullptr);

		/** create a texture entity from memory buffer. */
		static TextureEntity* CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels = 0, D3DPOOL dwCreatePool= D3DPOOL_MANAGED, DWORD nFormat = 0);
		/** create a texture entity from file path. */
		static TextureEntity* CreateTexture(const char* pFileName, uint32 nMipLevels = 0, D3DPOOL dwCreatePool = D3DPOOL_MANAGED);
	public:
		virtual TextureEntity* LoadUint8Buffer(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels = 0, D3DPOOL dwCreatePool = D3DPOOL_DEFAULT, DWORD nFormat = 0) { return NULL; }
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		ATTRIBUTE_METHOD1(TextureEntity, SetIsRGBA_s, bool) { cls->SetIsRGBA(p1); return S_OK; }
		ATTRIBUTE_METHOD1(TextureEntity, SetTextureFramePointer_s, int) { cls->SetTextureFramePointer(p1); return S_OK; }
	protected:
		bool m_bRABG = false;
		void SetTextureFramePointer(int framePointer);
		void SetIsRGBA(bool isRGBA) { m_bRABG = isRGBA; }
	};
}

// chose an implementation as Texture Manager
#ifdef USE_DIRECTX_RENDERER
#include "TextureEntityDirectX.h"
#elif defined(USE_OPENGL_RENDERER)
#include "TextureEntityOpenGL.h"
#else
namespace ParaEngine{
	typedef AssetManager<TextureEntity, TextureEntity> TextureAssetManager;
}
#endif