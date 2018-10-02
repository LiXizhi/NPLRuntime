
#pragma once

#include"ParaEngine.h"
#include "VertexFVF.h"

namespace ParaEngine
{
	class WaveEffect
	{
	public:
		WaveEffect();
		~WaveEffect();

	public:
		HRESULT InvalidateDeviceObjects();
		HRESULT RestoreDeviceObjects();
		HRESULT Render();

		void SetWaveSpeed(float value){m_waveParam[3] = value;}
		float GetWaveSpeed(){return m_waveParam[3];}

		void SetWaveSize(float value){m_waveParam[1];}
		float GetWaveSize(){return m_waveParam[1];}


		void SetWaveStrength(float value){m_waveParam[2] = value;}
		float GetWaveStrength(){return m_waveParam[2];}

	private:
		asset_ptr<TextureEntity> m_pNoiseMap;
		IParaEngine::ITexture* m_pBackbufferCopy;
		float m_waveParam[4];
		mesh_vertex_plain m_quadVertices[4];
	};
}