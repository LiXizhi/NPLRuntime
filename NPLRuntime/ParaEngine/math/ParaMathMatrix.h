#pragma once
namespace ParaEngine
{
	class Matrix4;
	class ParaViewport;

	Matrix4* ParaMatrixRotationX(Matrix4* pMat, float Angle);
	Matrix4* ParaMatrixRotationY(Matrix4* pMat, float Angle);
	Matrix4* ParaMatrixRotationZ(Matrix4* pMat, float Angle);
	Matrix4* ParaMatrixScaling(Matrix4* pMat, float ScaleX,		float ScaleY,		float ScaleZ		);
	Matrix4* ParaMatrixRotationNormal(Matrix4* pOut, const Vector3& NormalAxis, float Angle);
	Matrix4* ParaMatrixRotationAxis(Matrix4* pMat, const Vector3& Axis, float Angle);

	/**
	* Multiplies two 4x4 matrices.
	*
	* @param Result	Pointer to where the result should be stored
	* @param Matrix1	Pointer to the first matrix
	* @param Matrix2	Pointer to the second matrix
	*/
	Matrix4* ParaMatrixMultiply(Matrix4* Result, const Matrix4* Matrix1, const Matrix4* Matrix2);
	/** transform matrix. */
	Vector3* ParaVec3TransformCoord(Vector3* pOut, const Vector3* pIn, const Matrix4* Matrix1);
	Vector3* ParaVec3TransformCoordArray(Vector3* out, uint32 outstride, const Vector3* in, uint32 instride, const Matrix4* matrix, uint32 elements);
	
	/** normalize*/
	Vector3* ParaVec3Normalize(Vector3* pOut, const Vector3* pIn);

	/** left handed, clockwise, roll, then pitch, and then yaw. 
	@note: d3dx9 version parameter order is: Yaw Pitch Rol. but the result are the same. 
	*/
	void ParaMatrixRotationRollPitchYaw(Matrix4* Result, float roll, float pitch, float yaw);

	Matrix4* ParaMatrixOrthoOffCenterLH(Matrix4 *pout, float l, float r, float b, float t, float zn, float zf);
	Matrix4* ParaMatrixOrthoOffCenterOpenGL(Matrix4 *pout, float l, float r, float b, float t, float zn, float zf);

	/**
	* LookAtLH creates a View transformation matrix for left-handed coordinate system.
	* The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
	* specifying the up vector.
	*/
	void ParaMatrixLookAtLH(Matrix4* Result, const Vector3* peye, const Vector3* pat, const Vector3* pup);
	void ParaMatrixLookAtLH(Matrix4* Result, const DVector3& peye, const DVector3& pat, const DVector3& pup);
	
	Matrix4* ParaMatrixPerspectiveLH(Matrix4 *pout, float w, float h, float zn, float zf);
	Matrix4* ParaMatrixOrthoLH(Matrix4 *pout, float w, float h, float zn, float zf);

	/** build transform matrix */
	void ParaMatrixTransformation2D(Matrix4* Result, const Vector2* ScalingOrigin, float ScalingOrientation, const Vector2* Scaling, const Vector2* RotationOrigin, float Rotation, const Vector2* Translation);

	/** perspective matrix */
	void ParaMatrixPerspectiveFovLH(Matrix4* Result, float FovAngleY, float AspectHByW, float NearZ, float FarZ);

	/** decompose matrix*/
	bool ParaMatrixDecompose(Vector3 *poutscale, Quaternion *poutrotation, Vector3 *pouttranslation, const Matrix4 *pm);
	Matrix4 *  ParaMatrixAffineTransformation(Matrix4 *out, float scaling, const Vector3 *rotationcenter, const Quaternion*rotation, const Vector3 *translation);
	Vector3* ParaVec3Project(Vector3 *pout, const Vector3 *pv, const ParaViewport *pviewport, const Matrix4 *pprojection, const Matrix4 *pview, const Matrix4 *pworld);
}