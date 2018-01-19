namespace ParaEngine
{
	inline float Radian::valueDegrees() const
	{
		return Math::RadiansToDegrees(mRad);
	}

	inline float Radian::valueAngleUnits() const
	{
		return Math::RadiansToAngleUnits(mRad);
	}

	inline float Degree::valueRadians() const
	{
		return Math::DegreesToRadians(mDeg);
	}

	inline float Degree::valueAngleUnits() const
	{
		return Math::DegreesToAngleUnits(mDeg);
	}

	inline Angle::operator Radian() const
	{
		return Radian(Math::AngleUnitsToRadians(mAngle));
	}

	inline Angle::operator Degree() const
	{
		return Degree(Math::AngleUnitsToDegrees(mAngle));
	}

	inline Radian operator * (float a, const Radian& b)
	{
		return Radian(a * b.valueRadians());
	}

	inline Radian operator / (float a, const Radian& b)
	{
		return Radian(a / b.valueRadians());
	}

	inline Degree operator * (float a, const Degree& b)
	{
		return Degree(a * b.valueDegrees());
	}

	inline Degree operator / (float a, const Degree& b)
	{
		return Degree(a / b.valueDegrees());
	}
}