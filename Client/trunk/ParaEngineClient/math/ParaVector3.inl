#pragma once

// inline functions to avoid header file dependency 
namespace ParaEngine
{
	inline Vector2 Vector2::operator * (const Matrix4& mat) const
	{
		return Vector2(
			x*mat[0][0] + y*mat[1][0] + mat[3][0],
			x*mat[0][1] + y*mat[1][1] + mat[3][1]
			);
	}

	inline Vector3 Vector3::operator * (const Matrix4& mat) const 
	{
		return Vector3(
			x*mat[0][0] + y*mat[1][0] + z*mat[2][0] + mat[3][0],
			x*mat[0][1] + y*mat[1][1] + z*mat[2][1] + mat[3][1],
			x*mat[0][2] + y*mat[1][2] + z*mat[2][2] + mat[3][2]
			);
	}
}