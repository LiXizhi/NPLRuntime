#pragma once
#include <map>
#include "UrlLoaders.h"
#include "IAttributeFields.h"

namespace ParaEngine
{
	class CUrlProcessorUserData;

	/** light-weighted file record header in memory. */
	struct AssetFileEntry
	{
	public:
		/** sync file call back function or class. */
		typedef boost::signals2::signal<void(int, AssetFileEntry*)>  SyncFile_Callback_t;

		enum AssetFileStatus {
			/** undetermined. all assets are undetermined until we query it. One can always safely set asset state to unknown at any time such as during md5 check error. */
			AssetFileStatus_Unknown = 0,
			/** In local computer and ready for use. If file is downloaded and then deleted by another process, this value will still be true, since it will cache last result.*/
			AssetFileStatus_Downloaded = 1,
			/** Asset is being downloaded but is not finished yet. This state is transient and will either be set to AssetFileStatus_Downloaded or AssetFileStatus_Failed after download is complete. */
			AssetFileStatus_Downloading = 2,
			/** Unable to download the file, after trying a number of times. We will cache last result. */
			AssetFileStatus_Failed = 3,
		};
		/** the asset file type is usually set according to the file extension. We will use file type to set download priority and queue type. */
		enum AssetFileTypeEnum {
			AssetFileType_default = 0,
			AssetFileType_dds_file,
			AssetFileType_x_file,
			AssetFileType_ui_texture,
			AssetFileType_script,
			AssetFileType_audio,
			AssetFileType_big_file,
			AssetFileType_web_file,
		};

		/** this is concatenation of hex md5 with file size*/
		string m_localFileName;
		string m_url;

		bool m_bIsZipFile;
		/** how many times we have tried to download this file since start. */
		int m_nDownloadCount;
		/** the compressed file size*/
		int m_nFileSize;
		/** the asset file type */
		AssetFileTypeEnum m_file_type;

		/** the sync call back. if this is not NULL, it means that we are downloading this entity.
		* Note: this callback is not thread safe.
		*/
		SyncFile_Callback_t* m_sync_callback;

		/** asset file status. */
		AssetFileStatus m_nStatus;
	public:
		AssetFileEntry();
		~AssetFileEntry();

		/** whether an up to date file exist locally. one can use the latest file at GetLocalFileName()*/
		bool DoesFileExist();

		/** get asset file status */
		inline AssetFileStatus GetStatus() { return m_nStatus; }

		/** set asset file status */
		inline void SetStatus(AssetFileStatus status) { m_nStatus = status; }

		/** the asset file type is usually set according to the file extension. We will use file type to set download priority and queue type. */
		inline AssetFileTypeEnum GetFileType() { return m_file_type; }

		/** set the asset file type according to file extension.
		* @note: FileSize must be set before calling this function.
		* @return: the asset file type is returned. */
		AssetFileTypeEnum SetFileType(const std::string& file_extension);

		/** get the proper request queue. Internally it is set according to GetFileType()*/
		ResourceRequestID GetRequestQueueID();

		/** whether this asset is currently being downloaded.
		[thread safety]: This function can only be called in the main thread.
		*/
		bool IsDownloading();

		/** download the file even it is up to date. this function is synchronous.
		* consider using CheckSyncFile() if one just wants to ensure an up to date copy is available.
		* it may retry download if download failed such as md5 mismatch. If this function is called and failed multiple times, and
		* HasReachedMaxRetryCount() is true, the function will do nothing and return false.
		* Please use DoesFileExist() to check if file exit before calling sync file to avoid redownload the file.
		* @return true if file is downloaded and up to date. */
		bool SyncFile();

		/** similar to SyncFile(), except that this function will return immediately and does not redownload or call AddDownloadCount. And use callback.
		* @param pFuncCallback: the call back function to use. if none is specified, it will pick a default one to use according to pRequestData->m_nAssetType;
		* this function is always called by the main thread.
		* @param lpUserData is expected to be derived class of CUrlProcessorUserData
		* @param bDeleteUserData: if true, we will delete lpUserData.
		* @return S_OK if download is initiated.
		*/
		HRESULT SyncFile_Async(URL_LOADER_CALLBACK pFuncCallback = NULL, CUrlProcessorUserData* pUserData = NULL, bool bDeleteUserData = false);
		
		/** this function is not thread safe, it must be called from the main render thread.
		* This function is same as SyncFile_Async(), except that it allows multiple event listeners for the same entity.
		* if the current file is already being updated, it will not be called multiple times.
		* Example:

		AssetFileEntry entry;
		class SomeCallBack
		{
		public:
			void operator() (int nResult, AssetFileEntry* pEntry) const
			{
				if(nResult == 0)
				{
					OUTPUT_LOG("asset is successfully downloaded!");
				}
			}
		};
		entry.SyncFile_Async(SomeCallBack());

		* @return: S_OK if file is already downloaded and can be used right away. E_PENDING if file is being downloaded. E_FAIL if some error occurs.
		*/
		HRESULT SyncFile_Async(const SyncFile_Callback_t::slot_type& slot);

