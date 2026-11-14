#pragma once
#include "RenderDevice.h"
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"

namespace ParaEngine
{
	class RenderDeviceNull : public IRenderDevice
	{
	public:
		RenderDeviceNull();
		virtual ~RenderDeviceNull() override = default;

		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::Null; }
		virtual const RHIDeviceCaps& GetCaps() const override { return m_caps; }
		virtual int GetStencilBits() override { return 8; }
		virtual uint32_t GetBackBufferWidth() const override { return 1280; }
		virtual uint32_t GetBackBufferHeight() const override { return 720; }

		virtual IRHITexture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels = 1, ERHITextureUsage usage = ERHITextureUsage::ShaderResource, const RHIMipData* initialMips = nullptr, uint32_t initialMipCount = 0) override;
		virtual IRHIBuffer* CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage = ERHIBufferUsage::Static, const void* initialData = nullptr) override;
		virtual IRHIRenderTarget* CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat = PixelFormat::A8R8G8B8, PixelFormat depthFormat = PixelFormat::D24S8, uint32_t colorTargetCount = 1, uint32_t msaaSamples = 0) override;
		virtual IRHIOcclusionQuery* CreateOcclusionQuery() override;

		virtual bool SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr) override;
		virtual bool BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect = nullptr, const RECT* dstRect = nullptr, bool linearFilter = true) override;
		virtual bool SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider) override;
		virtual bool DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount) override;
		virtual bool ReadDepthPixel(int x, int y, float& outDepth) override;

		virtual IRHIShaderProgram* CompileShaderProgram(const RHIShaderProgramDesc& desc) override;
		virtual bool BindShaderProgram(IRHIShaderProgram* program) override;
		virtual bool SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize) override;

		virtual uint32_t GetRenderState(const ERenderState& State) override;
		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) override;
		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) override;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) override;
		virtual int GetMaxSimultaneousTextures() override;
		virtual bool SetTexture(uint32_t stage, DeviceTexturePtr_type texture) override;
		virtual bool DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount) override;
		virtual bool DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount) override;
		virtual bool DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) override;
		virtual bool DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) override;
		virtual bool SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix) override;
		virtual bool SetFVF(uint32_t FVF) override;
		virtual void SetCursorPosition(int X, int Y, uint32_t Flags) override;
		virtual bool SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value) override;
		virtual bool GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value) override;
		virtual bool SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration) override;
		virtual bool CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl) override;
		virtual bool SetIndices(IndexBufferDevicePtr_type pIndexData) override;
		virtual bool SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride) override;
		virtual ParaViewport GetViewport() override;
		virtual bool SetViewport(const ParaViewport& viewport) override;
		virtual bool SetClearColor(const Color4f& color) override;
		virtual bool SetClearDepth(const float depth) override;
		virtual bool SetClearStencil(const int stencil) override;
		virtual bool Clear(bool color, bool depth, bool stencil) override;
		virtual bool SetScissorRect(RECT* pRect) override;
		virtual bool GetScissorRect(RECT* pRect) override;
		virtual bool BeginScene() override;
		virtual bool EndScene() override;
		virtual bool Present() override;
		virtual void Flush() override;

	protected:
		RHIDeviceCaps m_caps;
		RHIStateTracker m_stateTracker;
	};
}
