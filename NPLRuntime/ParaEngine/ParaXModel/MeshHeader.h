#pragma once

namespace ParaEngine
{
	/** mesh header*/
	struct MeshHeader
	{
	public:
		MeshHeader() :m_bIsValid(false), m_bHasNormal(true), m_bHasTex2(false), m_vMin(0, 0, 0), m_vMax(0, 0, 0) {};
	public:
		// set this to true when we have already computed mesh header. 
		bool m_bIsValid;
		bool m_bHasNormal;
		bool m_bHasTex2;
		Vector3 m_vMin;
		Vector3 m_vMax;
	};
}