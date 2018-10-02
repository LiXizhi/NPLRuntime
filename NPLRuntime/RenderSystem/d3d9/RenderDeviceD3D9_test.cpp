#include "RenderDeviceD3D9.h"
#include "D3DMapping.h"
namespace ParaEngine {

#define TMP_ASSERT(x) if (!(x)) *((char*)0) = 0;

	bool IRenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
	{
		return true;
	}

	RenderDeviceD3D9::RenderDeviceD3D9(IDirect3DDevice9* device, IDirect3D9* context)
		: m_pD3DDevice(device)
		, m_pContext(context)
	{
	}

	bool RenderDeviceD3D9::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		auto ret = m_pD3DDevice->SetTexture(stage, texture);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		//return m_pD3DDevice->DrawPrimitive(D3DMapping::toD3DPrimitiveType(PrimitiveType), StartVertex, PrimitiveCount) == S_OK;

		auto ret = m_pD3DDevice->DrawPrimitive(D3DMapping::toD3DPrimitiveType(PrimitiveType), StartVertex, PrimitiveCount);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
	{
		HRESULT ret = m_pD3DDevice->DrawIndexedPrimitive(D3DMapping::toD3DPrimitiveType(Type), BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		//return m_pD3DDevice->DrawPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) == S_OK;

		HRESULT ret = m_pD3DDevice->DrawPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void * pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		//return m_pD3DDevice->DrawIndexedPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), MinVertexIndex,
		//	NumVertices, PrimitiveCount, pIndexData, D3DMapping::toD3DFromat(IndexDataFormat), pVertexStreamZeroData, VertexStreamZeroStride) == S_OK;

		HRESULT ret = m_pD3DDevice->DrawIndexedPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, D3DMapping::toD3DFromat(IndexDataFormat), pVertexStreamZeroData, VertexStreamZeroStride);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::Clear(bool color, bool depth, bool stencil)
	{
		DWORD clearFlags = 0;
		if (color)clearFlags |= D3DCLEAR_TARGET;
		if (depth)clearFlags |= D3DCLEAR_ZBUFFER;
		if (stencil)clearFlags |= D3DCLEAR_STENCIL;
		DWORD d3dColor = D3DCOLOR_RGBA((int)(m_CurrentClearColor.r * 255), (int)(m_CurrentClearColor.g * 255), (int)(m_CurrentClearColor.b * 255), (int)(m_CurrentClearColor.a * 255));
		//return  m_pD3DDevice->Clear(0, nullptr, clearFlags, d3dColor, m_CurrentDepth, m_CurrentStencil) == S_OK;

		HRESULT ret = m_pD3DDevice->Clear(0, nullptr, clearFlags, d3dColor, m_CurrentDepth, m_CurrentStencil);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::SetScissorRect(RECT* pRect)
	{
		//return m_pD3DDevice->SetScissorRect(pRect) == S_OK;

		HRESULT ret = m_pD3DDevice->SetScissorRect(pRect);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::GetScissorRect(RECT* pRect)
	{
		//return m_pD3DDevice->GetScissorRect(pRect) == S_OK;

		HRESULT ret = m_pD3DDevice->GetScissorRect(pRect);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::BeginScene()
	{
		//return m_pD3DDevice->BeginScene() == S_OK;

		HRESULT ret = m_pD3DDevice->BeginScene();
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::EndScene()
	{
		//return  m_pD3DDevice->EndScene() == S_OK;

		HRESULT ret = m_pD3DDevice->EndScene();
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::Present()
	{
		//return m_pD3DDevice->Present(NULL, NULL, NULL, NULL) == S_OK;

		HRESULT ret = m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	uint32_t RenderDeviceD3D9::GetRenderState(const ERenderState& State)
	{
		auto d3dRenderState = D3DMapping::toD3DRenderState(State);
		DWORD state = 0;
		if (m_pD3DDevice->GetRenderState(d3dRenderState, &state) == S_OK)
		{
			return (uint32_t)state;
		}

		printf("2222222222 \n");

		return 0;
	}

	bool RenderDeviceD3D9::SetRenderState(const ERenderState State, const uint32_t Value)
	{

		auto rs = D3DMapping::toD3DRenderState(State);
		HRESULT hr = m_pD3DDevice->SetRenderState(rs, D3DMapping::toD3DRenderStateValue(State, Value));
		TMP_ASSERT(hr == S_OK);
		return hr == D3D_OK;
	}

	bool RenderDeviceD3D9::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		//return m_pD3DDevice->SetClipPlane(Index, pPlane) == S_OK;

		HRESULT ret = m_pD3DDevice->SetClipPlane(Index, pPlane);
		TMP_ASSERT(ret == S_OK);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat /*= 0*/, uint32_t nDataType /*= 0*/)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return false;
	}

	int RenderDeviceD3D9::GetMaxSimultaneousTextures()
	{
		return 8;
	}

	bool RenderDeviceD3D9::SetClearColor(const Color4f& color)
	{
		m_CurrentClearColor = color;
		return true;
	}

	bool RenderDeviceD3D9::SetClearDepth(const float depth)
	{
		m_CurrentDepth = depth;
		return true;
	}

	bool RenderDeviceD3D9::SetClearStencil(const int stencil)
	{
		m_CurrentStencil = stencil;
		return true;
	}



	bool RenderDeviceD3D9::SetViewport(const Rect& viewport)
	{
		D3DVIEWPORT9 vp;
		vp.X = (DWORD)viewport.x;
		vp.Y = (DWORD)viewport.y;
		vp.Width = (DWORD)viewport.z;
		vp.Height = (DWORD)viewport.w;
		vp.MinZ = 0;
		vp.MaxZ = 1.f;
		return m_pD3DDevice->SetViewport(&vp) == S_OK;
	}

	Rect RenderDeviceD3D9::GetViewport()
	{
		D3DVIEWPORT9 vp;
		m_pD3DDevice->GetViewport(&vp);

		Rect rect;
		rect.x = (float)vp.X;
		rect.y = (float)vp.Y;
		rect.z = (float)vp.Width;
		rect.w = (float)vp.Height;
		return rect;
	}
}