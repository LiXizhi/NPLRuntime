/** 
author: LiXizhi
date: 2009.4.15
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/
#include "ParaEngine.h"
#include "ParaMatrix4.h"
#include "ParaVector3.h"
#include "ParaMatrix3.h"
#include "PaintEngine/qmatrix.h"

namespace ParaEngine
{

    const Matrix4 Matrix4::ZERO(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0 );

    const Matrix4 Matrix4::IDENTITY(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 );

    const Matrix4 Matrix4::CLIPSPACE2DTOIMAGESPACE(
        0.5,    0,  0, 0.5, 
          0, -0.5,  0, 0.5, 
          0,    0,  1,   0,
          0,    0,  0,   1);

    //-----------------------------------------------------------------------
    inline static float
        MINOR(const Matrix4& m, const size_t r0, const size_t r1, const size_t r2, 
								const size_t c0, const size_t c1, const size_t c2)
    {
        return m[r0][c0] * (m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2]) -
            m[r0][c1] * (m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2]) +
            m[r0][c2] * (m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1]);
    }
    //-----------------------------------------------------------------------
    Matrix4 Matrix4::adjoint() const
    {
        return Matrix4( MINOR(*this, 1, 2, 3, 1, 2, 3),
            -MINOR(*this, 0, 2, 3, 1, 2, 3),
            MINOR(*this, 0, 1, 3, 1, 2, 3),
            -MINOR(*this, 0, 1, 2, 1, 2, 3),

            -MINOR(*this, 1, 2, 3, 0, 2, 3),
            MINOR(*this, 0, 2, 3, 0, 2, 3),
            -MINOR(*this, 0, 1, 3, 0, 2, 3),
            MINOR(*this, 0, 1, 2, 0, 2, 3),

            MINOR(*this, 1, 2, 3, 0, 1, 3),
            -MINOR(*this, 0, 2, 3, 0, 1, 3),
            MINOR(*this, 0, 1, 3, 0, 1, 3),
            -MINOR(*this, 0, 1, 2, 0, 1, 3),

            -MINOR(*this, 1, 2, 3, 0, 1, 2),
            MINOR(*this, 0, 2, 3, 0, 1, 2),
            -MINOR(*this, 0, 1, 3, 0, 1, 2),
            MINOR(*this, 0, 1, 2, 0, 1, 2));
    }
    //-----------------------------------------------------------------------
    float Matrix4::determinant() const
    {
        return m[0][0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
            m[0][1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
            m[0][2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
            m[0][3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
    }
    //-----------------------------------------------------------------------
    Matrix4 Matrix4::inverse() const
    {
		float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
        float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
        float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
        float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

        float v0 = m20 * m31 - m21 * m30;
        float v1 = m20 * m32 - m22 * m30;
        float v2 = m20 * m33 - m23 * m30;
        float v3 = m21 * m32 - m22 * m31;
        float v4 = m21 * m33 - m23 * m31;
        float v5 = m22 * m33 - m23 * m32;

        float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
        float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
        float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
        float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

        float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

        float d00 = t00 * invDet;
        float d10 = t10 * invDet;
        float d20 = t20 * invDet;
        float d30 = t30 * invDet;

        float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m10 * m31 - m11 * m30;
        v1 = m10 * m32 - m12 * m30;
        v2 = m10 * m33 - m13 * m30;
        v3 = m11 * m32 - m12 * m31;
        v4 = m11 * m33 - m13 * m31;
        v5 = m12 * m33 - m13 * m32;

        float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m21 * m10 - m20 * m11;
        v1 = m22 * m10 - m20 * m12;
        v2 = m23 * m10 - m20 * m13;
        v3 = m22 * m11 - m21 * m12;
        v4 = m23 * m11 - m21 * m13;
        v5 = m23 * m12 - m22 * m13;

        float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        return Matrix4(
            d00, d01, d02, d03,
            d10, d11, d12, d13,
            d20, d21, d22, d23,
            d30, d31, d32, d33);

    }
    
    //-----------------------------------------------------------------------
    void Matrix4::makeTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation)
    {
        // Ordering:
        //    1. Scale
        //    2. Rotate
        //    3. Translate

        Matrix3 rot3x3, scale3x3;
        orientation.ToRotationMatrix(rot3x3);
        scale3x3 = Matrix3::ZERO;
        scale3x3[0][0] = scale.x;
        scale3x3[1][1] = scale.y;
        scale3x3[2][2] = scale.z;

        // Set up final matrix with scale, rotation and translation
        *this = rot3x3 * scale3x3;
        this->setTrans(position);

        // No projection term
        m[0][3] = 0; m[1][3] = 0; m[2][3] = 0; m[3][3] = 1;
    }
    //-----------------------------------------------------------------------
    void Matrix4::makeInverseTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation)
    {
        // Invert the parameters
        Vector3 invTranslate = -position;
        Vector3 invScale(1 / scale.x, 1 / scale.y, 1 / scale.z);
        Quaternion invRot = orientation.Inverse();

        // Because we're inverting, order is translation, rotation, scale
        // So make translation relative to scale & rotation
        invTranslate *= invScale; // scale
        invTranslate = invRot * invTranslate; // rotate

        // Next, make a 3x3 rotation matrix and apply inverse scale
        Matrix3 rot3x3, scale3x3;
        invRot.ToRotationMatrix(rot3x3);
        scale3x3 = Matrix3::ZERO;
        scale3x3[0][0] = invScale.x;
        scale3x3[1][1] = invScale.y;
        scale3x3[2][2] = invScale.z;

        // Set up final matrix with scale, rotation and translation
        *this = scale3x3 * rot3x3;
        this->setTrans(invTranslate);

        // No projection term
        m[0][3] = 0; m[1][3] = 0; m[2][3] = 0; m[3][3] = 1;
    }

	void Matrix4::makeRot(const Quaternion& orientation, const Vector3& origin)
	{
		orientation.ToRotationMatrix(*this, origin);
	}

	Matrix4 Matrix4::InvertPRMatrix() const
	{
		Matrix4 dest;
		const Matrix4& src = *this;
		dest._11 = src._11;
		dest._21 = src._12;
		dest._31 = src._13;
		dest._41 = -(src._41*src._11 + src._42*src._12 + src._43*src._13);

		dest._12 = src._21;
		dest._22 = src._22;
		dest._32 = src._23;
		dest._42 = -(src._41*src._21 + src._42*src._22 + src._43*src._23);

		dest._13 = src._31;
		dest._23 = src._32;
		dest._33 = src._33;
		dest._43 = -(src._41*src._31 + src._42*src._32 + src._43*src._33);

		dest._14 = 0.f;
		dest._24 = 0.f;
		dest._34 = 0.f;
		dest._44 = 1.f;
		return dest;
	}

	void Matrix4::RemoveScaling(float Tolerance/*=SMALL_NUMBER*/)
	{
		// For each row, find magnitude, and if its non-zero re-scale so its unit length.
		const float SquareSum0 = (m[0][0] * m[0][0]) + (m[0][1] * m[0][1]) + (m[0][2] * m[0][2]);
		const float SquareSum1 = (m[1][0] * m[1][0]) + (m[1][1] * m[1][1]) + (m[1][2] * m[1][2]);
		const float SquareSum2 = (m[2][0] * m[2][0]) + (m[2][1] * m[2][1]) + (m[2][2] * m[2][2]);
		const float Scale0 = Math::FloatSelect(SquareSum0 - Tolerance, Math::InvSqrt(SquareSum0), 1.0f);
		const float Scale1 = Math::FloatSelect(SquareSum1 - Tolerance, Math::InvSqrt(SquareSum1), 1.0f);
		const float Scale2 = Math::FloatSelect(SquareSum2 - Tolerance, Math::InvSqrt(SquareSum2), 1.0f);
		m[0][0] *= Scale0;		m[0][1] *= Scale0;		m[0][2] *= Scale0;
		m[1][0] *= Scale1;		m[1][1] *= Scale1;		m[1][2] *= Scale1;
		m[2][0] *= Scale2;		m[2][1] *= Scale2;		m[2][2] *= Scale2;
	}

	ParaEngine::Matrix4 Matrix4::GetMatrixWithoutScale(float Tolerance/*=SMALL_NUMBER*/) const
	{
		Matrix4 Result = *this;
		Result.RemoveScaling(Tolerance);
		return Result;
	}

	ParaEngine::Vector3 Matrix4::ExtractScaling(float Tolerance/*=SMALL_NUMBER*/)
	{
		Vector3 Scale3D(0, 0, 0);

		// For each row, find magnitude, and if its non-zero re-scale so its unit length.
		const float SquareSum0 = (m[0][0] * m[0][0]) + (m[0][1] * m[0][1]) + (m[0][2] * m[0][2]);
		const float SquareSum1 = (m[1][0] * m[1][0]) + (m[1][1] * m[1][1]) + (m[1][2] * m[1][2]);
		const float SquareSum2 = (m[2][0] * m[2][0]) + (m[2][1] * m[2][1]) + (m[2][2] * m[2][2]);

		if (SquareSum0 > Tolerance)
		{
			float Scale0 = Math::Sqrt(SquareSum0);
			Scale3D[0] = Scale0;
			float InvScale0 = 1.f / Scale0;
			m[0][0] *= InvScale0;
			m[0][1] *= InvScale0;
			m[0][2] *= InvScale0;
		}
		else
		{
			Scale3D[0] = 0;
		}

		if (SquareSum1 > Tolerance)
		{
			float Scale1 = Math::Sqrt(SquareSum1);
			Scale3D[1] = Scale1;
			float InvScale1 = 1.f / Scale1;
			m[1][0] *= InvScale1;
			m[1][1] *= InvScale1;
			m[1][2] *= InvScale1;
		}
		else
		{
			Scale3D[1] = 0;
		}

		if (SquareSum2 > Tolerance)
		{
			float Scale2 = Math::Sqrt(SquareSum2);
			Scale3D[2] = Scale2;
			float InvScale2 = 1.f / Scale2;
			m[2][0] *= InvScale2;
			m[2][1] *= InvScale2;
			m[2][2] *= InvScale2;
		}
		else
		{
			Scale3D[2] = 0;
		}

		return Scale3D;
	}

	ParaEngine::Vector3 Matrix4::GetScaleVector(float Tolerance/*=SMALL_NUMBER*/) const
	{
		Vector3 Scale3D(1, 1, 1);

		// For each row, find magnitude, and if its non-zero re-scale so its unit length.
		for (int32 i = 0; i<3; i++)
		{
			const float SquareSum = (m[i][0] * m[i][0]) + (m[i][1] * m[i][1]) + (m[i][2] * m[i][2]);
			if (SquareSum > Tolerance)
			{
				Scale3D[i] = Math::Sqrt(SquareSum);
			}
			else
			{
				Scale3D[i] = 0.f;
			}
		}

		return Scale3D;
	}


	float Matrix4::GetScaleByAxis(int axis, float Tolerance /*= SMALL_NUMBER*/) const
	{
		int i = axis;
		const float SquareSum = (m[i][0] * m[i][0]) + (m[i][1] * m[i][1]) + (m[i][2] * m[i][2]);
		if (SquareSum > Tolerance)
		{
			return Math::Sqrt(SquareSum);
		}
		else
		{
			return 0.f;
		}
	}

	ParaEngine::Matrix4 Matrix4::RemoveTranslation() const
	{
		Matrix4 Result = *this;
		Result.m[3][0] = 0.0f;		Result.m[3][1] = 0.0f;		Result.m[3][2] = 0.0f;
		return Result;
	}

	bool Matrix4::hasScale() const
	{
		// check magnitude of row vectors (==local axes)
		float t = m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2];
		if (!Math::RealEqual(t, 1.0f, 1e-04f))
			return true;
		t = m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2];
		if (!Math::RealEqual(t, 1.0f, 1e-04f))
			return true;
		t = m[2][0] * m[2][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2];
		if (!Math::RealEqual(t, 1.0f, 1e-04f))
			return true;

		return false;
	}

	ParaEngine::Matrix4 Matrix4::Multiply4x3(const Matrix4 &m2) const
	{
		Matrix4 r;
		r.m[0][0] = m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0];
		r.m[0][1] = m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1];
		r.m[0][2] = m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2];
		r.m[0][3] = 0;

		r.m[1][0] = m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0];
		r.m[1][1] = m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1];
		r.m[1][2] = m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2];
		r.m[1][3] = 0;

		r.m[2][0] = m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0];
		r.m[2][1] = m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1];
		r.m[2][2] = m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2];
		r.m[2][3] = 0;

		r.m[3][0] = m[3][0] * m2.m[0][0] + m[3][1] * m2.m[1][0] + m[3][2] * m2.m[2][0] + m[3][3] * m2.m[3][0];
		r.m[3][1] = m[3][0] * m2.m[0][1] + m[3][1] * m2.m[1][1] + m[3][2] * m2.m[2][1] + m[3][3] * m2.m[3][1];
		r.m[3][2] = m[3][0] * m2.m[0][2] + m[3][1] * m2.m[1][2] + m[3][2] * m2.m[2][2] + m[3][3] * m2.m[3][2];
		r.m[3][3] = m[3][3] * m2.m[3][3];
		return r;
	}

	Matrix4::Matrix4(const QMatrix& mat2DAffine)
	{
		operator=(IDENTITY);
		_11 = mat2DAffine.m11();
		_12 = mat2DAffine.m12();
		_21 = mat2DAffine.m21();
		_22 = mat2DAffine.m22();
		_41 = mat2DAffine.dx();
		_42 = mat2DAffine.dy();
	}
}
