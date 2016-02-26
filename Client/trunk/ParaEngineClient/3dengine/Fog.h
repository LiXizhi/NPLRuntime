#pragma once

namespace ParaEngine
{
	/**
	* TODO: fog parameters
	*/
	class CFog
	{
	public:

		bool					m_bRenderFog;
		/// original fog color, it will be multiplied by m_fFogDensity to attain the real fog color.
		DWORD					m_dwFogColor;
		FLOAT					m_fFogStart;
		FLOAT					m_fFogEnd;
		FLOAT					m_fFogDensity;
		void SetFogColor(const LinearColor& fogColor);
		LinearColor GetFogColor();
		void SetFogStart(float fFogStart);
		float GetFogStart();
		void SetFogEnd(float fFogEnd);
		float GetFogEnd();
		void SetFogDensity(float fFogDensity);
		float GetFogDensity();
		void EnableFog(bool bEnableFog);
		bool IsFogEnabled();
	public:
		CFog();
		~CFog(){};
	};
}