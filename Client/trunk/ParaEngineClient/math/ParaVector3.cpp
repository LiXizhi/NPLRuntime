/** 
author: LiXizhi
date: 2009.4.15
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/
#include "ParaEngine.h"
#include "ParaVector3.h"
#include "ParaMath.h"

namespace ParaEngine
{
	const Vector3 Vector3::ZERO(0.f, 0.f, 0.f);

	const Vector3 Vector3::UNIT_X(1.f, 0.f, 0.f);
	const Vector3 Vector3::UNIT_Y(0.f, 1.f, 0.f);
	const Vector3 Vector3::UNIT_Z(0.f, 0.f, 1.f);
	const Vector3 Vector3::NEGATIVE_UNIT_X(-1.f, 0.f, 0.f);
	const Vector3 Vector3::NEGATIVE_UNIT_Y(0.f, -1.f, 0.f);
	const Vector3 Vector3::NEGATIVE_UNIT_Z(0.f, 0.f, -1.f);
	const Vector3 Vector3::UNIT_SCALE(1.f, 1.f, 1.f);

#ifdef __D3DX9_H__
	Vector3::Vector3(const D3DXVECTOR3& v) :x(v.x), y(v.y), z(v.z)
	{
	}

	Vector3::operator D3DXVECTOR3&()
	{
		return reinterpret_cast<D3DXVECTOR3&>(*this);
	}

	Vector3::operator const D3DXVECTOR3&() const
	{
		return reinterpret_cast<const D3DXVECTOR3&>(*this);
	}
#endif

	ParaEngine::Vector3 Vector3::TransformNormal(const Matrix4& M) const
	{
		Vector3 v;
		v.x = M.m[0][0] * x + M.m[1][0] * y + M.m[2][0] * z;
		v.y = M.m[0][1] * x + M.m[1][1] * y + M.m[2][1] * z;
		v.z = M.m[0][2] * x + M.m[1][2] * y + M.m[2][2] * z;
		return v;
	}

	ParaEngine::Quaternion Vector3::getRotationTo(const Vector3& dest, const Vector3& fallbackAxis /*= Vector3::ZERO*/) const
	{
		// Based on Stan Melax's article in Game Programming Gems
		Quaternion q;
		// Copy, since cannot modify local
		Vector3 v0 = *this;
		Vector3 v1 = dest;
		v0.normalise();
		v1.normalise();

		float d = v0.dotProduct(v1);
		// If dot == 1, vectors are the same
		if (d >= 1.0f)
		{
			return Quaternion::IDENTITY;
		}
		if (d < (1e-6f - 1.0f))
		{
			if (fallbackAxis != Vector3::ZERO)
			{
				// rotate 180 degrees about the fallback axis
				q.FromAngleAxis(Radian(Math::PI), fallbackAxis);
			}
			else
			{
				// Generate an axis
				Vector3 axis = Vector3::UNIT_X.crossProduct(*this);
				if (axis.isZeroLength()) // pick another if colinear
					axis = Vector3::UNIT_Y.crossProduct(*this);
				axis.normalise();
				q.FromAngleAxis(Radian(Math::PI), axis);
			}
		}
		else
		{
			float s = Math::Sqrt((1 + d) * 2);
			float invs = 1 / s;

			Vector3 c = v0.crossProduct(v1);

			q.x = c.x * invs;
			q.y = c.y * invs;
			q.z = c.z * invs;
			q.w = s * 0.5f;
			q.normalise();
		}
		return q;
	}

	ParaEngine::Radian Vector3::angleBetween(const Vector3& dest)
	{
		float lenProduct = length() * dest.length();

		// Divide by zero check
		if (lenProduct < 1e-6f)
			lenProduct = 1e-6f;

		float f = dotProduct(dest) / lenProduct;

		f = Math::Clamp(f, (float)-1.0, (float)1.0);
		return Math::ACos(f);
	}

	ParaEngine::Vector3 Vector3::randomDeviant(const Radian& angle, const Vector3& up /*= Vector3::ZERO */) const
	{
		Vector3 newUp;

		if (up == Vector3::ZERO)
		{
			// Generate an up vector
			newUp = this->perpendicular();
		}
		else
		{
			newUp = up;
		}

		// Rotate up vector by random amount around this
		Quaternion q;
		q.FromAngleAxis(Radian(Math::UnitRandom() * Math::TWO_PI), *this);
		newUp = q * newUp;

		// Finally rotate this by given angle around randomized up
		q.FromAngleAxis(angle, newUp);
		return q * (*this);
	}

	ParaEngine::Vector3 Vector3::perpendicular(void) const
	{
		static const float fSquareZero = 1e-06f * 1e-06f;

		Vector3 perp = this->crossProduct(Vector3::UNIT_X);

		// Check length
		if (perp.squaredLength() < fSquareZero)
		{
			/* This vector is the Y axis multiplied by a scalar, so we have
			   to use another axis.
			   */
			perp = this->crossProduct(Vector3::UNIT_Y);
		}
		perp.normalise();

		return perp;
	}

	int Vector3::dominantAxis() const
	{
		float xx, yy;

		if ((xx = Math::Abs(x)) > (yy = Math::Abs(y))) {
			if (xx > Math::Abs(z)) {
				return XAxis;
			}
			else {
				return ZAxis;
			}
		}
		else {
			if (yy > Math::Abs(z)) {
				return YAxis;
			}
			else {
				return ZAxis;
			}
		}
	}
}
