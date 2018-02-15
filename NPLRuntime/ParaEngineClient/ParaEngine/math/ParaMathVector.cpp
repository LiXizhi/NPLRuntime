//-----------------------------------------------------------------------------
// Class:	
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8.18
// Desc: cross platform. In future this file should support SIMD (SSE2)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaMathVector.h"

using namespace ParaEngine;


ParaEngine::Vector3 ParaEngine::ParaVector3Normalize(const Vector3& v)
{
	return v.normalisedCopy();
}

ParaEngine::Vector4 ParaEngine::ParaVectorSplatZ(const Vector4& V)
{
	Vector4 vResult;
	vResult.x =
		vResult.vector4_f32[1] =
		vResult.vector4_f32[2] =
		vResult.vector4_f32[3] = V.vector4_f32[2];
	return vResult;
}

ParaEngine::Vector4 ParaEngine::ParaVectorSplatX(const Vector4& V)
{
	Vector4 vResult;
	vResult.vector4_f32[0] =
		vResult.vector4_f32[1] =
		vResult.vector4_f32[2] =
		vResult.vector4_f32[3] = V.vector4_f32[0];
	return vResult;
}

ParaEngine::Vector4 ParaEngine::ParaVectorSplatY(const Vector4& V)
{
	Vector4 vResult;
	vResult.vector4_f32[0] =
		vResult.vector4_f32[1] =
		vResult.vector4_f32[2] =
		vResult.vector4_f32[3] = V.vector4_f32[1];
	return vResult;
}

ParaEngine::Vector4 ParaEngine::ParaVectorSwizzle(const Vector4& V, uint32 E0, uint32 E1, uint32 E2, uint32 E3)
{
	Vector4 Result = { V.vector4_f32[E0],
		V.vector4_f32[E1],
		V.vector4_f32[E2],
		V.vector4_f32[E3] };
	return Result;
}

bool ParaEngine::ParaComputeBoundingBox(const Vector3 * pfirstposition, uint32 numvertices, uint32 dwstride, Vector3 *pmin, Vector3 *pmax)
{
	Vector3 vec;

	if (!pfirstposition || !pmin || !pmax) return false;

	*pmin = *pfirstposition;
	*pmax = *pmin;

	for (uint32 i = 0; i < numvertices; i++)
	{
		vec = *((const Vector3*)((const char*)pfirstposition + dwstride * i));

		if (vec.x < pmin->x) pmin->x = vec.x;
		if (vec.x > pmax->x) pmax->x = vec.x;

		if (vec.y < pmin->y) pmin->y = vec.y;
		if (vec.y > pmax->y) pmax->y = vec.y;

		if (vec.z < pmin->z) pmin->z = vec.z;
		if (vec.z > pmax->z) pmax->z = vec.z;
	}

	return true;
}
