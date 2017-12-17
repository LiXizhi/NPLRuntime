#pragma once
#include <string>
#include "WindowsRenderWindow.h"
#include "d3d9.h"
namespace ParaEngine
{
	class D3D9RenderContext
	{
	public:
		D3D9RenderContext(WindowsRenderWindow* window);		
		~D3D9RenderContext() = default;
		bool Initialize();
		void Uninitialize();
		bool IsInvalid() const;
	
	private:
		bool FindBestFullscreenMode(bool bRequireHAL, bool bRequireREF);
		bool FindBestWindowedMode(bool bRequireHAL, bool bRequireREF);
		HRESULT ChooseInitialD3DSettings();
		HRESULT Initialize3DEnvironment();
		static bool ConfirmDeviceHelper(D3DCAPS9 * pCaps,
			VertexProcessingType vertexProcessingType,
			D3DFORMAT adapterFormat,
			D3DFORMAT backBufferFormat);

	private:
		D3DPRESENT_PARAMETERS m_d3dpp;
		WindowsRenderWindow* m_pRenderWindow;
		IDirect3D9* m_pD3D9;
		IDirect3DDevice9* m_pD3Device;
		CD3DEnumeration   m_d3dEnumeration;
		CD3DSettings      m_d3dSettings;
		std::string m_strDeviceStats;
		D3DCAPS9          m_d3dCaps;
		bool m_invalid;
	};
}