//-----------------------------------------------------------------------------
// Class: AttributeField
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.9, refactored 2015.9.2
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AttributeField.h"

/**@def */
#define MAX_SIMPLE_SCHEMA_STRING_LENGTH	256

using namespace ParaEngine;

CAttributeField::CAttributeField()
	:m_type(FieldType_Deprecated)
{
	m_offsetSetFunc.ptr_fun = NULL;
	m_offsetGetFunc.ptr_fun = NULL;
}

CAttributeField::~CAttributeField()
{

}

const char* CAttributeField::GetTypeAsString()
{
	return CVariable::GetTypeAsString(m_type);
}


#define SIMPLE_SCHEMATYPE_COUNT  6
const char g_schemaType[][20] = {
	":rgb",
	":file",
	":script",
	":int",
	":float",
	":dialog",
};

const char* CAttributeField::GetSimpleSchema(SIMPLE_SCHEMA schema)
{
	return g_schemaType[schema];
}
static char tmp[MAX_SIMPLE_SCHEMA_STRING_LENGTH + 1];

const char* CAttributeField::GetSimpleSchemaOfInt(int nMin, int nMax)
{
	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, MAX_SIMPLE_SCHEMA_STRING_LENGTH, ":int{%d,%d}", nMin, nMax);
	return tmp;
}
const char* CAttributeField::GetSimpleSchemaOfFloat(float fMin, float fMax)
{
	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, MAX_SIMPLE_SCHEMA_STRING_LENGTH, ":float{%f,%f}", fMin, fMax);
	return tmp;
}

const char* CAttributeField::GetSchematicsType()
{
	if (m_sSchematics.empty())
		return CGlobals::GetString(G_STR_EMPTY).c_str();
	else if (m_sSchematics[0] == ':')
	{
		// this is a simple schema, so let us perform further parse 
		int nSize = (int)m_sSchematics.size();
		for (int i = 0; i < SIMPLE_SCHEMATYPE_COUNT; ++i)
		{
			char c = 0;
			bool bFound = true;
			for (int j = 1; j < nSize && (c = g_schemaType[i][j]) != '\0'; ++j)
			{
				if (c != m_sSchematics[j]){
					bFound = false;
					break;
				}
			}
			if (bFound)
			{
				return g_schemaType[i];
			}
		}
	}
	else
	{
		// TODO: parse more advanced schema here.
	}
	return CGlobals::GetString(G_STR_EMPTY).c_str();
}

bool CAttributeField::GetSchematicsMinMax(float& fMin, float& fMax)
{
	float min = -99999999.f;
	float max = 99999999.f;
	int nFrom = (int)m_sSchematics.find_first_of('{');
	int nTo = (int)m_sSchematics.find_last_of('}');
	bool res = false;
	if (nFrom != string::npos && nTo != string::npos && nTo > nFrom)
	{
		string str = m_sSchematics.substr(nFrom, nTo - nFrom + 1);
#ifdef WIN32
		res = _snscanf(str.c_str(), (int)str.size(), "{%f,%f}", &min, &max) >= 2;
#else
		res = sscanf(str.c_str(), "{%f,%f}", &min, &max) >= 2;
#endif
	}
	fMin = min;
	fMax = max;
	return res;
}