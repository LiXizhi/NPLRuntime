//-----------------------------------------------------------------------------
// Class:	CSunLight
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.4.14
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SunLight.h"

using namespace ParaEngine;

/**@def default day length in minutes.*/
#define DAY_LENGTH_MINS	  300.f

/**@def default maximum sun angle. range in (0, 3.1416/2) */
#define MAX_SUN_ANGLE	 1.5f 

CSunLight::CSunLight()
	:m_bAutoSunColor(true), m_bHasNightHours(false)
{
	m_fYaxisRotation = 0.3f;
	m_fMaxAngle = MAX_SUN_ANGLE;
	m_fCurrentAngle = m_fMaxAngle;
	
	///create default sun light direction.
	ZeroMemory( &m_light, sizeof(CLightParam) );
	m_light.Type        = D3DLIGHT_DIRECTIONAL;
	m_light.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
	m_light.Position  = Vector3(0.0f, -1.0f, 1.0f);
	m_light.Range        = (1.5*SUN_HEIGHT);

	m_shadowFactor = 0.35f;

	SetDayLength(DAY_LENGTH_MINS);
	// set initial time
	// 30: at noon
	// 60: at sun set, this matches the current sky
	SetTimeOfDay(DAY_LENGTH_MINS*45); 
}

CSunLight::~CSunLight()
{
}

Para3DLight* CSunLight::GetD3DLight()
{
	return (Para3DLight*)(&m_light);
}

float CSunLight::GetTimeOfDay()
{
	return m_seconds;
}

LinearColor CSunLight::GetSunDiffuse()
{
	return LinearColor(m_light.Diffuse);
}

void CSunLight::SetSunDiffuse(const LinearColor& diffuse)
{
	EnableAutoSunColor(false);
	m_light.Diffuse = diffuse;
}

LinearColor CSunLight::GetSunAmbient()
{
	return LinearColor(m_light.Ambient);
}

void CSunLight::SetSunAmbient(const LinearColor& ambient)
{
	EnableAutoSunColor(false);
	m_light.Ambient = ambient;
}

void CSunLight::EnableAutoSunColor(bool bEnable)
{
	if(m_bAutoSunColor!= bEnable)
	{
		m_bAutoSunColor = bEnable;
		if(m_bAutoSunColor)
		{
			AdvanceTimeOfDay(0.f);
		}
	}
}

bool CSunLight::IsAutoSunColorEnable()
{
	return m_bAutoSunColor;
}


LinearColor CSunLight::GetSunColor()
{
	return m_light.Diffuse;
}

Vector3 CSunLight::GetSunDirection()
{
	return m_light.Direction;
}

LinearColor CSunLight::GetSunAmbientHue()
{
	return LinearColor(m_light.Ambient);
}

float CSunLight::GetSunAngle()
{
	return m_fCurrentAngle;
}

void CSunLight::SetSunAngle(float fSunAngle)
{
	EnableAutoSunColor(false);
	m_fCurrentAngle = fSunAngle;
}

CLightScatteringData* CSunLight::GetLightScatteringData()
{
	return &m_LightScatteringData;
}

void CSunLight::SetMaximumAngle(float fMaxAngle)
{
	m_fMaxAngle = fMaxAngle;
}

float CSunLight::GetMaximumAngle()
{
	return m_fMaxAngle;
}

Vector3 CSunLight::GetSunPosition()
{
	return m_light.Position;
}

void CSunLight::SetPosition(const Vector3& newpos)
{
	m_light.Position = newpos;
}

float CSunLight::GetDayLength()
{
	return m_fDayLength/60.f;
}

void CSunLight::SetDayLength(float fMinutes)
{
	m_fDayLength = max(1.0f, fMinutes*60.f); 
	UpdateSunParameters();
}

void CSunLight::SetTimeOfDay(float time)
{
	m_seconds = time;
	UpdateSunParameters();
}

