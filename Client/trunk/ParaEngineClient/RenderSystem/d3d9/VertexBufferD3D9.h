#pragma once
#include "d3d9.h"
#include "Framework/Interface/Render/IVertexBuffer.h"
namespace ParaEngine
{
	class VertexBufferD3D9 : public IVertexBuffer
	{
	public:
		VertexBufferD3D9(IDirect3DVertexBuffer9* pD3DVertexBuffer);
		virtual ~VertexBufferD3D9() override;
		IDirect3DVertexBuffer9* GetD3DVertexBuffer() const;
	private:
		IDirect3DVertexBuffer9 * m_pD3DVertexBuffer;
	};

	inline IVertexBufferPtr XCeateVertexBuffer(IDirect3DDevice9* pD3Device,uint32_t Length, uint32_t Usage, uint32_t FVF, D3DPOOL Pool)
	{
		IDirect3DVertexBuffer9* pTempVB = nullptr;
		HRESULT hr = pD3Device->CreateVertexBuffer(Length, Usage, FVF, Pool, &pTempVB,nullptr);
		if (hr == S_OK)
		{
			return IVertexBufferPtr(new VertexBufferD3D9(pTempVB));
		}

		return nullptr;

	}

	inline IDirect3DVertexBuffer9* XGetVB(IVertexBufferPtr vb)
	{
		return ((VertexBufferD3D9*)vb.get())->GetD3DVertexBuffer();
	}
}