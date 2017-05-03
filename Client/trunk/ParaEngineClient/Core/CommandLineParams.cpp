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

#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>

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

void CCommandLineParams::SetAppCommandLine( const char* pCommandLine )
{
	if(pCommandLine)
		m_sAppCmdLine = pCommandLine;
	else
		m_sAppCmdLine.clear();

	/// extract parameters

	auto bHaveBootstrapper = m_sAppCmdLine.find("bootstrapper") != string::npos;

	std::vector<std::string> fields;
	boost::split(fields, m_sAppCmdLine, boost::is_any_of(" "));

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (it->find('=') != string::npos)
		{
			std::vector<std::string> keyValue;
			boost::split(keyValue, *it, boost::is_any_of("="));
			m_commandParams[keyValue[0]] = keyValue[1];
		}
		else
		{
			if (it->front() == '-')
			{
				m_commandParams[it->substr(1)] = "true";
			}
			else
			{
				if (!bHaveBootstrapper)
				{
					m_commandParams["bootstrapper"] = *it;
				}
			}
		}
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