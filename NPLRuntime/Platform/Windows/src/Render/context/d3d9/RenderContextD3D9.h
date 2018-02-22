#pragma once
#include "Framework/Interface/Render/IRenderContext.h"
#include <vector>
#include <string>
//#include "d3d9.h"
namespace ParaEngine
{
	class RenderContextD3D9 : public IRenderContext
	{
	public:	
		RenderContextD3D9();
		virtual ~RenderContextD3D9() override;
	protected:

		IDirect3D9* m_D3D;
	public:
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		IDirect3D9* GetD3D() const;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;

	};
}