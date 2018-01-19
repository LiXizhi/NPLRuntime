//-----------------------------------------------------------------------------
// Class: FilePath
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4, 2014.8
// Revised: refactored from file manager
// Notes: Only the server version is cross-platformed, the client version is NOT cross-platformed for performance reasons.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "regularexpression.h"
#include "FileManager.h"
#include "FileUtils.h"
#include "FilePath.h"
using namespace ParaEngine;

ParaEngine::CPathVariable::CPathVariable(const std::string& sVarName, const std::string& sVarValue, bool bEditable, bool bEnableEncoding /*= true*/) :m_sVarName(sVarName), m_sVarValue(sVarValue), m_bEditable(bEditable), m_bEnableEncoding(bEnableEncoding)
{

}

ParaEngine::CPathVariable::CPathVariable(const std::string& sVarName, const std::string& sVarValue) : m_sVarName(sVarName), m_sVarValue(sVarValue), m_bEditable(false), m_bEnableEncoding(true)
{

}

ParaEngine::CPathVariable::CPathVariable() : m_bEditable(false), m_bEnableEncoding(true)
{

}

CPathReplaceables::CPathReplaceables()
{

}

CPathReplaceables::~CPathReplaceables()
{

}

CPathReplaceables& CPathReplaceables::GetSingleton()
{
	static CPathReplaceables g_sington;
	return g_sington;
}

CPathVariable * CPathReplaceables::GetVariable(const char* sVarName)
{
	if (sVarName == NULL)
		return NULL;
	vector <CPathVariable>::iterator itCur, itEnd = m_variables.end();
	for (itCur = m_variables.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur).m_sVarName == sVarName){
			return &(*itCur);
		}
	}
	return NULL;
}

void CPathReplaceables::DecodePath(string& output, const string& input)
{
	output = input;
	// input
	int nFrom = 0;
	int nTo = 0;
	if ((nFrom = (int)output.find('%', nTo)) != string::npos)
	{
		nTo = (int)output.find('%', nFrom + 2);
		if (nTo != string::npos)
		{
			string varName = output.substr(nFrom + 1, nTo - nFrom - 1);
			const CPathVariable * pVar = GetVariable(varName.c_str());
			if (pVar)
			{
				// found and replace. 
				output.replace(nFrom, nTo - nFrom + 1, pVar->m_sVarValue);
				// tricky reuse of output
				CPathReplaceables::DecodePath(output, output);
			}
		}
	}
}

void CPathReplaceables::EncodePath(string& output, const string& input)
{
	output = input;
	vector <CPathVariable>::const_iterator itCur, itEnd = m_variables.end();
	for (itCur = m_variables.begin(); itCur != itEnd; ++itCur)
	{
		if (itCur->m_bEnableEncoding)
		{
			int nFrom = 0;
			int i = 0;
			while (++i<10 && (nFrom = (int)output.find(itCur->m_sVarValue, nFrom)) != string::npos)
			{
				if (itCur->m_sVarValue.size()>0 && (itCur->m_sVarValue[itCur->m_sVarValue.size() - 1] == '/' || output[nFrom + itCur->m_sVarValue.size()] == '/'))
				{
					// found and replace. 
					string var = "%";
					var += itCur->m_sVarName;
					var += "%";
					output.replace(nFrom, itCur->m_sVarValue.size(), var);
					nFrom += (int)var.size();
				}
				else
					nFrom += (int)itCur->m_sVarValue.size();

			}
		}
	}
}

void CPathReplaceables::EncodePath(string& output, const string& input, const string& varNames)
{
	output = input;
#ifdef _PCRE_H
	pcrecpp::StringPiece spNames(varNames);  // Wrap in a StringPiece

	string name;
	pcrecpp::RE re("(\\w+)[^\\w]*");
	while (re.Consume(&spNames, &name))
	{
		CPathVariable* pVar = GetVariable(name.c_str());
		if (pVar)
		{
			int nFrom = 0;
			int i = 0;
			while (++i<10 && ((nFrom = (int)output.find(pVar->m_sVarValue, nFrom)) != string::npos))
			{
				if (pVar->m_sVarValue.size()>0 && (pVar->m_sVarValue[pVar->m_sVarValue.size() - 1] == '/' || output[nFrom + pVar->m_sVarValue.size()] == '/'))
				{
					// found and replace. 
					string var = "%";
					var += name;
					var += "%";
					output.replace(nFrom, pVar->m_sVarValue.size(), var);
					nFrom += (int)var.size();
				}
				else
					nFrom += (int)pVar->m_sVarValue.size();
			}
		}
	}
#endif
}

bool CPathReplaceables::AddVariable(const CPathVariable& variable, bool bCheckCanEdit /*= true*/)
{
	CPathVariable * pVar = GetVariable(variable.m_sVarName.c_str());
	if (pVar)
	{
		if ((!bCheckCanEdit) || (pVar->m_bEditable))
		{
			(*pVar) = variable;
			return true;
		}
	}
	else
	{
		m_variables.push_back(variable);
		return true;
	}
	return false;
}

bool CPathReplaceables::RemoveVariable(const char* sVarName, bool bCheckCanEdit)
{
	vector <CPathVariable>::iterator itCur, itEnd = m_variables.end();
	for (itCur = m_variables.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur).m_sVarName == sVarName){
			if ((!bCheckCanEdit) || (itCur->m_bEditable))
			{
				m_variables.erase(itCur);
				return true;
			}
			break;
		}
	}
	return false;
}

bool CPathReplaceables::Clear()
{
	m_variables.clear();
	return true;
}



bool ISearchPathManager::AddSearchPath(const char* sFile, int nPriority/*=0*/)
{
	if (sFile == NULL)
		return false;
	string path;
	CParaFile::ToCanonicalFilePath(path, sFile, false);
	if (path.size() > 0)
	{
		if (path[path.size() - 1] != '/')
		{
			path.append("/");
		}
		std::list<SearchPath>::iterator itCurCP, itEndCP = m_searchpaths.end();
		for (itCurCP = m_searchpaths.begin(); itCurCP != itEndCP; ++itCurCP)
		{
			if ((*itCurCP) == path)
			{
				return true;
			}
		}
		m_searchpaths.push_front(SearchPath(path.c_str(), nPriority));
		OUTPUT_LOG("search path: %s\n", path.c_str());
		return true;
	}
	return false;
}

bool ISearchPathManager::RemoveSearchPath(const char* sFile)
{
	if (sFile == NULL)
		return false;
	std::string path;
	CParaFile::ToCanonicalFilePath(path, sFile, false);
	if (path.size() > 0)
	{
		if (path[path.size() - 1] != '/')
		{
			path.append("/");
		}
		std::list<SearchPath>::iterator itCurCP, itEndCP = m_searchpaths.end();
		for (itCurCP = m_searchpaths.begin(); itCurCP != itEndCP; ++itCurCP)
		{
			if ((*itCurCP) == path)
			{
				m_searchpaths.erase(itCurCP);
				return true;
			}
		}
	}
	return false;
}

bool ISearchPathManager::ClearAllSearchPath()
{
	m_searchpaths.clear();
	return true;
}

bool ISearchPathManager::AddDiskSearchPath(const std::string& sFile, bool nFront /*= false*/)
{
	return CFileUtils::AddDiskSearchPath(sFile, nFront);
}
