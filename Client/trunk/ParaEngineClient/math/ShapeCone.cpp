//-----------------------------------------------------------------------------
// Class:	CShapeCone
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4
// Note: partially based on nvidia sdk sample:PracticalPSM 2005
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"
#include "ShapeCone.h"

using namespace ParaEngine;


CShapeCone::CShapeCone(const CShapeAABB* boxes, int nCount, const Matrix4* projection, const Vector3* _apex, const Vector3* _direction): apex(*_apex), direction(*_direction)
{
	const Vector3 yAxis(0.f, 1.f, 0.f);
	const Vector3 zAxis(0.f, 0.f, 1.f);
	direction.normalise();

	Vector3 axis = yAxis;

	if ( fabsf(yAxis.dotProduct(direction))>0.99f )
		axis = zAxis;

	Vector3 at = apex + direction;
	ParaMatrixLookAtLH(&m_LookAt, &apex, &(at), &axis);

	float maxx = 0.f, maxy = 0.f;
	fNear = 1e32f;
	fFar =  0.f;

	Matrix4 concatMatrix;
	ParaMatrixMultiply(&concatMatrix, projection, &m_LookAt);

	for (int i=0; i<nCount; i++)
	{
		const CShapeAABB& bbox = boxes[i];
		for (int j=0; j<8; j++)
		{
			Vector3 vec = bbox.Point(j);
			vec = vec * concatMatrix;
			maxx = max(maxx, fabsf(vec.x / vec.z));
			maxy = max(maxy, fabsf(vec.y / vec.z));
			fNear = min(fNear, vec.z);
			fFar  = max(fFar, vec.z);
		}
	}
	fovx = atanf(maxx);
	fovy = atanf(maxy);
}

CShapeCone::CShapeCone(const CShapeAABB* boxes, int nCount, const Matrix4* projection, const Vector3* _apex) : apex(*_apex)
{
	const Vector3 yAxis(0.f, 1.f, 0.f);
	const Vector3 zAxis(0.f, 0.f, 1.f);
	const Vector3 negZAxis(0.f, 0.f, -1.f);
	switch (nCount)
	{
	case 0: 
		{
			direction = negZAxis;
			fovx = 0.f;
			fovy = 0.f;
			m_LookAt = Matrix4::IDENTITY;
			break;
		}
	default:
		{
			int i, j;


			//  compute a tight bounding sphere for the vertices of the bounding boxes.
			//  the vector from the apex to the center of the sphere is the optimized view direction
			//  start by xforming all points to post-projective space
			std::vector<Vector3> ppPts;
			ppPts.reserve(nCount * 8);

			for (i=0; i<nCount; i++) 
			{
				for (j=0; j<8; j++)
				{
					Vector3 tmp = boxes[i].Point(j);
					tmp = tmp * (*projection);

					ppPts.push_back(tmp);
				}
			}

			//  get minimum bounding sphere
			CShapeSphere bSphere( (int)ppPts.size(),  &ppPts[0]);

			float min_cosTheta = 1.f;

			direction = bSphere.GetCenter()- apex;
			direction.normalise();

			Vector3 axis = yAxis;

			if ( fabsf(yAxis.dotProduct(direction)) > 0.99f )
				axis = zAxis;

			Vector3 at = apex + direction;
			ParaMatrixLookAtLH(&m_LookAt, &apex, &(at), &axis);

			fNear = 1e32f;
			fFar = 0.f;

			float maxx=0.f, maxy=0.f;
			int nSize = (int)ppPts.size();
			for (i=0; i<nSize; i++)
			{
				Vector3 tmp;
				tmp = ppPts[i] * m_LookAt;
				maxx = max(maxx, fabsf(tmp.x / tmp.z));
				maxy = max(maxy, fabsf(tmp.y / tmp.z));
				fNear = min(fNear, tmp.z);
				fFar  = max(fFar, tmp.z);
			}

			fovx = atanf(maxx);
			fovy = atanf(maxy);
			break;
		}
	} // switch
}

CShapeCone::~CShapeCone(void)
{
}
