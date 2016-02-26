#pragma once


namespace ParaEngine
{
	class CShapeBox;
	class CShapeSphere;
	class CShapeOBB;

	/** AABB-related code. (axis-aligned bounding box). Use CShapeBox, if one wants min, max box.*/
	class CShapeAABB
	{
	public:
		CShapeAABB() : mCenter(0.f, 0.f, 0.f), mExtents(-1e33f, -1e33f, -1e33f){};
		CShapeAABB(const CShapeBox& box);
		CShapeAABB(const Vector3& center, const Vector3& extents);
		CShapeAABB(const CShapeAABB* boxes, int nSize);
		CShapeAABB( const Vector3* points, int nSize);
		~CShapeAABB(void);
		
		/**
		*	Setups an AABB from min & max vectors.
		*	\param		min			[in] the min point
		*	\param		max			[in] the max point
		*/
		void		SetMinMax(const Vector3& min, const Vector3& max)		{ mCenter = (max + min)*0.5f; mExtents = (max - min)*0.5f;		}

		/**
		*	Setups an AABB from center & extents vectors.
		*	\param		c			[in] the center point
		*	\param		e			[in] the extents vector
		*/
		void		SetCenterExtents(const Vector3& c, const Vector3& e)	{ mCenter = c;	 mExtents = e;									}

		/**
		*	Setups an empty AABB.
		*/
		void		SetEmpty();

		/** negative extents*/
		void	SetInvalid();

		/**
		*	Setups a point AABB.
		*/
		void		SetPointAABB(const Vector3& pt)							{ mCenter = pt; mExtents=Vector3(0,0,0);}

		/**
		*	Gets the size of the AABB. The size is defined as the longest extent.
		*	\return		the size of the AABB
		*/
		float		GetSize()								const;


		/** get the i  of the 8 corner points 
		*@param i: 0-7 */
		Vector3 Point(int i) const { return Vector3( (i&1)?(mCenter.x-mExtents.x):(mCenter.x+mExtents.x), (i&2)?(mCenter.y-mExtents.y):(mCenter.y+mExtents.y), (i&4)?(mCenter.z-mExtents.z):(mCenter.z+mExtents.z));  }

		/**
		*	Extends the AABB.
		*	\param		p	[in] the next point
		*/
		void		Extend(const Vector3& p);

		/** Extends the AABB.*/
		void Extend(const CShapeOBB& p);

		/** Extends the AABB.*/
		void ExtendByPointList( const Vector3* points, int nSize);

		/** Extends the AABB.*/
		void ExtendByAABBList( const CShapeAABB* boxes, int nSize );

		// Data access

		//! Get min point of the box
		void		GetMin(Vector3& min)						const		{ min = mCenter - mExtents;					}
		//! Get max point of the box
		void		GetMax(Vector3& max)						const		{ max = mCenter + mExtents;					}

		//! Get min point of the box
		Vector3 GetMin()						const		{ return  mCenter - mExtents;					}
		//! Get max point of the box
		Vector3 GetMax()						const		{ return mCenter + mExtents;					}

		//! Get component of the box's min point along a given axis
		float		GetMin(DWORD axis)						const		{ return mCenter[axis] - mExtents[axis];	}
		//! Get component of the box's max point along a given axis
		float		GetMax(DWORD axis)						const		{ return mCenter[axis] + mExtents[axis];	}

		//! Get box center
		void		GetCenter(Vector3& center)				const		{ center = mCenter;							}
		//! Get box extents
		void		GetExtents(Vector3& extents)				const		{ extents = mExtents;						}

		//! Get box center
		void		SetCenter(const Vector3& center)				{ mCenter = center ;							}
		//! Get box extents
		void		SetExtents(const Vector3& extents)				{ mExtents = extents;						}

		//! Get component of the box's center along a given axis
		float		GetCenter(DWORD axis)					const		{ return mCenter[axis];						}
		inline Vector3&		GetCenter()					{ return mCenter;						}
		//! Get component of the box's extents along a given axis
		float		GetExtents(DWORD axis)					const		{ return mExtents[axis];					}
		inline Vector3&		GetExtents()					{ return mExtents;						}

		//! Get box diagonal
		void		GetDiagonal(Vector3& diagonal)			const		{ diagonal = mExtents * 2.0f;				}
		float		GetWidth()								const		{ return mExtents.x * 2.0f;					}
		float		GetHeight()								const		{ return mExtents.y * 2.0f;					}
		float		GetDepth()								const		{ return mExtents.z * 2.0f;					}

		float  GetRadius();

		void SetWidth(float fWidth);
		void SetHeight(float fHeight);
		void SetDepth(float fDepth);

		//! Volume
		float		GetVolume()								const		{ return mExtents.x * mExtents.y * mExtents.z * 8.0f;	}

		/**
		*	Computes the intersection between two AABBs.
		*	\param		a		[in] the other AABB
		*	\return		true on intersection
		*/
		BOOL		Intersect(const CShapeAABB& a)				const;

		/**
		*	The standard intersection method. Just here to check its speed against the one above.
		*	\param		a		[in] the other CShapeAABB
		*	\return		true on intersection
		*/
		bool		GomezIntersect(const CShapeAABB& a);

