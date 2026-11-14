#include "RHIStateTracker.h"

namespace ParaEngine
{
	namespace
	{
		inline void HashU32(uint64_t& h, uint32_t v)
		{
			h ^= static_cast<uint64_t>(v);
			h *= 1099511628211ULL;
		}
		inline void HashU64(uint64_t& h, uint64_t v)
		{
			h ^= v;
			h *= 1099511628211ULL;
		}
	}

	RHIStateTracker::RHIStateTracker()
	{
		ResetToDefaults();
	}

	void RHIStateTracker::ResetToDefaults()
	{
		memset(m_renderStates, 0, sizeof(m_renderStates));
		memset(m_samplerStates, 0, sizeof(m_samplerStates));
		memset(m_clipPlanes, 0, sizeof(m_clipPlanes));

		m_renderStates[static_cast<size_t>(ERenderState::ZENABLE)]          = 1;
		m_renderStates[static_cast<size_t>(ERenderState::ZWRITEENABLE)]     = 1;
		m_renderStates[static_cast<size_t>(ERenderState::ZFUNC)]            = RSV_CMP_LESSEQUAL;
		m_renderStates[static_cast<size_t>(ERenderState::CULLMODE)]         = RSV_CULL_CCW;
		m_renderStates[static_cast<size_t>(ERenderState::ALPHABLENDENABLE)] = 0;
		m_renderStates[static_cast<size_t>(ERenderState::SRCBLEND)]         = RSV_BLEND_SRCALPHA;
		m_renderStates[static_cast<size_t>(ERenderState::DESTBLEND)]        = RSV_BLEND_INVSRCALPHA;
		m_renderStates[static_cast<size_t>(ERenderState::BLENDOP)]          = 1; // ADD
		m_renderStates[static_cast<size_t>(ERenderState::SRCBLENDALPHA)]    = RSV_BLEND_SRCALPHA;
		m_renderStates[static_cast<size_t>(ERenderState::DESTBLENDALPHA)]   = RSV_BLEND_INVSRCALPHA;
		m_renderStates[static_cast<size_t>(ERenderState::BLENDOPALPHA)]     = 1; // ADD
		m_renderStates[static_cast<size_t>(ERenderState::ALPHAFUNC)]        = RSV_CMP_ALWAYS;
		m_renderStates[static_cast<size_t>(ERenderState::COLORWRITEENABLE)] = 0x0F;
		m_renderStates[static_cast<size_t>(ERenderState::COLORWRITEENABLE1)]= 0x0F;
		m_renderStates[static_cast<size_t>(ERenderState::COLORWRITEENABLE2)]= 0x0F;
		m_renderStates[static_cast<size_t>(ERenderState::COLORWRITEENABLE3)]= 0x0F;
		m_renderStates[static_cast<size_t>(ERenderState::STENCILFUNC)]      = RSV_CMP_ALWAYS;
		m_renderStates[static_cast<size_t>(ERenderState::STENCILFAIL)]      = RSV_STENCILOP_KEEP;
		m_renderStates[static_cast<size_t>(ERenderState::STENCILZFAIL)]     = RSV_STENCILOP_KEEP;
		m_renderStates[static_cast<size_t>(ERenderState::STENCILPASS)]      = RSV_STENCILOP_KEEP;
		m_renderStates[static_cast<size_t>(ERenderState::STENCILMASK)]      = 0xFFFFFFFFu;
		m_renderStates[static_cast<size_t>(ERenderState::STENCILWRITEMASK)] = 0xFFFFFFFFu;
		m_renderStates[static_cast<size_t>(ERenderState::CCW_STENCILFUNC)]  = RSV_CMP_ALWAYS;
		m_renderStates[static_cast<size_t>(ERenderState::CCW_STENCILFAIL)]  = RSV_STENCILOP_KEEP;
		m_renderStates[static_cast<size_t>(ERenderState::CCW_STENCILZFAIL)] = RSV_STENCILOP_KEEP;
		m_renderStates[static_cast<size_t>(ERenderState::CCW_STENCILPASS)]  = RSV_STENCILOP_KEEP;

		for (uint32_t s = 0; s < MAX_SAMPLER_STAGES; ++s)
		{
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::ADDRESSU)] = 1; // WRAP
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::ADDRESSV)] = 1; // WRAP
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::ADDRESSW)] = 1; // WRAP
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::MAGFILTER)] = 2; // LINEAR
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::MINFILTER)] = 2; // LINEAR
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::MIPFILTER)] = 2; // LINEAR
			m_samplerStates[s][static_cast<size_t>(ESamplerStateType::MAXANISOTROPY)] = 1;
		}

		m_viewport = ParaViewport();
		m_scissorRect = { 0, 0, 0, 0 };
		m_clearColor = { 0.f, 0.f, 0.f, 1.f };
		m_clearDepth = 1.0f;
		m_clearStencil = 0;

		m_dirtyBlend = true;
		m_dirtyRasterizer = true;
		m_dirtyDepthStencil = true;
		m_dirtySamplersMask = 0xFFFFu;
	}

	bool RHIStateTracker::SetRenderState(ERenderState state, uint32_t value)
	{
		size_t idx = static_cast<size_t>(state);
		if (idx >= static_cast<size_t>(ERenderState::COUNT))
			return false;
		if (m_renderStates[idx] == value)
			return false;

		m_renderStates[idx] = value;

		switch (state)
		{
		case ERenderState::ALPHABLENDENABLE:
		case ERenderState::SEPARATEALPHABLENDENABLE:
		case ERenderState::SRCBLEND:
		case ERenderState::DESTBLEND:
		case ERenderState::BLENDOP:
		case ERenderState::SRCBLENDALPHA:
		case ERenderState::DESTBLENDALPHA:
		case ERenderState::BLENDOPALPHA:
		case ERenderState::COLORWRITEENABLE:
		case ERenderState::COLORWRITEENABLE1:
		case ERenderState::COLORWRITEENABLE2:
		case ERenderState::COLORWRITEENABLE3:
			m_dirtyBlend = true;
			break;

		case ERenderState::CULLMODE:
		case ERenderState::FILLMODE:
		case ERenderState::SCISSORTESTENABLE:
		case ERenderState::DEPTHBIAS:
		case ERenderState::SLOPESCALEDEPTHBIAS:
		case ERenderState::MULTISAMPLEANTIALIAS:
		case ERenderState::CLIPPLANEENABLE:
			m_dirtyRasterizer = true;
			break;

		case ERenderState::ZENABLE:
		case ERenderState::ZWRITEENABLE:
		case ERenderState::ZFUNC:
		case ERenderState::STENCILENABLE:
		case ERenderState::STENCILFUNC:
		case ERenderState::STENCILFAIL:
		case ERenderState::STENCILZFAIL:
		case ERenderState::STENCILPASS:
		case ERenderState::STENCILMASK:
		case ERenderState::STENCILWRITEMASK:
		case ERenderState::TWOSIDEDSTENCILMODE:
		case ERenderState::CCW_STENCILFUNC:
		case ERenderState::CCW_STENCILFAIL:
		case ERenderState::CCW_STENCILZFAIL:
		case ERenderState::CCW_STENCILPASS:
			m_dirtyDepthStencil = true;
			break;

		default:
			break;
		}
		return true;
	}

	uint32_t RHIStateTracker::GetRenderState(ERenderState state) const
	{
		size_t idx = static_cast<size_t>(state);
		return (idx < static_cast<size_t>(ERenderState::COUNT)) ? m_renderStates[idx] : 0;
	}

	bool RHIStateTracker::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		size_t t = static_cast<size_t>(type);
		if (stage >= MAX_SAMPLER_STAGES || t >= MAX_SAMPLER_STATES)
			return false;
		if (m_samplerStates[stage][t] == value)
			return false;

		m_samplerStates[stage][t] = value;
		m_dirtySamplersMask |= (1u << stage);
		return true;
	}

	uint32_t RHIStateTracker::GetSamplerState(uint32_t stage, ESamplerStateType type) const
	{
		size_t t = static_cast<size_t>(type);
		if (stage >= MAX_SAMPLER_STAGES || t >= MAX_SAMPLER_STATES)
			return 0;
		return m_samplerStates[stage][t];
	}

	void RHIStateTracker::SetClipPlane(uint32_t index, const float plane[4])
	{
		if (index < MAX_CLIP_PLANES && plane)
		{
			memcpy(m_clipPlanes[index], plane, sizeof(float) * 4);
		}
	}

	const float* RHIStateTracker::GetClipPlane(uint32_t index) const
	{
		static const float s_zero[4] = { 0, 0, 0, 0 };
		return (index < MAX_CLIP_PLANES) ? m_clipPlanes[index] : s_zero;
	}

	void RHIStateTracker::SetScissorRect(const RECT* pRect)
	{
		if (pRect)
			m_scissorRect = *pRect;
		else
			m_scissorRect = { 0, 0, 0, 0 };
	}

	void RHIStateTracker::ClearAllDirty()
	{
		m_dirtyBlend = false;
		m_dirtyRasterizer = false;
		m_dirtyDepthStencil = false;
		m_dirtySamplersMask = 0;
	}

	uint64_t RHIStateTracker::ComputePSOHash(uint64_t shaderHash, uint64_t vertexDeclHash, EPrimitiveType topology, uint64_t renderTargetFormatHash) const
	{
		uint64_t h = 14695981039346656037ULL;
		HashU64(h, shaderHash);
		HashU64(h, vertexDeclHash);
		HashU64(h, renderTargetFormatHash);
		HashU32(h, static_cast<uint32_t>(topology));

		// Blend state
		HashU32(h, GetRenderState(ERenderState::ALPHABLENDENABLE));
		HashU32(h, GetRenderState(ERenderState::SEPARATEALPHABLENDENABLE));
		HashU32(h, GetRenderState(ERenderState::SRCBLEND));
		HashU32(h, GetRenderState(ERenderState::DESTBLEND));
		HashU32(h, GetRenderState(ERenderState::BLENDOP));
		HashU32(h, GetRenderState(ERenderState::SRCBLENDALPHA));
		HashU32(h, GetRenderState(ERenderState::DESTBLENDALPHA));
		HashU32(h, GetRenderState(ERenderState::BLENDOPALPHA));
		HashU32(h, GetRenderState(ERenderState::COLORWRITEENABLE));

		// Rasterizer state
		HashU32(h, GetRenderState(ERenderState::CULLMODE));
		HashU32(h, GetRenderState(ERenderState::FILLMODE));
		HashU32(h, GetRenderState(ERenderState::SCISSORTESTENABLE));
		HashU32(h, GetRenderState(ERenderState::DEPTHBIAS));
		HashU32(h, GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS));
		HashU32(h, GetRenderState(ERenderState::CLIPPLANEENABLE));

		// Depth-stencil state
		HashU32(h, GetRenderState(ERenderState::ZENABLE));
		HashU32(h, GetRenderState(ERenderState::ZWRITEENABLE));
		HashU32(h, GetRenderState(ERenderState::ZFUNC));
		HashU32(h, GetRenderState(ERenderState::STENCILENABLE));
		if (GetRenderState(ERenderState::STENCILENABLE))
		{
			HashU32(h, GetRenderState(ERenderState::STENCILFUNC));
			HashU32(h, GetRenderState(ERenderState::STENCILFAIL));
			HashU32(h, GetRenderState(ERenderState::STENCILZFAIL));
			HashU32(h, GetRenderState(ERenderState::STENCILPASS));
			HashU32(h, GetRenderState(ERenderState::STENCILMASK));
			HashU32(h, GetRenderState(ERenderState::STENCILWRITEMASK));
			HashU32(h, GetRenderState(ERenderState::TWOSIDEDSTENCILMODE));
			if (GetRenderState(ERenderState::TWOSIDEDSTENCILMODE))
			{
				HashU32(h, GetRenderState(ERenderState::CCW_STENCILFUNC));
				HashU32(h, GetRenderState(ERenderState::CCW_STENCILFAIL));
				HashU32(h, GetRenderState(ERenderState::CCW_STENCILZFAIL));
				HashU32(h, GetRenderState(ERenderState::CCW_STENCILPASS));
			}
		}

		return h;
	}
}
