// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.
#include "ParaEngine.h"
#include "Triangle.h"
using namespace ParaTerrain;

Triangle::Triangle()
{
}

Triangle::~Triangle()
{
}

void Triangle::DefineFromPoints(Vector3 & p1, Vector3 & p2, Vector3 & p3)
{
	m_pVertices[0].x = p1.x;
	m_pVertices[0].y = p1.y;
	m_pVertices[0].z = p1.z;
	m_pVertices[1].x = p2.x;
	m_pVertices[1].y = p2.y;
	m_pVertices[1].z = p2.z;
	m_pVertices[2].x = p3.x;
	m_pVertices[2].y = p3.y;
	m_pVertices[2].z = p3.z;
#if _USE_RAYTRACING_SUPPORT_ > 0
	m_Plane.DefineFromPoints(p3, p2, p1);
#endif
}

Plane Triangle::ComputePlane()
{
	Plane p;
	Vector3 p1, p2, p3;
	p1.x = m_pVertices[0].x ;
	p1.y = m_pVertices[0].y;
	p1.z = m_pVertices[0].z;
	p2.x = m_pVertices[1].x;
	p2.y = m_pVertices[1].y;
	p2.z = m_pVertices[1].z;
	p3.x = m_pVertices[2].x;
	p3.y = m_pVertices[2].y;
	p3.z = m_pVertices[2].z;

	p.redefine(p3, p2, p1);
	return p;
}

Vector3 *Triangle::GetVertex(int index)
{
	return &m_pVertices[index];
}

#if _USE_RAYTRACING_SUPPORT_ > 0
Plane *Triangle::GetPlane()
{
	return &m_Plane;
}
#endif
