#pragma once

#ifdef USE_DIRECTX_RENDERER
namespace ParaEngine
{
	typedef D3DVERTEXELEMENT9 VertexElement;
}
#else
#include "VertexDeclarationOpenGL.h"
#endif

