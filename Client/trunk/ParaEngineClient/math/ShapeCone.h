#pragma once

namespace ParaEngine
{
	class CShapeAABB;
	/** a bounding cone */
	class CShapeCone
	{
	public:
		CShapeCone(): direction(0.f, 0.f, 1.f), apex(0.f, 0.f, 0.f), fovx(0.f), fovy(0.f), fNear(0.001f), fFar(1.f) { }
		CShapeCone(const CShapeAABB* boxes, int nCount, const Matrix4* projection, const Vector3* _apex);
		CShapeCone(const CShapeAABB* boxes, int nCount, const Matrix4* projection, const Vector3* _apex, const Vector3* _direction);
		CShapeCone(const Vector3* points, int nCount, const Vector3* _apex, const Vector3* _direction);
		~CShapeCone(void);
	public:
		Vector3 direction;
		Vector3 apex;
		float       fovy;
		float       fovx;
		float       fNear;
		float       fFar;
		Matrix4  m_LookAt;
	};

}
