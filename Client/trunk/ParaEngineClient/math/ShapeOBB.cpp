//-----------------------------------------------------------------------------
// Class:	CShapeOBB
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.12.21
// Desc: Partially, Based on Pierre Terdiman's work in the ODE physics engine.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FPU.h"
#include "ShapeOBB.h"

using namespace ParaEngine;

CShapeOBB::CShapeOBB(void)
:mRot(Matrix4::IDENTITY)
{
}

ParaEngine::CShapeOBB::CShapeOBB(const CShapeAABB& aabb, const Matrix4& mat)
{
	Create(aabb, mat);
}

CShapeOBB::~CShapeOBB(void)
{
}

void CShapeOBB::SetEmpty()
{
	mCenter = Vector3(0,0,0);
	mExtents = Vector3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
	mRot = Matrix4::IDENTITY;
}

void CShapeOBB::Rotate(const Matrix4& mtx, CShapeOBB& obb)	const
{
	// The extents remain constant
	obb.mExtents = mExtents;
	// The center gets x-formed
	obb.mCenter = mCenter * mtx;
	// Combine rotations
	obb.mRot = mRot * mtx;
}

BOOL CShapeOBB::IsValid()	const
{
	// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
	if(mExtents.x < 0.0f)	return FALSE;
	if(mExtents.y < 0.0f)	return FALSE;
	if(mExtents.z < 0.0f)	return FALSE;
	return TRUE;
}

void CShapeOBB::GetRotatedExtents(Matrix4& extents)	const
{
	extents = mRot;
	extents.setScale(Vector3(mExtents.x, mExtents.y, mExtents.z));
}

bool CShapeOBB::ContainsPoint(const Vector3& p) const
{
	// Vector3 in CShapeOBB test using lazy evaluation and early exits

	// Translate to box space
	Vector3 RelPoint = p - mCenter;

	// Vector3 * mRot maps from box space to world space
	// mRot * Vector3 maps from world space to box space (what we need here)

	float f = mRot.m[0][0] * RelPoint.x + mRot.m[0][1] * RelPoint.y + mRot.m[0][2] * RelPoint.z;
	if(f >= mExtents.x || f <= -mExtents.x) return false;

	f = mRot.m[1][0] * RelPoint.x + mRot.m[1][1] * RelPoint.y + mRot.m[1][2] * RelPoint.z;
	if(f >= mExtents.y || f <= -mExtents.y) return false;

	f = mRot.m[2][0] * RelPoint.x + mRot.m[2][1] * RelPoint.y + mRot.m[2][2] * RelPoint.z;
	if(f >= mExtents.z || f <= -mExtents.z) return false;
	return true;
}

void CShapeOBB::Create(const CShapeAABB& aabb, const Matrix4& mat)
{
	// Note: must be coherent with Rotate()

	aabb.GetCenter(mCenter);
	aabb.GetExtents(mExtents);
	// Here we have the same as CShapeOBB::Rotate(mat) where the obb is (mCenter, mExtents, Identity).

	// So following what's done in Rotate:
	// - x-form the center
	mCenter = mCenter * mat;
	// - combine rotation with identity, i.e. just use given matrix
	mRot = mat;
}

bool CShapeOBB::ComputePlanes(Plane* planes)	const
{
	// Checking
	if(!planes)	return false;

	Vector3 Axis0(mRot._11, mRot._12, mRot._13);// = mRot[0];
	Vector3 Axis1(mRot._21, mRot._22, mRot._23);// = mRot[1];
	Vector3 Axis2(mRot._31, mRot._32, mRot._33);// = mRot[2];

	// Writes normals
	planes[0].normal = Axis0;
	planes[1].normal = -Axis0;
	planes[2].normal = Axis1;
	planes[3].normal = -Axis1;
	planes[4].normal = Axis2;
	planes[5].normal = -Axis2;

	// Compute a point on each plane
	Vector3 p0 = mCenter + Axis0 * mExtents.x;
	Vector3 p1 = mCenter - Axis0 * mExtents.x;
	Vector3 p2 = mCenter + Axis1 * mExtents.y;
	Vector3 p3 = mCenter - Axis1 * mExtents.y;
	Vector3 p4 = mCenter + Axis2 * mExtents.z;
	Vector3 p5 = mCenter - Axis2 * mExtents.z;

	// Compute d
	planes[0].d = -planes[0].normal.dotProduct(p0);
	planes[1].d = -planes[1].normal.dotProduct(p1);
	planes[2].d = -planes[2].normal.dotProduct(p2);
	planes[3].d = -planes[3].normal.dotProduct(p3);
	planes[4].d = -planes[4].normal.dotProduct(p4);
	planes[5].d = -planes[5].normal.dotProduct(p5);

	return true;
}

