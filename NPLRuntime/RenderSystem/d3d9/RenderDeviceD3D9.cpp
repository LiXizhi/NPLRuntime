
#include "RenderDeviceD3D9.h"
#include "D3DMapping.h"
namespace ParaEngine {

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
		return m_pD3DDevice->SetTexture(stage, texture) == S_OK;
	}

	bool RenderDeviceD3D9::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		return m_pD3DDevice->DrawPrimitive(D3DMapping::toD3DPrimitiveType(PrimitiveType), StartVertex, PrimitiveCount) == S_OK;
	}

	bool RenderDeviceD3D9::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
	{
		HRESULT ret = m_pD3DDevice->DrawIndexedPrimitive(D3DMapping::toD3DPrimitiveType(Type), BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount);
		return ret == S_OK;
	}

	bool RenderDeviceD3D9::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		return m_pD3DDevice->DrawPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) == S_OK;
	}

	bool RenderDeviceD3D9::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void * pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		return m_pD3DDevice->DrawIndexedPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), MinVertexIndex,
			NumVertices, PrimitiveCount, pIndexData, D3DMapping::toD3DFromat(IndexDataFormat), pVertexStreamZeroData, VertexStreamZeroStride) == S_OK;
	}

	bool RenderDeviceD3D9::SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix)
	{
		D3DTRANSFORMSTATETYPE state = D3DTS_VIEW;
		switch (State)
		{
		case ParaEngine::ETransformsStateType::WORLD:
			state = D3DTS_WORLD;
			break;
		case ParaEngine::ETransformsStateType::VIEW:
			state = D3DTS_VIEW;
			break;
		case ParaEngine::ETransformsStateType::PROJECTION:
			state = D3DTS_PROJECTION;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE0:
			state = D3DTS_TEXTURE0;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE1:
			state = D3DTS_TEXTURE1;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE2:
			state = D3DTS_TEXTURE2;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE3:
			state = D3DTS_TEXTURE3;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE4:
			state = D3DTS_TEXTURE4;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE5:
			state = D3DTS_TEXTURE5;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE6:
			state = D3DTS_TEXTURE6;
			break;
		case ParaEngine::ETransformsStateType::TEXTURE7:
			state = D3DTS_TEXTURE7;
			break;
		default:
			break;
		}

		HRESULT hr = m_pD3DDevice->SetTransform(state, pMatrix);
		return hr == S_OK;
	}

	bool RenderDeviceD3D9::SetFVF(uint32_t FVF)
	{
		return m_pD3DDevice->SetFVF(FVF) == S_OK;
	}

	void RenderDeviceD3D9::SetCursorPosition(int X, int Y, uint32_t Flags)
	{
		m_pD3DDevice->SetCursorPosition(X, Y, Flags);
	}

	bool RenderDeviceD3D9::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		return m_pD3DDevice->SetSamplerState(stage, D3DMapping::toD3DSamplerSatetType(type), value) == S_OK;
	}

	bool RenderDeviceD3D9::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		return m_pD3DDevice->GetSamplerState(stage, D3DMapping::toD3DSamplerSatetType(type), (DWORD*)value) == S_OK;
	}

	bool RenderDeviceD3D9::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
	{
		return m_pD3DDevice->SetVertexDeclaration(pVertexDeclaration) == S_OK;
	}

	bool RenderDeviceD3D9::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
	{
		return m_pD3DDevice->CreateVertexDeclaration(pVertexElements, ppDecl) == S_OK;
	}

	bool RenderDeviceD3D9::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		return m_pD3DDevice->SetIndices(pIndexData) == S_OK;
	}

	bool RenderDeviceD3D9::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
	{
		return m_pD3DDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride) == S_OK;
	}

	ParaViewport RenderDeviceD3D9::GetViewport()
	{
		D3DVIEWPORT9 vp;
		m_pD3DDevice->GetViewport(&vp);
		ParaViewport pvp;
		
		pvp.X = vp.X;
		pvp.Y = vp.Y;
		pvp.Width = vp.Width;
		pvp.Height = vp.Height;
		pvp.MinZ = vp.MinZ;
		pvp.MaxZ = vp.MaxZ;

		return pvp;
	}

	bool RenderDeviceD3D9::SetViewport(const ParaViewport& viewport)
	{
		D3DVIEWPORT9 vp;
		vp.X = (DWORD)viewport.X;
		vp.Y = (DWORD)viewport.Y;
		vp.Width = (DWORD)viewport.Width;
		vp.Height = (DWORD)viewport.Height;
		vp.MinZ = viewport.MinZ;
		vp.MaxZ = viewport.MaxZ;

		return m_pD3DDevice->SetViewport(&vp) == S_OK;
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

	bool RenderDeviceD3D9::Clear(bool color, bool depth, bool stencil)
	{
		DWORD clearFlags = 0;
		if (color)clearFlags |= D3DCLEAR_TARGET;
		if (depth)clearFlags |= D3DCLEAR_ZBUFFER;
		if (stencil)clearFlags |= D3DCLEAR_STENCIL;
		DWORD d3dColor = D3DCOLOR_RGBA((int)(m_CurrentClearColor.r * 255), (int)(m_CurrentClearColor.g * 255), (int)(m_CurrentClearColor.b * 255), (int)(m_CurrentClearColor.a * 255));
		return  m_pD3DDevice->Clear(0, nullptr, clearFlags, d3dColor, m_CurrentDepth, m_CurrentStencil) == S_OK;
	}

	bool RenderDeviceD3D9::SetScissorRect(RECT* pRect)
	{
		return m_pD3DDevice->SetScissorRect(pRect) == S_OK;
	}

	bool RenderDeviceD3D9::GetScissorRect(RECT* pRect)
	{
		return m_pD3DDevice->GetScissorRect(pRect) == S_OK;
	}

	bool RenderDeviceD3D9::BeginScene()
	{
		return m_pD3DDevice->BeginScene() == S_OK;
	}

	bool RenderDeviceD3D9::EndScene()
	{
		return  m_pD3DDevice->EndScene() == S_OK;
	}

	bool RenderDeviceD3D9::Present()
	{
		return m_pD3DDevice->Present(NULL, NULL, NULL, NULL) == S_OK;
	}

	void RenderDeviceD3D9::Flush()
	{
		
	}


	uint32_t RenderDeviceD3D9::GetRenderState(const ERenderState& State)
	{
		auto d3dRenderState = D3DMapping::toD3DRenderState(State);
		DWORD state = 0;
		if (m_pD3DDevice->GetRenderState(d3dRenderState, &state) == S_OK)
		{
			return (uint32_t)state;
		}
		return 0;
	}

	bool RenderDeviceD3D9::SetRenderState(const ERenderState State, const uint32_t Value)
	{

		auto rs = D3DMapping::toD3DRenderState(State);
		HRESULT hr = m_pD3DDevice->SetRenderState(rs, D3DMapping::toD3DRenderStateValue(State, Value));
		return hr == D3D_OK;
	}


	bool RenderDeviceD3D9::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		return m_pD3DDevice->SetClipPlane(Index, pPlane) == S_OK;
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

}