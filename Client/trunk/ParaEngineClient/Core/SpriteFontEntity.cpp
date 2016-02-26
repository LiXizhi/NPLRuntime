//-----------------------------------------------------------------------------
// Class:	SpriteFontEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12, 2014.8(cross platform)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SpriteFontEntity.h"
#include "memdebug.h"

using namespace ParaEngine;

namespace ParaEngine
{
	// font name mapping define
	std::map<string, string> SpriteFontEntity::g_mapFontNames;
}

SpriteFontEntity::SpriteFontEntity(const AssetKey& key)
:AssetEntity(key)
{

}
SpriteFontEntity::SpriteFontEntity()
{
};

SpriteFontEntity::~SpriteFontEntity()
{
}

const std::string& SpriteFontEntity::GetFontName() const
{
	return m_sFontName;
}

int SpriteFontEntity::GetFontSize() const
{
	return m_nFontSize;
}

void SpriteFontEntity::AddFontName(const string& sLocalName, const string& sTypeFaceName)
{
	if (sLocalName.empty())
		g_mapFontNames.clear();
	else
	{
		g_mapFontNames[sLocalName] = sTypeFaceName;
	}
}

const string& SpriteFontEntity::TranslateFontName(const string& sLocalName)
{
	if (g_mapFontNames.find(sLocalName) != g_mapFontNames.end())
	{
		return g_mapFontNames[sLocalName];
	}
	return sLocalName;
}