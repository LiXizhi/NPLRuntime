//-----------------------------------------------------------------------------
// Class: CBootStrapper
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2007.1
// Notes: cross platformed
// Desc: bootstrapper xml now supports new tag <ConfigFile>. We can specify a different config file other than config/config.txt to load at startup, such as safe mode. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BootStrapper.h"


using namespace ParaEngine;

#define DEFAULT_XML_PATH	"config/bootstrapper.xml"
#define DEFAULT_GAME_LOOP_FILE	"(gl)script/gameinterface.lua"

CBootStrapper::CBootStrapper(void)
{
	LoadDefault();
}

CBootStrapper::~CBootStrapper(void)
{
}

void CBootStrapper::LoadDefault()
{
	m_sMainLoopFile = DEFAULT_GAME_LOOP_FILE;
}

CBootStrapper* CBootStrapper::GetSingleton()
{
	static CBootStrapper g_singleton;
	return &g_singleton;
}

const string& CBootStrapper::GetMainLoopFile()
{
	return m_sMainLoopFile;
}

void CBootStrapper::SetMainLoopFile( const string& sMainFile )
{
	m_sMainLoopFile = sMainFile;
}


const string& CBootStrapper::GetConfigFile()
{
	return m_sConfigFile;
}


void CBootStrapper::SetConfigFile(const string& sConfigFile)
{
	m_sConfigFile = sConfigFile;
}

#ifdef PARAENGINE_MOBILE
#include <tinyxml2.h>
namespace TXML = tinyxml2;
bool CBootStrapper::LoadFromFile(const string& sXMLfile)
{
	string sFileName = sXMLfile;
	if (sXMLfile.empty())
		sFileName = DEFAULT_XML_PATH;

	int nSize = (int)(sXMLfile.size());
	if (nSize > 4 && (sXMLfile[nSize - 4] == '.') && (((sXMLfile[nSize - 3] == 'l') && (sXMLfile[nSize - 2] == 'u') && (sXMLfile[nSize - 1] == 'a'))
		|| ((sXMLfile[nSize - 3] == 'n') && (sXMLfile[nSize - 2] == 'p') && (sXMLfile[nSize - 1] == 'l'))))
	{
		// if the file extension is "lua", we will use it directly. 
		SetMainLoopFile(sXMLfile);
		return true;
	}

	int nResult = 0;

	// assume it is UTF-8?, or use XML_DEFAULT_ENCODING if one want it to be automatically determined from data 
	CParaFile file(sFileName.c_str());
	if (!file.isEof())
	{
		try
		{
			TXML::XMLDocument doc(true, TXML::COLLAPSE_WHITESPACE);

			string sCode;
			if ((int)file.getSize() > 0)
			{
				sCode.resize((int)file.getSize());
				memcpy(&(sCode[0]), file.getBuffer(), (int)file.getSize());
			}

			// XML_ENCODING_UTF8?
			doc.Parse(sCode.c_str(), (int)sCode.size());

			TXML::XMLHandle hDoc(&doc);
			TXML::XMLElement* pElem;
			TXML::XMLHandle hRoot(0);
			// block: <MainGameLoop>script/gameinterface.lua</MainGameLoop>
			{
				pElem = hDoc.FirstChildElement("MainGameLoop").ToElement();
				// should always have a valid root but handle gracefully if it does
				if (pElem != 0)
				{
					SetMainLoopFile(pElem->GetText());
				}
				pElem = hDoc.FirstChildElement("ConfigFile").ToElement();
				// should always have a valid root but handle gracefully if it does
				if (pElem != 0)
				{
					SetConfigFile(pElem->GetText());
				}
				// save this for later
				hRoot = TXML::XMLHandle(pElem);
			}
			return true;
		}
		catch (...)
		{
			OUTPUT_LOG("error: failed parsing xml file : %s \n", sFileName.c_str());
		}
	}
	return false;
}

