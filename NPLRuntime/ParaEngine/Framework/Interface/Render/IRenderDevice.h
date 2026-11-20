#pragma once

#include <memory>
#include <vector>
#include <stdint.h>
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Interface/Render/RHITypes.h"
#include "Framework/Common/Math/Rect.h"
#include "Framework/Common/Math/Color4f.h"
#include "Core/PEtypes.h"
#include "math/ParaViewport.h"

#if defined(DEBUG) 
	#define PE_CHECK_GL_ERROR_DEBUG() IRenderDevice::CheckRenderError(__FILE__, __FUNCTION__, __LINE__);
#else
	#define PE_CHECK_GL_ERROR_DEBUG() ((void)0)
#endif

namespace ParaEngine
{
	struct VertexElement;
	class CVertexDeclaration;
	typedef CVertexDeclaration* VertexDeclarationPtr;
	typedef IRHITexture* DeviceTexturePtr_type;
	typedef IRHIBuffer* VertexBufferDevicePtr_type;
	typedef IRHIBuffer* IndexBufferDevicePtr_type;
}


namespace ParaEngine
{
	class CVertexDeclaration;
	class ParaViewport;
	class IRenderContext;

	struct RHIRenderTargetBinding
	{
		uint32_t count = 0;
		IRHIRenderTarget* targets[4] = { nullptr, nullptr, nullptr, nullptr };
		IRHIRenderTarget* depthStencilOverride = nullptr;
	};

	class IRenderDevice
	{
	public:
		IRenderDevice() = default;
		virtual ~IRenderDevice() = default;

		virtual ERHIBackend GetBackendType() const { return ERHIBackend::Null; }
		virtual const RHIDeviceCaps& GetCaps() const
		{
			static const RHIDeviceCaps s_defaultCaps{};
			return s_defaultCaps;
		}
		virtual int GetStencilBits() { return 8; }
		virtual uint32_t GetBackBufferWidth() const { return 0; }
		virtual uint32_t GetBackBufferHeight() const { return 0; }
		virtual bool CheckDeviceError(const char* filename = nullptr, const char* func = nullptr, int nLine = 0) { return true; }

