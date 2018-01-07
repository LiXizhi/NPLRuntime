#pragma once
#include "IRenderContext.h"
#include <vector>
#include <string>

struct IDirect3D9;
namespace ParaEngine
{
	class D3D9RenderContext : public IRenderContext
	{
	public:	
		static D3D9RenderContext* Create();
		virtual ~D3D9RenderContext() override;
	private:
		D3D9RenderContext();
		IDirect3D9* m_D3D;
	public:
		virtual IRenderDevice* CreateDevice(const RenderDeviceConfiguration& cfg) override;
		IDirect3D9* GetD3D() const;


		virtual bool ResetDevice(IRenderDevice* device, const RenderDeviceConfiguration& cfg) override;

	};
	using D3D9RenderContextPtr = std::shared_ptr<D3D9RenderContext>;
}