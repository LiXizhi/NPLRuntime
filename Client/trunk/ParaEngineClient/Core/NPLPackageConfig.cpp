//-----------------------------------------------------------------------------
// Class: ParaFile
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2017.4
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPL/NPLHelper.h"
#include "NPLPackageConfig.h"
#include "FileManager.h"
#include "BootStrapper.h"

using namespace ParaEngine;

CNPLPackageConfig::CNPLPackageConfig()
	: m_bIsSearchPath(true), m_bIsOpened(false)
{

}

bool ParaEngine::CNPLPackageConfig::IsOpened()
{
	return m_bIsOpened;
}

void ParaEngine::CNPLPackageConfig::open(const char* filename)
{
	CParaFile file;
	if (file.OpenFile(filename))
	{
		open((const char*)file.getBuffer(), (int)file.getSize());
		file.close();
	}
}

bool ParaEngine::CNPLPackageConfig::IsSearchPath() const
{
	return m_bIsSearchPath;
}

void ParaEngine::CNPLPackageConfig::IsSearchPath(bool val)
{
	m_bIsSearchPath = val;
}

const std::string& ParaEngine::CNPLPackageConfig::GetMainFile() const
{
	return m_sMainFile;
}

void ParaEngine::CNPLPackageConfig::SetMainFile(const std::string& val)
{
	m_sMainFile = val;
}

void ParaEngine::CNPLPackageConfig::open(const char* pBuffer, int nSize)
{
	m_bIsOpened = true;
	NPL::NPLObjectProxy packageConfig = NPL::NPLHelper::StringToNPLTable(pBuffer, nSize);
	if (packageConfig.GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
	{
		auto isSearchPath = packageConfig.GetField("searchpath");
		if (isSearchPath.GetType() == NPL::NPLObjectBase::NPLObjectType_Bool)
		{
			IsSearchPath((bool)isSearchPath);
		}

		// change bootstrapper if none. 
		if (CBootStrapper::GetSingleton()->IsEmpty())
		{
			auto sBootstrapper = packageConfig.GetField("bootstrapper");
			if (sBootstrapper.GetType() == NPL::NPLObjectBase::NPLObjectType_String)
			{
				m_sBootstrapper = (const std::string&)sBootstrapper;
			}
		}

		// output main file
		auto sMainFile = packageConfig.GetField("main");
		if (sMainFile.GetType() == NPL::NPLObjectBase::NPLObjectType_String)
		{
			SetMainFile(sMainFile);
		}
	}
}

