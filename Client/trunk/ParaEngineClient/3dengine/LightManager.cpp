//-----------------------------------------------------------------------------
// Class:	CLightManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4.17
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include <algorithm>

#include "LightManager.h"

/** @def default maximum number of light*/
#define   DefaultMaxNumLights      8
/** @def  max light score*/
#define   MAX_LIGHT_SCORE		100

using namespace ParaEngine;

CLightManager::CLightManager(void)
:m_lightsResNum(0)
{
	m_lights.reserve(10);

}

CLightManager::~CLightManager(void)
{
}

CLightManager& CLightManager::GetSingleton()
{
	static CLightManager g_singleton;
	return g_singleton;
}

int CLightManager::GetNumLights() const
{
	return (int)m_lights.size();
}

LightList& CLightManager::GetLights()
{
	return m_lights;
}

void CLightManager::CleanupLights()
{
	m_lights.clear();
}
bool CLightManager::RegisterLight(CLightParam* pLight)
{
	if(pLight==NULL)
		return false;
	int nSize = (int)m_lights.size();
	for(int i = 0; i<nSize; i++)
	{
		if(pLight == m_lights[i])
		{
			return true; // already been registered before
		}
	}
	m_lights.push_back(pLight);// add a new light
	return true;
}

bool CLightManager::UnRegisterLight(CLightParam* pLight)
{
	for(vector<CLightParam*>::iterator itCur = m_lights.begin(); itCur!=m_lights.end();)
	{
		if(pLight == (*itCur))
			itCur = m_lights.erase(itCur);
		else
			++itCur;
	}
	return true;
}
int CLightManager::GetBestLights(const Vector3& vCenter , float fRadius, LightList & listOutLights, int nMaxNumLights)
{
	if(nMaxNumLights>0)
	{
		// compute scores for every light
		int nSize = (int)m_lights.size();
		for(int i = 0; i < nSize; i++)
			ComputeLightScore(m_lights[i], vCenter, fRadius);

		return FillLightList(listOutLights, nMaxNumLights);
	}
	else
		return 0;
}

int CLightManager::GetBestLights(const Vector3& vCenter , float fRadius, int nMaxNumLights)
{
	m_lightsResNum = GetBestLights(vCenter, fRadius, m_lightsRes, nMaxNumLights);
	return m_lightsResNum;
}

LightList* CLightManager::GetLastResult()
{
	return &m_lightsRes;
}

int CLightManager::GetLastResultNum()
{
	return m_lightsResNum;
}

int CLightManager::ComputeLightScore(CLightParam* pLight, const Vector3& vCenter, float fRadius) const
{
	switch(pLight->Type)
	{
	case D3DLIGHT_DIRECTIONAL:   
		{
			pLight->m_nScore = MAX_LIGHT_SCORE;
			break;
		}

	case D3DLIGHT_POINT:
		{
			Vector3 diff = pLight->Position;
			diff -= vCenter;
			
			float lenSq = diff.squaredLength();
			float sumRadiusSq = (fRadius + pLight->Range);
			sumRadiusSq *= sumRadiusSq;

			if(lenSq > sumRadiusSq)
			{
				pLight->m_nScore = 0;
				break;
			}

			float radSq = fRadius * fRadius;
			/*pLight->m_nScore = 1 + int((sumRadiusSq - lenSq) / radSq*MAX_LIGHT_SCORE);
			if(pLight->m_nScore > MAX_LIGHT_SCORE)
				pLight->m_nScore = MAX_LIGHT_SCORE;*/

			pLight->m_nScore = int((sumRadiusSq - lenSq) / radSq*MAX_LIGHT_SCORE);
			
			break;
		}
	default:
		pLight->m_nScore = 1;
		break;
	}
	return 0;
}

int CLightManager::FillLightList(LightList & listOutLights, int nMaxNumLights)
{
	sort(m_lights.begin(), m_lights.end(), CLightParam::IsGreaterPt);
	
	int nSize = (int)m_lights.size();
	int i=0;
	for(i = 0; (i<nSize) && ( m_lights[i]->m_nScore>0 && i< nMaxNumLights ); i++)
	{
		if(i>=(int)listOutLights.size())
			listOutLights.push_back(m_lights[i]);
		else
			listOutLights[i] = m_lights[i];
	}
	return i;
}

