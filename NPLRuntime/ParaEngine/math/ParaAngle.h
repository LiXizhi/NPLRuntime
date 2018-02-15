#pragma once

namespace ParaEngine
{
	/** Wrapper class which indicates a given angle value is in Radians.
	@remarks
	Radian values are interchangeable with Degree values, and conversions
	will be done automatically between them.
	*/
	class Radian
	{
		float mRad;

	public:
		explicit Radian(float r = 0) : mRad(r) {}
		Radian(const Degree& d);
		Radian& operator = (const float& f) { mRad = f; return *this; }
		Radian& operator = (const Radian& r) { mRad = r.mRad; return *this; }
		Radian& operator = (const Degree& d);

		float valueDegrees() const; // see bottom of this file
		float valueRadians() const { return mRad; }
		float valueAngleUnits() const;

		operator float() { return mRad; }

		const Radian& operator + () const { return *this; }
		Radian operator + (const Radian& r) const { return Radian(mRad + r.mRad); }
		Radian operator + (const Degree& d) const;
		Radian& operator += (const Radian& r) { mRad += r.mRad; return *this; }
		Radian& operator += (const Degree& d);
		Radian operator - () const { return Radian(-mRad); }
		Radian operator - (const Radian& r) const { return Radian(mRad - r.mRad); }
		Radian operator - (const Degree& d) const;
		Radian& operator -= (const Radian& r) { mRad -= r.mRad; return *this; }
		Radian& operator -= (const Degree& d);
		Radian operator * (float f) const { return Radian(mRad * f); }
		Radian operator * (const Radian& f) const { return Radian(mRad * f.mRad); }
		Radian& operator *= (float f) { mRad *= f; return *this; }
		Radian operator / (float f) const { return Radian(mRad / f); }
		Radian& operator /= (float f) { mRad /= f; return *this; }

		bool operator <  (const Radian& r) const { return mRad <  r.mRad; }
		bool operator <= (const Radian& r) const { return mRad <= r.mRad; }
		bool operator == (const Radian& r) const { return mRad == r.mRad; }
		bool operator != (const Radian& r) const { return mRad != r.mRad; }
		bool operator >= (const Radian& r) const { return mRad >= r.mRad; }
		bool operator >(const Radian& r) const { return mRad >  r.mRad; }
	};

	/** Wrapper class which indicates a given angle value is in Degrees.
	@remarks
	Degree values are interchangeable with Radian values, and conversions
	will be done automatically between them.
	*/
	class Degree
	{
		float mDeg; // if you get an error here - make sure to define/typedef 'float' first

	public:
		explicit Degree(float d = 0) : mDeg(d) {}
		Degree(const Radian& r) : mDeg(r.valueDegrees()) {}
		Degree& operator = (const float& f) { mDeg = f; return *this; }
		Degree& operator = (const Degree& d) { mDeg = d.mDeg; return *this; }
		Degree& operator = (const Radian& r) { mDeg = r.valueDegrees(); return *this; }

		float valueDegrees() const { return mDeg; }
		float valueRadians() const; // see bottom of this file
		float valueAngleUnits() const;

		const Degree& operator + () const { return *this; }
		Degree operator + (const Degree& d) const { return Degree(mDeg + d.mDeg); }
		Degree operator + (const Radian& r) const { return Degree(mDeg + r.valueDegrees()); }
		Degree& operator += (const Degree& d) { mDeg += d.mDeg; return *this; }
		Degree& operator += (const Radian& r) { mDeg += r.valueDegrees(); return *this; }
		Degree operator - () const { return Degree(-mDeg); }
		Degree operator - (const Degree& d) const { return Degree(mDeg - d.mDeg); }
		Degree operator - (const Radian& r) const { return Degree(mDeg - r.valueDegrees()); }
		Degree& operator -= (const Degree& d) { mDeg -= d.mDeg; return *this; }
		Degree& operator -= (const Radian& r) { mDeg -= r.valueDegrees(); return *this; }
		Degree operator * (float f) const { return Degree(mDeg * f); }
		Degree operator * (const Degree& f) const { return Degree(mDeg * f.mDeg); }
		Degree& operator *= (float f) { mDeg *= f; return *this; }
		Degree operator / (float f) const { return Degree(mDeg / f); }
		Degree& operator /= (float f) { mDeg /= f; return *this; }

		bool operator <  (const Degree& d) const { return mDeg <  d.mDeg; }
		bool operator <= (const Degree& d) const { return mDeg <= d.mDeg; }
		bool operator == (const Degree& d) const { return mDeg == d.mDeg; }
		bool operator != (const Degree& d) const { return mDeg != d.mDeg; }
		bool operator >= (const Degree& d) const { return mDeg >= d.mDeg; }
		bool operator >(const Degree& d) const { return mDeg >  d.mDeg; }
	};

	/** Wrapper class which identifies a value as the currently default angle
		type, as defined by Math::setAngleUnit.
		@remarks
		Angle values will be automatically converted between radians and degrees,
		as appropriate.
		*/
	class Angle
	{
		float mAngle;
	public:
		explicit Angle(float angle) : mAngle(angle) {}
		operator Radian() const;
		operator Degree() const;
	};

	// these functions could not be defined within the class definition of class
	// Radian because they required class Degree to be defined
	inline Radian::Radian(const Degree& d) : mRad(d.valueRadians()) {
	}
	inline Radian& Radian::operator = (const Degree& d) {
		mRad = d.valueRadians(); return *this;
	}
	inline Radian Radian::operator + (const Degree& d) const {
		return Radian(mRad + d.valueRadians());
	}
	inline Radian& Radian::operator += (const Degree& d) {
		mRad += d.valueRadians();
		return *this;
	}
	inline Radian Radian::operator - (const Degree& d) const {
		return Radian(mRad - d.valueRadians());
	}
	inline Radian& Radian::operator -= (const Degree& d) {
		mRad -= d.valueRadians();
		return *this;
	}
}