//-----------------------------------------------------------------------------
// Class: CWorldNameFactory	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.25
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "WorldNameFactory.h"
#include "memdebug.h"

using namespace ParaTerrain;
using namespace ParaEngine;

CWorldNameFactory::CWorldNameFactory(void)
{
}

CWorldNameFactory::CWorldNameFactory(const string& sWorldConfigFile)
{
	ResetWorldName(sWorldConfigFile);
}

CWorldNameFactory::~CWorldNameFactory(void)
{
}

string CWorldNameFactory::GetWorldName()
{
	return m_strWorldName;
}
void CWorldNameFactory::ResetWorldName(const string& sWorldConfigFile)
{
	m_strWorldPath.clear();
	int nSize = (int)sWorldConfigFile.size();

	if(CParaFile::GetFileExtension(sWorldConfigFile) != "")
	{
		/*int nFrom = 0;
		int i;
		for(i=0; i<nSize; ++i)
		{
			if(sWorldConfigFile[i]=='\\' || sWorldConfigFile[i]=='/')
			{
				if(i==0)
					++nFrom;
				else
					break;
			}
		}
		if(i>nFrom)
			m_strWorldPath = sWorldConfigFile.substr(nFrom, i-nFrom);
		else
			m_strWorldPath = "unknown";*/

		m_strWorldPath = CParaFile::GetParentDirectoryFromPath(sWorldConfigFile);
		if(m_strWorldPath.size()>1)
			m_strWorldPath = m_strWorldPath.substr(0, m_strWorldPath.size()-1);

		m_strWorldName = CParaFile::GetFileName(sWorldConfigFile);
		if(m_strWorldName == "worldconfig.txt")
		{
			// use empty worldname if worldconfig also does not contain the world name. 
			// all worlds created since 2013.12.30 has this turned on.  
			m_strWorldName.clear();
		}
		else
		{
			m_strWorldName = CParaFile::GetFileName(m_strWorldPath);
		}
	}
	else
	{
		if (sWorldConfigFile.size() > 0 && sWorldConfigFile[sWorldConfigFile.size() - 1] == '/')
			m_strWorldPath = sWorldConfigFile.substr(0, sWorldConfigFile.size() - 1);
		else
			m_strWorldPath = sWorldConfigFile;

		m_strWorldName.clear();
	}
}

string CWorldNameFactory::GetTerrainConfigFile(int x, int y)
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
		snprintf(tmp, MAX_PATH, "%s/config/%d_%d.config.txt", m_strWorldPath.c_str(), x, y);
	else
		snprintf(tmp, MAX_PATH, "%s/config/%s_%d_%d.config.txt", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y);
	return string(tmp);
}
string CWorldNameFactory::GetTerrainElevationFile(int x, int y)
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
		snprintf(tmp, MAX_PATH, "%s/elev/%d_%d.raw", m_strWorldPath.c_str(), x, y);
	else
		snprintf(tmp, MAX_PATH, "%s/elev/%s_%d_%d.raw", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y);
	return string(tmp);
}
string CWorldNameFactory::GetTerrainOnloadFile(int x, int y)
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
		snprintf(tmp, MAX_PATH, "%s/script/%d_%d.onload.lua", m_strWorldPath.c_str(), x, y);
	else
		snprintf(tmp, MAX_PATH, "%s/script/%s_%d_%d.onload.lua", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y);
	return string(tmp);
}
string CWorldNameFactory::GetTerrainMaskFile(int x, int y)
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
		snprintf(tmp, MAX_PATH, "%s/config/%d_%d.mask", m_strWorldPath.c_str(), x, y);
	else
		snprintf(tmp, MAX_PATH, "%s/config/%s_%d_%d.mask", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y);
	return string(tmp);
}
	
string CWorldNameFactory::GetDefaultWorldConfigName()
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
		snprintf(tmp, MAX_PATH, "%s/worldconfig.txt", m_strWorldPath.c_str());
	else
		snprintf(tmp, MAX_PATH, "%s/%s.worldconfig.txt", m_strWorldPath.c_str(), m_strWorldName.c_str());
	return string(tmp);
}

string CWorldNameFactory::GetWorldDirectory()
{
	return (m_strWorldPath+"/");
}

string CWorldNameFactory::GetWorldCharacterDirectory()
{
	char tmp[MAX_PATH+1];
	snprintf(tmp, MAX_PATH, "%s/character/", m_strWorldPath.c_str());
	return string(tmp);
}

string CWorldNameFactory::GetDefaultTileConfigName()
{
	string filename = m_strWorldPath+"/flat.txt";
	return filename;
}


string CWorldNameFactory::GetTerrainInfoDataFile(int x, int y)
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
		snprintf(tmp, MAX_PATH, "%s/elev/%d_%d_%s.raw", m_strWorldPath.c_str(), x, y,"info");
	else
		snprintf(tmp, MAX_PATH, "%s/elev/%s_%d_%d_%s.raw", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y,"info");
	return string(tmp);
}

string CWorldNameFactory::GetBlockRegionFileName(int x,int y,bool usingTempFile)
{
	char tmp[MAX_PATH+1];
	if(m_strWorldName.empty())
	{
		if(usingTempFile)
			snprintf(tmp, MAX_PATH, "%s/blockWorld.lastsave/%d_%d.raw", m_strWorldPath.c_str(), x, y);
		else	
			snprintf(tmp, MAX_PATH, "%s/blockWorld/%d_%d.raw", m_strWorldPath.c_str(), x, y);
	}
	else
	{
		if(usingTempFile)
			snprintf(tmp, MAX_PATH, "%s/blockWorld.lastsave/%s_%d_%d.raw", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y);
		else	
			snprintf(tmp, MAX_PATH, "%s/blockWorld/%s_%d_%d.raw", m_strWorldPath.c_str(), m_strWorldName.c_str(), x, y);
	}
	
	return string(tmp);
}

string CWorldNameFactory::GetBlockGameSaveDir(bool usingTempFile)
{
	char tmp[MAX_PATH+1];
	if(usingTempFile)
		snprintf(tmp, MAX_PATH, "%s/blockWorld.lastsave/", m_strWorldPath.c_str());
	else	
		snprintf(tmp, MAX_PATH, "%s/blockWorld/", m_strWorldPath.c_str());
	return string(tmp);
}

string CWorldNameFactory::GetBlockTemplateFileName(bool usingTempFile)
{
	char tmp[MAX_PATH+1];
	if(usingTempFile)
		snprintf(tmp, MAX_PATH, "%s/blockWorld.lastsave/blockTemplate.xml", m_strWorldPath.c_str());
	else
		snprintf(tmp, MAX_PATH, "%s/blockWorld/blockTemplate.xml", m_strWorldPath.c_str());
	return string(tmp);
}

void ParaTerrain::CWorldNameFactory::SetWorldName( const string& sWorldName )
{
	m_strWorldName = sWorldName;
}