		/** retry with current callback*/
		HRESULT SyncFile_AsyncRetry();

		/** this is similar to SyncFile, except that it will only sync file if the file is not up to date. */
		bool CheckSyncFile();

		/** fire the complete signal, causing all callback to be invoked and then delete all callbacks.*/
		void SignalComplete(int nResult = 0);

		/** whether we have downloaded the file so many times, but still can not get it right.
		* by default we only try to download once. */
		bool HasReachedMaxRetryCount();

		/** add the download count and return !HasReachedMaxRetryCount()
		* @return true if we can go on downloading.
		*/
		bool AddDownloadCount();

		/** get relative url */
		const string& GetUrl() { return m_url; };

		/** get the absolute url by prepending the default asset domain. */
		string GetAbsoluteUrl();

		/** return true if url file is compressed. By default, we will assume url file is a compressed file unless the file name ends with .p */
		bool IsUrlFileCompressed();

		/** check whether the MD5 of the input buffer matches. */
		bool CheckMD5AndSize(const char* buffer, int nSize);

		/** save a give buffer to local file. */
		bool SaveToDisk(const char* buffer, int nSize, bool bCheckMD5 = true);

		/** get the compressed file size of the asset entry. */
		int GetFileSize() { return m_nFileSize; }

		/** get local file name. */
		inline const std::string& GetLocalFileName() { return m_localFileName; }
		/** get the full disk file path. on win32 this still return relative file path. On mobile platform, this will return writable file path. */
		std::string GetFullFilePath();
	};


	/** mapping a file to another file
	* a replace file mapping. it is just file name to file name pairs. If the file is requested on the left during GetFile(filename), file on the right is returned instead.
	* This function is very useful to temperarily change the 3D and 2D theme of the entire game world, in which only a text file needs to be updated.
	* Example:
	*	CFileReplaceMap::GetSingleton().ReplaceFile(filename);
	*/
	class CFileReplaceMap
	{
	public:
		CFileReplaceMap();

		/* load a replace file mapping. it is just file to file pairs. If the file is requested on the left during GetFile(filename), file on the right is returned instead.
		* This function is very useful to temperarily change the 3D and 2D theme of the entire game world, in which only a text file needs to be updated.
		* @param filename: the file map file.
		* @param bReplaceExistingOnes: whether we will overwrite any previous calls to this function.
		*/
		CFileReplaceMap(const string& filename, bool bReplaceExistingOnes = true);

		~CFileReplaceMap();

		/* load a replace file mapping. it is just file to file pairs. If the file is requested on the left during GetFile(filename), file on the right is returned instead.
		* This function is very useful to temperarily change the 3D and 2D theme of the entire game world, in which only a text file needs to be updated.
		* @param filename: the file map file.
		* @param bReplaceExistingOnes: whether we will overwrite any previous calls to this function.
		*/
		void LoadReplaceFile(const string& filename, bool bReplaceExistingOnes = true);

		/** get a global singleton instance. */
		static CFileReplaceMap& GetSingleton();

		/** whether there is no files in the file. */
		bool IsEmpty();

		/** Replace file.
		* @param inout: the file name to replace. the replaced file will also be written to it after function returns.
		* @return true if file is replaced. or false if not.
		*/
		bool ReplaceFile(string& inout);
	private:
		typedef std::map<string, string>  Asset_Replace_Map_Type;

		/* a replace file mapping. it is just file name to file name pairs. If the file is requested on the left during GetFile(filename), file on the right is returned instead.
		* This function is very useful to temperarily change the 3D and 2D theme of the entire game world, in which only a text file needs to be updated.
		*/
		Asset_Replace_Map_Type m_replace_map;

		/** a mutex to protect the map. */
		ParaEngine::mutex m_mutex;
	};

