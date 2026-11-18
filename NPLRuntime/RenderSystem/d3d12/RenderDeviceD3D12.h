#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"
#include "Framework/Interface/Render/IBlockSkyRayTracer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace ParaEngine
{
	class D3D12Texture;
	class D3D12Buffer;
	class D3D12RenderTarget;
	class D3D12ShaderProgram;

	class D3D12CpuDescriptorAllocator
	{
	public:
		bool Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxDescriptors);
		uint32_t Allocate();
		void Free(uint32_t index);
		D3D12_CPU_DESCRIPTOR_HANDLE GetHandle(uint32_t index) const;
		uint32_t GetDescriptorSize() const { return m_descriptorSize; }

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart = {};
		uint32_t m_descriptorSize = 0;
		uint32_t m_capacity = 0;
		uint32_t m_nextLinear = 0;
		std::vector<uint32_t> m_freeList;
	};

	class RenderDeviceD3D12 : public IRenderDevice, public IBlockSkyRayTracer
	{
	public:
		static constexpr uint32_t FRAME_COUNT = 3;

		RenderDeviceD3D12(
			ID3D12Device* device,
			ID3D12CommandQueue* commandQueue,
			IDXGISwapChain3* swapChain,
			D3D_FEATURE_LEVEL featureLevel,
			bool isWarp,
			const std::string& adapterName);

		virtual ~RenderDeviceD3D12() override;

		bool Initialize(uint32_t initialWidth, uint32_t initialHeight);
		bool ResizeSwapChain(uint32_t width, uint32_t height, bool isWindowed);

		ID3D12Device* GetD3D12Device() const { return m_device.Get(); }
		ID3D12CommandQueue* GetCommandQueue() const { return m_cmdQueue.Get(); }
		ID3D12GraphicsCommandList* EnsureRecording();
		bool SupportsBlockSkyTracing() override;
		bool TraceBlockSky(const std::vector<BlockSkySpan>& spans, IRHITexture* atlas) override;
		bool TraceBlockReflections(const BlockReflectionView& view, IRHITexture* depth, IRHITexture* normal,
			IRHITexture* material, IRHITexture* sceneColor, IRHITexture* output) override;
		void ExecuteAndSyncGPU();
		void DeferRelease(IUnknown* obj);
		void NotifyTextureDestroyed(D3D12Texture* tex);
		void NotifyBufferDestroyed(D3D12Buffer* buf);
		void NotifyProgramDestroyed(D3D12ShaderProgram* prog);
		void NotifyRenderTargetDestroyed(D3D12RenderTarget* rt);

		bool AllocateUpload(
			uint32_t byteSize,
			uint32_t alignment,
			void*& outCpuPtr,
			D3D12_GPU_VIRTUAL_ADDRESS& outGpuVA,
			ID3D12Resource*& outResource,
			uint64_t& outOffset);

		void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES& ioCurrentState, D3D12_RESOURCE_STATES targetState);
		D3D12CpuDescriptorAllocator& GetSrvAllocator() { return m_cpuSrvAlloc; }
		D3D12CpuDescriptorAllocator& GetRtvAllocator() { return m_cpuRtvAlloc; }
		D3D12CpuDescriptorAllocator& GetDsvAllocator() { return m_cpuDsvAlloc; }

		// Occlusion query slot management
		uint32_t AllocateOcclusionQuerySlot();
		void FreeOcclusionQuerySlot(uint32_t slot);
		ID3D12QueryHeap* GetOcclusionQueryHeap() const { return m_occlusionHeap.Get(); }
		ID3D12Resource* GetOcclusionReadbackBuffer() const { return m_occlusionReadback.Get(); }
		const uint64_t* GetOcclusionMappedResults() const { return m_occlusionMappedPtr; }

		// IRenderDevice overrides
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::D3D12; }
		virtual const RHIDeviceCaps& GetCaps() const override { return m_caps; }
		virtual int GetStencilBits() override { return 8; }
		virtual uint32_t GetBackBufferWidth() const override { return m_backBufferWidth; }
		virtual uint32_t GetBackBufferHeight() const override { return m_backBufferHeight; }
		virtual bool CheckDeviceError(const char* filename = nullptr, const char* func = nullptr, int nLine = 0) override;

		// Resource creation (ParaRHI v2)
		virtual IRHITexture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels = 1, ERHITextureUsage usage = ERHITextureUsage::ShaderResource, const RHIMipData* initialMips = nullptr, uint32_t initialMipCount = 0) override;
		virtual IRHIBuffer* CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage = ERHIBufferUsage::Static, const void* initialData = nullptr) override;
		virtual IRHIRenderTarget* CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat = PixelFormat::A8R8G8B8, PixelFormat depthFormat = PixelFormat::D24S8, uint32_t colorTargetCount = 1, uint32_t msaaSamples = 0) override;
		virtual IRHIOcclusionQuery* CreateOcclusionQuery() override;

		// Render targets & MRT (ParaRHI v2)
		virtual bool SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr) override;
		virtual void BeginRenderTarget(uint32_t width, uint32_t height) override;
		virtual void EndRenderTarget() override;
		virtual bool BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect = nullptr, const RECT* dstRect = nullptr, bool linearFilter = true) override;
		virtual bool ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height) override;

		// Hardware instancing & depth picking (ParaRHI v2)
		virtual bool SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider) override;
		virtual bool DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount) override;
		virtual bool ReadDepthPixel(int x, int y, float& outDepth) override;
		virtual bool ReadTexturePixels(IRHITexture* texture, void* pDataOut) override;

		// Shader program compilation & constant upload (ParaRHI v2)
		virtual IRHIShaderProgram* CompileShaderProgram(const RHIShaderProgramDesc& desc) override;
		virtual bool BindShaderProgram(IRHIShaderProgram* program) override;
		virtual bool SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize) override;

		// Pipeline state & draw commands
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

	private:
		bool CopyRayBuffer(ID3D12Resource* source, IRHITexture* output, UINT pitch);
		bool BindReflectionInputs(IRHITexture* depth, IRHITexture* normal, IRHITexture* material, IRHITexture* sceneColor);
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_reflectionRoot;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_reflectionPipeline;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_blockBLAS, m_blockTLAS, m_reflectionBuffer;
		D3D12_RESOURCE_STATES m_reflectionBufferState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		bool m_checkedSkyTracing = false;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_skyRoot;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_skyPipeline;
		void InitCaps();
		bool CreateRootSignatureAndHeaps();
		bool CreateBackBufferViews(uint32_t width, uint32_t height);
		void ReleaseBackBufferViews();
		void WaitForFenceValue(uint64_t fenceValue);
		void ReclaimCompletedFrameResources(uint32_t frameSlot);
		void BindActiveRenderTargetsToCmdList();
		bool PrepareForDraw(D3D12_PRIMITIVE_TOPOLOGY topo, D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType);
		ID3D12PipelineState* GetOrCreatePSO(D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType);
		uint32_t GetOrCreateCpuSampler(uint32_t stage);

	private:
		struct StreamBinding
		{
			D3D12Buffer* buffer = nullptr;
			uint32_t offsetInBytes = 0;
			uint32_t stride = 0;
			uint32_t frequencyDivider = 0;
		};

		struct UploadPage
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> resource;
			uint8_t* cpuBase = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS gpuBase = 0;
			uint64_t capacity = 0;
			uint64_t offset = 0;
		};

		struct DeferredReleaseEntry
		{
			uint64_t fenceValue = 0;
			Microsoft::WRL::ComPtr<IUnknown> object;
		};

		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		D3D_FEATURE_LEVEL m_featureLevel;
		bool m_isWarp = false;
		std::string m_adapterName;
		RHIDeviceCaps m_caps;
		RHIStateTracker m_stateTracker;

		// Command recording & synchronization
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocators[FRAME_COUNT];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_cmdList;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent = nullptr;
		uint64_t m_nextFenceValue = 1;
		uint64_t m_frameFenceValues[FRAME_COUNT] = {};
		uint32_t m_frameIndex = 0;
		uint32_t m_backBufferIndex = 0;
		bool m_isRecording = false;
		bool m_frameResetDone[FRAME_COUNT] = {};
		std::vector<UploadPage> m_uploadPages[FRAME_COUNT];
		std::vector<DeferredReleaseEntry> m_deferredReleases;

		// Root signature & descriptor heaps
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		D3D12CpuDescriptorAllocator m_cpuRtvAlloc;
		D3D12CpuDescriptorAllocator m_cpuDsvAlloc;
		D3D12CpuDescriptorAllocator m_cpuSrvAlloc;
		D3D12CpuDescriptorAllocator m_cpuSamplerAlloc;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_gpuSrvHeap;
		uint32_t m_gpuSrvDescriptorSize = 0;
		uint32_t m_gpuSrvCapacity = 65536;
		uint32_t m_gpuSrvFrameBase[FRAME_COUNT] = {};
		uint32_t m_gpuSrvOffset = 0;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_gpuSamplerHeap;
		uint32_t m_gpuSamplerDescriptorSize = 0;
		uint32_t m_gpuSamplerCapacity = 2048;
		uint32_t m_gpuSamplerOffset = 0;
		std::unordered_map<uint64_t, uint32_t> m_gpuSamplerTableCache; // 16-sampler combo hash -> base slot in m_gpuSamplerHeap
		std::unordered_map<uint64_t, uint32_t> m_cpuSamplerCache;      // single sampler hash -> slot in m_cpuSamplerAlloc

		// Default resources
		D3D12Texture* m_whiteFallbackTex = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_zeroCbvBuffer;
		D3D12_GPU_VIRTUAL_ADDRESS m_zeroCbvGpuVA = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_defaultAttrVB;
		D3D12_VERTEX_BUFFER_VIEW m_defaultAttrVBV = {};

		// Swapchain backbuffers + default depth-stencil
		Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[FRAME_COUNT];
		D3D12_RESOURCE_STATES m_backBufferStates[FRAME_COUNT] = {};
		uint32_t m_backBufferRtvSlots[FRAME_COUNT] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };

		Microsoft::WRL::ComPtr<ID3D12Resource> m_defaultDepthTex;
		D3D12_RESOURCE_STATES m_defaultDepthState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		uint32_t m_defaultDsvSlot = UINT32_MAX;

		uint32_t m_backBufferWidth = 0;
		uint32_t m_backBufferHeight = 0;
		uint32_t m_renderTargetWidth = 0;
		uint32_t m_renderTargetHeight = 0;
		std::stack<std::pair<uint32_t, uint32_t>> m_rtSizeStack;

		// Active render targets
		uint32_t m_nullRtvSlot = UINT32_MAX;
		uint32_t m_activeNullRtvMask = 0;
		bool m_usingBackBufferRTV = true;
		D3D12Texture* m_activeColorTexObjs[4] = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_activeRTVs[4] = {};
		DXGI_FORMAT m_activeRtvFormats[4] = { DXGI_FORMAT_R8G8B8A8_UNORM };
		uint32_t m_activeRTVCount = 1;
		uint32_t m_activeSampleCount = 1;

		bool m_hasActiveDSV = true;
		bool m_usingDefaultDSV = true;
		D3D12Texture* m_activeDepthTexObj = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_activeDSV = {};
		DXGI_FORMAT m_activeDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// Bound shader, constants, textures, vertex/index streams
		D3D12ShaderProgram* m_currentProgram = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS m_currentVsCbvGpuVA = 0;
		D3D12_GPU_VIRTUAL_ADDRESS m_currentPsCbvGpuVA = 0;
		std::vector<uint8_t> m_shadowVsCbv;
		std::vector<uint8_t> m_shadowPsCbv;
		bool m_cbvDirtyAfterReset = false;
		D3D12Texture* m_boundTextures[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		bool m_srvTableDirty = true;
		D3D12_GPU_DESCRIPTOR_HANDLE m_lastGpuSrvTable = {};

		StreamBinding m_streams[4] = {};
		D3D12Buffer* m_currentIndexBuffer = nullptr;
		CVertexDeclaration* m_currentVertexDecl = nullptr;
		std::unordered_map<uint32_t, CVertexDeclaration*> m_fvfDecls;

		// Cached PSOs
		std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoCache;

		// BlitRenderTarget quad pipeline
		D3D12ShaderProgram* m_blitProgram = nullptr;
		CVertexDeclaration* m_blitVertexDecl = nullptr;

		// Occlusion query heap & readback
		static constexpr uint32_t MAX_OCCLUSION_QUERIES = 256;
		Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_occlusionHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_occlusionReadback;
		uint64_t* m_occlusionMappedPtr = nullptr;
		uint32_t m_nextOcclusionSlot = 0;
		std::vector<uint32_t> m_freeOcclusionSlots;

		ParaViewport m_currentViewport = {};
		bool m_vsyncEnabled = false;
		bool m_isShuttingDown = false;
	};
}
