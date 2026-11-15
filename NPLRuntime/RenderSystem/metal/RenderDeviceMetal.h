#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ParaEngine
{
	class MetalTexture;
	class MetalBuffer;
	class MetalRenderTarget;
	class MetalShaderProgram;

	struct MetalDeferredRelease
	{
		uint64_t retireSerial = 0;
		void* objcObject = nullptr; // CFBridgingRetain / manual retain pointer released on GPU completion
	};

	class RenderDeviceMetal : public IRenderDevice
	{
	public:
		static constexpr uint32_t FRAME_COUNT = 3;
		static constexpr uint32_t MAX_VERTEX_STREAMS = 4;
		static constexpr uint32_t FALLBACK_VB_SLOT = 15;
		static constexpr uint32_t VS_UBO_SLOT = 16;
		static constexpr uint32_t PS_UBO_SLOT = 17;
		static constexpr uint32_t MAX_OCCLUSION_QUERIES = 512;

#ifdef __OBJC__
		RenderDeviceMetal(id<MTLDevice> device, id<MTLCommandQueue> queue, CAMetalLayer* layer);
#else
		RenderDeviceMetal(void* device, void* queue, void* layer);
#endif
		virtual ~RenderDeviceMetal() override;

		bool Initialize(uint32_t width, uint32_t height, bool vsync);
		bool ResizeSwapChain(uint32_t width, uint32_t height);

		void DeferRelease(void* retainedObjcPtr);
		void ExecuteAndSyncGPU();
		bool AllocateUpload(uint32_t byteSize, uint32_t alignment, void*& outCpuPtr, void*& outMtlBuffer, uint32_t& outOffset);

		uint32_t AllocateOcclusionSlot();
		void FreeOcclusionSlot(uint32_t slot);
		uint64_t ReadOcclusionSlotValue(uint32_t slot) const;
		void BeginOcclusionOnActiveEncoder(uint32_t slot);
		void EndOcclusionOnActiveEncoder();

		// IRenderDevice overrides
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::Metal; }
		virtual const RHIDeviceCaps& GetCaps() const override { return m_caps; }
		virtual int GetStencilBits() override { return 8; }
		virtual uint32_t GetBackBufferWidth() const override { return m_backBufferWidth; }
		virtual uint32_t GetBackBufferHeight() const override { return m_backBufferHeight; }
		virtual bool CheckDeviceError(const char* filename = nullptr, const char* func = nullptr, int nLine = 0) override;

		virtual IRHITexture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels = 1, ERHITextureUsage usage = ERHITextureUsage::ShaderResource, const RHIMipData* initialMips = nullptr, uint32_t initialMipCount = 0) override;
		virtual IRHIBuffer* CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage = ERHIBufferUsage::Static, const void* initialData = nullptr) override;
		virtual IRHIRenderTarget* CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat = PixelFormat::A8R8G8B8, PixelFormat depthFormat = PixelFormat::D24S8, uint32_t colorTargetCount = 1, uint32_t msaaSamples = 0) override;
		virtual IRHIOcclusionQuery* CreateOcclusionQuery() override;

		virtual bool SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr) override;
		virtual void BeginRenderTarget(uint32_t width, uint32_t height) override;
		virtual void EndRenderTarget() override;
		virtual bool BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect = nullptr, const RECT* dstRect = nullptr, bool linearFilter = true) override;
		virtual bool ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height) override;

		virtual bool SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider) override;
		virtual bool DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount) override;
		virtual bool ReadDepthPixel(int x, int y, float& outDepth) override;
		virtual bool ReadTexturePixels(IRHITexture* texture, void* pDataOut) override;

		virtual IRHIShaderProgram* CompileShaderProgram(const RHIShaderProgramDesc& desc) override;
		virtual bool BindShaderProgram(IRHIShaderProgram* program) override;
		virtual bool SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize) override;

		virtual uint32_t GetRenderState(const ERenderState& State) override;
		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) override;
		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) override;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) override;
		virtual int GetMaxSimultaneousTextures() override { return 16; }
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

	private:
		void InitCaps();
		bool CreateDefaultResources(uint32_t width, uint32_t height);
		void EndActiveRenderPass();
		bool EnsureCommandBuffer();
		bool EnsureDrawableAcquired();
		bool EnsureRenderPassEncoder(bool clearColor, bool clearDepth, bool clearStencil);
		void* GetOrCreatePipelineState(EPrimitiveType primitiveType);
		void* GetOrCreateDepthStencilState();
		void* GetOrCreateSamplerState(uint32_t stage);
		bool PrepareForDraw(EPrimitiveType primitiveType);
		void AcquireFrameSlotIfNeeded();
		void ReclaimCompletedResources(uint64_t completedSerial);

	private:
		struct StreamBinding
		{
			MetalBuffer* buffer = nullptr;
			uint32_t offsetInBytes = 0;
			uint32_t stride = 0;
			uint32_t frequencyDivider = 0;
		};

		struct UploadPage
		{
			void* mtlBuffer = nullptr;
			uint8_t* cpuBase = nullptr;
			uint32_t capacity = 0;
			uint32_t offset = 0;
		};

		struct FrameSlot
		{
			std::vector<UploadPage> uploadPages;
			uint64_t submitSerial = 0;
		};

		void* m_device = nullptr;            // id<MTLDevice>
		void* m_commandQueue = nullptr;      // id<MTLCommandQueue>
		void* m_metalLayer = nullptr;        // CAMetalLayer*
		void* m_frameSemaphore = nullptr;    // dispatch_semaphore_t
		void* m_activeCmdBuf = nullptr;      // id<MTLCommandBuffer>
		void* m_activeEncoder = nullptr;     // id<MTLRenderCommandEncoder>
		void* m_currentDrawable = nullptr;   // id<CAMetalDrawable>
		void* m_defaultDepthTex = nullptr;   // id<MTLTexture>
		void* m_fallbackVertexBuf = nullptr; // id<MTLBuffer> [0,0,0,1]
		void* m_occlusionBuf = nullptr;      // id<MTLBuffer>
		void* m_importedColorTex = nullptr;  // external id<MTLTexture>

		MetalTexture* m_defaultWhiteTex = nullptr;
		MetalTexture* m_defaultCubeTex = nullptr;

		std::string m_adapterName;
		RHIDeviceCaps m_caps;
		RHIStateTracker m_state;

		uint32_t m_backBufferWidth = 1280;
		uint32_t m_backBufferHeight = 720;
		uint32_t m_activeRTWidth = 1280;
		uint32_t m_activeRTHeight = 720;
		bool m_usingBackBufferRT = true;
		bool m_passClearedSinceTargetBind = false;

		MetalTexture* m_activeColorTargets[4] = {};
		uint32_t m_activeColorTargetCount = 0;
		MetalTexture* m_activeDepthTarget = nullptr;

		FrameSlot m_frames[FRAME_COUNT];
		uint32_t m_frameIndex = 0;
		bool m_frameSlotAcquired = false;
		uint64_t m_nextSubmitSerial = 1;
		uint64_t m_completedSerial = 0;
		std::vector<MetalDeferredRelease> m_deferredReleases;
		std::vector<uint32_t> m_freeOcclusionSlots;

		MetalShaderProgram* m_currentProgram = nullptr;
		VertexDeclarationPtr m_currentVertexDecl = nullptr;
		MetalBuffer* m_currentIndexBuffer = nullptr;
		StreamBinding m_streams[MAX_VERTEX_STREAMS];
		MetalTexture* m_boundTextures[16] = {};

		std::vector<uint8_t> m_vsConstShadow;
		std::vector<uint8_t> m_psConstShadow;
		uint32_t m_vsConstSize = 0;
		uint32_t m_psConstSize = 0;
		bool m_vsConstDirty = false;
		bool m_psConstDirty = false;

		std::unordered_map<uint64_t, void*> m_psoCache;
		std::unordered_map<uint64_t, void*> m_depthStencilCache;
		std::unordered_map<uint64_t, void*> m_samplerCache;

		void* m_blitLibrary = nullptr;        // id<MTLLibrary>
		void* m_blitSamplerNearest = nullptr; // id<MTLSamplerState>
		void* m_blitSamplerLinear = nullptr;  // id<MTLSamplerState>
		std::unordered_map<uint64_t, void*> m_blitPsoCache;
	};
}
