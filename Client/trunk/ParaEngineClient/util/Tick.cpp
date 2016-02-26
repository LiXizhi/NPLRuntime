//-----------------------------------------------------------------------------
// Class: Tick 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.12.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Tick.h"

using namespace ParaEngine;

ParaEngine::Tick::Tick() :m_fElapsedTime(0.f)
{
	SetInterval(1 / 20.f);
}

ParaEngine::Tick::Tick(float fInterval)
	: m_fElapsedTime(0.f)
{
	SetInterval(fInterval);
}

void ParaEngine::Tick::SetFPS(float fFPS)
{
	SetInterval(1 / fFPS);
}

void ParaEngine::Tick::SetInterval(float fInterval)
{
	m_invervalSeconds = fInterval;
}

float ParaEngine::Tick::GetInverval() const
{
	return m_invervalSeconds;
}

bool ParaEngine::Tick::IsTick(float deltaTime)
{
	m_fElapsedTime += deltaTime;
	bool bIsTick = false;
	if (m_fElapsedTime >= m_invervalSeconds)
	{
		m_fElapsedTime -= m_invervalSeconds;
		bIsTick = true;
	}
	if (m_fElapsedTime > m_invervalSeconds)
		m_fElapsedTime = m_invervalSeconds;
	return bIsTick;
}
