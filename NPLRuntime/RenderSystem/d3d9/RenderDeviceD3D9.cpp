
#include "RenderDeviceD3D9.h"
#include "D3DMapping.h"
#include "EffectD3D9.h"
#include "TextureD3D9.h"
using namespace ParaEngine;


class D3D9ShaderInclude : public ID3DXInclude
{

public:
	D3D9ShaderInclude(IParaEngine::IEffectInclude* pIncludeImpl)
	{
		m_pIncludeImpl = pIncludeImpl;
	}
	STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{

		if (m_pIncludeImpl)
		{
			if (m_pIncludeImpl->Open(pFileName, (void**)ppData, pBytes))
			{
				return S_OK;
			}
		}

		return E_FAIL;

	}
	/// close an include file
	STDMETHOD(Close)(LPCVOID pData)
	{
		if (m_pIncludeImpl)
		{
			m_pIncludeImpl->Close((void*)pData);
			return S_OK;
		}
		return E_FAIL;
	}
private:
	IParaEngine::IEffectInclude* m_pIncludeImpl;
};



bool ParaEngine::IRenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
{
	return true;
}


ParaEngine::RenderDeviceD3D9::RenderDeviceD3D9(IDirect3DDevice9* device, IDirect3D9* context)
	:m_pD3DDevice(device)
	,m_pContext(context)
	,m_backbufferDepthStencil(nullptr)
	,m_backbufferRenderTarget(nullptr)
	,m_CurrentDepthStencil(nullptr)
	,m_CurrentRenderTargets()
{
	InitCaps();
	memset(m_CurrentRenderTargets, 0, sizeof(m_CurrentRenderTargets));

	// get backbuffer render target

	LPDIRECT3DSURFACE9 renderTargetSurface,depthSurface;
	device->GetRenderTarget(0, &renderTargetSurface);
	device->GetDepthStencilSurface(&depthSurface);

	m_backbufferRenderTarget = static_cast<TextureD3D9*>(TextureD3D9::Create(this, renderTargetSurface, ETextureUsage::RenderTarget));
	m_backbufferDepthStencil = static_cast<TextureD3D9*>(TextureD3D9::Create(this, depthSurface, ETextureUsage::DepthStencil));

	m_CurrentRenderTargets[0] = m_backbufferRenderTarget;
	m_CurrentDepthStencil = m_backbufferDepthStencil;
}

