#pragma once
#include "TerrainCommon.h"

namespace ParaTerrain
{
	using namespace ParaEngine;
	class Triangle
	{
	public:
		Triangle();
		~Triangle();
		void DefineFromPoints(Vector3 & p1, Vector3 & p2, Vector3 & p3);
		Vector3 *GetVertex(int index);
		Plane *GetPlane();
		Plane ComputePlane();

	private:
		Vector3 m_pVertices[3];
#if _USE_RAYTRACING_SUPPORT_ > 0
		Plane m_Plane;
#endif
	};
}