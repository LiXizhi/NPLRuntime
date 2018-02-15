//-----------------------------------------------------------------------------
// Class:	CSunLight
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.4.14
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Fog.h"

using namespace ParaEngine;

CFog::CFog()
{

}

LinearColor CFog::GetFogColor()
{
	LinearColor color(m_dwFogColor);
	color *= m_fFogDensity;
	color.a = 1.0f;
	return color;
}

void CFog::SetFogColor(const LinearColor& fogColor)
{
	m_dwFogColor = fogColor;
}
void CFog::SetFogStart(float fFogStart)
{
	m_fFogStart = fFogStart;
}

float CFog::GetFogStart()
{
	return m_fFogStart;
}
void CFog::SetFogEnd(float fFogEnd)
{
	m_fFogEnd = fFogEnd;
}
float CFog::GetFogEnd()
{
	return m_fFogEnd;
}
void CFog::SetFogDensity(float fFogDensity)
{
	m_fFogDensity = fFogDensity;
}
float CFog::GetFogDensity()
{
	return m_fFogDensity;
}

void CFog::EnableFog(bool bEnableFog)
{
	m_bRenderFog = bEnableFog;
}

bool CFog::IsFogEnabled()
{
	return m_bRenderFog;
}