bool ParaEngine::RenderDeviceD3D9::SetTexture(uint32_t stage, IParaEngine::ITexture* texture)
{
	LPDIRECT3DTEXTURE9 tex = NULL;
	if (texture)
	{
		tex = (static_cast<TextureD3D9*>(texture))->GetTexture();
	}
	return m_pD3DDevice->SetTexture(stage, tex) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
{
	return m_pD3DDevice->DrawPrimitive(D3DMapping::toD3DPrimitiveType(PrimitiveType), StartVertex, PrimitiveCount) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
{
	HRESULT ret = m_pD3DDevice->DrawIndexedPrimitive(D3DMapping::toD3DPrimitiveType(Type), BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount);
	return ret == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	return m_pD3DDevice->DrawPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void * pIndexData, EPixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	return m_pD3DDevice->DrawIndexedPrimitiveUP(D3DMapping::toD3DPrimitiveType(PrimitiveType), MinVertexIndex,
		NumVertices, PrimitiveCount, pIndexData, D3DMapping::toD3DFromat(IndexDataFormat), pVertexStreamZeroData, VertexStreamZeroStride) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix)
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

	HRESULT hr = m_pD3DDevice->SetTransform(state,pMatrix);
	return hr == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetFVF(uint32_t FVF)
{
	return m_pD3DDevice->SetFVF(FVF) == S_OK;
}

void ParaEngine::RenderDeviceD3D9::SetCursorPosition(int X, int Y, uint32_t Flags)
{
	m_pD3DDevice->SetCursorPosition(X, Y, Flags);
}

bool ParaEngine::RenderDeviceD3D9::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
{
	return m_pD3DDevice->SetSamplerState(stage, D3DMapping::toD3DSamplerSatetType(type), value) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
{
	return m_pD3DDevice->GetSamplerState(stage, D3DMapping::toD3DSamplerSatetType(type), (DWORD*)value) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
{
	return m_pD3DDevice->SetVertexDeclaration(pVertexDeclaration) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
{
	return m_pD3DDevice->CreateVertexDeclaration(pVertexElements, ppDecl) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetIndices(IndexBufferDevicePtr_type pIndexData)
{
	return m_pD3DDevice->SetIndices(pIndexData) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
{
	return m_pD3DDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride) == S_OK;
}

ParaViewport ParaEngine::RenderDeviceD3D9::GetViewport()
{
	D3DVIEWPORT9 vp;
	m_pD3DDevice->GetViewport(&vp);
	ParaViewport outVP;
	outVP.X = vp.X;
	outVP.Y = vp.Y;
	outVP.Width = vp.Width;
	outVP.Height = vp.Height;
	outVP.MinZ = vp.MinZ;
	outVP.MaxZ = vp.MaxZ;
	return outVP;
}

bool ParaEngine::RenderDeviceD3D9::SetViewport(const ParaViewport& viewport)
{
	D3DVIEWPORT9 vp;
	vp.X = viewport.X;
	vp.Y = viewport.Y;
	vp.Width = viewport.Width;
	vp.Height = viewport.Height;
	vp.MinZ = viewport.MinZ;
	vp.MaxZ = viewport.MaxZ;
	return m_pD3DDevice->SetViewport(&vp) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetClearColor(const Color4f& color)
{
	m_CurrentClearColor = color;
	return true;
}

bool ParaEngine::RenderDeviceD3D9::SetClearDepth(const float depth)
{
	m_CurrentDepth = depth;
	return true;
}

bool ParaEngine::RenderDeviceD3D9::SetClearStencil(const int stencil)
{
	m_CurrentStencil = stencil;
	return true;
}

bool ParaEngine::RenderDeviceD3D9::Clear(bool color, bool depth, bool stencil)
{
	DWORD clearFlags = 0;
	if (color)clearFlags |= D3DCLEAR_TARGET;
	if (depth)clearFlags |= D3DCLEAR_ZBUFFER;
	if (stencil)clearFlags |= D3DCLEAR_STENCIL;
	DWORD d3dColor = D3DCOLOR_RGBA((int)(m_CurrentClearColor.r*255), (int)(m_CurrentClearColor.g * 255), (int)(m_CurrentClearColor.b * 255), (int)(m_CurrentClearColor.a * 255));
	return  m_pD3DDevice->Clear(0, nullptr, clearFlags, d3dColor, m_CurrentDepth, m_CurrentStencil) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::SetScissorRect(RECT* pRect)
{
	return m_pD3DDevice->SetScissorRect(pRect) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::GetScissorRect(RECT* pRect)
{
	return m_pD3DDevice->GetScissorRect(pRect) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::BeginScene()
{
	return m_pD3DDevice->BeginScene() == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::EndScene()
{
	return  m_pD3DDevice->EndScene() == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::Present()
{
	return m_pD3DDevice->Present(NULL, NULL, NULL, NULL) == S_OK;
}



uint32_t ParaEngine::RenderDeviceD3D9::GetRenderState(const ERenderState& State)
{
	auto d3dRenderState = D3DMapping::toD3DRenderState(State);
	DWORD state = 0;
	if (m_pD3DDevice->GetRenderState(d3dRenderState, &state) == S_OK)
	{
		return (uint32_t)state;
	}
	return 0;
}

bool ParaEngine::RenderDeviceD3D9::SetRenderState(const ERenderState State, const uint32_t Value)
{

	auto rs = D3DMapping::toD3DRenderState(State);
	HRESULT hr = m_pD3DDevice->SetRenderState(rs, D3DMapping::toD3DRenderStateValue(State, Value));
	return hr == D3D_OK;
}


bool ParaEngine::RenderDeviceD3D9::SetClipPlane(uint32_t Index, const float* pPlane)
{
	return m_pD3DDevice->SetClipPlane(Index, pPlane) == S_OK;
}

bool ParaEngine::RenderDeviceD3D9::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat /*= 0*/, uint32_t nDataType /*= 0*/)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return false;
}

int ParaEngine::RenderDeviceD3D9::GetMaxSimultaneousTextures()
{
	return 8;
}

std::shared_ptr<IParaEngine::IEffect> ParaEngine::RenderDeviceD3D9::CreateEffect(const void* pSrcData, uint32_t srcDataLen, IParaEngine::IEffectInclude* include, std::string& error)
{

	std::shared_ptr<D3D9ShaderInclude> d3d9Include = nullptr;


	LPD3DXEFFECT pEffect = NULL;
	LPD3DXBUFFER pBufferErrors = NULL;

	LPD3DXINCLUDE d3dInlcude = NULL;
	if (include != nullptr)
	{
		d3d9Include = std::make_shared<D3D9ShaderInclude>(include);
		d3dInlcude = d3d9Include.get();
	}

	HRESULT result = D3DXCreateEffect(
		m_pD3DDevice,
		pSrcData,
		srcDataLen,
		NULL, 
		d3dInlcude,
		NULL, // D3DXSHADER_PREFER_FLOW_CONTROL | D3DXSHADER_OPTIMIZATION_LEVEL2,
		0,
		&pEffect,
		&pBufferErrors);


	if (result != S_OK)
	{
		if (pBufferErrors != nullptr)
		{
		error = (char*)pBufferErrors->GetBufferPointer();
		}
		return nullptr;
	}
	return std::make_shared<EffectD3D9>(pEffect);
}

IParaEngine::ITexture* ParaEngine::RenderDeviceD3D9::CreateTexture(uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage)
{
	return TextureD3D9::Create(this, width, height, format, usage);
}


const ParaEngine::RenderDeviceCaps& ParaEngine::RenderDeviceD3D9::GetCaps()
{
	throw std::logic_error("The method or operation is not implemented.");
}


bool ParaEngine::RenderDeviceD3D9::SetRenderTarget(uint32_t index, IParaEngine::ITexture* target)
{
	LPDIRECT3DSURFACE9 surface = NULL;
	if (target)
	{
		surface = (static_cast<TextureD3D9*>(target))->GetSurface();
	}
	bool ret = m_pD3DDevice->SetRenderTarget(index, surface) == S_OK;
	if (ret)
	{
		m_CurrentRenderTargets[index] = target;
	}
	return ret;
}


bool ParaEngine::RenderDeviceD3D9::SetDepthStencil(IParaEngine::ITexture* target)
{
	LPDIRECT3DSURFACE9 surface = NULL;
	if (target)
	{
		surface = (static_cast<TextureD3D9*>(target))->GetSurface();
	}
	bool ret = m_pD3DDevice->SetDepthStencilSurface(surface) == S_OK;
	if (ret)
	{
		m_CurrentDepthStencil = target;
	}
	return ret;
}


IParaEngine::ITexture* ParaEngine::RenderDeviceD3D9::GetRenderTarget(uint32_t index)
{
	return m_CurrentRenderTargets[index];
}


IParaEngine::ITexture* ParaEngine::RenderDeviceD3D9::GetDepthStencil()
{
	return m_CurrentDepthStencil;
}


IParaEngine::ITexture* ParaEngine::RenderDeviceD3D9::GetBackbufferRenderTarget()
{
	return m_backbufferRenderTarget;
}


IParaEngine::ITexture* ParaEngine::RenderDeviceD3D9::GetBackbufferDepthStencil()
{
	return m_backbufferDepthStencil;
}

void ParaEngine::RenderDeviceD3D9::InitCaps()
{
	D3DCAPS9 caps;
	ZeroMemory(&caps, sizeof(D3DCAPS9));
	HRESULT hr = m_pD3DDevice->GetDeviceCaps(&caps);
	if (hr != S_OK) return;

	if (caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES)
	{
		m_Cpas.DynamicTextures = true;
	}

	if (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
	{
		m_Cpas.NPOT = true;
	}
	if (caps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS)
	{
		m_Cpas.MRT = true;
	}
	if (caps.RasterCaps&D3DPRASTERCAPS_SCISSORTEST)
	{
		m_Cpas.ScissorTest = true;
	}
	if ((caps.StencilCaps&(D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE)) == (D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE))
	{
		m_Cpas.Stencil = true;
	}

	m_Cpas.NumSimultaneousRTs = caps.NumSimultaneousRTs;
	m_Cpas.MaxSimultaneousTextures = caps.MaxSimultaneousTextures;
}

IParaEngine::ITexture* ParaEngine::RenderDeviceD3D9::CreateTexture(const char* buffer, uint32_t size, EPixelFormat format, uint32_t colorKey)
{
	D3DFORMAT d3dFormat = D3DMapping::toD3DFromat(format);

	LPDIRECT3DTEXTURE9 pTexture = nullptr;

	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(m_pD3DDevice, buffer, size,
		D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, d3dFormat,
		D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		colorKey, NULL, NULL, &(pTexture));


	if (hr == S_OK && pTexture)
	{
		return TextureD3D9::Create(this,pTexture);
	}


	return nullptr;
}
