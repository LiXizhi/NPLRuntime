#pragma once
#include "TextureEntity.h"
#include "RenderDeviceOpenGL.h"
#include "Framework/RenderSystem/RenderTypes.h"
#include "OpenGLWrapper/GLTexture2D.h"
namespace ParaEngine
{
	class CRenderTarget;

	/** texture entity. 
	*/
	struct TextureEntityOpenGL : public TextureEntity
	{
	public:
		virtual AssetEntity::AssetType GetType(){ return TextureEntity::texture; };

	public:
		TextureEntityOpenGL(const AssetKey& key);
		TextureEntityOpenGL(GLTexture2D* texture);
		TextureEntityOpenGL();
		virtual ~TextureEntityOpenGL();

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual HRESULT RendererRecreated();

		/**  Initializes a texture from an ImageEntity object.
		* NOTE: It will not convert the pvr image file.
		* @param dwTextureFormat: if 0, we will use the image file format.
		*/
		virtual bool LoadFromImage(ImageEntity * image, PixelFormat dwTextureFormat = PixelFormat::Unknown, UINT nMipLevels = 0, void** ppTexture = NULL);
		virtual bool LoadFromImage(const ParaImage* pImage, UINT nMipLevels, PixelFormat dwTextureFormat = PixelFormat::Unknown, void** ppTexture = nullptr) override;

		/** load from memory buffer.
		* @param ppTexture: if NULL, we will save to current asset, if not we will save to this object.
		*/
		virtual HRESULT LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, PixelFormat dwTextureFormat = PixelFormat::Unknown, void** ppTexture = NULL);

		/** whether we will use blocky or non-blocky (anti-aliased) texture */
		virtual void SetSamplerStateBlocky(bool bIsBlocky);
		virtual bool IsSamplerStateBlocky();

		virtual int32 GetWidth();
		virtual int32 GetHeight();
		
		/**
		* save any texture to a different texture file format and save with full mipmapping to disk.
		*/
		virtual bool SaveToFile(const char* filename, PixelFormat dwFormat, int width, int height, UINT MipLevels = 1, DWORD Filter = -1, Color ColorKey = 0);

		void SetAliasTexParameters();
	public:
		void SetInnerTexture(GLTexture2D* texture);
		virtual DeviceTexturePtr_type GetTexture();
		virtual const TextureInfo* GetTextureInfo();

		GLTexture2D* CreateTextureFromFile_Serial(const std::string& sFileName);
		
		/** in most cases this is false, where the image origin is at left, top.
		* however, in opengl frame buffer, the frame buffer's origin is at left, bottom.
		*/
		virtual bool IsFlipY();

		/** load image of any format to buffer.
		* @param sBufMemFile: the memory file buffer.
		* @param sizeBuf: size of the memory file buffer
		* @param width: [out]
		* @param height: [out]
		* @param ppBuffer: buffer containing the read data. it may be 4 bytes per pixel or 3 bytes per pixel.
		*		this function will allocate buffer using new [] operator, the caller is responsible to delete it using delete [] operator.
		* @param bAlpha: if (bAlpha) D3DFMT_A8R8G8B8 4 bytes per pixel else D3DFMT_R8G8B8 3 bytes per pixel. */
		static void LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, unsigned char ** ppBuffer, bool bAlpha);
		/** this function uses FreeImage lib and support more formats. and width, height can be any number instead of square of 2.
		* @param sTextureFileName: only used for file format deduction.
		* @param nFormat: default to 32bits ARGB. PixelFormat24bppRGB, PixelFormat16bppGrayScale, PixelFormat8bppIndexed, PixelFormat32bppARGB, etc
		*/
		static bool LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, unsigned char ** ppBuffer, int* pBytesPerPixel = NULL, int nFormat = -1);

		/** create a texture entity from memory buffer. */
		static TextureEntity* CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels = 0, EPoolType dwCreatePool = EPoolType::Managed, DWORD nFormat = 0);
		/** create a texture entity from file path. */
		static TextureEntity* CreateTexture(const char* pFileName, uint32 nMipLevels = 0, EPoolType dwCreatePool = EPoolType::Managed);
		virtual TextureEntity* LoadUint8Buffer(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels = 0, D3DPOOL dwCreatePool = D3DPOOL_MANAGED, DWORD nFormat = 0);

		/** calling this function will change the texture to DynamicTexture.
		* @param pImageString: long command string like
		* "paintrect fromX,fromY,toX,toY,data:image/png;base64,xxxxx"
		*/
		virtual bool LoadImageFromString(const char* pImageString);

	protected:
		friend class CRenderTarget;
		union{
			GLTexture2D* m_texture;
			GLTexture2D** m_pTextureSequence;
		};
		/** 0 for anti-aliased, 1 for blocky texture. */
		uint32 m_nSamplerState;
	};

	// the manager class 
	typedef AssetManager<TextureEntity, TextureEntityOpenGL> TextureAssetManager;

}