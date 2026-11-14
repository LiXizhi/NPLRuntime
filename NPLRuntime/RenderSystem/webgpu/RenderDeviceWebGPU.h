#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"
#include "WebGPULoader.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ParaEngine
{
	class WebGPUTexture;
	class WebGPUBuffer;
	class WebGPURenderTarget;
	class WebGPUShaderProgram;

	class RenderDeviceWebGPU : public IRenderDevice
	{
	public:
		static constexpr uint32_t MAX_VERTEX_STREAMS = 4;
		static constexpr uint32_t MAX_RENDER_TARGETS = 4;
		static constexpr uint32_t FALLBACK_VB_SLOT = 7; // WebGPU default maxVertexBuffers = 8 (slots 0..7)
		static constexpr uint32_t MAX_OCCLUSION_QUERIES = 512;
		static constexpr uint32_t UBO_MIN_ALIGN = 256;

		RenderDeviceWebGPU(
			const WebGPUDispatch& wgpu,
			WGPUInstance instance,
			WGPUSurface surface,
			WGPUAdapter adapter,
			WGPUDevice device,
			WGPUQueue queue,
			WGPUTextureFormat surfaceFormat,
			bool supportsBC,
			const std::string& adapterName);
		virtual ~RenderDeviceWebGPU() override;

		bool Initialize(uint32_t width, uint32_t height, bool vsync);
		bool ResizeSwapChain(uint32_t width, uint32_t height, bool vsync);

		const WebGPUDispatch& GetWGPU() const { return m_wgpu; }
		WGPUInstance GetWGPUInstance() const { return m_instance; }
		WGPUDevice GetWGPUDevice() const { return m_device; }
		WGPUQueue GetWGPUQueue() const { return m_queue; }
		bool SupportsHardwareBC() const { return m_supportsBC; }
		uint64_t NextUniqueViewId() { return ++m_nextViewUniqueId; }

		void ExecuteAndSyncGPU();
		void EndActiveRenderPass();
		WGPUCommandEncoder EnsureEncoder();
		bool EnsureActiveRenderPass(bool clearColor, bool clearDepth, bool clearStencil);

		uint32_t AllocateOcclusionSlot();
		void FreeOcclusionSlot(uint32_t slot);
		bool ReadOcclusionSlotValue(uint32_t slot, bool flush, uint64_t& outSamples);
		void BeginOcclusionOnActivePass(uint32_t slot);
		void EndOcclusionOnActivePass();

		// IRenderDevice overrides
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::WebGPU; }
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

		bool ReadBufferSync(WGPUBuffer srcBuffer, uint64_t byteSize, void* dstData);
		WGPUBuffer AllocateTransientBuffer(WGPUBufferUsageFlags usage, const void* data, uint32_t byteSize, uint32_t minSize = 16);
		void DeferReleaseBuffer(WGPUBuffer buf) { if (buf) m_transientBuffers.push_back(buf); }

	private:
		void InitCaps();
		bool ConfigureSurface(uint32_t width, uint32_t height, bool vsync);
		bool CreateDefaultResources(uint32_t width, uint32_t height);
		bool EnsureSwapChainTexture();
		WGPURenderPipeline GetOrCreateRenderPipeline(EPrimitiveType primitiveType);
		WGPUSampler GetOrCreateSampler(uint32_t stage);
		WGPUBindGroup BuildDrawBindGroup();
		bool PrepareForDraw(EPrimitiveType primitiveType);
		void FlushFrameTransientBuffers();

	private:
		struct StreamBinding
		{
			WebGPUBuffer* buffer = nullptr;
			uint32_t offsetInBytes = 0;
			uint32_t stride = 0;
			uint32_t frequencyDivider = 0;
		};

		WebGPUDispatch m_wgpu;
		WGPUInstance m_instance = nullptr;
		WGPUSurface m_surface = nullptr;
		WGPUAdapter m_adapter = nullptr;
		WGPUDevice m_device = nullptr;
		WGPUQueue m_queue = nullptr;
		WGPUTextureFormat m_surfaceFormat = WGPUTextureFormat_BGRA8Unorm;
		bool m_supportsBC = false;
		bool m_vsync = true;
		std::string m_adapterNameStr;
		RHIDeviceCaps m_caps;

		uint32_t m_backBufferWidth = 1020;
		uint32_t m_backBufferHeight = 680;
		uint32_t m_activeRTWidth = 1020;
		uint32_t m_activeRTHeight = 680;

		WGPUTexture m_currentSurfaceTex = nullptr;
		WGPUTextureView m_currentSurfaceView = nullptr;
		WebGPUTexture* m_defaultDepthTex = nullptr;
		WebGPUTexture* m_fallbackWhite2D = nullptr;
		WebGPUTexture* m_fallbackWhiteCube = nullptr;
		WebGPUTexture* m_fallbackWhite3D = nullptr;
		WGPUBuffer m_fallbackZeroVB = nullptr;
		WGPUBuffer m_zeroUboBuffer = nullptr;

		WGPUCommandEncoder m_cmdEncoder = nullptr;
		WGPURenderPassEncoder m_renderPass = nullptr;
		bool m_surfaceClearedThisFrame = false;

		uint32_t m_boundRTCount = 0;
		WebGPURenderTarget* m_boundRTs[MAX_RENDER_TARGETS] = {};
		WebGPURenderTarget* m_boundDepthOverride = nullptr;

		RHIStateTracker m_state;
		WebGPUShaderProgram* m_boundProgram = nullptr;
		VertexDeclarationPtr m_vertexDecl = nullptr;
		WebGPUTexture* m_boundTextures[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		StreamBinding m_streams[MAX_VERTEX_STREAMS] = {};
		WebGPUBuffer* m_boundIndexBuffer = nullptr;

		std::vector<uint8_t> m_vsConstantsCpu;
		std::vector<uint8_t> m_psConstantsCpu;
		WGPUBuffer m_currentVsUbo = nullptr;
		uint32_t m_currentVsUboSize = 0;
		WGPUBuffer m_currentPsUbo = nullptr;
		uint32_t m_currentPsUboSize = 0;
		bool m_vsUboDirty = true;
		bool m_psUboDirty = true;

		std::unordered_map<uint64_t, WGPURenderPipeline> m_psoCache;
		std::unordered_map<uint64_t, WGPUSampler> m_samplerCache;
		std::vector<WGPUBuffer> m_transientBuffers;
		std::vector<WGPUBindGroup> m_transientBindGroups;

		WGPUQuerySet m_occlusionQuerySet = nullptr;
		WGPUBuffer m_occlusionResolveBuf = nullptr;
		std::vector<uint32_t> m_freeOcclusionSlots;
		std::vector<uint32_t> m_pendingOcclusionResolveSlots;
		bool m_occlusionActiveInPass = false;

		uint64_t m_nextViewUniqueId = 0;
	};
}