bool CShapeOBB::ComputePoints(Vector3* pts)	const
{
	// Checking
	if(!pts)	return false;

	Vector3 Axis0(mRot._11, mRot._12, mRot._13);// = mRot[0];
	Vector3 Axis1(mRot._21, mRot._22, mRot._23);// = mRot[1];
	Vector3 Axis2(mRot._31, mRot._32, mRot._33);// = mRot[2];

	Axis0 *= mExtents.x;
	Axis1 *= mExtents.y;
	Axis2 *= mExtents.z;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	pts[0] = mCenter - Axis0 - Axis1 - Axis2;
	pts[1] = mCenter + Axis0 - Axis1 - Axis2;
	pts[2] = mCenter + Axis0 + Axis1 - Axis2;
	pts[3] = mCenter - Axis0 + Axis1 - Axis2;
	pts[4] = mCenter - Axis0 - Axis1 + Axis2;
	pts[5] = mCenter + Axis0 - Axis1 + Axis2;
	pts[6] = mCenter + Axis0 + Axis1 + Axis2;
	pts[7] = mCenter - Axis0 + Axis1 + Axis2;

	return true;
}

bool CShapeOBB::ComputeVertexNormals(Vector3* pts)	const
{
	static float VertexNormals[] = 
	{
			-INVSQRT3,	-INVSQRT3,	-INVSQRT3,
			INVSQRT3,	-INVSQRT3,	-INVSQRT3,
			INVSQRT3,	INVSQRT3,	-INVSQRT3,
			-INVSQRT3,	INVSQRT3,	-INVSQRT3,
			-INVSQRT3,	-INVSQRT3,	INVSQRT3,
			INVSQRT3,	-INVSQRT3,	INVSQRT3,
			INVSQRT3,	INVSQRT3,	INVSQRT3,
			-INVSQRT3,	INVSQRT3,	INVSQRT3
	};

	if(!pts)	return false;

	const Vector3* VN = (const Vector3*)VertexNormals;
	for(DWORD i=0;i<8;i++)
	{
		pts[i] = VN[i] * mRot;
	}

	return true;
}

const DWORD* CShapeOBB::GetEdges() const
{
	static DWORD Indices[] = {
		0, 1,	1, 2,	2, 3,	3, 0,
			7, 6,	6, 5,	5, 4,	4, 7,
			1, 5,	6, 2,
			3, 7,	4, 0
	};
	return Indices;
}

const Vector3* CShapeOBB::GetLocalEdgeNormals() const
{
	static float EdgeNormals[] = 
	{
			0,			-INVSQRT2,	-INVSQRT2,	// 0-1
			INVSQRT2,	0,			-INVSQRT2,	// 1-2
			0,			INVSQRT2,	-INVSQRT2,	// 2-3
			-INVSQRT2,	0,			-INVSQRT2,	// 3-0

			0,			INVSQRT2,	INVSQRT2,	// 7-6
			INVSQRT2,	0,			INVSQRT2,	// 6-5
			0,			-INVSQRT2,	INVSQRT2,	// 5-4
			-INVSQRT2,	0,			INVSQRT2,	// 4-7

			INVSQRT2,	-INVSQRT2,	0,			// 1-5
			INVSQRT2,	INVSQRT2,	0,			// 6-2
			-INVSQRT2,	INVSQRT2,	0,			// 3-7
			-INVSQRT2,	-INVSQRT2,	0			// 4-0
	};
	return (const Vector3*)EdgeNormals;
}

void CShapeOBB::ComputeWorldEdgeNormal(DWORD edge_index, Vector3& world_normal) const
{
	PE_ASSERT(edge_index<12);
	world_normal = (GetLocalEdgeNormals()[edge_index]) * mRot;
}

BOOL CShapeOBB::IsInside(const CShapeOBB& box) const
{
	// Make a 4x4 from the box & inverse it
	Matrix4 M0Inv;
	{
		Matrix4 M0 = box.mRot;
		M0.makeTrans(box.mCenter.x, box.mCenter.y, box.mCenter.z);
		M0Inv = M0.inverse();
	}

	// With our inversed 4x4, create box1 in space of box0
	CShapeOBB _1in0;
	Rotate(M0Inv, _1in0);

	// This should cancel out box0's rotation, i.e. it's now an CShapeOBB.
	// => Center(0,0,0), Rot(identity)

	// The two boxes are in the same space so now we can compare them.

	// Create the CShapeOBB of (box1 in space of box0)
	const Matrix4& mtx = _1in0.mRot;

	float f = fabsf(mtx.m[0][0] * mExtents.x) + fabsf(mtx.m[1][0] * mExtents.y) + fabsf(mtx.m[2][0] * mExtents.z) - box.mExtents.x;
	if(f > _1in0.mCenter.x)		return FALSE;
	if(-f < _1in0.mCenter.x)	return FALSE;

	f = fabsf(mtx.m[0][1] * mExtents.x) + fabsf(mtx.m[1][1] * mExtents.y) + fabsf(mtx.m[2][1] * mExtents.z) - box.mExtents.y;
	if(f > _1in0.mCenter.y)		return FALSE;
	if(-f < _1in0.mCenter.y)	return FALSE;

	f = fabsf(mtx.m[0][2] * mExtents.x) + fabsf(mtx.m[1][2] * mExtents.y) + fabsf(mtx.m[2][2] * mExtents.z) - box.mExtents.z;
	if(f > _1in0.mCenter.z)		return FALSE;
	if(-f < _1in0.mCenter.z)	return FALSE;

	return TRUE;
}
