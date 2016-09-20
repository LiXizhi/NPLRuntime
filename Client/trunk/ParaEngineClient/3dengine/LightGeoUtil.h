#pragma once
#include <string>
#include <vector>
#include "ParaVector3.h"
namespace ParaEngine
{
	class LightGeomUtil
	{
	public:
		// Create a sphere Mesh with a given radius, number of rings and number of segments
		static void createSphere(std::vector<Vector3> & outPositions
			, std::vector<unsigned short> & outIndices
			, float radius
			, int nRings, int nSegments
			);

		// Create a cone Mesh with a given radius and number of vertices in base
		// Created cone will have its head at 0,0,0, and will 'expand to' positive y
		static void createCone(std::vector<Vector3> & outPositions
			, std::vector<unsigned short> & outIndices
			, float radius
			, float height
			, int nVerticesInBase);

		// Fill up a fresh copy of outPositions with a normalized quad
		static void createQuad(std::vector<Vector3> & outPositions);


	};
}