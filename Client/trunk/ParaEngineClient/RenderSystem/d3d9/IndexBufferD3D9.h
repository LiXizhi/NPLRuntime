#pragma once
#include "d3d9.h"
#include "Framework/Interface/Render/IIndexBuffer.h"
namespace ParaEngine
{
	class IndexBufferD3D9 : public IIndexBuffer
	{
	public:
		IndexBufferD3D9(IDirect3DIndexBuffer9* pD3DIndexBuffer);
		virtual ~IndexBufferD3D9() override;
		IDirect3DIndexBuffer9* GetD3DIndexBuffer() const;
	private:
		IDirect3DIndexBuffer9 * m_pD3DIndexBuffer;
	};

	inline IIndexBufferPtr XCeateIndexBuffer(IDirect3DDevice9* pD3Device, UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool)
	{
		IDirect3DIndexBuffer9* pTempVB = nullptr;
		HRESULT hr = pD3Device->CreateIndexBuffer(Length, Usage, Format, Pool, &pTempVB, nullptr);
		if (hr == S_OK)
		{
			return IIndexBufferPtr(new IndexBufferD3D9(pTempVB));
		}

		return nullptr;

	}

	inline IDirect3DIndexBuffer9* XGetIB(IIndexBufferPtr vb)
	{
		return ((IndexBufferD3D9*)vb.get())->GetD3DIndexBuffer();
	}
}