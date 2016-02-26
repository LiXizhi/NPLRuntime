//----------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2015.3.5
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "util/StringHelper.h"
#include "qfont.h"

using namespace ParaEngine;


ParaEngine::QFont::QFont()
	:QFont("System")
{
}

ParaEngine::QFont::QFont(const std::string &family, int pointSize /*= -1*/, int weight /*= -1*/, bool italic /*= false*/)
	: m_pointSize(pointSize), m_weight(weight), m_style(italic ? StyleItalic : StyleNormal)
{
	setFamily(family);
}

ParaEngine::QFont::~QFont()
{

}

bool ParaEngine::QFont::operator==(const QFont & r) const
{
	return !m_pFontSprite.get() && m_pFontSprite == r.m_pFontSprite;
}

std::string ParaEngine::QFont::key() const
{
	return m_pFontSprite ? m_pFontSprite->GetKey() : "";
}

SpriteFontEntity* ParaEngine::QFont::GetSpriteFont()
{
	if (!m_pFontSprite)
	{
		if (m_family.empty())
			setFamily("System");
		if (m_pointSize < 1)
			m_pointSize = 12;
		if (m_weight < 0)
			m_weight = Normal;
		
		char temp[30];
		ParaEngine::StringHelper::fast_itoa(m_pointSize, temp, 30);
		string stemp = m_family;
		stemp += ";";
		stemp += temp;
		stemp += ";";
		if (bold())
			stemp += "bold";
		else
			stemp += "norm";
		SpriteFontEntity *pFontEntity = CGlobals::GetAssetManager()->GetFont(stemp.c_str());
		if (pFontEntity == NULL) {
			pFontEntity = CGlobals::GetAssetManager()->LoadGDIFont(stemp, m_family, m_pointSize, bold());
		}
		m_pFontSprite = pFontEntity;
	}
	return m_pFontSprite.get();
}

std::string ParaEngine::QFont::family() const
{
	return m_family;
}

void ParaEngine::QFont::setFamily(const std::string & fontname)
{
	if (m_family != fontname)
	{
		m_family = fontname;
		SetDirty();
	}
}

int ParaEngine::QFont::pointSize() const
{
	return m_pointSize;
}

void ParaEngine::QFont::setPointSize(int nSize)
{
	if (m_pointSize != nSize)
	{
		m_pointSize = nSize;
		SetDirty();
	}
}

int ParaEngine::QFont::weight() const
{
	return m_weight;
}

void ParaEngine::QFont::setWeight(int weight)
{
	if (m_weight != weight)
	{
		m_weight = weight;
		SetDirty();
	}
}

void ParaEngine::QFont::setStyle(Style style)
{
	if (m_style != style)
	{
		m_style = style;
		SetDirty();
	}
}

ParaEngine::QFont::Style ParaEngine::QFont::style() const
{
	return m_style;
}

void ParaEngine::QFont::SetDirty()
{
	m_pFontSprite.reset();
}
