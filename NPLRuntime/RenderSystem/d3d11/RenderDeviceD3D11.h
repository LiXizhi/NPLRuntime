#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "RHIStateTracker.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <string>
#include <vector>
#include <stack>
#include <unordered_map>

namespace ParaEngine
{
	class D3D11Texture;
	class D3D11Buffer;
	class D3D11ShaderProgram;
	class D3D11RenderTarget;

	class RenderDeviceD3D11 : public IRenderDevice
	{
	public:
		RenderDeviceD3D11(
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			IDXGISwapChain* swapChain,
			D3D_FEATURE_LEVEL featureLevel,
			bool isWarp,
			const std::string& adapterName);

		virtual ~RenderDeviceD3D11() override;

		void NotifyTextureDestroyed(D3D11Texture* tex);
		void NotifyBufferDestroyed(D3D11Buffer* buf);
		void NotifyProgramDestroyed(D3D11ShaderProgram* prog);
		void NotifyRenderTargetDestroyed(D3D11RenderTarget* rt);

		bool ResizeSwapChain(uint32_t width, uint32_t height, bool isWindowed);

		ID3D11Device* GetD3DDevice() const { return m_device.Get(); }
		ID3D11DeviceContext* GetD3DContext() const { return m_context.Get(); }
		IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }
		D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_featureLevel; }

		// IRenderDevice overrides
		virtual ERHIBackend GetBackendType() const override { return ERHIBackend::D3D11; }
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
		void InitCaps();
		void ApplyPipelineStatesBeforeDraw();
		ID3D11InputLayout* GetOrCreateInputLayout(CVertexDeclaration* decl, D3D11ShaderProgram* program);
		ID3D11Buffer* EnsureDynamicScratchBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer>& buf, uint32_t& currentCap, uint32_t requiredBytes, UINT bindFlags);
		void UnbindConflictingSRVs(ID3D11RenderTargetView* const* rtvs, uint32_t rtvCount, ID3D11DepthStencilView* dsv = nullptr);

	private:
		struct StreamBinding
		{
			IRHIBuffer* buffer = nullptr;
			uint32_t offsetInBytes = 0;
			uint32_t stride = 0;
			uint32_t frequencyDivider = 0;
		};

		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		D3D_FEATURE_LEVEL m_featureLevel;
		bool m_isWarp;
		bool m_isShuttingDown = false;
		std::string m_adapterName;
		RHIDeviceCaps m_caps;
		RHIStateTracker m_stateTracker;

		// Default swapchain backbuffer + depth-stencil
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBufferTex;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_defaultRTV;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_defaultDepthTex;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_defaultDSV;
		uint32_t m_backBufferWidth = 0;
		uint32_t m_backBufferHeight = 0;
		uint32_t m_renderTargetWidth = 0;
		uint32_t m_renderTargetHeight = 0;
		std::stack<std::pair<uint32_t, uint32_t>> m_rtSizeStack;

		// Currently bound RTs (for Clear, Blit, ReadPixels, ReadDepthPixel)
		ID3D11RenderTargetView* m_activeRTVs[4] = { nullptr, nullptr, nullptr, nullptr };
		uint32_t m_activeRTVCount = 1;
		ID3D11DepthStencilView* m_activeDSV = nullptr;
		ID3D11Texture2D* m_activeColorTex0 = nullptr;
		ID3D11Texture2D* m_activeDepthTex = nullptr;

		// Logical SRVs per engine sampler stage vs hardware-bound PS/VS t-register SRVs
		ID3D11ShaderResourceView* m_logicalSRVs[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		ID3D11ShaderResourceView* m_boundPSSRVs[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		ID3D11ShaderResourceView* m_boundVSSRVs[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		bool m_srvBindingsDirty = true;

		// Shaders & constant buffers
		D3D11ShaderProgram* m_currentProgram = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsGlobalCB;
		uint32_t m_vsGlobalCBCapacity = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_psGlobalCB;
		uint32_t m_psGlobalCBCapacity = 0;

		// Vertex / Index bindings & UP scratch buffers
		StreamBinding m_streams[4] = {};
		IRHIBuffer* m_currentIndexBuffer = nullptr;
		CVertexDeclaration* m_currentVertexDecl = nullptr;
		std::unordered_map<uint32_t, CVertexDeclaration*> m_fvfDecls;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_upVertexBuffer;
		uint32_t m_upVertexCapacity = 0;
		uint32_t m_upVertexOffset = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_upIndexBuffer;
		uint32_t m_upIndexCapacity = 0;
		uint32_t m_upIndexOffset = 0;

		// Fallback attribute buffer for missing VS inputs ([0, 0, 0, 1])
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_defaultAttrVB;

		// Fullscreen / sub-rect BlitRenderTarget resources
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_blitVS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_blitPS;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_blitInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_blitVB;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_blitLinearSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_blitPointSampler;

		// Cached D3D11 pipeline state objects
		std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D11BlendState>> m_blendStates;
		uint8_t m_lastAppliedPsOutputMask = 0xFFu;
		std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D11RasterizerState>> m_rasterizerStates;
		std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> m_depthStencilStates;
		std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplerStates;
		std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D11InputLayout>> m_inputLayouts;

		ParaViewport m_currentViewport = {};
		bool m_vsyncEnabled = false;
	};
}
