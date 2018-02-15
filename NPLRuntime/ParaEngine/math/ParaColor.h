#pragma once

#define COLOR_ARGB(a,r,g,b)		Color(r,g,b,a)
#define COLOR_RGBA(r,g,b,a)		Color(r,g,b,a)
#define COLOR_XRGB(r,g,b)		Color(r,g,b)

namespace ParaEngine
{
	/**
	* A linear, 32-bit/component floating point RGBA color.
	*/
	struct LinearColor
	{
		float	r,		g,		b,		a;

		/** Static lookup table used for color -> LinearColor conversion. */
		static float PowOneOver255Table[256];

		inline LinearColor() {}
		inline explicit LinearColor(EnumForceInit)
			: r(0), g(0), b(0), a(0)
		{}
		inline LinearColor(float InR, float InG, float InB, float InA = 1.0f) : r(InR), g(InG), b(InB), a(InA) {}
		LinearColor(const class Color& C);
		LinearColor(uint32 C);
		LinearColor(const float * colors);
		LinearColor(const class Vector3& Vector);
#ifdef __D3DX9_H__
		LinearColor(const D3DCOLORVALUE& color);
		operator D3DCOLORVALUE& ();
		operator const D3DCOLORVALUE& () const;
#endif		
		operator const float*() const;
	
		// Operators.

		inline float& Component(int32 Index)
		{
			return (&r)[Index];
		}

		inline const float& Component(int32 Index) const
		{
			return (&r)[Index];
		}

		LinearColor& operator=(uint32 color);
		LinearColor& operator=(const Color& color);

		operator uint32() const;

		inline LinearColor operator+(const LinearColor& ColorB) const
		{
			return LinearColor(
				this->r + ColorB.r,
				this->g + ColorB.g,
				this->b + ColorB.b,
				this->a + ColorB.a
				);
		}
		inline LinearColor& operator+=(const LinearColor& ColorB)
		{
			r += ColorB.r;
			g += ColorB.g;
			b += ColorB.b;
			a += ColorB.a;
			return *this;
		}

		inline LinearColor operator-(const LinearColor& ColorB) const
		{
			return LinearColor(
				this->r - ColorB.r,
				this->g - ColorB.g,
				this->b - ColorB.b,
				this->a - ColorB.a
				);
		}
		inline LinearColor& operator-=(const LinearColor& ColorB)
		{
			r -= ColorB.r;
			g -= ColorB.g;
			b -= ColorB.b;
			a -= ColorB.a;
			return *this;
		}

		inline LinearColor operator*(const LinearColor& ColorB) const
		{
			return LinearColor(
				this->r * ColorB.r,
				this->g * ColorB.g,
				this->b * ColorB.b,
				this->a * ColorB.a
				);
		}
		inline LinearColor& operator*=(const LinearColor& ColorB)
		{
			r *= ColorB.r;
			g *= ColorB.g;
			b *= ColorB.b;
			a *= ColorB.a;
			return *this;
		}

		inline LinearColor operator*(float Scalar) const
		{
			return LinearColor(
				this->r * Scalar,
				this->g * Scalar,
				this->b * Scalar,
				this->a * Scalar
				);
		}

		inline LinearColor& operator*=(float Scalar)
		{
			r *= Scalar;
			g *= Scalar;
			b *= Scalar;
			a *= Scalar;
			return *this;
		}

		inline LinearColor operator/(const LinearColor& ColorB) const
		{
			return LinearColor(
				this->r / ColorB.r,
				this->g / ColorB.g,
				this->b / ColorB.b,
				this->a / ColorB.a
				);
		}
		inline LinearColor& operator/=(const LinearColor& ColorB)
		{
			r /= ColorB.r;
			g /= ColorB.g;
			b /= ColorB.b;
			a /= ColorB.a;
			return *this;
		}

