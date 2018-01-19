//-----------------------------------------------------------------------------
// Class:	CLightParam
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4.17
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

void CLightParam::MakeWhitePointLight()
{
	Type = D3DLIGHT_POINT;
	Ambient = LinearColor(0,0,0,0);
	Diffuse = LinearColor(1.0f, 1.0f, 0,1.0f); // actually yellow light.
	Position = Vector3(0,0,0);
	Attenuation0 = 0.3f;
	Attenuation1 = 0.1f;
	Attenuation2 = 1.f;
	Range = 7.f;
}

const char* CLightParam::ToString()
{
	static char sParams[MAX_LIGHT_PARAM_STRING_LENGTH+1];
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
		Ambient = LinearColor(0,0,0,0);
		Position = Vector3(0,0,0);
	}
	catch ( ... )
	{
		MakeWhitePointLight();
	}
}
