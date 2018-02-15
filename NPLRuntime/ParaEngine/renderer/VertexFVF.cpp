//-----------------------------------------------------------------------------
// Class:	VertexFVF
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "VertexFVF.h"

namespace ParaEngine
{
	const DWORD UNDERWATER_VERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	const DWORD mesh_vertex_plain::FVF = (D3DFVF_XYZ | D3DFVF_TEX1);
	const DWORD mesh_vertex_normal::FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	const DWORD mesh_vertex_normal_color::FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
	const DWORD mesh_vertex_normal_tex2::FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2);
	const DWORD bmax_vertex::FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);

	const DWORD terrain_vertex_normal::FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2);
	const DWORD terrain_vertex::FVF = (D3DFVF_XYZ | D3DFVF_TEX2);
	const DWORD SPRITEVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	const DWORD SHADOWVERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	const DWORD LINEVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	const DWORD DXUT_SCREEN_VERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	const DWORD OCCLUSION_VERTEX::FVF = D3DFVF_XYZ;
	const DWORD block_vertex::FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR);
}