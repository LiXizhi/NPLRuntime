/** 
author: LiXizhi
date: 2009.4.15
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/
#include "ParaEngine.h"
#include "ParaQuaternion.h"

#include "ParaMath.h"
#include "ParaMatrix3.h"
#include "ParaVector3.h"

namespace ParaEngine {

    const float Quaternion::ms_fEpsilon = 1e-03f;
    const Quaternion Quaternion::ZERO(0.0f,0.0f,0.0f,0.0f);
    const Quaternion Quaternion::IDENTITY(0.0f,0.0f,0.0f,1.f);


	Quaternion::Quaternion(const Vector3& a, const Vector3& b)
		: w(1.0), x(0.0), y(0.0), z(0.0)
	{
		float factor = a.length() * b.length();

		if (Math::Abs(factor) > ms_fEpsilon)
		{
			// Vectors have length > 0
			Vector3 pivotVector;
			float dot = a.dotProduct(b) / factor;
			float theta = Math::ACos(Math::Clamp(dot, -1.0f, 1.0f));

			pivotVector = a*b;
			if (dot < 0.0 && pivotVector.length() < ms_fEpsilon) 
			{
				// Vectors parallel and opposite direction, therefore a rotation
				// of 180 degrees about any vector perpendicular to this vector
				// will rotate vector a onto vector b.
				//
				// The following guarantees the dot-product will be 0.0.
				//
				int dominantIndex = a.dominantAxis();
				pivotVector[dominantIndex] = -a[(dominantIndex + 1) % 3];
				pivotVector[(dominantIndex + 1) % 3] = a[dominantIndex];
				pivotVector[(dominantIndex + 2) % 3] = 0.0;
			}
			FromAngleAxis(Radian(theta), pivotVector);
		}
	}

	//-----------------------------------------------------------------------
    void Quaternion::FromRotationMatrix (const Matrix3& kRot)
    {
        // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
        // article "Quaternion Calculus and Fast Animation".

        float fTrace = kRot[0][0]+kRot[1][1]+kRot[2][2];
        float fRoot;

        if ( fTrace > 0.0f )
        {
            // |w| > 1/2, may as well choose w > 1/2
            fRoot = Math::Sqrt(fTrace + 1.0f);  // 2w
            w = 0.5f*fRoot;
            fRoot = 0.5f/fRoot;  // 1/(4w)
            x = (kRot[1][2]-kRot[2][1])*fRoot;
            y = (kRot[2][0]-kRot[0][2])*fRoot;
            z = (kRot[0][1]-kRot[1][0])*fRoot;
        }
        else
        {
            // |w| <= 1/2
            static size_t s_iNext[3] = { 1, 2, 0 };
            size_t i = 0;
            if ( kRot[1][1] > kRot[0][0] )
                i = 1;
            if ( kRot[2][2] > kRot[i][i] )
                i = 2;
            size_t j = s_iNext[i];
            size_t k = s_iNext[j];

            fRoot = Math::Sqrt(kRot[i][i]-kRot[j][j]-kRot[k][k] + 1.0f);
            float* apkQuat[3] = { &x, &y, &z };
            *apkQuat[i] = 0.5f*fRoot;
            fRoot = 0.5f/fRoot;
			w = (kRot[j][k]-kRot[k][j])*fRoot;
			*apkQuat[j] = (kRot[i][j]+kRot[j][i])*fRoot;
			*apkQuat[k] = (kRot[i][k]+kRot[k][i])*fRoot;
        }
    }

	void Quaternion::FromRotationMatrix(const Matrix4& kRot)
	{
		// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
		// article "Quaternion Calculus and Fast Animation".

		float fTrace = kRot[0][0] + kRot[1][1] + kRot[2][2];
		float fRoot;

		if (fTrace > 0.0f)
		{
			// |w| > 1/2, may as well choose w > 1/2
			fRoot = Math::Sqrt(fTrace + 1.0f);  // 2w
			w = 0.5f*fRoot;
			fRoot = 0.5f / fRoot;  // 1/(4w)
			x = (kRot[1][2] - kRot[2][1])*fRoot;
			y = (kRot[2][0] - kRot[0][2])*fRoot;
			z = (kRot[0][1] - kRot[1][0])*fRoot;
		}
		else
		{
			// |w| <= 1/2
			static size_t s_iNext[3] = { 1, 2, 0 };
			size_t i = 0;
			if (kRot[1][1] > kRot[0][0])
				i = 1;
			if (kRot[2][2] > kRot[i][i])
				i = 2;
			size_t j = s_iNext[i];
			size_t k = s_iNext[j];

			fRoot = Math::Sqrt(kRot[i][i] - kRot[j][j] - kRot[k][k] + 1.0f);
			float* apkQuat[3] = { &x, &y, &z };
			*apkQuat[i] = 0.5f*fRoot;
			fRoot = 0.5f / fRoot;
			w = (kRot[j][k] - kRot[k][j])*fRoot;
			*apkQuat[j] = (kRot[i][j] + kRot[j][i])*fRoot;
			*apkQuat[k] = (kRot[i][k] + kRot[k][i])*fRoot;
		}
	}

	//-----------------------------------------------------------------------
    void Quaternion::ToRotationMatrix (Matrix3& kRot) const
    {
		float fTx = x + x;
		float fTy = y + y;
		float fTz = z + z;
        float fTwx = fTx*w;
        float fTwy = fTy*w;
        float fTwz = fTz*w;
        float fTxx = fTx*x;
        float fTxy = fTy*x;
        float fTxz = fTz*x;
        float fTyy = fTy*y;
        float fTyz = fTz*y;
        float fTzz = fTz*z;

        kRot[0][0] = 1.0f-(fTyy+fTzz);      kRot[1][0] = fTxy-fTwz;				kRot[2][0] = fTxz+fTwy;
        kRot[0][1] = fTxy+fTwz;				kRot[1][1] = 1.0f-(fTxx+fTzz);      kRot[2][1] = fTyz-fTwx;
        kRot[0][2] = fTxz-fTwy;				kRot[1][2] = fTyz+fTwx;				kRot[2][2] = 1.0f-(fTxx+fTyy);
    }

	void Quaternion::ToRotationMatrix(Matrix4& kRot, const Vector3& Origin) const
	{
		float fTx = x+x;
		float fTy = y+y;
		float fTz = z+z;
		float fTwx = fTx*w;
		float fTwy = fTy*w;
		float fTwz = fTz*w;
		float fTxx = fTx*x;
		float fTxy = fTy*x;
		float fTxz = fTz*x;
		float fTyy = fTy*y;
		float fTyz = fTz*y;
		float fTzz = fTz*z;

		kRot[0][0] = 1.0f - (fTyy + fTzz);      kRot[1][0] = fTxy - fTwz;				kRot[2][0] = fTxz + fTwy;			kRot[3][0] = Origin.x;
		kRot[0][1] = fTxy + fTwz;				kRot[1][1] = 1.0f - (fTxx + fTzz);      kRot[2][1] = fTyz - fTwx;			kRot[3][1] = Origin.y;
		kRot[0][2] = fTxz - fTwy;				kRot[1][2] = fTyz + fTwx;				kRot[2][2] = 1.0f - (fTxx + fTyy);	kRot[3][2] = Origin.z;
		kRot[0][3] = 0.0f;						kRot[1][3] = 0.0f;						kRot[2][3] = 0.0f;					kRot[3][3] = 1.0f;
	}

	//-----------------------------------------------------------------------
    void Quaternion::FromAngleAxis (const Radian& rfAngle, const Vector3& rkAxis)
    {
        // PE_ASSERT:  axis[] is unit length
        //
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

        Radian fHalfAngle ( rfAngle * 0.5f);
        float fSin = Math::Sin(fHalfAngle);
        x = fSin*rkAxis.x;
        y = fSin*rkAxis.y;
        z = fSin*rkAxis.z;
		w = Math::Cos(fHalfAngle);
    }
    //-----------------------------------------------------------------------
    void Quaternion::ToAngleAxis (Radian& rfAngle, Vector3& rkAxis) const
    {
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

        float fSqrLength = x*x+y*y+z*z;
        if ( fSqrLength > 0.0f )
        {
            rfAngle = Math::ACos(w)*2.f;
            float fInvLength = Math::InvSqrt(fSqrLength);
            rkAxis.x = x*fInvLength;
            rkAxis.y = y*fInvLength;
            rkAxis.z = z*fInvLength;
        }
        else
        {
            // angle is 0 (mod 2*pi), so any axis will do
            rfAngle = Radian(0.0f);
            rkAxis.x = 1.0f;
            rkAxis.y = 0.0f;
            rkAxis.z = 0.0f;
        }
    }
    //-----------------------------------------------------------------------
    void Quaternion::FromAxes (const Vector3* akAxis)
    {
        Matrix3 kRot;

        for (size_t iCol = 0; iCol < 3; iCol++)
        {
            kRot[0][iCol] = akAxis[iCol].x;
            kRot[1][iCol] = akAxis[iCol].y;
            kRot[2][iCol] = akAxis[iCol].z;
        }

        FromRotationMatrix(kRot);
    }
    //-----------------------------------------------------------------------
    void Quaternion::FromAxes (const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis)
    {
        Matrix3 kRot;

        kRot[0][0] = xaxis.x;
        kRot[1][0] = xaxis.y;
        kRot[2][0] = xaxis.z;

        kRot[0][1] = yaxis.x;
        kRot[1][1] = yaxis.y;
        kRot[2][1] = yaxis.z;

        kRot[0][2] = zaxis.x;
        kRot[1][2] = zaxis.y;
        kRot[2][2] = zaxis.z;

        FromRotationMatrix(kRot);

    }
    //-----------------------------------------------------------------------
    void Quaternion::ToAxes (Vector3* akAxis) const
    {
        Matrix3 kRot;

        ToRotationMatrix(kRot);

        for (size_t iCol = 0; iCol < 3; iCol++)
        {
            akAxis[iCol].x = kRot[0][iCol];
            akAxis[iCol].y = kRot[1][iCol];
            akAxis[iCol].z = kRot[2][iCol];
        }
    }
    //-----------------------------------------------------------------------
    Vector3 Quaternion::xAxis(void) const
    {
        //float fTx  = 2.0f*x;
        float fTy  = 2.0f*y;
        float fTz  = 2.0f*z;
        float fTwy = fTy*w;
        float fTwz = fTz*w;
        float fTxy = fTy*x;
        float fTxz = fTz*x;
        float fTyy = fTy*y;
        float fTzz = fTz*z;

        return Vector3(1.0f-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);
    }
    //-----------------------------------------------------------------------
    Vector3 Quaternion::yAxis(void) const
    {
        float fTx  = 2.0f*x;
        float fTy  = 2.0f*y;
        float fTz  = 2.0f*z;
        float fTwx = fTx*w;
        float fTwz = fTz*w;
        float fTxx = fTx*x;
        float fTxy = fTy*x;
        float fTyz = fTz*y;
        float fTzz = fTz*z;

        return Vector3(fTxy-fTwz, 1.0f-(fTxx+fTzz), fTyz+fTwx);
    }
    //-----------------------------------------------------------------------
    Vector3 Quaternion::zAxis(void) const
    {
        float fTx  = 2.0f*x;
        float fTy  = 2.0f*y;
        float fTz  = 2.0f*z;
        float fTwx = fTx*w;
        float fTwy = fTy*w;
        float fTxx = fTx*x;
        float fTxz = fTz*x;
        float fTyy = fTy*y;
        float fTyz = fTz*y;

        return Vector3(fTxz+fTwy, fTyz-fTwx, 1.0f-(fTxx+fTyy));
    }
    //-----------------------------------------------------------------------
    void Quaternion::ToAxes (Vector3& xaxis, Vector3& yaxis, Vector3& zaxis) const
    {
        Matrix3 kRot;

        ToRotationMatrix(kRot);

        xaxis.x = kRot[0][0];
        xaxis.y = kRot[1][0];
        xaxis.z = kRot[2][0];

        yaxis.x = kRot[0][1];
        yaxis.y = kRot[1][1];
        yaxis.z = kRot[2][1];

        zaxis.x = kRot[0][2];
        zaxis.y = kRot[1][2];
        zaxis.z = kRot[2][2];
    }

    //-----------------------------------------------------------------------
    Quaternion Quaternion::operator+ (const Quaternion& rkQ) const
    {
		return Quaternion(x + rkQ.x, y + rkQ.y, z + rkQ.z, w + rkQ.w);
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::operator- (const Quaternion& rkQ) const
    {
		return Quaternion(x - rkQ.x, y - rkQ.y, z - rkQ.z, w - rkQ.w);
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::operator* (const Quaternion& rkQ) const
    {
        // NOTE:  Multiplication is not generally commutative, so in most
        // cases p*q != q*p.

        return Quaternion
        (
            w * rkQ.x + x * rkQ.w + y * rkQ.z - z * rkQ.y,
            w * rkQ.y + y * rkQ.w + z * rkQ.x - x * rkQ.z,
            w * rkQ.z + z * rkQ.w + x * rkQ.y - y * rkQ.x,
			w * rkQ.w - x * rkQ.x - y * rkQ.y - z * rkQ.z
        );
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::operator* (float fScalar) const
    {
		return Quaternion(fScalar*x, fScalar*y, fScalar*z, fScalar*w);
    }
    //-----------------------------------------------------------------------
    Quaternion operator* (float fScalar, const Quaternion& rkQ)
    {
		return Quaternion(fScalar*rkQ.x, fScalar*rkQ.y, fScalar*rkQ.z, fScalar*rkQ.w);
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::operator- () const
    {
		return Quaternion(-x, -y, -z, -w);
    }
    //-----------------------------------------------------------------------
    float Quaternion::Dot (const Quaternion& rkQ) const
    {
		return x*rkQ.x + y*rkQ.y + z*rkQ.z + w*rkQ.w;
    }
    //-----------------------------------------------------------------------
    float Quaternion::Norm () const
    {
		return x*x + y*y + z*z + w*w;
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::Inverse() const
    {
		float fNorm = x*x + y*y + z*z + w*w;
        if ( fNorm > 0.0f )
        {
            float fInvNorm = 1.0f/fNorm;
			return Quaternion(-x*fInvNorm, -y*fInvNorm, -z*fInvNorm, w*fInvNorm);
        }
        else
        {
            // return an invalid result to flag the error
            return ZERO;
        }
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::UnitInverse () const
    {
        // PE_ASSERT:  'this' is unit length
        return Quaternion(-x,-y,-z, w);
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::Exp () const
    {
        // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
        // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

        Radian fAngle ( Math::Sqrt(x*x+y*y+z*z) );
        float fSin = Math::Sin(fAngle);

        Quaternion kResult;
        kResult.w = Math::Cos(fAngle);

        if ( Math::Abs(fSin) >= ms_fEpsilon )
        {
            float fCoeff = fSin/(fAngle.valueRadians());
            kResult.x = fCoeff*x;
            kResult.y = fCoeff*y;
            kResult.z = fCoeff*z;
        }
        else
        {
            kResult.x = x;
            kResult.y = y;
            kResult.z = z;
        }

        return kResult;
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::Log () const
    {
        // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
        // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
        // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

        Quaternion kResult;
        kResult.w = 0.0f;

        if ( Math::Abs(w) < 1.0f )
        {
            Radian fAngle ( Math::ACos(w) );
            float fSin = Math::Sin(fAngle);
            if ( Math::Abs(fSin) >= ms_fEpsilon )
            {
                float fCoeff = fAngle.valueRadians()/fSin;
                kResult.x = fCoeff*x;
                kResult.y = fCoeff*y;
                kResult.z = fCoeff*z;
                return kResult;
            }
        }

        kResult.x = x;
        kResult.y = y;
        kResult.z = z;

        return kResult;
    }
    //-----------------------------------------------------------------------
    Vector3 Quaternion::operator* (const Vector3& v) const
    {
		// nVidia SDK implementation
		Vector3 uv, uuv;
		Vector3 qvec(x, y, z);
		uv = qvec.crossProduct(v);
		uuv = qvec.crossProduct(uv);
		uv *= (2.0f * w);
		uuv *= 2.0f;

		return v + uv + uuv;

    }
    //-----------------------------------------------------------------------
	bool Quaternion::equals(const Quaternion& rhs, const Radian& tolerance) const
	{
        float fCos = Dot(rhs);
        Radian angle = Math::ACos(fCos);

		return (Math::Abs(angle.valueRadians()) <= tolerance.valueRadians())
            || Math::RealEqual(angle.valueRadians(), Math::PI, tolerance.valueRadians());
	}
    //-----------------------------------------------------------------------
	Quaternion Quaternion::Slerp(float Slerp, const Quaternion& Quat1, const Quaternion& Quat2, bool shortestPath)
    {
		// Get cosine of angle between quats.
		const float RawCosom = Quat1.x * Quat2.x + Quat1.y * Quat2.y + Quat1.z * Quat2.z + Quat1.w * Quat2.w;
		// Unaligned quats - compensate, results in taking shorter route.
		const float Cosom = RawCosom >= 0.f ? RawCosom : -RawCosom;

		float Scale0, Scale1;

		if (Cosom < 0.9999f)
		{
			const float Omega = acosf(Cosom);
			const float InvSin = 1.f / sinf(Omega);
			Scale0 = sinf((1.f - Slerp) * Omega) * InvSin;
			Scale1 = sinf(Slerp * Omega) * InvSin;
		}
		else
		{
			// Use linear interpolation.
			Scale0 = 1.0f - Slerp;
			Scale1 = Slerp;
		}

		// In keeping with our flipped Cosom:
		Scale1 = RawCosom >= 0.f ? Scale1 : -Scale1;

		Quaternion Result;

		Result.x = Scale0 * Quat1.x + Scale1 * Quat2.x;
		Result.y = Scale0 * Quat1.y + Scale1 * Quat2.y;
		Result.z = Scale0 * Quat1.z + Scale1 * Quat2.z;
		Result.w = Scale0 * Quat1.w + Scale1 * Quat2.w;

		return Result;
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::SlerpExtraSpins (float fT,
        const Quaternion& rkP, const Quaternion& rkQ, int iExtraSpins)
    {
        float fCos = rkP.Dot(rkQ);
        Radian fAngle ( Math::ACos(fCos) );

        if ( Math::Abs(fAngle.valueRadians()) < ms_fEpsilon )
            return rkP;

        float fSin = Math::Sin(fAngle);
        Radian fPhase ( Math::PI*iExtraSpins*fT );
        float fInvSin = 1.0f/fSin;
        float fCoeff0 = Math::Sin((1.0f-fT)*fAngle - fPhase)*fInvSin;
        float fCoeff1 = Math::Sin(fT*fAngle + fPhase)*fInvSin;
        return fCoeff0*rkP + fCoeff1*rkQ;
    }
    //-----------------------------------------------------------------------
    void Quaternion::Intermediate (const Quaternion& rkQ0,
        const Quaternion& rkQ1, const Quaternion& rkQ2,
        Quaternion& rkA, Quaternion& rkB)
    {
        // PE_ASSERT:  q0, q1, q2 are unit quaternions

        Quaternion kQ0inv = rkQ0.UnitInverse();
        Quaternion kQ1inv = rkQ1.UnitInverse();
        Quaternion rkP0 = kQ0inv*rkQ1;
        Quaternion rkP1 = kQ1inv*rkQ2;
        Quaternion kArg = 0.25f*(rkP0.Log()-rkP1.Log());
        Quaternion kMinusArg = -kArg;

        rkA = rkQ1*kArg.Exp();
        rkB = rkQ1*kMinusArg.Exp();
    }
    //-----------------------------------------------------------------------
    Quaternion Quaternion::Squad (float fT,
        const Quaternion& rkP, const Quaternion& rkA,
        const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath)
    {
        float fSlerpT = 2.0f*fT*(1.0f-fT);
        Quaternion kSlerpP = Slerp(fT, rkP, rkQ, shortestPath);
        Quaternion kSlerpQ = Slerp(fT, rkA, rkB);
        return Slerp(fSlerpT, kSlerpP ,kSlerpQ);
    }
    //-----------------------------------------------------------------------
    float Quaternion::normalise(void)
    {
        float len = Norm();
        float factor = 1.0f / Math::Sqrt(len);
        *this = *this * factor;
        return len;
    }
    //-----------------------------------------------------------------------
	Radian Quaternion::getRoll(bool reprojectAxis) const
	{
		if (reprojectAxis)
		{
			// roll = atan2(localx.y, localx.x)
			// pick parts of xAxis() implementation that we need
			float fTx  = 2.0f*x;
			float fTy  = 2.0f*y;
			float fTz  = 2.0f*z;
			float fTwz = fTz*w;
			float fTxy = fTy*x;
			float fTyy = fTy*y;
			float fTzz = fTz*z;

			// Vector3(1.0f-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);

			return Radian(Math::ATan2(fTxy+fTwz, 1.0f-(fTyy+fTzz)));

		}
		else
		{
			return Radian(Math::ATan2(2*(x*y + w*z), w*w + x*x - y*y - z*z));
		}
	}
    //-----------------------------------------------------------------------
	Radian Quaternion::getPitch(bool reprojectAxis) const
	{
		if (reprojectAxis)
		{
			// pitch = atan2(localy.z, localy.y)
			// pick parts of yAxis() implementation that we need
			float fTx  = 2.0f*x;
			float fTy  = 2.0f*y;
			float fTz  = 2.0f*z;
			float fTwx = fTx*w;
			float fTxx = fTx*x;
			float fTyz = fTz*y;
			float fTzz = fTz*z;

			// Vector3(fTxy-fTwz, 1.0f-(fTxx+fTzz), fTyz+fTwx);
			return Radian(Math::ATan2(fTyz+fTwx, 1.0f-(fTxx+fTzz)));
		}
		else
		{
			// internal version
			return Radian(Math::ATan2(2*(y*z + w*x), w*w - x*x - y*y + z*z));
		}
	}
    //-----------------------------------------------------------------------
	Radian Quaternion::getYaw(bool reprojectAxis) const
	{
		if (reprojectAxis)
		{
			// yaw = atan2(localz.x, localz.z)
			// pick parts of zAxis() implementation that we need
			float fTx  = 2.0f*x;
			float fTy  = 2.0f*y;
			float fTz  = 2.0f*z;
			float fTwy = fTy*w;
			float fTxx = fTx*x;
			float fTxz = fTz*x;
			float fTyy = fTy*y;

			// Vector3(fTxz+fTwy, fTyz-fTwx, 1.0f-(fTxx+fTyy));

			return Radian(Math::ATan2(fTxz+fTwy, 1.0f-(fTxx+fTyy)));

		}
		else
		{
			// internal version
			return Radian(Math::ASin(-2*(x*z - w*y)));
		}
	}
    //-----------------------------------------------------------------------
    Quaternion Quaternion::nlerp(float fT, const Quaternion& rkP,
        const Quaternion& rkQ, bool shortestPath)
    {
		Quaternion result;
        float fCos = rkP.Dot(rkQ);
		if (fCos < 0.0f && shortestPath)
		{
			result = rkP + fT * ((-rkQ) - rkP);
		}
		else
		{
			result = rkP + fT * (rkQ - rkP);
		}
        result.normalise();
        return result;
    }

	// not tested: the w may be negated.
	void Quaternion::ToRadians(float *outX, float *outY, float *outZ) const
	{
		float sqw = w*w;
		float sqx = x*x;
		float sqy = y*y;
		float sqz = z*z;
		float unit = sqx + sqy + sqz + sqw; // if normalized is one, otherwise is correction factor
		float test = x*y + z*w;
		if (test > 0.499f*unit)
		{ // singularity at north pole
			*outY = 2 * atan2f(x, w);
			*outZ = 1.570796327f; //HALF PI
			*outX = 0;
			return;
		}
		if (test < -0.499f*unit)
		{ // singularity at south pole
			*outY = -2 * atan2f(x, w);
			*outZ = -1.570796327f;	//HALFPI
			*outX = 0;
			return;
		}
		*outY = atan2f(2 * y*w - 2 * x*z, sqx - sqy - sqz + sqw);
		*outX = asinf(2 * test / unit);
		*outZ = atan2f(2 * x*w - 2 * y*z, -sqx + sqy - sqz + sqw);
	}

}