float CSunLight::GetTimeOfDaySTD()
{
	float f24HourLength = m_fDayLength;
	float curTime = m_seconds;
	return (curTime - ((int)(curTime / f24HourLength))*f24HourLength) / f24HourLength * 2 - 1.f;
}

void CSunLight::SetTimeOfDaySTD(float fTime)
{
	m_seconds = ((fTime + 1.f) / 2.f) * m_fDayLength;
	UpdateSunParameters();
}

float CSunLight::AdvanceTimeOfDay(float timeDelta)
{
	m_seconds += timeDelta;
	
	UpdateSunParameters();
	return m_seconds;
}

void CSunLight::SetShadowFactor(float shadowFactor)
{
	m_shadowFactor = shadowFactor;
}

float CSunLight::GetShadowFactor()
{
	return m_shadowFactor;
}

void CSunLight::UpdateSunParameters()
{
	//////////////////////////////////////////////////////////////////////////
	// update the light direction by the current time(m_seconds)
	float m_fCelestialAngle = GetCelestialAngleRadian();
	m_fCurrentAngle = Math::ACos(Math::Abs(Math::Cos(m_fCelestialAngle))).valueRadians();
	if (m_fCelestialAngle > Math::PI)
		m_fCurrentAngle = - m_fCurrentAngle;

	Vector3 vecLightDir(0.0f, -1.0f, 0.f);
	Matrix4 mat, tmp;
	ParaMatrixRotationZ(&mat, m_fCurrentAngle);
	ParaMatrixRotationY(&tmp, m_fYaxisRotation);
	mat = mat*tmp;
	ParaVec3TransformCoord(&vecLightDir, &vecLightDir, &mat);
	m_light.Direction = vecLightDir;

	//////////////////////////////////////////////////////////////////////////
	// update the sun color: ambient and diffuse
	if(IsAutoSunColorEnable())
	{
		float fCosAngle = cosf(m_fCurrentAngle);
		m_light.Ambient.r = 0.15f+fCosAngle*0.5f; // 0.15 is minimum ambient, 0.65 is maximum ambient
		m_light.Ambient.g = m_light.Ambient.r;
		m_light.Ambient.b = m_light.Ambient.r;
		
		m_light.Diffuse.r = 0.2f + 1.f*fCosAngle; 
		m_light.Diffuse.g = m_light.Diffuse.r; // cosf(m_fCurrentAngle*1.3f); // make the light a little reddish at sun rise and sun set
		m_light.Diffuse.b = m_light.Diffuse.g; 
	}
}

LinearColor CSunLight::ComputeFogColor()
{
	//////////////////////////////////////////////////////////////////////////
	// set the fog color by the time of day as well
	// TODO: use a fog table, instead of parametric fog, might have better effect.
	LinearColor fogColorBase =  m_light.Ambient;
	LinearColor  fogColor = fogColorBase;
	 // make it a little bit blue
	fogColor.r*=0.9f;
	fogColor.b*=1.1f;
	fogColor =fogColorBase + fogColor*0.8f;
	return fogColor;
}

int CSunLight::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass!=NULL);
	pClass->AddField("TimeOfDay", FieldType_Float, (void*)SetTimeOfDay_s, (void*)GetTimeOfDay_s, NULL, NULL, bOverride);
	pClass->AddField("TimeOfDaySTD", FieldType_Float, (void*)SetTimeOfDaySTD_s, (void*)GetTimeOfDaySTD_s, NULL, NULL, bOverride);
	pClass->AddField("MaximumAngle", FieldType_Float, (void*)SetMaximumAngle_s, (void*)GetMaximumAngle_s, NULL, NULL, bOverride);
	pClass->AddField("Ambient", FieldType_Vector3, (void*)SetSunAmbient_s, (void*)GetSunAmbient_s, NULL, NULL, bOverride);
	pClass->AddField("Diffuse", FieldType_Vector3, (void*)SetSunDiffuse_s, (void*)GetSunDiffuse_s, NULL, NULL, bOverride);
	pClass->AddField("DayLength", FieldType_Float, (void*)SetDayLength_s, (void*)GetDayLength_s, NULL, NULL, bOverride);
	pClass->AddField("SunAngle", FieldType_Float, (void*)SetSunAngle_s, (void*)GetSunAngle_s, NULL, NULL, bOverride);
	pClass->AddField("AutoSunColor", FieldType_Bool, (void*)EnableAutoSunColor_s, (void*)IsAutoSunColorEnable_s, NULL, NULL, bOverride);
	pClass->AddField("ShadowFactor", FieldType_Float, (void*)SetShadowFactor_s, (void*)GetShadowFactor_s, NULL, NULL, bOverride);
	return S_OK;
}

