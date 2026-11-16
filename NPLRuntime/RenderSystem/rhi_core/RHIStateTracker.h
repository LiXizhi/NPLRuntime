#pragma once
#include <stdint.h>
#include <string.h>
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Interface/Render/RHITypes.h"
#include "Framework/Common/Math/Rect.h"
#include "Framework/Common/Math/Color4f.h"
#include "Core/PEtypes.h"
#include "math/ParaViewport.h"

namespace ParaEngine
{
	/**
	 * Backend-agnostic render state, sampler state, clip plane, viewport, and PSO hash tracker
	 * shared across all ParaRHI backends (Null, OpenGL, D3D11, D3D12, Vulkan, Metal, WebGPU).
	 */
	class RHIStateTracker
	{
	public:
		static constexpr uint32_t MAX_SAMPLER_STAGES = 16;
		static constexpr uint32_t MAX_SAMPLER_STATES = 16;
		static constexpr uint32_t MAX_CLIP_PLANES    = 6;

		RHIStateTracker();

		void ResetToDefaults();

		/** Returns true if the state value actually changed. */
		bool SetRenderState(ERenderState state, uint32_t value);
		uint32_t GetRenderState(ERenderState state) const;

		/** Returns true if the sampler state value actually changed. */
		bool SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value);
		uint32_t GetSamplerState(uint32_t stage, ESamplerStateType type) const;

		void SetClipPlane(uint32_t index, const float plane[4]);
		const float* GetClipPlane(uint32_t index) const;

		void SetViewport(const ParaViewport& vp) { m_viewport = vp; }
		const ParaViewport& GetViewport() const { return m_viewport; }

		void SetScissorRect(const RECT* pRect);
		const RECT& GetScissorRect() const { return m_scissorRect; }

		void SetClearColor(const Color4f& c) { m_clearColor = c; }
		const Color4f& GetClearColor() const { return m_clearColor; }

		void SetClearDepth(float d) { m_clearDepth = d; }
		float GetClearDepth() const { return m_clearDepth; }

		void SetClearStencil(int s) { m_clearStencil = s; }
		int GetClearStencil() const { return m_clearStencil; }

		bool IsBlendDirty() const { return m_dirtyBlend; }
		bool IsRasterizerDirty() const { return m_dirtyRasterizer; }
		bool IsDepthStencilDirty() const { return m_dirtyDepthStencil; }
		uint32_t GetDirtySamplerMask() const { return m_dirtySamplersMask; }

		void ClearBlendDirty() { m_dirtyBlend = false; }
		void ClearRasterizerDirty() { m_dirtyRasterizer = false; }
		void ClearDepthStencilDirty() { m_dirtyDepthStencil = false; }
		void ClearSamplerDirty(uint32_t stage) { if (stage < MAX_SAMPLER_STAGES) m_dirtySamplersMask &= ~(1u << stage); }
		void ClearAllDirty();
		void MarkAllDirty()
		{
			m_dirtyBlend = true;
			m_dirtyRasterizer = true;
			m_dirtyDepthStencil = true;
			m_dirtySamplersMask = (1u << MAX_SAMPLER_STAGES) - 1u;
		}

		/**
		 * Computes a deterministic 64-bit FNV-1a pipeline state key for D3D12 / Vulkan / Metal / WebGPU PSO caches.
		 */
		uint64_t ComputePSOHash(uint64_t shaderHash, uint64_t vertexDeclHash, EPrimitiveType topology, uint64_t renderTargetFormatHash = 0) const;

	private:
		uint32_t m_renderStates[static_cast<size_t>(ERenderState::COUNT)];
		uint32_t m_samplerStates[MAX_SAMPLER_STAGES][MAX_SAMPLER_STATES];
		float m_clipPlanes[MAX_CLIP_PLANES][4];
		ParaViewport m_viewport;
		RECT m_scissorRect;
		Color4f m_clearColor;
		float m_clearDepth;
		int m_clearStencil;

		bool m_dirtyBlend;
		bool m_dirtyRasterizer;
		bool m_dirtyDepthStencil;
		uint32_t m_dirtySamplersMask;

		mutable uint64_t m_cachedRenderStatePsoHash = 0;
		mutable bool m_dirtyPsoStateHash = true;
	};
}
