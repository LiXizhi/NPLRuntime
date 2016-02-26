#pragma once
namespace ParaEngine
{
	//------------------------------------------------------------------------------
	// Replicate the x component of the vector
	Vector4 ParaVectorSplatX(const Vector4& V);

	//------------------------------------------------------------------------------
	// Replicate the y component of the vector
	Vector4 ParaVectorSplatY(const Vector4& V);

	//------------------------------------------------------------------------------
	// Replicate the z component of the vector
	Vector4 ParaVectorSplatZ(const Vector4& V);


	Vector4 ParaVectorSwizzle(const Vector4& V, uint32 E0, uint32 E1, uint32 E2, uint32 E3);
	Vector3 ParaVector3Normalize(const Vector3& v);

	bool ParaComputeBoundingBox(const Vector3 * pfirstposition, uint32 numvertices,	uint32 dwstride,Vector3 *pmin,Vector3 *pmax);
}

