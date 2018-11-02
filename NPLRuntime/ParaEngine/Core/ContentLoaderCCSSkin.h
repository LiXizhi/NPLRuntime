#pragma once
#include "CustomCharCommon.h"
#include "IDataLoader.h"

namespace ParaEngine
{
	struct AssetFileEntry;
	struct TextureEntity;

	/**
	* CCCSSkinLoader implementation of IDataLoader
	* it will first search locally. If not found or version expired (as indicated in the assets_manifest file),
	* we will download from the asset update server.
	*/
	class CCCSSkinLoader : public IDataLoader
	{
	private:
		/** we shall assign to this char model instance after texture is composed. */
		CharModelInstance* m_pCharModelInstance;

		/** describing how to compose the texture.
		*/
		std::vector <CharTextureComponent> m_layers;

		/** the file name to be saved as. */
		std::string m_sFileName;

		/** how many times we have retried. */
		int m_nRetryCount;

		/** the loader failed because the asset file the following file needs to be downloaded from the asset web server. */
		AssetFileEntry* m_pAssetFileEntry;
	public:
		/**
		* @param sFileName: if this is "", m_asset->GetLocalFileName() is used.
		*/
		CCCSSkinLoader(CharModelInstance* pCharModelInstance, CharTexture&  charTexture, const char* sFileName);
		~CCCSSkinLoader();

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
		/** Load is called from the IO thread to load data. */
		HRESULT Load();

	protected:
		/** compose with Windows' GDIPlus engine. It is slow but can run in IO thread.
		* not a cross platform solution.
		*/
		HRESULT ComposeWithGDIPlus();
	};

	/**
	* CCCSSkinProcessor implementation of IDataProcessor
	*/
	class CCCSSkinProcessor : public IDataProcessor
	{
	private:
		/** we shall assign to this char model instance after texture is composed. */
		CharModelInstance* m_pCharModelInstance;
		/** the file name to be saved as. */
		std::string m_sFileName;

		/// if this is NULL, the default device is used. 
		RenderDevicePtr m_pDevice;

	public:
		CCCSSkinProcessor(CharModelInstance* pCharModelInstance, const char* sFileName);
		~CCCSSkinProcessor();

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
	protected:
		/** compose and return the render target as a texture entity. This function should be called from render thread,
		such as from UnLockDeviceObject() as the last step. */
		TextureEntity* ComposeRenderTarget();
	};
}
