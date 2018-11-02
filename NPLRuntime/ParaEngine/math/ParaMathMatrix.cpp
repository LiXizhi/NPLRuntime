//-----------------------------------------------------------------------------
// Class:	Matrix
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8.18
// Desc: cross platform (try to be compatible with DirectXMath). In future this file should support SIMD (SSE2)
// references: DirectXMath and Wine d3dx
// WINE: http://doxygen.reactos.org/d6/d12/directx_2wine_2d3dx9__36_2mesh_8c_a7282226f0bfa5295204c163bc68264d2.html
//       http://doxygen.reactos.org/de/d57/dll_2directx_2wine_2d3dx9__36_2math_8c_source.html
//		 http://fossies.org/dox/wine-1.7.24/d3dx9__36_2math_8c_source.html#l01244
/** the following functions needs to support sse2
Matrix4::Multiply4x3 : used by ParaXModel
Matrix4::operator* : used heavily for transform
Vector3::operator*(const Matrix4): used by ParaXModel for each vertex 
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaMathMisc.h"
#include "ParaQuaternion.h"
#include "ParaMathVector.h"
#include "ParaMathMatrix.h"

using namespace ParaEngine;

Matrix4* ParaEngine::ParaMatrixRotationY(Matrix4* pMat, float Angle)
{
	float    fSinAngle;
	float    fCosAngle;
	ParaScalarSinCos(&fSinAngle, &fCosAngle, Angle);

	Matrix4& M = *pMat;
	M.m[0][0] = fCosAngle;	M.m[0][1] = 0.0f;	M.m[0][2] = -fSinAngle;	M.m[0][3] = 0.0f;
	M.m[1][0] = 0.0f;	M.m[1][1] = 1.0f;	M.m[1][2] = 0.0f;	M.m[1][3] = 0.0f;
	M.m[2][0] = fSinAngle;	M.m[2][1] = 0.0f;	M.m[2][2] = fCosAngle;	M.m[2][3] = 0.0f;
	M.m[3][0] = 0.0f;	M.m[3][1] = 0.0f;	M.m[3][2] = 0.0f;	M.m[3][3] = 1.0f;
	return pMat;
}

Matrix4* ParaEngine::ParaMatrixRotationNormal(Matrix4* pOut, const Vector3& NormalAxis, float Angle)
{
	Quaternion q;
	q.FromAngleAxis(Radian(Angle), NormalAxis);
	pOut->makeRot(q, Vector3(0, 0, 0));
	return pOut;
}

Matrix4* ParaEngine::ParaMatrixRotationAxis(Matrix4* pMat, const Vector3& Axis, float Angle)
{
	Vector3 Normal = ParaVector3Normalize(Axis);
	return ParaMatrixRotationNormal(pMat, Normal, Angle);
}

// TODO: make this function use SIMD from DirectXMath. 
Matrix4* ParaEngine::ParaMatrixMultiply(Matrix4* Result, const Matrix4* Matrix1, const Matrix4* Matrix2)
{
	Matrix4 r;
	const Matrix4& m1 = *Matrix1;
	const Matrix4& m2 = *Matrix2;
	r.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	r.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	r.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	r.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];
										  
	r.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	r.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	r.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	r.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];
										  
	r.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	r.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	r.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	r.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];
										  
	r.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	r.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	r.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	r.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
	*Result = r;
	return Result;
}

// this function is heavily used by ParaEngine GUI rendering, so it is optimized as much as possible. 
void ParaEngine::ParaMatrixTransformation2D(Matrix4* Result, const Vector2* ScalingOrigin, float ScalingOrientation, const Vector2* Scaling, const Vector2* RotationOrigin, float fRotation, const Vector2* Translation)
{
	// M = Inverse(MScalingOrigin) * Transpose(MScalingOrientation) * MScaling * MScalingOrientation *
	//         MScalingOrigin * Inverse(MRotationOrigin) * MRotation * MRotationOrigin * MTranslation;

	// TODO: ScalingOrientation not supported yet

	Matrix4 r(Matrix4::IDENTITY);
	bool bHasScaling = false;
	if (Scaling && (*Scaling)!=Vector2::UNIT_SCALE)
	{
		bHasScaling = true;
		if (ScalingOrigin && (*ScalingOrigin) != Vector2::ZERO)
		{
			r.setTrans(Vector3(-ScalingOrigin->x, -ScalingOrigin->y, 0.f));
			Matrix4 scaleMat(Matrix4::IDENTITY);
			scaleMat.setScale(Vector3(Scaling->x, Scaling->y, 1.f));
			r *= scaleMat;
			r._41 += ScalingOrigin->x;
			r._42 += ScalingOrigin->y;
		}
		else
		{
			r.setScale(Vector3(Scaling->x, Scaling->y, 1.f));
		}
	}
	if (fRotation != 0)
	{
		if (RotationOrigin)
		{
			if (bHasScaling)
			{
				Matrix4 trans;
				trans.setTrans(Vector3(-RotationOrigin->x, -RotationOrigin->y, 0));
				r *= trans;
			}
			else
			{
				r.setTrans(Vector3(-RotationOrigin->x, -RotationOrigin->y, 0));
			}
			Matrix4 rot;
			rot.makeRot(Quaternion(Radian(fRotation), Vector3(0, 0, 1)), Vector3(0, 0, 0));
			r *= rot;
			r._41 += RotationOrigin->x;
			r._42 += RotationOrigin->y;
		}
		else
		{
			if (bHasScaling)
			{
				Matrix4 rot;
				rot.makeRot(Quaternion(Radian(fRotation), Vector3(0, 0, 1)), Vector3(0, 0, 0));
				r *= rot;
			}
			else
			{
				r.makeRot(Quaternion(Radian(fRotation), Vector3(0, 0, 1)), Vector3(0, 0, 0));
			}
		}
	}
	if (Translation)
	{
		r._41 += Translation->x;
		r._42 += Translation->y;
	}
	*Result = r;
}

Matrix4* ParaEngine::ParaMatrixRotationZ(Matrix4* pMat, float Angle)
{
	float    fSinAngle;
	float    fCosAngle;
	ParaScalarSinCos(&fSinAngle, &fCosAngle, Angle);

	Matrix4& M = *pMat;
	M.m[0][0] = fCosAngle;	M.m[0][1] = fSinAngle;	M.m[0][2] = 0.0f;	M.m[0][3] = 0.0f;
	M.m[1][0] = -fSinAngle;	M.m[1][1] = fCosAngle;	M.m[1][2] = 0.0f;	M.m[1][3] = 0.0f;
	M.m[2][0] = 0.0f;	M.m[2][1] = 0.0f;	M.m[2][2] = 1.0f;	M.m[2][3] = 0.0f;
	M.m[3][0] = 0.0f;	M.m[3][1] = 0.0f;	M.m[3][2] = 0.0f;	M.m[3][3] = 1.0f;
	return pMat;
}

Matrix4* ParaEngine::ParaMatrixRotationX(Matrix4* pMat, float Angle)
{
	float    fSinAngle;
	float    fCosAngle;
	ParaScalarSinCos(&fSinAngle, &fCosAngle, Angle);

	Matrix4& M = *pMat;
	M.m[0][0] = 1.0f;	M.m[0][1] = 0.0f;	M.m[0][2] = 0.0f;	M.m[0][3] = 0.0f;
	M.m[1][0] = 0.0f;	M.m[1][1] = fCosAngle;	M.m[1][2] = fSinAngle;	M.m[1][3] = 0.0f;
	M.m[2][0] = 0.0f;	M.m[2][1] = -fSinAngle;	M.m[2][2] = fCosAngle;	M.m[2][3] = 0.0f;
	M.m[3][0] = 0.0f;	M.m[3][1] = 0.0f;	M.m[3][2] = 0.0f;	M.m[3][3] = 1.0f;
	return pMat;
}

Vector3* ParaEngine::ParaVec3TransformCoord(Vector3* pOut, const Vector3* pv, const Matrix4* pm)
{
	Vector3 out;
	float norm;
	norm = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] * pv->z + pm->m[3][3];
	out.x = (pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0]) / norm;
	out.y = (pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1]) / norm;
	out.z = (pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2]) / norm;
	*pOut = out;
	return pOut;
}

Vector3* ParaEngine::ParaVec3TransformCoordArray(Vector3* out, uint32 outstride, const Vector3* in, uint32 instride, const Matrix4* matrix, uint32 elements)
{
    for (uint32 i = 0; i < elements; ++i) {
        ParaVec3TransformCoord(
            (Vector3*)((char*)out + outstride * i),
            (const Vector3*)((const char*)in + instride * i),
            matrix);
    }
    return out;
}

void ParaEngine::ParaMatrixLookAtLH(Matrix4* Result, const Vector3* peye, const Vector3* pat, const Vector3* pup)
{
	const Vector3& eye = *peye, at = *pat, up = *pup;
	Vector3 z = (at - eye).normalisedCopy();  // Forward
	Vector3 x = up.crossProduct(z).normalisedCopy(); // Right
	Vector3 y = z.crossProduct(x);

	Matrix4 m(  x.x, x.y, x.z, -(x.dotProduct(eye)),
				y.x, y.y, y.z, -(y.dotProduct(eye)),
				z.x, z.y, z.z, -(z.dotProduct(eye)),
				0, 0, 0, 1);
	*Result = m.transpose();
}

void ParaEngine::ParaMatrixLookAtLH(Matrix4* Result, const DVector3& eye, const DVector3& at, const DVector3& up)
{
	DVector3 z = (at - eye).normalisedCopy();  // Forward
	DVector3 x = up.crossProduct(z).normalisedCopy(); // Right
	DVector3 y = z.crossProduct(x);

	Matrix4 m((float)x.x, (float)x.y, (float)x.z, (float)(-(x.dotProduct(eye))),
		(float)y.x, (float)y.y, (float)y.z, (float)(-(y.dotProduct(eye))),
		(float)z.x, (float)z.y, (float)z.z, (float)(-(z.dotProduct(eye))),
		0, 0, 0, 1);
	*Result = m.transpose();
}

Vector3* ParaEngine::ParaVec3Normalize(Vector3* pOut, const Vector3* pIn)
{
	*pOut = pIn->normalisedCopy();
	return pOut;
}

void ParaEngine::ParaMatrixRotationRollPitchYaw(Matrix4* Result, float roll, float pitch, float yaw)
{
	Quaternion qRoll(Radian(roll), Vector3::UNIT_Z);
	Quaternion qPitch(Radian(pitch), Vector3::UNIT_X);
	Quaternion qYaw(Radian(yaw), Vector3::UNIT_Y);
	Quaternion q = qRoll*qPitch*qYaw;
	q.ToRotationMatrix(*Result, Vector3::ZERO);
}

void ParaEngine::ParaMatrixPerspectiveFovLH(Matrix4* pout, float fovy, float aspect, float zn, float zf)
{
	*pout = Matrix4::IDENTITY;
#ifdef USE_DIRECTX_RENDERER
	// directX, z axis is in range 0,1
	pout->m[0][0] = 1.0f / (aspect * tanf(fovy / 2.0f));
	pout->m[1][1] = 1.0f / tanf(fovy / 2.0f);
	pout->m[2][2] = zf / (zf - zn);
	pout->m[2][3] = 1.0f;
	pout->m[3][2] = (zf * zn) / (zn - zf);
	pout->m[3][3] = 0.0f;
#elif defined(USE_OPENGL_RENDERER)
	// opengl, z axis is in range -1,1
	/*float factor = 1.0f / tanf(fovy / 2.0f);
	float f_n = 1.0f / (zf - zn);
	pout->m[0][0] = (1.0f / aspect) * factor;
	pout->m[1][1] = factor;
	pout->m[2][2] = (-(zf + zn)) * f_n;
	pout->m[2][3] = -1.0f;
	pout->m[3][2] = -2.0f * zf * zn * f_n;
	pout->m[3][3] = 0.0f;*/

	pout->m[0][0] = 1.0f / (aspect * tanf(fovy / 2.0f));
	pout->m[1][1] = 1.0f / tanf(fovy / 2.0f);
	pout->m[2][2] = (zf + zn) / (zf - zn);
	pout->m[2][3] = 1.0f;
	pout->m[3][2] = 2.f * (zf * zn) / (zn - zf);
	pout->m[3][3] = 0.0f;
