#pragma once

#include <cassert>
#include <unordered_map>
#include "Framework/RenderSystem/RenderTypes.h"

#include <d3d9.h>

namespace ParaEngine
{
	class D3DMapping
	{
	public:
		static D3DRENDERSTATETYPE toD3DRenderState(const ERenderState& rs);
		static ERenderState toRenderState(D3DRENDERSTATETYPE format);
		static D3DFORMAT toD3DFromat(PixelFormat format);
		static D3DMULTISAMPLE_TYPE toD3DMSAAType(MultiSampleType msaa);
		static DWORD toD3DRenderStateValue(const ERenderState& rs, const uint32_t value);
		static D3DPOOL toD3DPool(const EPoolType& pool);
		static D3DSAMPLERSTATETYPE toD3DSamplerSatetType(const ESamplerStateType type);
		static D3DPRIMITIVETYPE toD3DPrimitiveType(const EPrimitiveType type);
	};
}
