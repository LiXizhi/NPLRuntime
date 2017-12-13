#include "ParaEngine.h"
#include "D3D9RenderDevice.h"

using namespace ParaEngine;
CD3D9RenderDevice::CD3D9RenderDevice():
	m_pD3DDevice(nullptr)
{
}

LPDIRECT3DDEVICE9 CD3D9RenderDevice::GetDirect3DDevice9() const
{
	return m_pD3DDevice;
}
