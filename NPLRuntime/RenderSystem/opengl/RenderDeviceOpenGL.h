#pragma once
#include <string>
#include "RenderDevice.h"
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"
#include "OpenGL.h"

namespace ParaEngine
{
	class GLProgram;

	class RenderDeviceOpenGL : public IRenderDevice
	{
	public:
		RenderDeviceOpenGL();
		virtual ~RenderDeviceOpenGL() override;

		void NotifyRenderTargetDestroyed(IRHIRenderTarget* rt);

		// ParaRHI v2 identity & capabilities
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::OpenGL; }
		virtual const RHIDeviceCaps& GetCaps() const override;
		virtual int GetStencilBits() override;
		virtual uint32_t GetBackBufferWidth() const override;
		virtual uint32_t GetBackBufferHeight() const override;
		virtual bool CheckDeviceError(const char* filename = nullptr, const char* func = nullptr, int nLine = 0) override;

		// ParaRHI v2 resource creation
		virtual IRHITexture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels = 1, ERHITextureUsage usage = ERHITextureUsage::ShaderResource, const RHIMipData* initialMips = nullptr, uint32_t initialMipCount = 0) override;
		virtual IRHIBuffer* CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage = ERHIBufferUsage::Static, const void* initialData = nullptr) override;
		virtual IRHIRenderTarget* CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat = PixelFormat::A8R8G8B8, PixelFormat depthFormat = PixelFormat::D24S8, uint32_t colorTargetCount = 1, uint32_t msaaSamples = 0) override;
		virtual IRHIOcclusionQuery* CreateOcclusionQuery() override;

		// ParaRHI v2 render targets & MRT
		virtual bool SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr) override;
		virtual void BeginRenderTarget(uint32_t width, uint32_t height) override;
		virtual void EndRenderTarget() override;
		virtual bool BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect = nullptr, const RECT* dstRect = nullptr, bool linearFilter = true) override;
		virtual bool ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height) override;

		// ParaRHI v2 hardware instancing & depth picking
		virtual bool SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider) override;
		virtual bool DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount) override;
		virtual bool ReadDepthPixel(int x, int y, float& outDepth) override;
		virtual bool ReadTexturePixels(IRHITexture* texture, void* pDataOut) override;

		// Core IRenderDevice state & draw interface
		virtual uint32_t GetRenderState(const ERenderState& State) override;
		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) override;
		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) override;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) override;
		virtual int GetMaxSimultaneousTextures() override;
		virtual bool SetTexture(uint32_t stage, DeviceTexturePtr_type texture) override;
		virtual bool DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount) override;
		virtual bool DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) override;
		virtual bool SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value) override;
		virtual bool SetVertexDeclaration(CVertexDeclaration* pVertexDeclaration) override;
		virtual bool CreateVertexDeclaration(VertexElement* pVertexElements, CVertexDeclaration** ppDecl) override;
		virtual bool SetIndices(IndexBufferDevicePtr_type pIndexData) override;
		virtual bool SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride) override;
		virtual bool BeginScene() override;
		virtual bool EndScene() override;
		virtual bool DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount) override;
		virtual bool DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) override;
		virtual bool SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix) override;
		virtual bool SetFVF(uint32_t FVF) override;
		virtual void SetCursorPosition(int X, int Y, uint32_t Flags) override;
		virtual bool GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value) override;
		virtual ParaViewport GetViewport() override;
		virtual bool SetViewport(const ParaViewport& viewport) override;
		virtual bool Clear(bool color, bool depth, bool stencil) override;
		virtual bool SetScissorRect(RECT* pRect) override;
		virtual bool GetScissorRect(RECT* pRect) override;
		virtual bool Present() override = 0;
		virtual bool SetClearColor(const Color4f& color) override;
		virtual bool SetClearDepth(const float depth) override;
		virtual bool SetClearStencil(const int stencil) override;
		virtual void Flush() override;

		void InitCaps() const;

		bool SupportsFloatColorBuffer() const { if (!m_CapsInitialized) InitCaps(); return m_SupportsFloatColorBuffer; }
		bool SupportsHalfFloatColorBuffer() const { if (!m_CapsInitialized) InitCaps(); return m_SupportsHalfFloatColorBuffer; }
		bool SupportsFloatLinearFilter() const { if (!m_CapsInitialized) InitCaps(); return m_SupportsFloatLinearFilter; }

	protected:
		void ApplyBlendingModeChange();
		void ApplyStencilStateChange();
		void ApplyDrawBuffersForCurrentProgram();
		void ApplyGLVertexDeclaration(CVertexDeclaration* pDecl, const void* streamZeroUserPtr, uint32_t streamZeroStrideOverride = 0);

	private:
		struct StreamBinding
		{
			IRHIBuffer* buffer = nullptr;
			uint32_t offsetInBytes = 0;
			uint32_t stride = 0;
			uint32_t frequencyDivider = 0;
		};

		mutable RHIDeviceCaps m_Caps;
		mutable std::string m_AdapterName;
		mutable bool m_CapsInitialized = false;
		mutable bool m_SupportsFloatColorBuffer = false;
		mutable bool m_SupportsHalfFloatColorBuffer = false;
		mutable bool m_SupportsFloatLinearFilter = false;

		RHIStateTracker m_StateTracker;

		bool m_AlphaBlendingChanged;
		bool m_BlendingChanged;
		bool m_EnableBlending;
		bool m_EnableSeparateAlphaBlending;
		uint32_t m_BlendingSource;
		uint32_t m_BlendingDest;
		uint32_t m_BlendingAlphaSource;
		uint32_t m_BlendingAlphaDest;
		bool m_TwoSidedStencil;
		uint32_t m_StencilRefValue;
		uint32_t m_StencilMask;
		uint32_t m_StencilFunc;
		uint32_t m_StencilFail;
		uint32_t m_StencilZFail;
		uint32_t m_StencilPass;
		uint32_t m_CCWStencilFunc;
		uint32_t m_CCWStencilFail;
		uint32_t m_CCWStencilZFail;
		uint32_t m_CCWStencilPass;
		bool m_StencilDirty;

		CVertexDeclaration* m_CurrentVertexDeclaration;
		StreamBinding m_Streams[4];
		VertexBufferDevicePtr_type m_CurrentVertexBuffer;
		IndexBufferDevicePtr_type m_CurrentIndexBuffer;
		uint32_t m_AttribDivisors[16] = {};

		GLuint m_DefaultFBO = 0;
		uint32_t m_BackBufferWidth = 0;
		uint32_t m_BackBufferHeight = 0;
		uint32_t m_RenderTargetWidth = 0;
		uint32_t m_RenderTargetHeight = 0;
		uint32_t m_LastRenderTargetWidth = 0;
		uint32_t m_LastRenderTargetHeight = 0;
		bool m_isBeginRenderTarget = false;
		bool m_isShuttingDown = false;
		uint32_t m_BoundMrtCount = 0;
		uint8_t m_ValidMrtMask = 0;
		uint8_t m_AppliedDrawBufferMask = 0;
		ParaViewport m_CurrentViewPort;
		GLProgram* m_BlitProgram = nullptr;
		int m_DefaultFboReadBlitSupported = -1;
	};

	inline RenderDeviceOpenGL* GETGL(IRenderDevice* device)
	{
		return static_cast<RenderDeviceOpenGL*>(device);
	}
}
