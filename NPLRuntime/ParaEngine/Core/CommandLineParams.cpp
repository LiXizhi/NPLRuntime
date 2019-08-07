//-----------------------------------------------------------------------------
// Class:	CCommandLineParams
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.8.4
// Desc: Parsing commandline. See also CommandLineParser.hpp
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CommandLineParams.h"

using namespace ParaEngine;

CCommandLineParams::CCommandLineParams()
{

}

CCommandLineParams::CCommandLineParams( const char* lpCmdLine )
{
	SetAppCommandLine(lpCmdLine);
}

const char* CCommandLineParams::GetAppCommandLine()
{
	return m_sAppCmdLine.c_str();
}

void CCommandLineParams::ConvertToCanonicalForm()
{
	//if the params are split by "," ";", convert them to " "
	if (!m_sAppCmdLine.empty())
	{
		size_t stringLength = m_sAppCmdLine.size();
		const char* pCmdLineStart = m_sAppCmdLine.c_str();
		size_t nCurrentPos = 0;
		while (nCurrentPos < stringLength)
		{
			char currentCharacter = *(pCmdLineStart + nCurrentPos);
			if (currentCharacter == '\"')
			{
				//find the closing double quote
				nCurrentPos++;
				while (*(pCmdLineStart + nCurrentPos) != '\"' && nCurrentPos < stringLength)
				{
					nCurrentPos++;
				}
			}
			else if (currentCharacter == ',' || currentCharacter == ';')
			{
				//replace it with space
				m_sAppCmdLine[nCurrentPos] = ' ';
			}
			nCurrentPos++;
		}
	}
}

void CCommandLineParams::SetAppCommandLine( const char* pCommandLine )
{
	if(pCommandLine)
		m_sAppCmdLine = pCommandLine;
	else
		m_sAppCmdLine.clear();

	// convert alternative splitters with spaces
	ConvertToCanonicalForm();

	/// extract parameters
	size_t nPos = 0;
	size_t nBegin = 0;
	while((nPos = m_sAppCmdLine.find('=', nBegin))!=string::npos)
	{
		auto nFromPosName = m_sAppCmdLine.rfind(" ", nPos);
		if (nFromPosName == string::npos)
			nFromPosName = 0;
		else
			nFromPosName++;
		int i = nPos - 1;
		auto nFromPos = nPos + 1;
		while(m_sAppCmdLine.size() > nFromPos && m_sAppCmdLine[nFromPos] == ' ')
		{
			// skip all spaces
			nFromPos++;
		}
		if (m_sAppCmdLine.size() > nFromPos && m_sAppCmdLine[nFromPos] == '"')
		{
			auto nToPos = m_sAppCmdLine.find('\"', nFromPos + 1);
			if (nToPos != string::npos)
			{
				if (nPos > nFromPosName)
				{
					// remove heading and trailing spaces
					for (; (m_sAppCmdLine[nFromPosName] == ' ' || m_sAppCmdLine[nFromPosName] == '\t' || m_sAppCmdLine[nFromPosName] == '\n' || m_sAppCmdLine[nFromPosName] == '\r'); nFromPosName++)
						;
					for (; nPos >= 1 && (m_sAppCmdLine[nPos - 1] == ' ' || m_sAppCmdLine[nPos - 1] == '\t' || m_sAppCmdLine[nPos - 1] == '\n' || m_sAppCmdLine[nPos - 1] == '\r'); nPos--)
						;
					if (nPos > nFromPosName)
					{
						m_commandParams[m_sAppCmdLine.substr(nFromPosName, nPos - nFromPosName)] = m_sAppCmdLine.substr(nFromPos + 1, nToPos - nFromPos - 1);
						nBegin = nToPos + 1;
						continue;
					}
				}
			}
		}
		else
		{
			// this will parse parameter without ""
			nFromPos = nPos + 1;
			auto nToPos = m_sAppCmdLine.find(' ', nFromPos);
			if (nToPos == std::string::npos)
				nToPos = m_sAppCmdLine.size();
			m_commandParams[m_sAppCmdLine.substr(nFromPosName, nPos - nFromPosName)] = m_sAppCmdLine.substr(nFromPos, nToPos - nFromPos);
			nBegin = nToPos + 1;
			continue;
		}
		nBegin = nPos+1;
	}
}

const char* CCommandLineParams::GetAppCommandLineByParam( const char* pParam , const char* defaultValue)
{
	std::map<string, string>::iterator it =  m_commandParams.find(pParam);
	if(it!= m_commandParams.end())
	{
		return (*it).second.c_str();
	}
	if(defaultValue!=NULL)
	{
		m_commandParams[pParam] = defaultValue;
		return m_commandParams[pParam].c_str();
	}
	return NULL;
}