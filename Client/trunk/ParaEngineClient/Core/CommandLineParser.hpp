#pragma once
//-----------------------------------------------------------------------------
// Class:	CommandLineParser
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.4.25
// Desc: Header only helper class. It can be used for applications who do not link with the core ParaEngine plugin. 
//-----------------------------------------------------------------------------
#include <string>
#include <map>

namespace ParaEngine
{
	/**
	* command line parameter parser
	e.g. 

	CCommandLineParser cmdLines("a=\"a_value\" b=\"b_value\"");

	assert(std::string("a_value") == cmdLines.GetValue("a", "default_value"));
	assert(std::string("b_value") == cmdLines.GetValue("b", "default_value"));
	*/
	class CCommandLineParser
	{
	public:
		CCommandLineParser(const char* lpCmdLine = NULL)
		{
			SetCommandLine(lpCmdLine);
		}


		/** get string specifying the command line for the application, excluding the program name.*/
		const char* GetCommandLine()
		{
			return m_sAppCmdLine.c_str();
		}

		/**
		* return a specified parameter value in the command line of the application. If the parameter does not exist, the return value is NULL. 
		* @param pParam: key to get
		* @param defaultValue: if the key does not exist, this value will be added and returned. This can be NULL.
		*/
		const char* GetValue(const char* pParam, const char* defaultValue)
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

		/** set string specifying the command line for the application, excluding the program name.
		* calling this function multiple times with different command line is permitted. The latter settings will merge and override the previous ones.
		* such as key="value" key2="value2"
		*/
		void SetCommandLine(const char* pCommandLine)
		{
			if(pCommandLine)
				m_sAppCmdLine = pCommandLine;
			else
				m_sAppCmdLine.clear();

			/// extract parameters

			int nPos = 0;
			int nBegin = 0;
			while((nPos =(int) m_sAppCmdLine.find('=', nBegin))!=string::npos)
			{
				int i = nPos-1; 
				int nFromPos = (int)m_sAppCmdLine.find('\"', nBegin+1);
				if(nFromPos!=(int)string::npos)
				{
					int nToPos = (int)m_sAppCmdLine.find('\"', nFromPos+1);
					if(nToPos!=(int)string::npos)
					{
						if(nPos>nBegin)
						{
							// remove heading and trailing spaces
							for(;(m_sAppCmdLine[nBegin] == ' '|| m_sAppCmdLine[nBegin] == '\t' || m_sAppCmdLine[nBegin] == '\n' || m_sAppCmdLine[nBegin] == '\r'); nBegin++)
								;
							for(;nPos>=1 && (m_sAppCmdLine[nPos-1] == ' '|| m_sAppCmdLine[nPos-1] == '\t' || m_sAppCmdLine[nPos-1] == '\n' || m_sAppCmdLine[nPos-1] == '\r'); nPos--)
								;
							if(nPos>nBegin)
							{
								m_commandParams[m_sAppCmdLine.substr(nBegin, nPos-nBegin)] = m_sAppCmdLine.substr(nFromPos+1, nToPos - nFromPos-1);
								nBegin = nToPos+1;
								continue;
							}
						}
					}
				}
				nBegin = nPos+1;
			}
		}

	private:
		/** string specifying the command line for the application, excluding the program name.
		* such as XX="YY" aa="pp"
		*/
		std::string m_sAppCmdLine;

		/** parameters from m_sAppCmdLine */
		std::map<std::string, std::string> m_commandParams;
	};
	
}
