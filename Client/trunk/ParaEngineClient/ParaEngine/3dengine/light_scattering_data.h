#pragma once

namespace ParaEngine
{
#ifdef USE_DIRECTX_RENDERER
	#include "shaders/light_scattering_constants.h"
#else
	//
	// Define a structure used by both C++ and HLSL
	//
	struct sLightScatteringShaderParams
	{
		Vector4 vBeta1;
		Vector4 vBeta2;
		Vector4 vBetaD1;
		Vector4 vBetaD2;
		Vector4 vSumBeta1Beta2;
		Vector4 vLog2eBetaSum;
		Vector4 vRcpSumBeta1Beta2;
		Vector4 vHG;
		Vector4 vConstants;
		Vector4 vTermMultipliers;
		Vector4 vSoilReflectivity;
	};
#endif
	class CLightScatteringData
	{
	public:
		CLightScatteringData();
		~CLightScatteringData();

	private:
		float m_henyeyG;
		float m_rayleighBetaMultiplier;
		float m_mieBetaMultiplier;
		float m_inscatteringMultiplier;
		float m_extinctionMultiplier;
		float m_reflectivePower;
		sLightScatteringShaderParams m_shaderParams;
	public:
		void recalculateShaderData();

		void setHenyeyG(float g);
		void setRayleighScale(float s);
		void setMieScale(float s);
		void setInscatteringScale(float s);
		void setExtinctionScale(float s);
		void setTerrainReflectionScale(float s);
		sLightScatteringShaderParams* getShaderData(){return &m_shaderParams;};
	protected:

	};
}