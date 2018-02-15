#pragma once
#include "ParaVector3.h"
#include "ParaMatrix4.h"

namespace ParaEngine {

	/** A 3D box aligned with the x/y/z axes.
	@remarks
	This class represents a simple box which is aligned with the
	axes. Internally it only stores 2 points as the extremeties of
	the box, one which is the minima of all 3 axes, and the other
	which is the maxima of all 3 axes. This class is typically used
	for an axis-aligned bounding box (AABB) for collision and
	visibility determination.
	*/
	class AxisAlignedBox
	{
	public:
		enum Extent
		{
			EXTENT_NULL,
			EXTENT_FINITE,
			EXTENT_INFINITE
		};
	protected:

		Vector3 mMinimum;
		Vector3 mMaximum;
		Extent mExtent;
		mutable Vector3* mpCorners;

	public:
		/*
		1-----2
		/|    /|
		/ |   / |
		5-----4  |
		|  0--|--3
		| /   | /
		|/    |/
		6-----7
		*/
		typedef enum {
			FAR_LEFT_BOTTOM = 0,
			FAR_LEFT_TOP = 1,
			FAR_RIGHT_TOP = 2,
			FAR_RIGHT_BOTTOM = 3,
			NEAR_RIGHT_BOTTOM = 7,
			NEAR_LEFT_BOTTOM = 6,
			NEAR_LEFT_TOP = 5,
			NEAR_RIGHT_TOP = 4
		} CornerEnum;
		inline AxisAlignedBox() : mpCorners(0)
		{
			// Default to a null box 
			setMinimum( -0.5, -0.5, -0.5 );
			setMaximum( 0.5, 0.5, 0.5 );
			mExtent = EXTENT_NULL;
		}
		inline AxisAlignedBox(Extent e) : mpCorners(0)
		{
			setMinimum( -0.5, -0.5, -0.5 );
			setMaximum( 0.5, 0.5, 0.5 );
			mExtent = e;
		}

		inline AxisAlignedBox(const AxisAlignedBox & rkBox) : mpCorners(0)
		{
			if (rkBox.isNull())
				setNull();
			else if (rkBox.isInfinite())
				setInfinite();
			else
				setExtents( rkBox.mMinimum, rkBox.mMaximum );
		}

		inline AxisAlignedBox( const Vector3& min, const Vector3& max ) : mpCorners(0)
		{
			setExtents( min, max );
		}

		inline AxisAlignedBox(
			float mx, float my, float mz,
			float Mx, float My, float Mz ) : mpCorners(0)
		{
			setExtents( mx, my, mz, Mx, My, Mz );
		}

		AxisAlignedBox& operator=(const AxisAlignedBox& rhs)
		{
			// Specifically override to avoid copying mpCorners
			if (rhs.isNull())
				setNull();
			else if (rhs.isInfinite())
				setInfinite();
			else
				setExtents(rhs.mMinimum, rhs.mMaximum);

			return *this;
		}

		~AxisAlignedBox()
		{
			if (mpCorners)
				delete [](mpCorners);
		}


		/** Gets the minimum corner of the box.
		*/
		inline const Vector3& getMinimum(void) const
		{ 
			return mMinimum; 
		}

		/** Gets a modifiable version of the minimum
		corner of the box.
		*/
		inline Vector3& getMinimum(void)
		{ 
			return mMinimum; 
		}

		/** Gets the maximum corner of the box.
		*/
		inline const Vector3& getMaximum(void) const
		{ 
			return mMaximum;
		}

		/** Gets a modifiable version of the maximum
		corner of the box.
		*/
		inline Vector3& getMaximum(void)
		{ 
			return mMaximum;
		}


		/** Sets the minimum corner of the box.
		*/
		inline void setMinimum( const Vector3& vec )
		{
			mExtent = EXTENT_FINITE;
			mMinimum = vec;
		}

		inline void setMinimum( float x, float y, float z )
		{
			mExtent = EXTENT_FINITE;
			mMinimum.x = x;
			mMinimum.y = y;
			mMinimum.z = z;
		}

		/** Changes one of the components of the minimum corner of the box
		used to resize only one dimension of the box
		*/
		void setMinimumX(float x);

		void setMinimumY(float y);

		void setMinimumZ(float z);

		/** Sets the maximum corner of the box.
		*/
		inline void setMaximum( const Vector3& vec )
		{
			mExtent = EXTENT_FINITE;
			mMaximum = vec;
		}

		inline void setMaximum( float x, float y, float z )
		{
			mExtent = EXTENT_FINITE;
			mMaximum.x = x;
			mMaximum.y = y;
			mMaximum.z = z;
		}

		/** Changes one of the components of the maximum corner of the box
		used to resize only one dimension of the box
		*/
		void setMaximumX( float x );

		void setMaximumY( float y );

