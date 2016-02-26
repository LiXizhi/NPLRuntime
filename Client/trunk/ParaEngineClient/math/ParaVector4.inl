#pragma once

namespace ParaEngine
{
	inline Vector4::operator Vector3() const
	{
		return Vector3(x, y, z);
	}

	inline ParaEngine::Vector4 Vector4::transformAffine(const Matrix4& mat) const
	{
		// PE_ASSERT(M.isAffine());
		return Vector4(
			x*mat[0][0] + y*mat[1][0] + z*mat[2][0] + w*mat[3][0],
			x*mat[0][1] + y*mat[1][1] + z*mat[2][1] + w*mat[3][1],
			x*mat[0][2] + y*mat[1][2] + z*mat[2][2] + w*mat[3][2],
			w);
	}

	inline Vector4 Vector4::operator * (const Matrix4& mat) const
	{
		return Vector4(
			x*mat[0][0] + y*mat[1][0] + z*mat[2][0] + w*mat[3][0],
			x*mat[0][1] + y*mat[1][1] + z*mat[2][1] + w*mat[3][1],
			x*mat[0][2] + y*mat[1][2] + z*mat[2][2] + w*mat[3][2],
			x*mat[0][3] + y*mat[1][3] + z*mat[2][3] + w*mat[3][3]
			);
	}
}