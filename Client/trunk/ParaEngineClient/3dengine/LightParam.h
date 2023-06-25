#pragma once

namespace ParaEngine
{
	/** a single light in ParaEngine. */
	class CLightParam : public Para3DLight
	{
	public:
		CLightParam(void);
		~CLightParam(void);
	public:

		// Return whether first element is greater than the second
		bool static IsGreater(const CLightParam& elem1, const CLightParam& elem2)
		{
			return elem1.m_nScore > elem2.m_nScore;
		}
		bool static IsGreaterPt(const CLightParam* elem1, const CLightParam* elem2)
		{
			return elem1->m_nScore > elem2->m_nScore;
		}

		/** make the current light a white point light with default value. */
		void MakeRedPointLight();
		void MakeRedSpotLight();
		void MakeRedDirectionalLight();

		// calculate direction base on yaw/pitch/roll
		void RecalculateDirection(const Matrix4 * pLocalTransform = NULL);

		/**
		* return the parameters as a string. This is usually used for serialization.
		* format is "Type Range (r g b a) att0 att1 att2"
		* D3DLIGHTTYPE    Type;            Type of light source
		* 						- D3DLIGHT_POINT          = 1,
		* 						- D3DLIGHT_SPOT           = 2,
		* 						- D3DLIGHT_DIRECTIONAL    = 3,
		* float           Range;           Cutoff range
		* D3DCOLORVALUE   Diffuse;         Diffuse color of light
		* float           Attenuation0;    Constant attenuation
		* float           Attenuation1;    Linear attenuation
		* float           Attenuation2;    Quadratic attenuation
		* e.g. "1 7.0 (1 1 0 1) 0.3 0.1 1"
		* light intensity is calculated as 1/(Attenuation0+d*Attenuation1+d*d*Attenuation2), where d is the distance from the light to object center.
		* @return see above
		*/
		const char* ToString();
		/** convert a string returned by ToString() to this object.
		* @see ToString() */
		void FromString(const char* str);

	public:
		// yaw, pitch, roll for recording the rotation
		// radian system
		float Yaw;
		float Pitch;
		float Roll;

	private:
		int m_nScore;
		friend class CLightManager;

		Vector3 InitDirection = { 0, 0, 1 };
	};
}