#endif
}

Matrix4* ParaEngine::ParaMatrixScaling(Matrix4* pMat, float ScaleX, float ScaleY, float ScaleZ)
{
	Matrix4 M;
	M.m[0][0] = ScaleX;	M.m[0][1] = 0.0f;	M.m[0][2] = 0.0f;	M.m[0][3] = 0.0f;
	M.m[1][0] = 0.0f;	M.m[1][1] = ScaleY;	M.m[1][2] = 0.0f;	M.m[1][3] = 0.0f;
	M.m[2][0] = 0.0f;	M.m[2][1] = 0.0f;	M.m[2][2] = ScaleZ;	M.m[2][3] = 0.0f;
	M.m[3][0] = 0.0f;	M.m[3][1] = 0.0f;	M.m[3][2] = 0.0f;	M.m[3][3] = 1.0f;
	*pMat = M;
	return pMat;
}

bool ParaEngine::ParaMatrixDecompose(Vector3 *poutscale, Quaternion *poutrotation, Vector3 *pouttranslation, const Matrix4 *pm)
{
	Matrix3 normalized;
	Vector3 vec;
	
	/*Compute the scaling part.*/
	vec.x=pm->m[0][0];
	vec.y=pm->m[0][1];
	vec.z=pm->m[0][2];
	poutscale->x=vec.length();
	
	vec.x=pm->m[1][0];
	vec.y=pm->m[1][1];
	vec.z=pm->m[1][2];
	poutscale->y = vec.length();
	
	vec.x=pm->m[2][0];
	vec.y=pm->m[2][1];
	vec.z=pm->m[2][2];
	poutscale->z=vec.length();
	
	/*Compute the translation part.*/
	pouttranslation->x=pm->m[3][0];
	pouttranslation->y=pm->m[3][1];
	pouttranslation->z=pm->m[3][2];
	
	/*Let's calculate the rotation now*/
	if ( (poutscale->x == 0.0f) || (poutscale->y == 0.0f) || (poutscale->z == 0.0f) ) return false;
	
	normalized.m[0][0]=pm->m[0][0]/poutscale->x;
	normalized.m[0][1]=pm->m[0][1]/poutscale->x;
	normalized.m[0][2]=pm->m[0][2]/poutscale->x;
	normalized.m[1][0]=pm->m[1][0]/poutscale->y;
	normalized.m[1][1]=pm->m[1][1]/poutscale->y;
	normalized.m[1][2]=pm->m[1][2]/poutscale->y;
	normalized.m[2][0]=pm->m[2][0]/poutscale->z;
	normalized.m[2][1]=pm->m[2][1]/poutscale->z;
	normalized.m[2][2]=pm->m[2][2]/poutscale->z;
	
	poutrotation->FromRotationMatrix(normalized);
	return true;
}

