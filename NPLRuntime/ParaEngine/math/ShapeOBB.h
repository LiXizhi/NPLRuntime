#pragma once
#include "ShapeAABB.h"

namespace ParaEngine
{
	class Plane;
	class CShapeAABB;

	/** AABB with orientation by a matrix
	*/
	class CShapeOBB
	{
	public:
		CShapeOBB(void);
		CShapeOBB(const CShapeAABB& aabb, const Matrix4& mat);
		~CShapeOBB(void);
		
		/**
		*	Setups an empty CShapeOBB.
		*/
		void SetEmpty();

		/**
		*	Tests if a point is contained within the CShapeOBB.
		*	\param		p	[in] the world point to test
		*	\return		true if inside the CShapeOBB
		*/
		bool ContainsPoint(const Vector3& p)	const;

		/**
		*	Builds an CShapeOBB from an CShapeAABB and a world transform.
		*	\param		aabb	[in] the aabb
		*	\param		mat		[in] the world transform
		*/
		void Create(const CShapeAABB& aabb, const Matrix4& mat);

		/**
		*	Recomputes the CShapeOBB after an arbitrary transform by a 4x4 matrix.
		*	\param		mtx		[in] the transform matrix
		*	\param		obb		[out] the transformed CShapeOBB
		*/
		void Rotate(const Matrix4& mtx, CShapeOBB& obb)	const;

		/**
		*	Checks the CShapeOBB is valid.
		*	\return		true if the box is valid
		*/
		BOOL IsValid()	const;

		/**
		*	Computes the obb planes.
		*	\param		planes	[out] 6 box planes
		*	\return		true if success
		*/
		bool ComputePlanes(Plane* planes)	const;

		/**
		*	Computes the obb points.
		*	\param		pts	[out] 8 box points
		*	\return		true if success
		*/
		bool ComputePoints(Vector3* pts)	const;

		/**
		*	Computes vertex normals.
		*	\param		pts	[out] 8 box points
		*	\return		true if success
		*/
		bool ComputeVertexNormals(Vector3* pts)	const;

		/**
		*	Returns edges.
		*	\return		24 indices (12 edges) indexing the list returned by ComputeD3DXVECTOR3s()
		*/
		const DWORD*		GetEdges()	const;

		/**
		*	Returns local edge normals.
		*	\return		edge normals in local space
		*/
		const Vector3*		GetLocalEdgeNormals()	const;

		/**
		*	Returns world edge normal
		*	\param		edge_index		[in] 0 <= edge index < 12
		*	\param		world_normal	[out] edge normal in world space
		*/
		void ComputeWorldEdgeNormal(DWORD edge_index, Vector3& world_normal)	const;

		/**
		*	Checks the CShapeOBB is inside another CShapeOBB.
		*	\param		box		[in] the other CShapeOBB
		*	\return		TRUE if we're inside the other box
		*/
		BOOL IsInside(const CShapeOBB& box)	const;

		const Vector3&		GetCenter()		const	{ return mCenter;	}
		const Vector3&		GetExtents()	const	{ return mExtents;	}
		const Matrix4&	GetRot()		const	{ return mRot; }

		void GetRotatedExtents(Matrix4& extents)	const;

		Vector3 mCenter;		//!< B for Box
		Vector3 mExtents;		//!< B for Bounding
		Matrix4 mRot;			//!< O for Oriented

		// Orientation is stored in row-major format,
		// i.e. rows = eigen vectors of the covariance matrix
	};

}
