#pragma once
#include "TextureEntity.h"

namespace ParaEngine
{
	//--------------------------------------------------------
	/// Texture Entity distinguish one template from other
	/// The following type are all supported by texture entity
	//--------------------------------------------------------
	struct TextureEntityDirectX : public TextureEntity
	{
	public:
		union
		{
			/// static texture
			LPDIRECT3DTEXTURE9    m_pTexture;
			/// cube texture
			LPDIRECT3DCUBETEXTURE9    m_pCubeTexture;
			/// RenderTarget
			LPDIRECT3DSURFACE9    m_pSurface;
			/// animated texture sequence
			LPDIRECT3DTEXTURE9*  m_pTextureSequence;
		};

	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::texture; };
	public:
		

		TextureEntityDirectX(const AssetKey& key);
		TextureEntityDirectX();
		virtual ~TextureEntityDirectX();

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual bool SetRenderTarget(int nIndex = 0);
		/** load from memory buffer. 
		* @param ppTexture: if NULL, we will save to current asset, if not we will save to this object. 
		*/
		virtual HRESULT LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN, void** ppTexture = NULL);

		/**
		* most assets are loaded asynchronously. This allows us to check if an asset is loaded.
		* For example, we can LoadAsset() for a number of assets that need preloading. and then use a timer to check if they are initialized and remove from the uninialized list.
		*/
		virtual bool IsLoaded();

		/**
		* For animated textures. Get the total frames in the animated texture. this provides a short cut to animated textures
		* @return frame number is returned
		*/
		int GetFrameCount();

		/** get texture info.
		* return NULL if the texture has never been set before.
		*/
		virtual const TextureInfo* GetTextureInfo();

		/** Get the texture for rendering */
		virtual DeviceTexturePtr_type GetTexture();

		/** the internal texture managed. it will add reference to pSrcTexture*/
		void SetTexture(LPDIRECT3DTEXTURE9 pSrcTexture);

		/** Copy pSrcTexture's surface to the current texture.
		* the current texture must be created with TextureEntity::StaticTexture in order to use this function.
		* it will delete existing texture if any and create using the pSrcTexture.
		* this function is mostly used to dynamically compose textures for some avatars, please see ParaXAnimInstance.cpp for more information.
		* @param pSrcTexture: it can be any texture, such as a render target.
		*/
		void CreateTexture(LPDIRECT3DTEXTURE9 pSrcTexture, D3DFORMAT dwFormat = D3DFMT_DXT3, int width = 256, int height = 256, UINT MipLevels = 0);
		
		/** this function is mostly used internally. It will load the texture from disk, unpack it and create the texture.
		* when this function returns, the texture will be already loaded to device pool.
		* @param pDev: if NULL, the default render device is used
		* @param sFileName: if NULL, m_asset->GetLocalFileName() is used.
		* @param ppTexture: if this is NULL, the m_asset->m_pTexture is sued.
		* @param dwTextureFormat: the format of the texture, default to D3DFMT_UNKNOWN
		* @param nMipLevels: Mip levels, default to D3DX_DEFAULT
		* @param dwColorKey: color key. default to 0(disabled). Use COLOR_XRGB(0,0,0) if blank is treated transparent.
		*/
		HRESULT CreateTextureFromFile_Serial(RenderDevicePtr pDev = NULL, const char* sFileName = NULL, IDirect3DTexture9** ppTexture = NULL, D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN, UINT nMipLevels = D3DX_DEFAULT, Color dwColorKey = 0);

		/**
		* save any texture to a different texture file format and save with full mipmapping to disk.
		*/
		virtual bool SaveToFile(const char* filename, D3DFORMAT dwFormat, int width, int height, UINT MipLevels = 1, DWORD Filter = D3DX_DEFAULT, Color ColorKey = 0);

		/**
		* determine the texture format and mip level from file name. the texture is usually static texture.
		* the rules are below:
		*	if file extension is "dds", both format and mip levels are loaded from the file.
		*	if file extension is "png" and file name ends with "_32bits", format is D3DFMT_A8R8G8B8, mip level is 1.
		*	if file extension is "png" and file name does not ends with "_32bits", format is DXT3, complete mip level chain is created.
		*	in all other cases, such as "tga", format is D3DFMT_UNKNOWN and mip level is D3DX_DEFAULT.
		*/
		static void GetFormatAndMipLevelFromFileName(const string& filename, D3DFORMAT* pdwTextureFormat, UINT* pnMipLevels);

		/**
		* determine the texture format and mip level from file name. the texture is usually static texture.
		* the rules are below:
		*	if file extension is "dds", both format and mip levels are loaded from the file.
		*	if file extension is "png" and file name CONTAINS "_32bits", format is D3DFMT_A8R8G8B8, mip level is 1.
		*	if file extension is "png" and file name does not CONTAINS "_32bits", format is DXT3, complete mip level chain is created.
		*	in all other cases, such as "tga", format is D3DFMT_UNKNOWN and mip level is D3DX_DEFAULT.
		*/
		static void GetFormatAndMipLevelFromFileNameEx(const string& filename, D3DFORMAT* pdwTextureFormat, UINT* pnMipLevels);

		/** secretly, change the m_pSurface */
		void SetSurface(LPDIRECT3DSURFACE9 pSurface);

		/** Get the surface*/
		LPDIRECT3DSURFACE9 GetSurface();

		/** get d3d format */
		D3DFORMAT GetD3DFormat();

		/** load image of any format to buffer.
		* @param sBufMemFile: the memory file buffer.
		* @param sizeBuf: size of the memory file buffer
		* @param width: [out]
		* @param height: [out]
		* @param ppBuffer: buffer containing the read data. it may be 4 bytes per pixel or 3 bytes per pixel.
		*		this function will allocate buffer using new [] operator, the caller is reponsible to delete it using delete [] operator.
		* @param bAlpha: if (bAlpha) D3DFMT_A8R8G8B8 4 bytes per pixel else D3DFMT_R8G8B8 3 bytes per pixel. */
		static void LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, bool bAlpha);
		/** this function uses FreeImage lib and support more formats. and width, height can be any number instead of square of 2.
		* @param sTextureFileName: only used for file format deduction.
		* @param nFormat: default to 32bits ARGB. PixelFormat24bppRGB, PixelFormat16bppGrayScale, PixelFormat8bppIndexed, PixelFormat32bppARGB, etc
		*/
		static bool LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel = NULL, int nFormat = -1);

		/** Copy render target content from one surface to another. they may be of different resolution */
		static bool StretchRect(TextureEntityDirectX * pSrcTexture, TextureEntityDirectX * pDestTexture);

		/** create a texture entity from memory buffer. */
		static TextureEntity* CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels = 0, D3DPOOL dwCreatePool = D3DPOOL_MANAGED, DWORD nFormat = 0);
		static TextureEntity* CreateTexture(const char* pFileName, uint32 nMipLevels = 0, D3DPOOL dwCreatePool = D3DPOOL_MANAGED);
	};

	// the manager class
	typedef AssetManager<TextureEntity, TextureEntityDirectX> TextureAssetManager;
}