		inline LinearColor operator/(float Scalar) const
		{
			const float	InvScalar = 1.0f / Scalar;
			return LinearColor(
				this->r * InvScalar,
				this->g * InvScalar,
				this->b * InvScalar,
				this->a * InvScalar
				);
		}
		inline LinearColor& operator/=(float Scalar)
		{
			const float	InvScalar = 1.0f / Scalar;
			r *= InvScalar;
			g *= InvScalar;
			b *= InvScalar;
			a *= InvScalar;
			return *this;
		}

		/** Comparison operators */
		inline bool operator==(const LinearColor& ColorB) const
		{
			return this->r == ColorB.r && this->g == ColorB.g && this->b == ColorB.b && this->a == ColorB.a;
		}
		inline bool operator!=(const LinearColor& Other) const
		{
			return this->r != Other.r || this->g != Other.g || this->b != Other.b || this->a != Other.a;
		}

		// Error-tolerant comparison.
		inline bool Equals(const LinearColor& ColorB, float Tolerance = KINDA_SMALL_NUMBER) const
		{
			return Math::Abs(this->r - ColorB.r) < Tolerance && Math::Abs(this->g - ColorB.g) < Tolerance && Math::Abs(this->b - ColorB.b) < Tolerance && Math::Abs(this->a - ColorB.a) < Tolerance;
		}

		LinearColor CopyWithNewOpacity(float NewOpacicty)
		{
			LinearColor NewCopy = *this;
			NewCopy.a = NewOpacicty;
			return NewCopy;
		}

		/**
		* Converts byte hue-saturation-brightness to floating point red-green-blue.
		*/
		static LinearColor FGetHSV(uint8 H, uint8 S, uint8 V);

		/**
		* Euclidean distance between two points.
		*/
		static inline float Dist(const LinearColor &V1, const LinearColor &V2)
		{
			return Math::Sqrt(Math::Sqr(V2.r - V1.r) + Math::Sqr(V2.g - V1.g) + Math::Sqr(V2.b - V1.b) + Math::Sqr(V2.a - V1.a));
		}

		/** Converts a linear space RGB color to an HSV color */
		LinearColor LinearRGBToHSV() const;

		/** Converts an HSV color to a linear space RGB color */
		LinearColor HSVToLinearRGB() const;

		/** Quantizes the linear color and returns the result as a color.  This bypasses the SRGB conversion. */
		Color Quantize() const;

		/** Quantizes the linear color and returns the result as a color with optional sRGB conversion and quality as goal. */
		Color ToColor(bool bSRGB = false) const;

		Vector3 ToVector3() const;

		/**
		* Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
		*
		* @param	Desaturation	Desaturation factor in range [0..1]
		* @return	Desaturated color
		*/
		LinearColor Desaturate(float Desaturation) const;

		/** Computes the perceptually weighted luminance value of a color. */
		float ComputeLuminance() const;

		/**
		* Returns the maximum value in this color structure
		*
		* @return	The maximum color channel value
		*/
		inline float GetMax() const
		{
			return Math::Max(Math::Max(Math::Max(r, g), b), a);
		}

		/** useful to detect if a light contribution needs to be rendered */
		bool IsAlmostBlack() const
		{
			return Math::Sqr(r) < DELTA && Math::Sqr(g) < DELTA && Math::Sqr(b) < DELTA;
		}

		/**
		* Returns the minimum value in this color structure
		*
		* @return	The minimum color channel value
		*/
		inline float GetMin() const
		{
			return Math::Min(Math::Min(Math::Min(r, g), b), a);
		}

		inline float GetLuminance() const
		{
			return r * 0.3f + g * 0.59f + b * 0.11f;
		}

		// Common colors.	
		static const LinearColor White;
		static const LinearColor Gray;
		static const LinearColor Black;
		static const LinearColor Transparent;
		static const LinearColor Red;
		static const LinearColor Green;
		static const LinearColor Blue;
		static const LinearColor Yellow;
	};

