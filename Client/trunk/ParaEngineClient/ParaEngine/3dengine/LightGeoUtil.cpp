//-----------------------------------------------------------------------------
// Class:	Light Geometry Util
// Authors:	devilwalk, lixizhi
// Company: 
// Date:	2016.9.18
// desc	: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "LightGeoUtil.h"

using namespace ParaEngine;
void LightGeomUtil::createSphere(std::vector<Vector3> & outPositions
	, std::vector<unsigned short> & outIndices
	, float radius
	, int nRings, int nSegments)
{
	outPositions.clear();
	outIndices.clear();

	outPositions.resize((nRings + 1) * (nSegments + 1));
	float* pVertex = &outPositions[0].x;

	outIndices.resize(6 * nRings * (nSegments + 1));
	unsigned short* pIndices = &outIndices[0];

	float fDeltaRingAngle = (Math::PI / nRings);
	float fDeltaSegAngle = (2 * Math::PI / nSegments);
	unsigned short wVerticeIndex = 0;

	// Generate the group of rings for the sphere
	for (int ring = 0; ring <= nRings; ring++) {
		float r0 = radius * sinf(ring * fDeltaRingAngle);
		float y0 = radius * cosf(ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for (int seg = 0; seg <= nSegments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

			if (ring != nRings)
			{
				// each vertex (except the last) has six indices pointing to it
				*pIndices++ = wVerticeIndex + nSegments + 1;
				*pIndices++ = wVerticeIndex;
				*pIndices++ = wVerticeIndex + nSegments;
				*pIndices++ = wVerticeIndex + nSegments + 1;
				*pIndices++ = wVerticeIndex + 1;
				*pIndices++ = wVerticeIndex;
				wVerticeIndex++;
			}
		}; // end for seg
	} // end for ring
}

void LightGeomUtil::createQuad(std::vector<Vector3> & outPositions)
{
	outPositions.clear();
	outPositions.resize(4);
	// Upload data
	float data[] = {
		-1, 1, -1,  // corner 1
		-1, -1, -1, // corner 2
		1, 1, -1,   // corner 3
		1, -1, -1 }; // corner 4
	memcpy(&outPositions[0], data, sizeof(data));
}

void LightGeomUtil::createCone(std::vector<Vector3> & outPositions
	, std::vector<unsigned short> & outIndices
	, float radius, float height, int nVerticesInBase)
{
	outPositions.clear();
	outIndices.clear();

	outPositions.resize(nVerticesInBase + 1);
	float* pVertex = &outPositions[0].x;

	outIndices.resize((3 * nVerticesInBase) + (3 * (nVerticesInBase - 2)));
	unsigned short* pIndices = &outIndices[0];

	//Positions : cone head and base
	for (int i = 0; i<3; i++)
		*pVertex++ = 0.0f;

	//Base :
	float fDeltaBaseAngle = (2 * Math::PI) / nVerticesInBase;
	for (int i = 0; i<nVerticesInBase; i++)
	{
		float angle = i * fDeltaBaseAngle;
		*pVertex++ = radius * cosf(angle);
		*pVertex++ = height;
		*pVertex++ = radius * sinf(angle);
	}

	//Indices :
	//Cone head to vertices
	for (int i = 0; i<nVerticesInBase; i++)
	{
		*pIndices++ = 0;
		*pIndices++ = (i%nVerticesInBase) + 1;
		*pIndices++ = ((i + 1) % nVerticesInBase) + 1;
	}
	//Cone base
	for (int i = 0; i<nVerticesInBase - 2; i++)
	{
		*pIndices++ = 1;
		*pIndices++ = i + 3;
		*pIndices++ = i + 2;
	}
}
