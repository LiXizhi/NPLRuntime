#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"
#include "VulkanLoader.h"

#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace ParaEngine
{
	class VulkanTexture;
	class VulkanBuffer;
	class VulkanRenderTarget;
	class VulkanShaderProgram;

	struct VulkanDeferredDelete
	{
		uint64_t retireSerial = 0;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkShaderModule shaderModule = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		uint64_t viewUniqueId = 0;
	};

	class RenderDeviceVulkan : public IRenderDevice
	{
	public:
		static constexpr uint32_t FRAME_COUNT = 3;

		RenderDeviceVulkan(
			const VulkanDispatch& vk,
			VkInstance instance,
			VkSurfaceKHR surface,
			VkPhysicalDevice physicalDevice,
			VkDevice device,
			VkQueue graphicsQueue,
			uint32_t graphicsQueueFamily,
			const VkPhysicalDeviceProperties& physProps,
			const VkPhysicalDeviceFeatures& enabledFeatures,
			const VkPhysicalDeviceMemoryProperties& memProps,
			const std::string& adapterName);

		virtual ~RenderDeviceVulkan() override;

		bool Initialize(uint32_t initialWidth, uint32_t initialHeight);
		bool ResizeSwapChain(uint32_t width, uint32_t height, bool isWindowed);

		const VulkanDispatch& GetVK() const { return m_vk; }
		VkDevice GetVkDevice() const { return m_device; }
		VkPhysicalDevice GetVkPhysicalDevice() const { return m_physicalDevice; }
		uint32_t GetGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
		uint64_t NextUniqueViewId() { return ++m_nextViewUniqueId; }

		uint32_t FindMemoryType(uint32_t typeBits, VkFlags requiredProps) const;
		bool CreateVkBufferWithMemory(
			VkDeviceSize byteSize,
			VkFlags usage,
			VkFlags memProps,
			VkBuffer& outBuffer,
			VkDeviceMemory& outMemory) const;
		bool CreateVkImageWithMemory(
			const VkImageCreateInfo& imgInfo,
			VkFlags memProps,
			VkImage& outImage,
			VkDeviceMemory& outMemory) const;

		VkCommandBuffer EnsureRecording();
		void EndActiveRenderPass();
		bool EnsureActiveRenderPass();
		void ExecuteAndSyncGPU();
		void DeferDelete(const VulkanDeferredDelete& entry);
		void NotifyViewDestroyed(uint64_t viewUniqueId);

		bool AllocateUpload(
			uint32_t byteSize,
			uint32_t alignment,
			void*& outCpuPtr,
			VkBuffer& outBuffer,
			VkDeviceSize& outOffset);

		void TransitionImageLayout(
			VkCommandBuffer cmd,
			VkImage image,
			VkFlags aspectMask,
			uint32_t mipLevels,
			uint32_t layerCount,
			VkImageLayout& ioCurrentLayout,
			VkImageLayout targetLayout);

		// Occlusion query helpers
		uint32_t AllocateOcclusionQuerySlot();
		void FreeOcclusionQuerySlot(uint32_t slot);
		VkQueryPool GetOcclusionQueryPool() const { return m_occlusionPool; }

		// IRenderDevice overrides
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::Vulkan; }
		virtual const RHIDeviceCaps& GetCaps() const override { return m_caps; }
		virtual int GetStencilBits() override { return m_defaultDepthHasStencil ? 8 : 0; }
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
		void InitCaps();
		bool CreateDescriptorLayoutsAndDefaults();
		bool CreateSwapchainAndDefaultDepth(uint32_t width, uint32_t height);
		void DestroySwapchainAndDefaultDepth();
		bool EnsureSwapchainImageAcquired();
		void BeginFrameSlotIfNeeded(uint32_t frameSlot);
		void ReclaimCompletedResources(uint64_t completedSerial);
		VkDescriptorPool CreateNewDescriptorPoolChunk();
		VkDescriptorSet AllocateFrameDescriptorSet();
		VkSampler GetOrCreateSampler(uint32_t stage);
		VkRenderPass GetOrCreateRenderPass(
			uint32_t colorCount,
			const VkFormat* colorFormats,
			bool hasDepth,
			VkFormat depthFormat,
			uint32_t sampleCount);
		VkFramebuffer GetOrCreateFramebuffer(
			VkRenderPass renderPass,
			uint32_t attachmentCount,
			const VkImageView* views,
			const uint64_t* viewUniqueIds,
			uint32_t width,
			uint32_t height);
		VkPipeline GetOrCreateGraphicsPipeline(VkPrimitiveTopology vkTopo, VkRenderPass renderPass);
		bool PrepareForDraw(VkPrimitiveTopology vkTopo);
		void ApplyDynamicViewportAndScissor(VkCommandBuffer cmd);

	private:
		struct StreamBinding
		{
			VulkanBuffer* buffer = nullptr;
			uint32_t offsetInBytes = 0;
			uint32_t stride = 0;
			uint32_t frequencyDivider = 0;
		};

		struct UploadPage
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			uint8_t* cpuBase = nullptr;
			VkDeviceSize capacity = 0;
			VkDeviceSize offset = 0;
		};

		struct FrameContext
		{
			VkCommandPool cmdPool = VK_NULL_HANDLE;
			VkCommandBuffer cmdBuf = VK_NULL_HANDLE;
			VkFence inFlightFence = VK_NULL_HANDLE;
			VkSemaphore imageAvailableSem = VK_NULL_HANDLE;
			uint64_t submitSerial = 0;
			bool resetDone = false;
			std::vector<VkDescriptorPool> descriptorPools;
			uint32_t activePoolIdx = 0;
			std::vector<UploadPage> uploadPages;
		};

		struct FramebufferEntry
		{
			VkFramebuffer framebuffer = VK_NULL_HANDLE;
			uint64_t viewIds[5] = {};
			uint32_t viewCount = 0;
		};

		VulkanDispatch m_vk;
		VkInstance m_instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		uint32_t m_graphicsQueueFamily = 0;
		VkPhysicalDeviceProperties m_physProps = {};
		VkPhysicalDeviceFeatures m_enabledFeatures = {};
		VkPhysicalDeviceMemoryProperties m_memProps = {};
		std::string m_adapterName;
		RHIDeviceCaps m_caps;
		RHIStateTracker m_stateTracker;
		bool m_supportsUbyte4Uscaled = false;

		// Frame synchronization & command recording
		FrameContext m_frames[FRAME_COUNT];
		uint32_t m_frameIndex = 0;
		uint64_t m_nextSubmitSerial = 1;
		uint64_t m_completedSubmitSerial = 0;
		bool m_isRecording = false;
		bool m_inRenderPass = false;
		VkRenderPass m_activeVkRenderPass = VK_NULL_HANDLE;
		std::vector<VulkanDeferredDelete> m_deferredDeletes;
		uint64_t m_nextViewUniqueId = 1;

		// Swapchain + default depth-stencil
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		VkFormat m_swapFormat = VK_FORMAT_B8G8R8A8_UNORM;
		VkColorSpaceKHR m_swapColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		std::vector<VkImage> m_swapImages;
		std::vector<VkImageView> m_swapImageViews;
		std::vector<uint64_t> m_swapViewIds;
		std::vector<VkImageLayout> m_swapImageLayouts;
		std::vector<VkSemaphore> m_presentSemaphores;
		uint32_t m_acquiredImageIndex = 0;
		bool m_hasAcquiredSwapImage = false;
		bool m_waitAcquireSemPending = false;

		VkImage m_defaultDepthImage = VK_NULL_HANDLE;
		VkDeviceMemory m_defaultDepthMemory = VK_NULL_HANDLE;
		VkImageView m_defaultDepthView = VK_NULL_HANDLE;
		uint64_t m_defaultDepthViewId = 0;
		VkFormat m_defaultDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		VkImageLayout m_defaultDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		bool m_defaultDepthHasStencil = true;

		uint32_t m_backBufferWidth = 0;
		uint32_t m_backBufferHeight = 0;
		uint32_t m_renderTargetWidth = 0;
		uint32_t m_renderTargetHeight = 0;
		std::stack<std::pair<uint32_t, uint32_t>> m_rtSizeStack;

		// Active render target state
		bool m_usingBackBufferRT = true;
		VulkanTexture* m_activeColorTexObjs[4] = {};
		VkFormat m_activeColorFormats[4] = {};
		uint32_t m_activeColorCount = 1;
		uint32_t m_activeSampleCount = 1;

		bool m_hasActiveDepth = true;
		bool m_usingDefaultDepth = true;
		VulkanTexture* m_activeDepthTexObj = nullptr;
		VkFormat m_activeDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

		// Descriptor layout, pipeline layout, caches, fallback resources
		VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		std::unordered_map<uint64_t, VkSampler> m_samplerCache;
		std::unordered_map<uint64_t, VkRenderPass> m_renderPassCache;
		std::unordered_map<uint64_t, FramebufferEntry> m_framebufferCache;
		std::unordered_map<uint64_t, VkPipeline> m_pipelineCache;

		VulkanTexture* m_whiteFallback2D = nullptr;
		VkImage m_whiteCubeImage = VK_NULL_HANDLE;
		VkDeviceMemory m_whiteCubeMem = VK_NULL_HANDLE;
		VkImageView m_whiteCubeView = VK_NULL_HANDLE;
		VkImage m_white3DImage = VK_NULL_HANDLE;
		VkDeviceMemory m_white3DMem = VK_NULL_HANDLE;
		VkImageView m_white3DView = VK_NULL_HANDLE;

		VkBuffer m_zeroUboBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_zeroUboMemory = VK_NULL_HANDLE;
		VkBuffer m_defaultAttrVB = VK_NULL_HANDLE;
		VkDeviceMemory m_defaultAttrMemory = VK_NULL_HANDLE;

		// Current shader, UBO slices, textures, streams
		VulkanShaderProgram* m_currentProgram = nullptr;
		VkBuffer m_currentVsUboBuf = VK_NULL_HANDLE;
		VkDeviceSize m_currentVsUboOffset = 0;
		VkDeviceSize m_currentVsUboRange = 16;
		VkBuffer m_currentPsUboBuf = VK_NULL_HANDLE;
		VkDeviceSize m_currentPsUboOffset = 0;
		VkDeviceSize m_currentPsUboRange = 16;
		VulkanTexture* m_boundTextures[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		bool m_descriptorsDirty = true;
		VkDescriptorSet m_lastDescriptorSet = VK_NULL_HANDLE;

		StreamBinding m_streams[4] = {};
		VulkanBuffer* m_currentIndexBuffer = nullptr;
		CVertexDeclaration* m_currentVertexDecl = nullptr;
		std::unordered_map<uint32_t, CVertexDeclaration*> m_fvfDecls;

		// Occlusion query pool
		static constexpr uint32_t MAX_OCCLUSION_QUERIES = 256;
		VkQueryPool m_occlusionPool = VK_NULL_HANDLE;
		uint32_t m_nextOcclusionSlot = 0;
		std::vector<uint32_t> m_freeOcclusionSlots;
		std::vector<uint32_t> m_pendingQueryResets;

		ParaViewport m_currentViewport = {};
		bool m_vsyncEnabled = false;
	};
}