Matrix4 *  ParaEngine::ParaMatrixAffineTransformation(Matrix4 *out, float scaling, const Vector3 *rotationcenter, const Quaternion*rotation, const Vector3 *translation)
{
    *out = Matrix4::IDENTITY;
    if (rotation)
    {
        float temp00, temp01, temp02, temp10, temp11, temp12, temp20, temp21, temp22;

        temp00 = 1.0f - 2.0f * (rotation->y * rotation->y + rotation->z * rotation->z);
        temp01 = 2.0f * (rotation->x * rotation->y + rotation->z * rotation->w);
        temp02 = 2.0f * (rotation->x * rotation->z - rotation->y * rotation->w);
        temp10 = 2.0f * (rotation->x * rotation->y - rotation->z * rotation->w);
        temp11 = 1.0f - 2.0f * (rotation->x * rotation->x + rotation->z * rotation->z);
        temp12 = 2.0f * (rotation->y * rotation->z + rotation->x * rotation->w);
        temp20 = 2.0f * (rotation->x * rotation->z + rotation->y * rotation->w);
        temp21 = 2.0f * (rotation->y * rotation->z - rotation->x * rotation->w);
        temp22 = 1.0f - 2.0f * (rotation->x * rotation->x + rotation->y * rotation->y);

		out->m[0][0] = scaling * temp00;
		out->m[0][1] = scaling * temp01;
		out->m[0][2] = scaling * temp02;
		out->m[1][0] = scaling * temp10;
		out->m[1][1] = scaling * temp11;
		out->m[1][2] = scaling * temp12;
		out->m[2][0] = scaling * temp20;
		out->m[2][1] = scaling * temp21;
		out->m[2][2] = scaling * temp22;

        if (rotationcenter)
        {
            out->m[3][0] = rotationcenter->x * (1.0f - temp00) - rotationcenter->y * temp10
					- rotationcenter->z * temp20;
            out->m[3][1] = rotationcenter->y * (1.0f - temp11) - rotationcenter->x * temp01
					- rotationcenter->z * temp21;
            out->m[3][2] = rotationcenter->z * (1.0f - temp22) - rotationcenter->x * temp02
                    - rotationcenter->y * temp12;
        }
    }
    else
    {
        out->m[0][0] = scaling;
        out->m[1][1] = scaling;
        out->m[2][2] = scaling;
    }

    if (translation)
    {
        out->m[3][0] += translation->x;
        out->m[3][1] += translation->y;
        out->m[3][2] += translation->z;
    }

    return out;
}


