#pragma once
#include <d3d9.h>
#include <cassert>
#include <unordered_map>
#include "Framework/RenderSystem/RenderTypes.h"

namespace ParaEngine
{
	_D3DRENDERSTATETYPE toD3DRenderState(const ERenderState& rs);
	ERenderState toRenderState(_D3DRENDERSTATETYPE format);
	D3DFORMAT toD3DFromat(PixelFormat format);
	PixelFormat toPixelFormat(D3DFORMAT format);
	D3DMULTISAMPLE_TYPE toD3DMSAAType(MultiSampleType msaa);
	DWORD toD3DRenderStateValue(const ERenderState& rs, const uint32_t value);
}

