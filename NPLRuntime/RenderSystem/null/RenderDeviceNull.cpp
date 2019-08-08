#include "RenderDeviceNull.h"
using namespace ParaEngine;

bool ParaEngine::IRenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
{
	return true;
}


uint32_t ParaEngine::RenderDeviceNull::GetRenderState(const ERenderState& State)
{
	return 0;
}

bool ParaEngine::RenderDeviceNull::SetRenderState(const ERenderState State, const uint32_t Value)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClipPlane(uint32_t Index, const float* pPlane)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat /*= 0*/, uint32_t nDataType /*= 0*/)
{
	return true;
}

int ParaEngine::RenderDeviceNull::GetMaxSimultaneousTextures()
{
	return 8;
}

bool ParaEngine::RenderDeviceNull::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void * pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetFVF(uint32_t FVF)
{
	return true;
}

void ParaEngine::RenderDeviceNull::SetCursorPosition(int X, int Y, uint32_t Flags)
{
	
}

bool ParaEngine::RenderDeviceNull::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetIndices(IndexBufferDevicePtr_type pIndexData)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
{
	return true;
}

ParaViewport ParaEngine::RenderDeviceNull::GetViewport()
{
	return m_ViewPort;
}

bool ParaEngine::RenderDeviceNull::SetViewport(const ParaViewport& viewport)
{
	m_ViewPort = viewport;
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClearColor(const Color4f& color)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClearDepth(const float depth)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClearStencil(const int stencil)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::Clear(bool color, bool depth, bool stencil)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetScissorRect(RECT* pRect)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::GetScissorRect(RECT* pRect)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::BeginScene()
{
	return true;
}

bool ParaEngine::RenderDeviceNull::EndScene()
{
	return true;
}

bool ParaEngine::RenderDeviceNull::Present()
{
	return true;
}

void ParaEngine::RenderDeviceNull::Flush()
{

}
