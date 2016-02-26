/** 
author: LiXizhi
date: 2009.4.15
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/
#include "ParaEngine.h"
#include "ParaAxisAlignedBox.h"

namespace ParaEngine
{
	const AxisAlignedBox AxisAlignedBox::BOX_NULL;
	const AxisAlignedBox AxisAlignedBox::BOX_INFINITE(AxisAlignedBox::EXTENT_INFINITE);

	bool AxisAlignedBox::operator!=(const AxisAlignedBox& rhs) const
	{
		return !(*this == rhs);
	}

	bool AxisAlignedBox::operator==(const AxisAlignedBox& rhs) const
	{
		if (this->mExtent != rhs.mExtent)
			return false;

		if (!this->isFinite())
			return true;

		return this->mMinimum == rhs.mMinimum &&
			this->mMaximum == rhs.mMaximum;
	}

	bool AxisAlignedBox::contains(const AxisAlignedBox& other) const
	{
		if (other.isNull() || this->isInfinite())
			return true;

		if (this->isNull() || other.isInfinite())
			return false;

		return this->mMinimum.x <= other.mMinimum.x &&
			this->mMinimum.y <= other.mMinimum.y &&
			this->mMinimum.z <= other.mMinimum.z &&
			other.mMaximum.x <= this->mMaximum.x &&
			other.mMaximum.y <= this->mMaximum.y &&
			other.mMaximum.z <= this->mMaximum.z;
	}

	bool AxisAlignedBox::contains(const Vector3& v) const
	{
		if (isNull())
			return false;
		if (isInfinite())
			return true;

		return mMinimum.x <= v.x && v.x <= mMaximum.x &&
			mMinimum.y <= v.y && v.y <= mMaximum.y &&
			mMinimum.z <= v.z && v.z <= mMaximum.z;
	}

	ParaEngine::Vector3 AxisAlignedBox::getHalfSize(void) const
	{
		switch (mExtent)
		{
		case EXTENT_NULL:
			return Vector3::ZERO;

		case EXTENT_FINITE:
			return (mMaximum - mMinimum) * 0.5;

		case EXTENT_INFINITE:
			return Vector3(
				Math::POS_INFINITY,
				Math::POS_INFINITY,
				Math::POS_INFINITY);

		default: // shut up compiler
			PE_ASSERT(false && "Never reached");
			return Vector3::ZERO;
		}
	}

	ParaEngine::Vector3 AxisAlignedBox::getSize(void) const
	{
		switch (mExtent)
		{
		case EXTENT_NULL:
			return Vector3::ZERO;

		case EXTENT_FINITE:
			return mMaximum - mMinimum;

		case EXTENT_INFINITE:
			return Vector3(
				Math::POS_INFINITY,
				Math::POS_INFINITY,
				Math::POS_INFINITY);

		default: // shut up compiler
			PE_ASSERT(false && "Never reached");
			return Vector3::ZERO;
		}
	}

	ParaEngine::Vector3 AxisAlignedBox::getCenter(void) const
	{
		PE_ASSERT((mExtent == EXTENT_FINITE) && "Can't get center of a null or infinite AAB");

		return Vector3(
			(mMaximum.x + mMinimum.x) * 0.5f,
			(mMaximum.y + mMinimum.y) * 0.5f,
			(mMaximum.z + mMinimum.z) * 0.5f);
	}

	bool AxisAlignedBox::intersects(const Vector3& v) const
	{
		switch (mExtent)
		{
		case EXTENT_NULL:
			return false;

		case EXTENT_FINITE:
			return(v.x >= mMinimum.x  &&  v.x <= mMaximum.x  &&
				v.y >= mMinimum.y  &&  v.y <= mMaximum.y  &&
				v.z >= mMinimum.z  &&  v.z <= mMaximum.z);

		case EXTENT_INFINITE:
			return true;

		default: // shut up compiler
			PE_ASSERT(false && "Never reached");
			return false;
		}
	}

	bool AxisAlignedBox::intersects(const Plane& p) const
	{
		return Math::intersects(p, *this);
	}

	bool AxisAlignedBox::intersects(const Sphere& s) const
	{
		return Math::intersects(s, *this);
	}

	bool AxisAlignedBox::intersects(const AxisAlignedBox& b2) const
	{
		// Early-fail for nulls
		if (this->isNull() || b2.isNull())
			return false;

		// Early-success for infinites
		if (this->isInfinite() || b2.isInfinite())
			return true;

		// Use up to 6 separating planes
		if (mMaximum.x < b2.mMinimum.x)
			return false;
		if (mMaximum.y < b2.mMinimum.y)
			return false;
		if (mMaximum.z < b2.mMinimum.z)
			return false;

		if (mMinimum.x > b2.mMaximum.x)
			return false;
		if (mMinimum.y > b2.mMaximum.y)
			return false;
		if (mMinimum.z > b2.mMaximum.z)
			return false;

		// otherwise, must be intersecting
		return true;
	}


	void AxisAlignedBox::scale(const Vector3& s)
	{
		// Do nothing if current null or infinite
		if (mExtent != EXTENT_FINITE)
			return;

		// NB assumes centered on origin
		Vector3 min = mMinimum * s;
		Vector3 max = mMaximum * s;
		setExtents(min, max);
	}

	float AxisAlignedBox::volume(void) const
	{
		switch (mExtent)
		{
		case EXTENT_NULL:
			return 0.0f;

		case EXTENT_FINITE:
		{
							  Vector3 diff = mMaximum - mMinimum;
							  return diff.x * diff.y * diff.z;
		}

		case EXTENT_INFINITE:
			return Math::POS_INFINITY;

		default: // shut up compiler
			PE_ASSERT(false && "Never reached");
			return 0.0f;
		}
	}

	ParaEngine::AxisAlignedBox AxisAlignedBox::intersection(const AxisAlignedBox& b2) const
	{
		if (this->isNull() || b2.isNull())
		{
			return AxisAlignedBox();
		}
		else if (this->isInfinite())
		{
			return b2;
		}
		else if (b2.isInfinite())
		{
			return *this;
		}

		Vector3 intMin = mMinimum;
		Vector3 intMax = mMaximum;

		intMin.makeCeil(b2.getMinimum());
		intMax.makeFloor(b2.getMaximum());

		// Check intersection isn't null
		if (intMin.x < intMax.x &&
			intMin.y < intMax.y &&
			intMin.z < intMax.z)
		{
			return AxisAlignedBox(intMin, intMax);
		}

		return AxisAlignedBox();
	}

	bool AxisAlignedBox::isInfinite(void) const
	{
		return (mExtent == EXTENT_INFINITE);
	}

	void AxisAlignedBox::setInfinite()
	{
		mExtent = EXTENT_INFINITE;
	}

	bool AxisAlignedBox::isFinite(void) const
	{
		return (mExtent == EXTENT_FINITE);
	}

	bool AxisAlignedBox::isNull(void) const
	{
		return (mExtent == EXTENT_NULL);
	}

	void AxisAlignedBox::setNull()
	{
		mExtent = EXTENT_NULL;
	}

	void AxisAlignedBox::transformAffine(const Matrix4& m)
	{
		PE_ASSERT(m.isAffine());

		// Do nothing if current null or infinite
		if (mExtent != EXTENT_FINITE)
			return;

		Vector3 centre = getCenter();
		Vector3 halfSize = getHalfSize();

		Vector3 newCentre = m.transformAffine(centre);
		Vector3 newHalfSize(
			Math::Abs(m[0][0]) * halfSize.x + Math::Abs(m[0][1]) * halfSize.y + Math::Abs(m[0][2]) * halfSize.z,
			Math::Abs(m[1][0]) * halfSize.x + Math::Abs(m[1][1]) * halfSize.y + Math::Abs(m[1][2]) * halfSize.z,
			Math::Abs(m[2][0]) * halfSize.x + Math::Abs(m[2][1]) * halfSize.y + Math::Abs(m[2][2]) * halfSize.z);

		setExtents(newCentre - newHalfSize, newCentre + newHalfSize);
	}

	void AxisAlignedBox::transform(const Matrix4& matrix)
	{
		// Do nothing if current null or infinite
		if (mExtent != EXTENT_FINITE)
			return;

		Vector3 oldMin, oldMax, currentCorner;

		// Getting the old values so that we can use the existing merge method.
		oldMin = mMinimum;
		oldMax = mMaximum;

		// reset
		setNull();

		// We sequentially compute the corners in the following order :
		// 0, 6, 5, 1, 2, 4 ,7 , 3
		// This sequence allows us to only change one member at a time to get at all corners.

		// For each one, we transform it using the matrix
		// Which gives the resulting point and merge the resulting point.

		// First corner 
		// min min min
		currentCorner = oldMin;
		merge(matrix * currentCorner);

		// min,min,max
		currentCorner.z = oldMax.z;
		merge(matrix * currentCorner);

		// min max max
		currentCorner.y = oldMax.y;
		merge(matrix * currentCorner);

		// min max min
		currentCorner.z = oldMin.z;
		merge(matrix * currentCorner);

		// max max min
		currentCorner.x = oldMax.x;
		merge(matrix * currentCorner);

		// max max max
		currentCorner.z = oldMax.z;
		merge(matrix * currentCorner);

		// max min max
		currentCorner.y = oldMin.y;
		merge(matrix * currentCorner);

		// max min min
		currentCorner.z = oldMin.z;
		merge(matrix * currentCorner);
	}

	void AxisAlignedBox::merge(const Vector3& point)
	{
		switch (mExtent)
		{
		case EXTENT_NULL: // if null, use this point
			setExtents(point, point);
			return;

		case EXTENT_FINITE:
			mMaximum.makeCeil(point);
			mMinimum.makeFloor(point);
			return;

		case EXTENT_INFINITE: // if infinite, makes no difference
			return;
		}

		PE_ASSERT(false && "Never reached");
	}

	void AxisAlignedBox::merge(const AxisAlignedBox& rhs)
	{
		// Do nothing if rhs null, or this is infinite
		if ((rhs.mExtent == EXTENT_NULL) || (mExtent == EXTENT_INFINITE))
		{
			return;
		}
		// Otherwise if rhs is infinite, make this infinite, too
		else if (rhs.mExtent == EXTENT_INFINITE)
		{
			mExtent = EXTENT_INFINITE;
		}
		// Otherwise if current null, just take rhs
		else if (mExtent == EXTENT_NULL)
		{
			setExtents(rhs.mMinimum, rhs.mMaximum);
		}
		// Otherwise merge
		else
		{
			Vector3 min = mMinimum;
			Vector3 max = mMaximum;
			max.makeCeil(rhs.mMaximum);
			min.makeFloor(rhs.mMinimum);

			setExtents(min, max);
		}
	}

	ParaEngine::Vector3 AxisAlignedBox::getCorner(CornerEnum cornerToGet) const
	{
		switch (cornerToGet)
		{
		case FAR_LEFT_BOTTOM:
			return mMinimum;
		case FAR_LEFT_TOP:
			return Vector3(mMinimum.x, mMaximum.y, mMinimum.z);
		case FAR_RIGHT_TOP:
			return Vector3(mMaximum.x, mMaximum.y, mMinimum.z);
		case FAR_RIGHT_BOTTOM:
			return Vector3(mMaximum.x, mMinimum.y, mMinimum.z);
		case NEAR_RIGHT_BOTTOM:
			return Vector3(mMaximum.x, mMinimum.y, mMaximum.z);
		case NEAR_LEFT_BOTTOM:
			return Vector3(mMinimum.x, mMinimum.y, mMaximum.z);
		case NEAR_LEFT_TOP:
			return Vector3(mMinimum.x, mMaximum.y, mMaximum.z);
		case NEAR_RIGHT_TOP:
			return mMaximum;
		default:
			return Vector3();
		}
	}

	const Vector3* AxisAlignedBox::getAllCorners(void) const
	{
		PE_ASSERT((mExtent == EXTENT_FINITE) && "Can't get corners of a null or infinite AAB");

		// The order of these items is, using right-handed co-ordinates:
		// Minimum Z face, starting with Min(all), then anticlockwise
		//   around face (looking onto the face)
		// Maximum Z face, starting with Max(all), then anticlockwise
		//   around face (looking onto the face)
		// Only for optimization/compatibility.
		if (!mpCorners)
			mpCorners = new Vector3[8];

		mpCorners[0] = mMinimum;
		mpCorners[1].x = mMinimum.x; mpCorners[1].y = mMaximum.y; mpCorners[1].z = mMinimum.z;
		mpCorners[2].x = mMaximum.x; mpCorners[2].y = mMaximum.y; mpCorners[2].z = mMinimum.z;
		mpCorners[3].x = mMaximum.x; mpCorners[3].y = mMinimum.y; mpCorners[3].z = mMinimum.z;

		mpCorners[4] = mMaximum;
		mpCorners[5].x = mMinimum.x; mpCorners[5].y = mMaximum.y; mpCorners[5].z = mMaximum.z;
		mpCorners[6].x = mMinimum.x; mpCorners[6].y = mMinimum.y; mpCorners[6].z = mMaximum.z;
		mpCorners[7].x = mMaximum.x; mpCorners[7].y = mMinimum.y; mpCorners[7].z = mMaximum.z;

		return mpCorners;
	}

	void AxisAlignedBox::setExtents(float mx, float my, float mz, float Mx, float My, float Mz)
	{
		PE_ASSERT((mx <= Mx && my <= My && mz <= Mz) &&
			"The minimum corner of the box must be less than or equal to maximum corner");

		mExtent = EXTENT_FINITE;

		mMinimum.x = mx;
		mMinimum.y = my;
		mMinimum.z = mz;

		mMaximum.x = Mx;
		mMaximum.y = My;
		mMaximum.z = Mz;
	}

	void AxisAlignedBox::setExtents(const Vector3& min, const Vector3& max)
	{
		PE_ASSERT((min.x <= max.x && min.y <= max.y && min.z <= max.z) &&
			"The minimum corner of the box must be less than or equal to maximum corner");

		mExtent = EXTENT_FINITE;
		mMinimum = min;
		mMaximum = max;
	}

	void AxisAlignedBox::setMaximumZ(float z)
	{
		mMaximum.z = z;
	}

	void AxisAlignedBox::setMaximumY(float y)
	{
		mMaximum.y = y;
	}

	void AxisAlignedBox::setMaximumX(float x)
	{
		mMaximum.x = x;
	}

	void AxisAlignedBox::setMinimumZ(float z)
	{
		mMinimum.z = z;
	}

	void AxisAlignedBox::setMinimumY(float y)
	{
		mMinimum.y = y;
	}

	void AxisAlignedBox::setMinimumX(float x)
	{
		mMinimum.x = x;
	}
}

