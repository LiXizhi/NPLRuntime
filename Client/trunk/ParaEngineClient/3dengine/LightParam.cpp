//-----------------------------------------------------------------------------
// Class:	CLightParam
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4.17
//
// d3d light model reference:
// https://docs.microsoft.com/en-us/windows/desktop/direct3d9/d3dlight9
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "LightParam.h"

/** @def max length of string in CLightParam::ToString()*/
#define MAX_LIGHT_PARAM_STRING_LENGTH	512

using namespace ParaEngine;

CLightParam::CLightParam(void)
{
}

CLightParam::~CLightParam(void)
{
}

void CLightParam::MakeRedPointLight()
{
	Type = D3DLIGHT_POINT;

	Diffuse = LinearColor(1.0f, 0, 1.f, 1.0f);
	Specular = LinearColor(1.0f, 0, 0, 1.0f);
	Ambient = LinearColor(1.0f, 0, 0, 1.0f);

	Position = Vector3(0, 0, 0);
	/* no Direction */
	Direction = Vector3(0, 0, 1);
	/* no Yaw Pitch Roll */
	Yaw = 0;
	Pitch = 0;
	Roll = 0;

	Range = 3.f;
	/* no Falloff */
	Falloff = 1.f;

	Attenuation0 = 0.3f;
	Attenuation1 = 0.1f;
	Attenuation2 = 1.f;

	/* no Theta */
	Theta = 0.8f;
	/* no Phi */
	Phi = 1.0f;
}

void ParaEngine::CLightParam::MakeRedSpotLight()
{
	Type = D3DLIGHT_SPOT;

	Diffuse = LinearColor(1.0f, 1.0f, 0, 1.0f);
	Specular = LinearColor(1.0f, 0, 0, 1.0f);
	Ambient = LinearColor(1.0f, 0, 0, 1.0f);

	Position = Vector3(0, 0, 0);
	Direction = Vector3(0, 0, 1);
	Yaw = 0;
	Pitch = 0;
	Roll = 0;

	Range = 3.0f;
	Falloff = 1.f;

	Attenuation0 = 0.3f;
	Attenuation1 = 0.1f;
	Attenuation2 = 1.f;

	Theta = 0.8f;
	Phi = 1.0f;
}

void ParaEngine::CLightParam::MakeRedDirectionalLight()
{
	Type = D3DLIGHT_DIRECTIONAL;

	Diffuse = LinearColor(1.0f, 0, 0, 1.0f);
	Specular = LinearColor(1.0f, 0, 0, 1.0f);
	Ambient = LinearColor(1.0f, 0, 0, 1.0f);

	/* no Position */
	Position = Vector3(0, 0, 0);
	Direction = Vector3(0, 0, 1);
	Yaw = 0;
	Pitch = 0;
	Roll = 0;

	/* no Range */
	Range = 3.f;
	/* no Falloff */
	Falloff = 1.f;

	/* no Attenuation0 */
	Attenuation0 = 0.3f;
	/* no Attenuation1 */
	Attenuation1 = 0.1f;
	/* no Attenuation2 */
	Attenuation2 = 1.0f;

	/* no Theta */
	Theta = 0.8f;
	/* no Phi */
	Phi = 1.0f;
}

void ParaEngine::CLightParam::RecalculateDirection(const Matrix4* pLocalTransform)
{
	// Yaw - around axis y
	// Pitch - around axis x
	// Roll - around axis z
	Matrix4 mat = Matrix4::IDENTITY;
	Matrix4 transform;

	// the order is roll -> pitch -> yaw
	// calculate the direction using extrinsic rotation
	// TODO: refer to blog - extrinsic and intrinsic
	if (Roll != 0.f)
		mat = (*ParaMatrixRotationZ(&transform, Roll))*mat;
	if (Pitch != 0.f)
		mat = (*ParaMatrixRotationX(&transform, Pitch))*mat;
	if (Yaw != 0.f)
		mat = (*ParaMatrixRotationY(&transform, Yaw))*mat;

	if (pLocalTransform) {
		mat = mat * (*pLocalTransform);
	}
	Direction = InitDirection.TransformNormal(mat);
}

const char* CLightParam::ToString()
{
	static char sParams[MAX_LIGHT_PARAM_STRING_LENGTH + 1];
	memset(sParams, 0, sizeof(sParams));

	snprintf(sParams, MAX_LIGHT_PARAM_STRING_LENGTH, "%d %f (%f %f %f %f) %f %f %f",
		Type, Range, Diffuse.r, Diffuse.g, Diffuse.b, Diffuse.a, Attenuation0, Attenuation1, Attenuation2);
	return sParams;
}

void CLightParam::FromString(const char* str)
{
	try
	{
		int nType = 0;
		sscanf(str, "%d %f (%f %f %f %f) %f %f %f",
			&nType, &Range, &Diffuse.r, &Diffuse.g, &Diffuse.b, &Diffuse.a, &Attenuation0, &Attenuation1, &Attenuation2);
		Type = (D3DLIGHTTYPE)nType;
		// just assume these values. 
		Ambient = LinearColor(0, 0, 0, 0);
		Position = Vector3(0, 0, 0);
	}
	catch (...)
	{
		MakeRedPointLight();
	}
}