		// Resource creation (ParaRHI v2)
		virtual IRHITexture* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels = 1, ERHITextureUsage usage = ERHITextureUsage::ShaderResource, const RHIMipData* initialMips = nullptr, uint32_t initialMipCount = 0) { return nullptr; }
		virtual IRHIBuffer* CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage = ERHIBufferUsage::Static, const void* initialData = nullptr) { return nullptr; }
		virtual IRHIRenderTarget* CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat = PixelFormat::A8R8G8B8, PixelFormat depthFormat = PixelFormat::D24S8, uint32_t colorTargetCount = 1, uint32_t msaaSamples = 0) { return nullptr; }
		virtual IRHIOcclusionQuery* CreateOcclusionQuery() { return nullptr; }

		// Render targets & MRT (ParaRHI v2)
		virtual bool SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr) { NotifyRenderTargetsBound(count, targets, depthStencilOverride); return false; }
		virtual void BeginRenderTarget(uint32_t width, uint32_t height) {}
		void BeginRenderTarget(IRHIRenderTarget* rt)
		{
			if (rt)
				BeginRenderTarget(rt->GetWidth(), rt->GetHeight());
		}
		virtual void EndRenderTarget() {}
		virtual bool BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect = nullptr, const RECT* dstRect = nullptr, bool linearFilter = true) { return false; }
		virtual bool ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height) { return false; }

		bool SetRenderTarget(uint32_t index, IRHIRenderTarget* target)
		{
			if (index >= 4)
				return false;
			if (index > 0 && m_currentRTBinding.targets[index] == target)
				return true;
			IRHIRenderTarget* newTargets[4] = {
				m_currentRTBinding.targets[0],
				m_currentRTBinding.targets[1],
				m_currentRTBinding.targets[2],
				m_currentRTBinding.targets[3]
			};
			newTargets[index] = target;
			uint32_t newCount = 0;
			for (uint32_t i = 0; i < 4; ++i)
			{
				if (newTargets[i] != nullptr)
					newCount = i + 1;
			}
			return SetRenderTargets(newCount, newCount > 0 ? newTargets : nullptr, m_currentRTBinding.depthStencilOverride);
		}

		void NotifyRenderTargetsBound(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr)
		{
			RHIRenderTargetBinding binding;
			if (targets && count > 0)
			{
				bool anyValid = false;
				uint32_t clamped = count > 4u ? 4u : count;
				for (uint32_t i = 0; i < clamped; ++i)
				{
					binding.targets[i] = targets[i];
					if (targets[i] != nullptr)
						anyValid = true;
				}
				if (anyValid)
					binding.count = clamped;
			}
			binding.depthStencilOverride = depthStencilOverride;
			m_currentRTBinding = binding;
		}

		bool PushRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride = nullptr)
		{
			m_rtBindingStack.push_back(m_currentRTBinding);
			return SetRenderTargets(count, targets, depthStencilOverride);
		}

		bool PushRenderTarget(IRHIRenderTarget* rt, IRHIRenderTarget* depthStencilOverride = nullptr)
		{
			if (!rt)
				return PushRenderTargets(0, nullptr, depthStencilOverride);
			return PushRenderTargets(1, &rt, depthStencilOverride);
		}

		bool PopRenderTarget()
		{
			if (m_rtBindingStack.empty())
				return SetRenderTargets(0, nullptr, nullptr);
			RHIRenderTargetBinding prev = m_rtBindingStack.back();
			m_rtBindingStack.pop_back();
			return SetRenderTargets(prev.count, prev.count > 0 ? prev.targets : nullptr, prev.depthStencilOverride);
		}

		const RHIRenderTargetBinding& GetCurrentRenderTargetBinding() const { return m_currentRTBinding; }

		// Hardware instancing & depth picking (ParaRHI v2)
		virtual bool SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider) { return false; }
		virtual bool DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount) { return false; }
		virtual bool ReadDepthPixel(int x, int y, float& outDepth) { outDepth = 1.0f; return false; }
		virtual bool ReadTexturePixels(IRHITexture* texture, void* pDataOut) { (void)texture; (void)pDataOut; return false; }

		// Shader program compilation & constant upload (ParaRHI v2)
		virtual IRHIShaderProgram* CompileShaderProgram(const RHIShaderProgramDesc& desc) { return nullptr; }
		virtual bool BindShaderProgram(IRHIShaderProgram* program) { return false; }
		virtual bool SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize) { return false; }

		virtual uint32_t GetRenderState(const ERenderState& State) = 0;
		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) = 0;
		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) = 0;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) = 0;
		virtual int GetMaxSimultaneousTextures() = 0;
		virtual bool SetTexture(uint32_t stage, DeviceTexturePtr_type texture) = 0;
		virtual bool DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount) = 0;
		virtual bool DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount) = 0;
		virtual bool DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) = 0;
		virtual bool DrawIndexedPrimitiveUP(
			EPrimitiveType PrimitiveType,
			uint32_t MinVertexIndex,
			uint32_t NumVertices,
			uint32_t PrimitiveCount,
			const void * pIndexData,
			PixelFormat IndexDataFormat,
			const void* pVertexStreamZeroData,
			uint32_t VertexStreamZeroStride) = 0;

		virtual bool SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix) = 0;
		virtual bool SetFVF(uint32_t FVF) = 0;
		virtual void SetCursorPosition(int X, int Y, uint32_t Flags) = 0;

		virtual bool SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value) = 0;
		virtual bool GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value) = 0;

		virtual bool SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration) = 0;
		virtual bool CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl) = 0;
		virtual bool SetIndices(IndexBufferDevicePtr_type pIndexData) = 0;
		virtual bool SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride) = 0;
		virtual ParaViewport GetViewport() = 0;
		virtual bool SetViewport(const ParaViewport& viewport) = 0;

		virtual bool SetClearColor(const Color4f& color) = 0;
		virtual bool SetClearDepth(const float depth) = 0;
		virtual bool SetClearStencil(const int stencil) = 0;

		virtual bool Clear(bool color,bool depth,bool stencil) = 0;

		virtual bool SetScissorRect(RECT* pRect) = 0;
		virtual bool GetScissorRect(RECT* pRect) = 0;


		virtual bool BeginScene() = 0;
		virtual bool EndScene() = 0;

		virtual bool Present() = 0;
		virtual void Flush() = 0;
		// Only called on profiling transitions. Backends with fixed presentation modes keep them.
		virtual void SetProfilingPresentation(bool enabled) {}


		/** check render error and print to log. only call this in debug mode, since it breaks parallelism between gpu and cpu. */
		static bool CheckRenderError(const char* filename = NULL, const char* func = NULL, int nLine = 0);

	protected:
		RHIRenderTargetBinding m_currentRTBinding;
		std::vector<RHIRenderTargetBinding> m_rtBindingStack;
	};

	using IRenderDevicePtr = std::shared_ptr<IRenderDevice>;
}

