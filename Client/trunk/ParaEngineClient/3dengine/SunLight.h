#pragma once
#include "IAttributeFields.h"
#include "light_scattering_data.h"
#include "LightParam.h"
#include "Fog.h"

namespace ParaEngine
{
	/**
	* Modeling the global sun and its directional light, including sun position, direction, color, time of day, etc. 
	the sun's direction is defined as below
	y
	|
	|
	|------>x
	/ \
	/   \
	z     \
	sun direction
	when m_fYaxisRotation = 0, the direction vector is in the x,y plane, point in the opposite y direction.
	its angle with the y axis is in the range [-m_fMaxAngle, +m_fMaxAngle],
	when m_fYaxisRotation != 0, the vector is rotated m_fYaxisRotation rad counter clockwise around the y axis.
	The simulation of the path of the sun only covers very limited situations, e.g. winter and spring sun does
	not follow this rule exactly.
	*/
	class CSunLight : public IAttributeFields
	{
	public:
		CSunLight();
		~CSunLight();
		
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CSunLight;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CSunLight"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CSunLight, GetTimeOfDay_s, float*)	{*p1 = cls->GetTimeOfDay(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, SetTimeOfDay_s, float)	{cls->SetTimeOfDay(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, GetTimeOfDaySTD_s, float*)	{*p1 = cls->GetTimeOfDaySTD(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, SetTimeOfDaySTD_s, float)	{cls->SetTimeOfDaySTD(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, GetMaximumAngle_s, float*)	{*p1 = cls->GetMaximumAngle(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, SetMaximumAngle_s, float)	{cls->SetMaximumAngle(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, GetSunDiffuse_s, Vector3*) { *p1 = cls->GetSunDiffuse().ToVector3(); return S_OK; }
		ATTRIBUTE_METHOD1(CSunLight, SetSunDiffuse_s, Vector3) {LinearColor c(p1.x, p1.y,p1.z,1); cls->SetSunDiffuse(c); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, GetSunAmbient_s, Vector3*) { *p1 = cls->GetSunAmbient().ToVector3();  return S_OK; }
		ATTRIBUTE_METHOD1(CSunLight, SetSunAmbient_s, Vector3) {LinearColor c(p1.x, p1.y,p1.z,1); cls->SetSunAmbient(c); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, IsAutoSunColorEnable_s, bool*)	{*p1 = cls->IsAutoSunColorEnable(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, EnableAutoSunColor_s, bool)	{cls->EnableAutoSunColor(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, GetDayLength_s, float*)	{*p1 = cls->GetDayLength(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, SetDayLength_s, float)	{cls->SetDayLength(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSunLight, GetSunAngle_s, float*)	{*p1 = cls->GetSunAngle(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, SetSunAngle_s, float)	{cls->SetSunAngle(p1); return S_OK;}

		//shadow factor   --clayman 2011.9.20
		ATTRIBUTE_METHOD1(CSunLight, GetShadowFactor_s, float*)	{*p1 = cls->GetShadowFactor(); return S_OK;}
		ATTRIBUTE_METHOD1(CSunLight, SetShadowFactor_s, float)	{cls->SetShadowFactor(p1); return S_OK;}

	public:
		/** advance time is in seconds, this will also change the light direction, sun color and sun ambient color.
		* this function is automatically called by the environment simulator
		*/
		float AdvanceTimeOfDay(float timeDelta);

		/** get the equivalent light in d3d format*/
		Para3DLight* GetD3DLight();
		/** set time of day in seconds. Use SetDayLength() to set the total number of minutes in a day.
		* @param time: time in seconds. This can be any non-negative value. 0 means 6:00am at dawn.
		*/
		void SetTimeOfDay(float time);

		/** get the current time in seconds*/
		float GetTimeOfDay();

		/** angle between [0, 1].  0.75 is dawn(6:00am), 0 is noon(12:00am), 0.25 is twilight(18:00pm), 0.5 is mid night(24:00pm).
		* matching to a single cycle of day time. beginning from 6 clock.
		*/
		float GetCelestialAngle();
		/** (GetCelestialAngle() * 2 * PI) */
		float GetCelestialAngleRadian();

		/** in seconds */
		float CalculateCelestialAngle(float curTime, float relativeTime = 0.f);
		/** return NULL if no sun set colors. */
		LinearColor* CalcSunriseSunsetColors(float fCelestialAngle);

		/** set standard time. see SetTimeOfDay()
		* old system does not have night system, hence a day only starts from 6:00am to 18:00pm.
		* newer system (like the simulated sky) have night system. Hence the even number of day is actually rendered as night.
		*@param time: always in the range [-1,1], -0.5 is dawn, 0 means at noon, 0.5 is twilight, 1,-1 is midnight
		*/
		void SetTimeOfDaySTD(float time);

		/** get standard time. see GetTimeOfDay() 
		*@return: *@param time: always in the range [-1,1], -0.5 is dawn, 0 means at noon, 0.5 is twilight, 1,-1 is midnight*/
		float GetTimeOfDaySTD();

		/** set the maximum sun angle with the Y axis.*/
		void SetMaximumAngle(float fMaxAngle);

		/** get the maximum sun angle with the Y axis.*/
		float GetMaximumAngle();

		/** get the ambient sun color. */
		LinearColor GetSunDiffuse();
		/** Set the ambient sun color. 
		@note: it will disable AutoSunColor 
		*/
		void SetSunDiffuse(const LinearColor& diffuse);

		/** get the ambient sun color. */
		LinearColor GetSunAmbient();
		/** Set the ambient sun color. 
		@note: it will disable AutoSunColor 
		*/
		void SetSunAmbient(const LinearColor& ambient);

		/** set how many minutes are there in a day, this is used in time simulation.
		* default value is 300. 
		* @param fMinutes: 
		*/
		void SetDayLength(float fMinutes);
		/** return how many minutes are there in a day in minutes*/
		float GetDayLength();

		/** Get the sun angle, where angle 0 means the noon. +/-1.57 means night or morning. 
		* @return :the angle between the sun direction and the Y(up) axis.
		* in the range [-m_fMaxAngle, +m_fMaxAngle]
		*/
		float GetSunAngle();
		/** set the sun angle. 
		@note: it will disable AutoSunColor 
		*/
		void SetSunAngle(float fSunAngle);

		/** set whether we will automatically adjust sun diffuse and ambient colors according to time of day. 
		* if one calls SetSunDiffuse() or SetSunAmbient(), m_bAutoSunColor will be disabled. And one needs to enable by calling EnableAutoSunColor explicitly. 
		*/
		void EnableAutoSunColor(bool bEnable);
		/** get whether we will automatically adjust sun diffuse and ambient colors according to time of day. 
		* if one calls SetSunDiffuse() or SetSunAmbient(), m_bAutoSunColor will be disabled. And one needs to enable by calling EnableAutoSunColor explicitly. 
		*/
		bool IsAutoSunColorEnable();

		///////////////////////////////////////////
		// following for used by effect manager to get the current state.
		///////////////////////////////////////////

		/** compute and return the fog color*/
		LinearColor ComputeFogColor();

		/** get the ambient sun color. Get the color and intensity of the sun.*/
		LinearColor GetSunColor();
		
		/** get the Ambient color of the sun. 
		All objects under the sun will share this ambient color. This parameter will change according to the current time of day. */
		LinearColor GetSunAmbientHue();

		/** get the normalized sun direction vector, please note the vector always points from the sun position to the origin. 
		One may need to reverse the direction to be used in a shader.
		*/
		Vector3 GetSunDirection();

		/** get light Scattering Data object. More advanced simulation of light. */
		CLightScatteringData* GetLightScatteringData();

		float GetShadowFactor();

		void SetShadowFactor(float shadowFactor);

		bool HasNightHours() const;
		void SetHasNightHours(bool val);
	private:
		CLightParam	m_light;
		/** how many seconds pasted since the beginning of the sun simulation. */
		float		m_seconds; 
		/** rotation around the Y(up) axis. It denotes where the sun should rise. */
		float		m_fYaxisRotation;
		/** the maximum sun angle with the Y axis. see m_fCurrentAngle. */
		float		m_fMaxAngle;
		/** the angle between the sun direction and the Y(up) axis.
		in the range [-m_fMaxAngle, +m_fMaxAngle] */
		float		m_fCurrentAngle;
		/** day length in seconds */
		float		m_fDayLength;
		/** some more pre-calculated data.*/
		CLightScatteringData m_LightScatteringData;
		LinearColor m_colorsSunriseSunset;

		/** whether we will automatically adjust sun diffuse and ambient colors according to time of day. 
		* if one calls SetSunDiffuse() or SetSunAmbient(), m_bAutoSunColor will be disabled. And one needs to enable by calling EnableAutoSunColor explicitly. 
		*/
		bool	m_bAutoSunColor;
		/** by default night hours are disabled. For simulated sky, there may be night hours. */
		bool	m_bHasNightHours;
		
		float   m_shadowFactor;
		
	private:
		/** update the sun parameters such as position, color, from the time of day. 
		The sun's direction vector is also recalculated by: 
		given the m_fYaxisRotation, m_fCurrentAngle, calculate the light direction.
		*/
		void UpdateSunParameters();

		/** set the new position of the sun. This function is only used for rendering with d3d. And is called only CSkyMesh*/
		void SetPosition(const Vector3& newpos);
		Vector3 GetSunPosition();

		friend class CSkyMesh;
		friend class CSceneObject;
	};
}