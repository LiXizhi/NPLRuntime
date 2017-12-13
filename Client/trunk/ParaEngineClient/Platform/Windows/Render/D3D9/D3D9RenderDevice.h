#include "IRenderDevice.h"
#include "d3d9.h"


namespace ParaEngine
{
	class CD3D9RenderDevice : public IRenderDevice
	{
	public:
		CD3D9RenderDevice();
		virtual ~CD3D9RenderDevice() = default;
	
		LPDIRECT3DDEVICE9 GetDirect3DDevice9() const;
	
	private:
		LPDIRECT3DDEVICE9 m_pD3DDevice;
	};
}