		void setMaximumZ( float z );

		/** Sets both minimum and maximum extents at once.
		*/
		void setExtents( const Vector3& min, const Vector3& max );

		void setExtents(
			float mx, float my, float mz,
			float Mx, float My, float Mz );

		/** Returns a pointer to an array of 8 corner points, useful for
		collision vs. non-aligned objects.
		@remarks
		If the order of these corners is important, they are as
		follows: The 4 points of the minimum Z face (note that
		because Ogre uses right-handed coordinates, the minimum Z is
		at the 'back' of the box) starting with the minimum point of
		all, then anticlockwise around this face (if you are looking
		onto the face from outside the box). Then the 4 points of the
		maximum Z face, starting with maximum point of all, then
		anticlockwise around this face (looking onto the face from
		outside the box). Like this:
		<pre>
		1-----2
		/|    /|
		/ |   / |
		5-----4  |
		|  0--|--3
		| /   | /
		|/    |/
		6-----7
		</pre>
		@remarks as this implementation uses a static member, make sure to use your own copy !
		*/
		const Vector3* getAllCorners(void) const;

		/** gets the position of one of the corners
		*/
		Vector3 getCorner(CornerEnum cornerToGet) const;

		friend std::ostream& operator<<( std::ostream& o, const AxisAlignedBox aab )
		{
			switch (aab.mExtent)
			{
			case EXTENT_NULL:
				o << "AxisAlignedBox(null)";
				return o;

			case EXTENT_FINITE:
				o << "AxisAlignedBox(min=" << aab.mMinimum << ", max=" << aab.mMaximum << ")";
				return o;

			case EXTENT_INFINITE:
				o << "AxisAlignedBox(infinite)";
				return o;

			default: // shut up compiler
				PE_ASSERT( false && "Never reached" );
				return o;
			}
		}

		/** Merges the passed in box into the current box. The result is the
		box which encompasses both.
		*/
		void merge( const AxisAlignedBox& rhs );

		/** Extends the box to encompass the specified point (if needed).
		*/
		void merge( const Vector3& point );

		/** Transforms the box according to the matrix supplied.
		@remarks
		By calling this method you get the axis-aligned box which
		surrounds the transformed version of this box. Therefore each
		corner of the box is transformed by the matrix, then the
		extents are mapped back onto the axes to produce another
		AABB. Useful when you have a local AABB for an object which
		is then transformed.
		*/
		void transform( const Matrix4& matrix );

		/** Transforms the box according to the affine matrix supplied.
		@remarks
		By calling this method you get the axis-aligned box which
		surrounds the transformed version of this box. Therefore each
		corner of the box is transformed by the matrix, then the
		extents are mapped back onto the axes to produce another
		AABB. Useful when you have a local AABB for an object which
		is then transformed.
		@note
		The matrix must be an affine matrix. @see Matrix4::isAffine.
		*/
		void transformAffine(const Matrix4& m);

		/** Sets the box to a 'null' value i.e. not a box.
		*/
		void setNull();

		/** Returns true if the box is null i.e. empty.
		*/
		bool isNull(void) const;

		/** Returns true if the box is finite.
		*/
		bool isFinite(void) const;

		/** Sets the box to 'infinite'
		*/
		void setInfinite();

		/** Returns true if the box is infinite.
		*/
		bool isInfinite(void) const;

		/** Returns whether or not this box intersects another. */
		bool intersects(const AxisAlignedBox& b2) const;

		/// Calculate the area of intersection of this box and another
		AxisAlignedBox intersection(const AxisAlignedBox& b2) const;

		/// Calculate the volume of this box
		float volume(void) const;

		/** Scales the AABB by the vector given. */
		void scale(const Vector3& s);

		/** Tests whether this box intersects a sphere. */
		bool intersects(const Sphere& s) const;
		/** Tests whether this box intersects a plane. */
		bool intersects(const Plane& p) const;
		/** Tests whether the vector point is within this box. */
		bool intersects(const Vector3& v) const;
		/// Gets the centre of the box
		Vector3 getCenter(void) const;
		/// Gets the size of the box
		Vector3 getSize(void) const;
		/// Gets the half-size of the box
		Vector3 getHalfSize(void) const;

        /** Tests whether the given point contained by this box.
        */
        bool contains(const Vector3& v) const;

        /** Tests whether another box contained by this box.
        */
        bool contains(const AxisAlignedBox& other) const;

        /** Tests 2 boxes for equality.
        */
        bool operator== (const AxisAlignedBox& rhs) const;

        /** Tests 2 boxes for inequality.
        */
        bool operator!= (const AxisAlignedBox& rhs) const;

		// special values
		static const AxisAlignedBox BOX_NULL;
		static const AxisAlignedBox BOX_INFINITE;


	};

} // namespace ParaEngine