		/**
		*	Computes the 1D-intersection between two CShapeAABBs, on a given axis.
		*	\param		a		[in] the other CShapeAABB
		*	\param		axis	[in] the axis (0, 1, 2)
		*	\return		true on intersection
		*/
		BOOL		Intersect(const CShapeAABB& a, DWORD axis)	const;

		/** ray AABB intersection 
		* @param pHitSide: 0 negativeX,1 positiveX,2 NZ,3 PZ,4 NY, 5PY
		* @return: true if ray is inside AABB
		*/
		bool Intersect(float* hitDist, const Vector3* origPt, const Vector3* dir, int* pHitSide = NULL);

		/** ray AABB intersection 
		* @param pHitSide: 0 negativeX,1 positiveX,2 NZ,3 PZ,4 NY, 5PY
		* @return: always false if ray is inside AABB
		*/
		bool IntersectOutside(float* hitDist, const Vector3* origPt, const Vector3* dir, int* pHitSide = NULL);

		/**
		*	Recomputes the CShapeAABB after an arbitrary transform by a 4x4 matrix.
		*	\param		mtx			[in] the transform matrix
		*	\param		aabb		[out] the transformed CShapeAABB [can be *this]
		*/
		void		Rotate(const Matrix4& mtx, CShapeAABB& aabb)	const;

		/**
		*	Checks the CShapeAABB is valid.
		*	\return		true if the box is valid
		*/
		BOOL		IsValid()	const;

		//! Operator for CShapeAABB *= float. Scales the extents, keeps same center.
		CShapeAABB&		operator*=(float s)		{ mExtents*=s;	return *this;	}

		//! Operator for CShapeAABB /= float. Scales the extents, keeps same center.
		CShapeAABB&		operator/=(float s)		{ mExtents/=s;	return *this;	}

		//! Operator for CShapeAABB += Vector3. Translates the box.
		CShapeAABB&		operator+=(const Vector3& trans)
		{
			mCenter+=trans;
			return *this;
		}
	private:
		
		Vector3		mCenter;			//!< CShapeAABB Center
		Vector3		mExtents;			//!< x, y and z extents

	};

	/** a min max box.*/
	class CShapeBox
	{
	public:
		Vector3 m_Max;
		Vector3 m_Min;

		CShapeBox(): m_Min(1e33f, 1e33f, 1e33f), m_Max(-1e33f, -1e33f, -1e33f) { }
		CShapeBox( const CShapeBox& other ): m_Min(other.m_Min), m_Max(other.m_Max) { }
		CShapeBox( const Vector3& vMin, const Vector3& vMax): m_Min(vMin), m_Max(vMax) { }
		CShapeBox(const CShapeBox* boxes, int nSize);
		CShapeBox( const Vector3* points, int nSize);
		CShapeBox(const CShapeAABB& other);
		//! Get min point of the box
		inline const Vector3& GetMin()	const					{ return  m_Min;					}
		//! Get max point of the box
		inline const Vector3& GetMax()	const					{ return  m_Max;					}

		/**
		*	Setups a box from min & max vectors.
		*	\param		min			[in] the min point
		*	\param		max			[in] the max point
		*/
		inline void		SetMinMax(const Vector3& min, const Vector3& max)		{ m_Min = min; m_Max = max;}

		/**
		*	Setups a box from center & extents vectors.
		*	\param		c			[in] the center point
		*	\param		e			[in] the extents vector
		*/
		void		SetCenterExtents(const Vector3& c, const Vector3& e);

		/**
		*	Setups an empty box
		*/
		void		SetEmpty();
		/** return false if it is empty. i.e. having negative extents*/
		bool		IsValid();

		/**
		*	Setups a point box.
		*/
		void		SetPointBox(const Vector3& pt)							{ m_Min = m_Max = pt;}

		float		GetWidth()								const		{ return m_Max.x - m_Min.x; }
		float		GetHeight()								const		{ return m_Max.y - m_Min.y; }
		float		GetDepth()								const		{ return m_Max.z - m_Min.z; }

		inline void GetCenter( Vector3* vec) const { *vec = 0.5f*(m_Min+m_Max); }

		inline Vector3 GetCenter() const { return 0.5f*(m_Min+m_Max); }
		inline Vector3 GetExtent() const { return 0.5f*(m_Max-m_Min); }

		/** translate by a offset */
		void Translate(const Vector3& offset);

		/** merge a point to box*/
		void Extend( const Vector3& vec );

		/** Extends the point list.*/
		void Extend( const Vector3* points, int nSize);

		/** Extends the box list.*/
		void Extend( const CShapeBox* boxes, int nSize );

		/** Extends the AABB.*/
		void Extend(const CShapeOBB& p);

		bool Intersect( float* hitDist, const Vector3* origPt, const Vector3* dir ) const;

		/** whether it intersect with a sphere*/
		bool Intersect(const CShapeSphere& sphere) const;

		/**
		// Point-AABB distance based on code from Dave Eberly's Magic Software:
		// Magic Software, Inc. http://www.magic-software.com
		return a rough distance square
		*/
		float DistanceToPoint(const Vector3& p, Vector3* where) const; 

		Vector3 Point(int i) const { return Vector3( (i&1)?m_Min.x:m_Max.x, (i&2)?m_Min.y:m_Max.y, (i&4)?m_Min.z:m_Max.z );  }

	};

}