Vector3* ParaEngine::ParaVec3Project(Vector3 *pout, const Vector3 *pv, const ParaViewport *pviewport, const Matrix4 *pprojection, const Matrix4 *pview, const Matrix4 *pworld)
{
    Matrix4 m;

    m = Matrix4::IDENTITY;
    if (pworld) 
		m = (*pworld);
    if (pview) 
		m = m * (*pview);
    if (pprojection) 
		m = m * (*pprojection);

	ParaVec3TransformCoord(pout, pv, &m);

    if (pviewport)
    {
        pout->x = pviewport->X +  ( 1.0f + pout->x ) * pviewport->Width / 2.0f;
        pout->y = pviewport->Y +  ( 1.0f - pout->y ) * pviewport->Height / 2.0f;
        pout->z = pviewport->MinZ + pout->z * ( pviewport->MaxZ - pviewport->MinZ );
    }
    return pout;
}

Matrix4* ParaEngine::ParaMatrixOrthoOffCenterLH(Matrix4 *pout, float l, float r, float b, float t, float zn, float zf)
{
    *pout = Matrix4::IDENTITY;
    pout->m[0][0] = 2.0f / (r - l);
    pout->m[1][1] = 2.0f / (t - b);
    pout->m[2][2] = 1.0f / (zf -zn);
    pout->m[3][0] = -1.0f -2.0f *l / (r - l);
    pout->m[3][1] = 1.0f + 2.0f * t / (b - t);
    pout->m[3][2] = zn / (zn -zf);
    return pout;
}

