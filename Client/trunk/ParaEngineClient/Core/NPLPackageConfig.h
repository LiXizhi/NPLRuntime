#pragma once

#include <string>

namespace ParaEngine
{
	/** for parsing package.npl file */
	class CNPLPackageConfig
	{
	public:
		CNPLPackageConfig();
	public:
		bool IsOpened();
		void open(const char* filename);
		void open(const char* pBuffer, int nSize);

	public:
		bool IsSearchPath() const;
		void IsSearchPath(bool val);
		const std::string& GetMainFile() const;
		void SetMainFile(const std::string& val);

	private:
		bool m_bIsOpened;
		bool m_bIsSearchPath;
		std::string m_sMainFile;
		std::string m_sBootstrapper;
	};

}