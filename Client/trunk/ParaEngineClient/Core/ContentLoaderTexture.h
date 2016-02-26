#pragma once
#include "TextureEntity.h"
#include "IDataLoader.h"

namespace ParaEngine
{
	struct AssetFileEntry;

	/**
	* CTextureLoader implementation of IDataLoader
	* it will first search locally. If not found or version expired (as indicated in the assets_manifest file),
	* we will download from the asset update server.
	*/
	class CTextureLoader : public IDataLoader
	{
	public:
		friend struct TextureEntity;

		/** the asset file that this texture loader will update to*/
		asset_ptr<TextureEntity> m_asset;
		// if this is "", m_asset->GetLocalFileName() is used. 
		std::string m_sFileName;
		/// if this is NULL, the m_asset->m_pTexture is sued. 
		void** m_ppTexture;

		/// the format of the texture, default to D3DFMT_UNKNOWN
		D3DFORMAT m_dwTextureFormat;
		///  Mip levels, default to D3DX_DEFAULT
		UINT m_nMipLevels;
		// the color key(default to 0):  Color value to replace with transparent black, or 0 to disable the color key. This is always a 32-bit ARGB color, 
		// independent of the source image format. Alpha is significant and should usually be set to FF for opaque color keys. 
		// Thus, for opaque black, the value would be equal to 0xFF000000. COLOR_XRGB(0,0,0)
		Color m_dwColorKey;

		char* m_pData;
		int m_cBytes;
		CParaFile m_file;

		/** the loader failed because the asset file the following file needs to be downloaded from the asset web server. */
		AssetFileEntry* m_pAssetFileEntry;
	public:
		/**
		* @param sFileName: if this is "", m_asset->GetLocalFileName() is used.
		*/
		CTextureLoader(asset_ptr<TextureEntity>& pAsset, const char* sFileName = NULL);
		~CTextureLoader();

		/** clean up everything */
		HRESULT CleanUp();

		// overrides
	public:
		/** get file name */
		const char* GetFileName();
		/** Decompress is called by one of the processing threads to decompress the data.*/
		HRESULT Decompress(void** ppData, int* pcBytes);
		/** Destroy is called by the graphics thread when it has consumed the data. */
		HRESULT Destroy();
		/** Load is called from the IO thread to load data. Load the texture from the packed file.
		*/
		HRESULT Load();
	};

	/**
	* CTextureProcessor implementation of IDataProcessor
	*/
	class CTextureProcessor : public IDataProcessor
	{
	public:
		friend struct TextureEntity;

		/** the asset file that this texture loader will update to*/
		asset_ptr<TextureEntity> m_asset;
		/// if this is NULL, the default device is used. 
		RenderDevicePtr m_pDevice;
		/// if this is NULL, the m_asset->m_pTexture is sued. 
		void** m_ppTexture;

		/// the format of the texture, default to D3DFMT_UNKNOWN
		D3DFORMAT m_dwTextureFormat;
		///  Mip levels, default to D3DX_DEFAULT
		UINT m_nMipLevels;
		// the color key(default to 0):  Color value to replace with transparent black, or 0 to disable the color key. This is always a 32-bit ARGB color, 
		// independent of the source image format. Alpha is significant and should usually be set to FF for opaque color keys. 
		// Thus, for opaque black, the value would be equal to 0xFF000000. COLOR_XRGB(0,0,0)
		Color m_dwColorKey;

		char* m_pData;
		int m_cBytes;
	private:
		// This is a private function that either Locks and copies the data (D3D9) 
		bool PopulateTexture();

	public:
		CTextureProcessor(asset_ptr<TextureEntity>& pAsset);
		~CTextureProcessor();

		/** Get render device */
		RenderDevicePtr GetRenderDevice();

		/** clean up everything */
		HRESULT CleanUp();

		// overrides
	public:
		HRESULT LockDeviceObject();
		HRESULT UnLockDeviceObject();
		HRESULT Destroy();
		HRESULT Process(void* pData, int cBytes);
		HRESULT CopyToResource();
		void    SetResourceError();
	};
}