bool CBootStrapper::SaveToFile(const string& sXMLfile)
{
	string sFileName = sXMLfile;
	if (sXMLfile.empty())
		sFileName = DEFAULT_XML_PATH;

	/// Make xml: <?xml ..>
	TXML::XMLDocument doc(true, TXML::COLLAPSE_WHITESPACE);
	TXML::XMLDeclaration * decl = doc.NewDeclaration(nullptr);
	doc.LinkEndChild(decl);
	/// <MainGameLoop>script/gameinterface.lua</MainGameLoop>
	TXML::XMLElement * element = doc.NewElement("MainGameLoop");
	TXML::XMLText * text = doc.NewText(GetMainLoopFile().c_str());
	element->LinkEndChild(text);
	doc.LinkEndChild(element);

	return !!doc.SaveFile(sFileName.c_str(), false);
}
#else
#include <tinyxml.h>

bool CBootStrapper::LoadFromFile( const string& sXMLfile )
{
	string sFileName = sXMLfile;
	if(sXMLfile.empty())
		sFileName = DEFAULT_XML_PATH;

	int nSize = (int)(sXMLfile.size());
	if(nSize>4 && (sXMLfile[nSize-4] == '.') && (((sXMLfile[nSize-3] == 'l') && (sXMLfile[nSize-2] == 'u') && (sXMLfile[nSize-1] == 'a')) 
		|| ((sXMLfile[nSize - 3] == 'n') && (sXMLfile[nSize - 2] == 'p') && (sXMLfile[nSize - 1] == 'l'))))
	{
		// if the file extension is "lua" or "npl", we will use it directly. 
		SetMainLoopFile(sXMLfile);
		return true;
	}

	int nResult = 0;

	// assume it is UTF-8?, or use TIXML_DEFAULT_ENCODING if one want it to be automatically determined from data 
	CParaFile file(sFileName.c_str());
	if(!file.isEof())
	{
		OUTPUT_LOG("load bootstrapper from xml file : %s \n", sFileName.c_str());
		try
		{
			TiXmlDocument doc;

			string sCode;
			if((int)file.getSize()> 0)
			{
				sCode.resize((int)file.getSize());
				memcpy(&(sCode[0]), file.getBuffer(), (int)file.getSize());
			}

			doc.Parse(sCode.c_str(),0,TIXML_ENCODING_UTF8);

			TiXmlHandle hDoc(&doc);
			TiXmlElement* pElem;
			TiXmlHandle hRoot(0);
			// block: <MainGameLoop>script/gameinterface.lua</MainGameLoop>
			{
				pElem=hDoc.FirstChildElement("MainGameLoop").Element();
				// should always have a valid root but handle gracefully if it does
				if (pElem!=0)
				{
					SetMainLoopFile(pElem->GetText());
				}
				pElem=hDoc.FirstChildElement("ConfigFile").Element();
				// should always have a valid root but handle gracefully if it does
				if (pElem!=0)
				{
					SetConfigFile(pElem->GetText());
				}
				// save this for later
				hRoot=TiXmlHandle(pElem);
			}
			return true;
		}
		catch (...)
		{
			OUTPUT_LOG("error: failed parsing xml file : %s \n", sFileName.c_str());
		}
	}
	return false;
}

bool CBootStrapper::SaveToFile( const string& sXMLfile )
{
	string sFileName = sXMLfile;
	if(sXMLfile.empty())
		sFileName = DEFAULT_XML_PATH;

	/// Make xml: <?xml ..>
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );
	/// <MainGameLoop>script/gameinterface.lua</MainGameLoop>
	TiXmlElement * element = new TiXmlElement( "MainGameLoop" );
	TiXmlText * text = new TiXmlText( GetMainLoopFile() );
	element->LinkEndChild( text );
	doc.LinkEndChild( element );

	return doc.SaveFile( sFileName );
}
#endif