Matrix4* ParaEngine::ParaMatrixOrthoOffCenterOpenGL(Matrix4 *pout, float left, float right, float bottom, float top, float zNearPlane, float zFarPlane)
{
	*pout = Matrix4::IDENTITY;
	pout->m[0][0] = 2 / (right - left);
	pout->m[1][1] = 2 / (top - bottom);
	pout->m[2][2] = 2 / (zNearPlane - zFarPlane);
	pout->m[3][0] = (left + right) / (left - right);
	pout->m[3][1] = (top + bottom) / (bottom - top);
	pout->m[3][2] = (zNearPlane + zFarPlane) / (zNearPlane - zFarPlane);
	return pout;
}


Matrix4* ParaEngine::ParaMatrixPerspectiveLH(Matrix4 *pout, float w, float h, float zn, float zf)
{
	*pout = Matrix4::IDENTITY;
    pout->m[0][0] = 2.0f * zn / w;
    pout->m[1][1] = 2.0f * zn / h;
    pout->m[2][2] = zf / (zf - zn);
    pout->m[3][2] = (zn * zf) / (zn - zf);
    pout->m[2][3] = 1.0f;
    pout->m[3][3] = 0.0f;
    return pout;
}


Matrix4* ParaEngine::ParaMatrixOrthoLH(Matrix4* Result, float Width, float Height, float zn, float zf)
{
	Matrix4 M(1.0f / Width, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f / Height, 0.0f, 0.0f,
		0.0f, 0.0f, 1 / (zf - zn), 0.0f,
		0.0f, 0.0f, -zn / (zf - zn), 1.0f);
	*Result = M;
	return Result;
}