float ParaEngine::CSunLight::CalculateCelestialAngle(float curTime, float relativeTime)
{
	float f24HourLength = m_fDayLength;
	curTime += relativeTime;
	float fAngleSTD = (curTime - ((int)(curTime / f24HourLength))*f24HourLength) / (HasNightHours() ? f24HourLength : f24HourLength*2.0f) - (HasNightHours() ? 0.5f : 0.25f);
	if (fAngleSTD < 0.f)
		fAngleSTD += 1.f;

	float lastTime = fAngleSTD;
	// just making full sun/moon(12:00am and 12:00pm) a little bit longer
	fAngleSTD = 1.0f - (Math::Cos(fAngleSTD * Math::PI) + 1.0f) / 2.0f;
	// smooth it. 
	fAngleSTD = lastTime + (fAngleSTD - lastTime) / 3.0f;
	return fAngleSTD;
}

float ParaEngine::CSunLight::GetCelestialAngle()
{
	float f24HourLength = m_fDayLength;
	float curTime = m_seconds;
	float fAngleSTD = (curTime - ((int)(curTime / f24HourLength))*f24HourLength) / (HasNightHours() ? f24HourLength : f24HourLength*2.0f) - (HasNightHours() ? 0.5f : 0.25f);
	if (fAngleSTD < 0.f)
		fAngleSTD += 1.f;
	return fAngleSTD;
}

float ParaEngine::CSunLight::GetCelestialAngleRadian()
{
	return (float)(GetCelestialAngle()*Math::PI * 2);
}

LinearColor* ParaEngine::CSunLight::CalcSunriseSunsetColors(float fCelestialAngle)
{
	float fMaxSunColorHeight = 0.4f;
	float fSunHeight = Math::Cos(fCelestialAngle * (float)Math::PI * 2.0f);
	float fHorizonHeight = 0.0f;

	if (fSunHeight >= fHorizonHeight - fMaxSunColorHeight && fSunHeight <= fHorizonHeight + fMaxSunColorHeight)
	{
		float fGradient = (fSunHeight - fHorizonHeight) / fMaxSunColorHeight * 0.5f + 0.5f;
		float fBlendingFactor = 1.0f - (1.0f - Math::Sin(fGradient * (float)Math::PI)) * 0.99f;
		fBlendingFactor *= fBlendingFactor;
		m_colorsSunriseSunset.r = fGradient * 0.3F + 0.7F;
		m_colorsSunriseSunset.g = fGradient * fGradient * 0.7F + 0.2F;
		m_colorsSunriseSunset.b = fGradient * fGradient * 0.0F + 0.2F;
		m_colorsSunriseSunset.a = fBlendingFactor;
		return &m_colorsSunriseSunset;
	}
	return NULL;
}

bool ParaEngine::CSunLight::HasNightHours() const
{
	return m_bHasNightHours;
}

void ParaEngine::CSunLight::SetHasNightHours(bool val)
{
	if (m_bHasNightHours != val)
	{
		m_bHasNightHours = val;
		UpdateSunParameters();
	}
}