	/**
	* Asset manifest manager.
	* When the application starts, we will read all "Assets_manifest*.txt" file under the root directory. Each file has following content

	format is [relative path],md5,fileSize
	if the name ends with .z, it is zipped. This could be 4MB uncompressed in size
	md5 is checksum code of the file. fileSize is the compressed file size.

	audio/music.mp3.z,3799134715,22032
	model/building/tree.dds.z,2957514200,949
	model/building/tree.x.z,2551621901,816
	games/tutorial.swf,1157008036,171105

	When one of the async loader try to load an application asset(texture, model, etc), it will first search in AssetManifest
	using the TO-LOWER-CASED asset path, such as (model/building/tree.x). if will then search the "temp/cache/" directory for a matching file

	The file matching is done by comparing the line in the asset file with the filename in the cache directory, using their md5 and size.

	audio/music.mp3.z,3799134715,22032 matches to file 379913471522032


	Example Usage:
	<verbatim>
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile("Texture/somefile.dds");
	if(pEntry && pEntry->DoesFileExist())
	{
		// Load from file pEntry->GetLocalFileName();
	}
	</verbatim>
	*/
	class CAssetManifest : public IAttributeFields
	{
	public:
		CAssetManifest(void);
		virtual ~CAssetManifest(void);

		ATTRIBUTE_DEFINE_CLASS(CAssetManifest);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD(CAssetManifest, Clear_s) { cls->CleanUp(); return S_OK; }
		ATTRIBUTE_METHOD(CAssetManifest, LoadManifest_s) { cls->LoadManifest(); return S_OK; }
		ATTRIBUTE_METHOD1(CAssetManifest, LoadManifestFile_s, const char*) { cls->LoadManifestFile(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAssetManifest, IsEnabled_s, bool*) { *p1 = cls->IsEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CAssetManifest, SetEnabled_s, bool) { cls->EnableManifest(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAssetManifest, IsUseLocalFileFirst_s, bool*) { *p1 = cls->IsUseLocalFileFirst(); return S_OK; }
		ATTRIBUTE_METHOD1(CAssetManifest, SetUseLocalFileFirst_s, bool) { cls->SetUseLocalFileFirst(p1); return S_OK; }

	public:
		/** get the global singleton object */
		static CAssetManifest& GetSingleton();

		/* load all "Assets_manifest*.txt" manifest file under the root directory */
		void LoadManifest();

		void PrepareCacheFolders();

		/* load a given manifest file. */
		void LoadManifestFile(const string& filename);

		/** check whether file exist. file name is case insensitive */
		bool DoesFileExist(const char* filename);

		/** check whether file exist. file name is case insensitive */
		bool DoesFileExist(const string& filename);

		/** if the file is not asset file, it returns 0.
		* if the file is asset file, it will sync the file, if the file is not up to date. this function is synchronous.
		* it may retry download if download failed such as md5 mismatch. If this function is called and failed multiple times, and
		* HasReachedMaxRetryCount() is true, the function will do nothing and return false.
		* @return if the file is not asset file, it returns 0.
		if file is downloaded and up to date, it return 1.
		if asset file failed, it return -1 */
		int CheckSyncFile(const char* filename);

		/** get a asset file entry based on file name.
		* the actually file entity may be replaced by CFileReplaceMap::GetSingleton()
		* [thread-safe]: it is thread safe only after manifest file and replace file map are loaded.
		* @param filename: file name of the file to search such as "model/building/tree.x". file name is case insensitive
		* @param bUseReplaceMap: default to true. whether to use CFileReplaceMap::GetSingleton() to replace the input filename.
		* @return: if there is no update to date file, it will return NULL;
		*/
		AssetFileEntry* GetFile(const string& filename, bool bUseReplaceMap = true, bool bIgnoreLocalFile = false);

		/** add an entry to the list.
		* @param filename: string with format [relative path],md5,fileSize
		*/
		void AddEntry(const char* filename);

		/** clean up all loaded files. */
		void CleanUp();

		/** print stat to log */
		void PrintStat();

		/** if false, manifest will be temporarily disabled and all GetFile() and DoesFileExist() functions will not return anything. */
		inline bool IsEnabled() { return m_bEnableManifest; }

		/** if false, manifest will be temporarily disabled and all GetFile() and DoesFileExist() functions will not return anything.
		* default to true.
		*/
		void EnableManifest(bool bEnable = true);

		/** if true, GetFile() will return null, if a local disk or zip file is found even there is an entry in the assetmanifest. */
		bool IsUseLocalFileFirst() const;
		void SetUseLocalFileFirst(bool val);

	private:
		bool ParseAssetEntryStr(const string &sFilename, string &fileKey, string &md5, string& filesize);
	private:
		typedef std::map<string, AssetFileEntry*>  Asset_Manifest_Map_Type;

		/** a mapping from asset key string to their manifest attributes
		* such as "model/building/tree.x"  to {"3799134715,22032"}
		*/
		Asset_Manifest_Map_Type m_files;

		/** if false, manifest will be temporarily disabled and all GetFile() and DoesFileExist() functions will not return anything. */
		bool m_bEnableManifest;
		/** if true, GetFile() will return null, if a local disk or zip file is found even there is an entry in the assetmanifest. */
		bool m_bUseLocalFileFirst;
	};
}
