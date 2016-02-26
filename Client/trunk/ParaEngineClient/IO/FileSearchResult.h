#pragma once
#include <vector>
#include <string>
#include <set>

namespace ParaEngine
{
	/** file find item */
	struct CFileFindData
	{
		std::string m_sFileName;
		DWORD m_dwFileAttributes;
		FILETIME m_ftCreationTime;
		FILETIME m_ftLastAccessTime;
		FILETIME m_ftLastWriteTime;
		DWORD m_dwFileSize;
	public:
		CFileFindData(const std::string& sFileName, DWORD dwFileSize = 0, DWORD dwFileAttributes = 0, const FILETIME* ftLastWriteTime = 0, const FILETIME* ftCreationTime = 0, const FILETIME* ftLastAccessTime = 0);
	};

	/** it represents the search result.*/
	class CSearchResult
	{
	public:
		PE_CORE_DECL CSearchResult();
		PE_CORE_DECL ~CSearchResult();
	public:
		/** release results. */
		PE_CORE_DECL void Release();

		/** get the root path */
		PE_CORE_DECL const std::string& GetRootPath();

		/** get the total number of result found */
		PE_CORE_DECL int GetNumOfResult();

		/** add a new item. return false if the search is completed. */
		PE_CORE_DECL bool AddResult(const CFileFindData& item);

		/** add a new item. return false if the search is completed. */
		PE_CORE_DECL bool AddResult(const std::string& sItem, DWORD dwFileSize = 0, DWORD dwFileAttributes = 0, const FILETIME* ftLastWriteTime = 0, const FILETIME* ftCreationTime = 0, const FILETIME* ftLastAccessTime = 0);

		/** get the item at the specified index. return NULL if nIndex is out of range.*/
		PE_CORE_DECL const CFileFindData* GetItemData(int nIndex);

		/** get the item at the specified index. return "" if nIndex is out of range.*/
		PE_CORE_DECL const std::string& GetItem(int nIndex);
	
		/** clear previous search result and begin a new search
		* @param sRootPath: the root path. for example: "", "xmodel/","xmodel/models/". Other format is not acceptable
		* @param nSubLevel: how many sub folders of sRootPath to look into. default value is 0, which only searches the sRootPath folder.
		* @param nMaxFilesNum: one can limit the total number of files in the search result. Default value is 50. the search will stop at this value even there are more matching files.
		* @param nFrom: only contains results from nFrom to (nFrom+nMaxFilesNum)
		*/
		void InitSearch(const std::string& sRootPath, int nSubLevel = 0, int nMaxFilesNum = 50, int nFrom = 0);

		void SetRootPath(const std::string& sRoot);
	protected:
		std::string m_rootPath;
		std::vector<CFileFindData> m_results;
		std::set<std::string> m_filenames;
		/// how many sub folders of sRootPath to look into. default value is 0, which only searches the sRootPath folder.
		int m_nSubLevel;
		/// one can limit the total number of files in the search result. Default value is 50. the search will stop at this value even there are more matching files.
		int m_nMaxFilesNum;
		/// only contains results from nFrom to (nFrom+nMaxFilesNum)
		int m_nFrom;
		/// 
		int m_nFileNumber;
	};
}