	inline LinearColor operator*(float Scalar, const LinearColor& Color)
	{
		return Color.operator*(Scalar);
	}

	//
	//	color
	//

	class Color
	{
	public:
		// Variables.
#if PLATFORM_LITTLE_ENDIAN
#if _MSC_VER
		// Win32 x86
		union { struct{ uint8 b, g, r, a; }; uint32 AlignmentDummy; };
#else
		// Linux x86, etc
		/*uint8 b GCC_ALIGN(4);
		uint8 g, r, a;*/
		union { struct{ uint8 b, g, r, a; }; uint32 AlignmentDummy; };
#endif
#else // PLATFORM_LITTLE_ENDIAN
		union { struct{ uint8 a, r, g, b; }; uint32 AlignmentDummy; };
#endif

		uint32& DWColor(void) { return *((uint32*)this); }
		const uint32& DWColor(void) const { return *((uint32*)this); }
		operator const uint32 () const { return DWColor(); };
		void operator=(const uint32 color);
		void operator=(const LinearColor& color);

		// Constructors.
		inline Color() {}
		inline explicit Color(EnumForceInit)
		{
			// put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER
			r = g = b = a = 0;
		}
		inline Color(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255)
		{
			// put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER
			r = InR;
			g = InG;
			b = InB;
			a = InA;
		}

		// fast, for more accuracy use LinearColor::ToColor()
		// TODO: doesn't handle negative colors well, implicit constructor can cause
		// accidental conversion (better use .ToColor(true))
		Color(const LinearColor& C);
		// put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER;
		Color(uint32 InColor);

		/** "255 255 255" */
		std::string ToRGBString();
		/** "255 255 255 255" */
		std::string ToRGBAString();

		// Operators.
		inline bool operator==(const Color &C) const
		{
			return DWColor() == C.DWColor();
		}

		inline bool operator!=(const Color& C) const
		{
			return DWColor() != C.DWColor();
		}

		inline void operator+=(const Color& C)
		{
			r = (uint8)Math::Min((int32)r + (int32)C.r, (int32)255);
			g = (uint8)Math::Min((int32)g + (int32)C.g, (int32)255);
			b = (uint8)Math::Min((int32)b + (int32)C.b, (int32)255);
			a = (uint8)Math::Min((int32)a + (int32)C.a, (int32)255);
		}

		inline Color MutiplyOpacity(float fOpacity)
		{
			if (fOpacity == 1.f)
				return *this;
			else
			{
				Color c = *this;
				c.a = (uint8)(c.a * fOpacity);
				return c;
			}
		}

		LinearColor FromRGBE() const;

		/** conversion between 32bits color and 16 bits color */
		static uint16 convert32_16(uint32 rgb);

		/** conversion between 32bits color and 16 bits color */
		static uint32 convert16_32(uint16 rgb);

		/**
		* Makes a random but quite nice color.
		*/
		static Color MakeRandomColor();

		/**
		* Makes a color red->green with the passed in scalar (e.g. 0 is red, 1 is green)
		*/
		static Color MakeRedToGreenColorFromScalar(float Scalar);

		
		/**
		* Reinterprets the color as a linear color.
		*
		* @return The linear color representation.
		*/
		inline LinearColor ReinterpretAsLinear() const
		{
			return LinearColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
		}
		/** @param sColor: RGBA or RGB "#ff000000" or "255 255 255 255" or "255 255 255"*/
		static Color FromString(const char* sColor);

		/** Some pre-inited colors, useful for debug code */
		static const Color White;
		static const Color Black;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Yellow;
		static const Color Cyan;
		static const Color Magenta;
	};

	inline uint32 GetTypeHash(const Color& Color)
	{
		return Color.DWColor();
	}

	/** Computes a brightness and a fixed point color from a floating point color. */
	extern void ComputeAndFixedColorAndIntensity(const LinearColor& InLinearColor, Color& OutColor, float& OutIntensity);
}

