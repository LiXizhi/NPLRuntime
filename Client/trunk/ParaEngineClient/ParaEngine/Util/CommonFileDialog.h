#pragma once
#include <string>

namespace ParaEngine
{
	class CCommonFileDialog
	{
	public:
		CCommonFileDialog();
		~CCommonFileDialog();

		static CCommonFileDialog* GetSingleton();
	public:
		/** let the user to select a folder. 
		* @param sDefaultFolder: 
		* @return NULL if no folder is selected. 
		*/
		const char* OpenFolder(const char* sDefaultFolder = NULL);

		const char* OpenFileDialog(const char* sDefaultFolder = NULL);

		void SetDefaultOpenFileFolder(const char* sDefaultOpenFileFolder = NULL);

		/** get system directory */
		const char* GetDirectory(const char* sKnownDir);

	public:
		std::string m_sDefaultFolder;
	};
}

