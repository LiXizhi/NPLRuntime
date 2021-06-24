// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once 

#include <math.h>
#include <limits>

namespace cAudio
{
	//! Smallest number that can be represented with a 32 bit float (may not match your compiler/os varient)
	const float Epsilon = std::numeric_limits<float>::epsilon();

	//! Internal function that compares two floats while keeping the Epsilon in mind.
	inline bool float_equals(const float a, const float b)
	{
		return (a + Epsilon >= b) && (a - Epsilon <= b);
	}

	//! Class for manipulating vectors in 3D space.
	class cVector3
	{
	public:
		float x, y, z;

		//! Default constructor, initializes everything to 0.
		cVector3(void) : x(0), y(0), z(0)
		{
		}

		cVector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz)
		{
		}

		//! Constructor, initializes all 3 axes to the same value.
		cVector3(float n) : x(n), y(n), z(n)
		{
		}

		cVector3(const cVector3& other) : x(other.x), y(other.y), z(other.z)
		{
		}

		cVector3(float* vector) : x(vector[0]), y(vector[1]), z(vector[2])
		{
		}

		cVector3 operator-() const { return cVector3(-x, -y, -z); }
		cVector3& operator=(const cVector3& other) { x = other.x; y = other.y; z = other.z; return *this; }
		cVector3 operator+(const cVector3& other) const { return cVector3(x + other.x, y + other.y, z + other.z); }
		cVector3& operator+=(const cVector3& other) { x+=other.x; y+=other.y; z+=other.z; return *this; }
		cVector3 operator+(const float val) const { return cVector3(x + val, y + val, z + val); }
		cVector3& operator+=(const float val) { x+=val; y+=val; z+=val; return *this; }

		cVector3 operator-(const cVector3& other) const { return cVector3(x - other.x, y - other.y, z - other.z); }
		cVector3& operator-=(const cVector3& other) { x-=other.x; y-=other.y; z-=other.z; return *this; }
		cVector3 operator-(const float val) const { return cVector3(x - val, y - val, z - val); }
		cVector3& operator-=(const float val) { x-=val; y-=val; z-=val; return *this; }

		cVector3 operator*(const cVector3& other) const { return cVector3(x * other.x, y * other.y, z * other.z); }
		cVector3& operator*=(const cVector3& other) { x*=other.x; y*=other.y; z*=other.z; return *this; }
		cVector3 operator*(const float v) const { return cVector3(x * v, y * v, z * v); }
		cVector3& operator*=(const float v) { x*=v; y*=v; z*=v; return *this; }

		cVector3 operator/(const cVector3& other) const { return cVector3(x / other.x, y / other.y, z / other.z); }
		cVector3& operator/=(const cVector3& other) { x/=other.x; y/=other.y; z/=other.z; return *this; }
		cVector3 operator/(const float v) const { float i=(float)1.0/v; return cVector3(x * i, y * i, z * i); }
		cVector3& operator/=(const float v) { float i=(float)1.0/v; x*=i; y*=i; z*=i; return *this; }

		bool operator<=(const cVector3& other) const { return x<=other.x && y<=other.y && z<=other.z;}
		bool operator>=(const cVector3& other) const { return x>=other.x && y>=other.y && z>=other.z;}
		bool operator<(const cVector3& other) const { return x<other.x && y<other.y && z<other.z;}
		bool operator>(const cVector3& other) const { return x>other.x && y>other.y && z>other.z;}

		bool operator==(const cVector3& other) const
		{
			return float_equals(x, other.x) &&
					float_equals(y, other.y) &&
					float_equals(z, other.z);
		}

		bool operator!=(const cVector3& other) const
		{
			return !(float_equals(x, other.x) &&
					float_equals(y, other.y) &&
					float_equals(z, other.z));
		}

		operator const float*() const { return &x; }

		operator float*() { return &x; }

		const float operator[] ( int i ) const { return ( ( float* ) &x ) [i]; }

		float &operator[] ( int i ) { return ( ( float* ) &x ) [i]; }

		//! Returns the length (magnitude) of the vector.
		float length() const
		{
			return sqrtf( x*x + y*y + z*z );
		}

		//! Forces the current vector to have a length of 1 while preserving the ratio of components.
		void normalize()
		{
			float invLen = 1.0f / length();
			x *= invLen;
			y *= invLen;
			z *= invLen;
		}

		//! Returns the dot product of this vector with the input vector.
		float dot( const cVector3& other ) const
		{
			return ( x * other.x + y * other.y + z * other.z );
		}

		//! Returns the cross product of this vector with the input vector.
		cVector3 cross( const cVector3& other ) const
		{
			return cVector3( y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x );
		}

		//! Sets the components of this vector.
		void set( float nx, float ny, float nz )
		{
			x = nx;
			y = ny;
			z = nz;
		}

		//! Sets all components of this vector to the same number.
		void set( float n )
		{
			x = y = z = n;
		}

		//! Sets this vector's components to match the input vector's.
		void set( const cVector3& other )
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}

		void getAsArray(float* output)
		{
			output[0] = x;
			output[1] = y;
			output[2] = z;
		}
	};
};