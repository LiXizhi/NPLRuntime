#pragma once
#include <vector>
#include "LightParam.h"

namespace ParaEngine
{
	using namespace std;
	typedef vector<CLightParam*> LightList;

	/** managing local lights.
	its basics functionality is to return a number of lights which are interested to a given (spherical) area. */
	class CLightManager
	{
	public:
		CLightManager(void);
		~CLightManager(void);

		static CLightManager& GetSingleton();
	public:
		/** clean up the light pool */
		void CleanupLights();

		/** add a new light to the pool */
		bool RegisterLight(CLightParam* pLight);
		/** remove a light to the pool*/
		bool UnRegisterLight(CLightParam* pLight);

		/** select up to nMaxNumLights lights, which are affecting a sphere area (vCenter, fRadius).
		* this function is automatically called by the effect manager to determine which lights are to be enabled for a certain drawable object. 
		@param vCenter: center of the sphere 
		@param fRadius: radius of the sphere 
		@param listOutLights: this lists will be filled with lights.
		@param nMaxNumLights: the returned number of lights is always smaller than this value.
		@return: the number of light, returned in listOutLights. Please do not use the size of listOutLights for the number of light. Always use this returned value.*/
		int GetBestLights(const Vector3& vCenter , float fRadius, LightList & listOutLights, int nMaxNumLights = 0xffffffff);
		/** the result can by get by GetLastResult()*/
		int GetBestLights(const Vector3& vCenter , float fRadius, int nMaxNumLights = 0xffffffff);
		/** get the result returned from GetBestLights(). Please note that the light manager just return the member point which is 
		* used to keep the best light result if the user does not supply it. So the light list returned is not persistent across multiple calls of GetBestLights*/
		LightList* GetLastResult();
		/** the number of lights returned in the last GetBestLights() call. Please note that the light manager just return the member point which is 
		* used to keep the best light result if the user does not supply it. So the light list returned is not persistent across multiple calls of GetBestLights*/
		int GetLastResultNum();

		/** number of lights in the pool */
		int GetNumLights() const;
		/** get the light pool */
		LightList& GetLights();

		/** The more the given light affects the sphere area(vCenter, fRadius), the higher the scored light will be. 
		* score is 0 or negative if the light is not affecting the area at all.
		* @param pLight: the given light. The score will be saved as well.
		* @param vCenter: the center of the sphere
		* @param fRadius: the radius of the sphere
		* @return: the score is returned.
		*/
		int ComputeLightScore(CLightParam* pLight, const Vector3& vCenter, float fRadius) const;

	private:
		/** sort the light and fill the nMaxNumLights number of lights with the highest score to listOutLights 
		@param listOutLights: this lists will be filled with lights.
		@param nMaxNumLights: the returned number of lights is always smaller than this value.
		@return: the number of light, returned in listOutLights. Please do not use the size of listOutLights for the number of light. Always use this returned value.*/
		int FillLightList(LightList & listOutLights, int nMaxNumLights = 0xffffffff);

		// light pool
		LightList m_lights;
		// light result
		LightList m_lightsRes;
		int m_lightsResNum;
	};
}
