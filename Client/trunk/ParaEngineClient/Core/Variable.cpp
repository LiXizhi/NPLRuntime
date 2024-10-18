//-----------------------------------------------------------------------------
// Class: Variable
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.9.2
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLHelper.h"
#include "IAnimated.h"
#include "Variable.h"

using namespace ParaEngine;

/*
Buffer size required to be passed to _gcvt, fcvt and other fp conversion routines
*/
#ifndef	_CVTBUFSIZE
#define _CVTBUFSIZE (309+40) /* # of digits in max. dp value + slop */
#endif

ParaEngine::CVariable::CVariable(DWORD dwType) :m_type(FieldType_unknown), m_pAnimated(NULL)
{
	ChangeType((ATTRIBUTE_FIELDTYPE)dwType);
}

ParaEngine::CVariable::~CVariable()
{
	SAFE_DELETE(m_pAnimated);
}

void ParaEngine::CVariable::Clone(const CVariable& value)
{
	ChangeType(value.GetType());
	m_strVal = value.m_strVal;
	memcpy(m_vector4, value.m_vector4, sizeof(float) * 4);
	if (value.m_pAnimated)
	{
		// TODO: copy animated data. 
		PE_ASSERT(false);
	}
}

bool ParaEngine::CVariable::IsNil() const
{
	return m_type == FieldType_unknown;
}

bool ParaEngine::CVariable::IsStringType() const
{
	return m_type == FieldType_String;
}

bool ParaEngine::CVariable::IsAnimated()
{
	return m_type >= FieldType_Animated;
}

void ParaEngine::CVariable::ChangeType(ATTRIBUTE_FIELDTYPE newType)
{
	if (m_type != newType)
	{
		if (m_type == FieldType_String)
			m_strVal.clear();
		if (m_pAnimated)
		{
			// NOT supported;
			SAFE_DELETE(m_pAnimated);
		}
		m_type = newType;

		if (IsAnimated())
		{
			if (m_type == FieldType_AnimatedQuaternion)
			{
				m_pAnimated = new AnimatedVariable<Quaternion>();
			}
			else if (m_type == FieldType_AnimatedVector3)
			{
				m_pAnimated = new AnimatedVariable<Vector3>();
			}
			else if (m_type == FieldType_AnimatedVector2)
			{
				m_pAnimated = new AnimatedVariable<Vector2>();
			}
			else if (m_type == FieldType_AnimatedFloat)
			{
				m_pAnimated = new AnimatedVariable<float>();
			}
		}
	}
}

const char* ParaEngine::CVariable::GetTypeAsString(DWORD dwType)
{
	switch (dwType)
	{
	case FieldType_void:
		return "void";
		break;
	case FieldType_Int:
		return "int";
		break;
	case FieldType_Bool:
		return "bool";
		break;
	case FieldType_Float:
		return "float";
		break;
	case FieldType_Float_Float:
		return "float_float";
		break;
	case FieldType_Float_Float_Float:
		return "float_float_float";
		break;
	case FieldType_Float_Float_Float_Float:
		return "float_float_float_float";
		break;
	case FieldType_Enum:
		return "enum";
		break;
	case FieldType_Double:
		return "double";
		break;
	case FieldType_Vector2:
		return "vector2";
		break;
	case FieldType_Vector3:
		return "vector3";
		break;
	case FieldType_DVector3:
		return "dvector3";
		break;
	case FieldType_Vector4:
		return "vector4";
		break;
	case FieldType_Quaternion:
		return "quaternion";
		break;
	case FieldType_String:
		return "string";
		break;
	case FieldType_DWORD:
		return "DWORD";
		break;
	case FieldType_String_String:
		return "string_string";
		break;
	case FieldType_String_Float:
		return "string_float";
		break;
	case FieldType_Deprecated:
		return "deprecated";
		break;
	default:
		return "";
		break;
	}
}

const char* ParaEngine::CVariable::GetTypeName() const
{
	return GetTypeAsString((DWORD)m_type);
}

ParaEngine::CVariable::operator double()
{
	if (m_type == FieldType_Double)
		return m_doubleVal;
	else if (m_type == FieldType_Float)
		return m_floatVal;
	else if (m_type == FieldType_Int || m_type == FieldType_DWORD || m_type == FieldType_Enum)
		return m_intVal;
	else if (m_type == FieldType_String)
	{
		double value = atof(m_strVal.c_str());
		return value;
	}
	else
		return 0;
}

ParaEngine::CVariable::operator float()
{
	if (m_type == FieldType_Float)
		return m_floatVal;
	else if (m_type == FieldType_Double)
		return (float)m_doubleVal;
	else if (m_type == FieldType_Int || m_type == FieldType_DWORD || m_type == FieldType_Enum)
		return (float)m_intVal;
	else if (m_type == FieldType_String)
	{
		double value = atof(m_strVal.c_str());
		return (float)value;
	}
	else
		return 0;
}

ParaEngine::CVariable::operator int()
{
	if (m_type == FieldType_Int || m_type == FieldType_DWORD || m_type == FieldType_Enum)
		return m_intVal;
	else if (m_type == FieldType_Double)
		return (int)m_doubleVal;
	else if (m_type == FieldType_Float)
		return (int)m_floatVal;
	else if (m_type == FieldType_String)
	{
		int value = atoi(m_strVal.c_str());
		return value;
	}
	else
		return 0;
}

ParaEngine::CVariable::operator DWORD()
{
	if (m_type == FieldType_DWORD || m_type == FieldType_Int || m_type == FieldType_Enum)
		return (DWORD)m_intVal;
	else if (m_type == FieldType_Double)
		return (DWORD)m_doubleVal;
	else if (m_type == FieldType_Float)
		return (DWORD)m_floatVal;
	else if (m_type == FieldType_String)
	{
		int value = atoi(m_strVal.c_str());
		return value;
	}
	else
		return 0;
}

ParaEngine::CVariable::operator Vector3()
{
	if (m_type == FieldType_Vector3)
		return Vector3(m_vector3[0], m_vector3[1], m_vector3[2]);
	else
		return Vector3::ZERO;
}

ParaEngine::CVariable::operator Quaternion()
{
	if (m_type == FieldType_Quaternion)
		return Quaternion(m_vector4);
	else
		return Quaternion::ZERO;
}

ParaEngine::CVariable::operator bool()
{
	if (m_type == FieldType_String)
	{
		return (m_strVal == "true");
	}
	return m_boolVal;
}

ParaEngine::CVariable::operator const string&()
{
	if (m_type == FieldType_String)
		return m_strVal;
	else if (m_type == FieldType_Bool)
	{
		m_strVal = (m_boolVal) ? "true" : "false";
	}
	else if (m_type == FieldType_Int || m_type == FieldType_DWORD || m_type == FieldType_Enum)
	{
		char tmp[128];
		int value = (int)m_intVal;
		snprintf(tmp, 128, "%d", value);
		m_strVal = tmp;
	}
	else if (m_type == FieldType_Double)
	{
		char tmp[128];
		float value = (float)m_doubleVal;
		snprintf(tmp, 128, "%f", value);
		m_strVal = tmp;
	}
	else if (m_type == FieldType_Float)
	{
		char tmp[128];
		float value = m_floatVal;
		snprintf(tmp, 128, "%f", value);
		m_strVal = tmp;
	}
	return m_strVal;
}

ParaEngine::CVariable::operator const char*()
{
	return operator const string&().c_str();
}

void ParaEngine::CVariable::operator=(int value)
{
	ChangeType(FieldType_Int);
	m_intVal = value;
}

void ParaEngine::CVariable::operator=(float value)
{
	ChangeType(FieldType_Float);
	m_floatVal = value;
}

void ParaEngine::CVariable::operator=(const Quaternion& value)
{
	ChangeType(FieldType_Quaternion);
	memcpy(m_vector4, &value, sizeof(Quaternion));
}

void ParaEngine::CVariable::operator=(const Vector3& value)
{
	ChangeType(FieldType_Vector3);
	memcpy(m_vector3, &value, sizeof(Vector3));
}

void ParaEngine::CVariable::operator=(double value)
{
	ChangeType(FieldType_Double);
	m_doubleVal = value;
}

void ParaEngine::CVariable::operator=(bool value)
{
	ChangeType(FieldType_Bool);
	m_boolVal = value;
}

bool ParaEngine::CVariable::operator==(const string& value)
{
	return m_strVal == value;
}

bool ParaEngine::CVariable::operator==(const char* value)
{
	return m_strVal == value;
}

void ParaEngine::CVariable::operator=(const std::string& value)
{
	ChangeType(FieldType_String);
	m_strVal = value;
}

void ParaEngine::CVariable::operator=(const char* value)
{
	ChangeType(FieldType_String);
	if (value)
		m_strVal = value;
	else
		m_strVal.clear();
}

void ParaEngine::CVariable::ToNPLString(std::string& output)
{
	switch (m_type)
	{
	case FieldType_Bool:
	{
		output.append(m_boolVal ? "true" : "false");
		break;
	}
	case FieldType_Int:
	case FieldType_DWORD:
	case FieldType_Enum:
	{
		char tmp[101];
		output.append(itoa((int)m_intVal, tmp, 10));
		break;
	}
	case FieldType_Float:
	{
		char temp[_CVTBUFSIZE];
		output.append(_gcvt(m_floatVal, 15, temp));
		break;
	}
	case FieldType_Double:
	{
		char temp[_CVTBUFSIZE];
		output.append(_gcvt(m_doubleVal, 15, temp));
		break;
	}
	case FieldType_String:
	{
		NPL::NPLHelper::EncodeStringInQuotation(output, (int)output.size(), m_strVal);
		break;
	}
	default:
		output.append("nil");
		break;
	}
}

void ParaEngine::CVariable::operator=(const CVariable& val)
{
	Clone(val);
}

int ParaEngine::CVariable::GetNumKeys()
{
	return m_pAnimated ? m_pAnimated->GetNumKeys() : 1;
}

void ParaEngine::CVariable::SetNumKeys(int nKeyCount)
{
	if (m_pAnimated)
		m_pAnimated->SetNumKeys(nKeyCount);
}

int ParaEngine::CVariable::AddKey(int nTime, bool* isKeyExist)
{
	return (m_pAnimated) ? m_pAnimated->AddKey(nTime, isKeyExist) : 0;
}

void ParaEngine::CVariable::SetTime(int nIndex, int nTime)
{
	if (m_pAnimated)
		m_pAnimated->SetTime(nIndex, nTime);
}

int ParaEngine::CVariable::GetTime(int nIndex)
{
	return (m_pAnimated) ? m_pAnimated->GetTime(nIndex) : 0;
}


bool ParaEngine::CVariable::GetValue(int nIndex, Quaternion& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValue(nIndex, val) : false;
}

bool ParaEngine::CVariable::GetValueByTime(int nTime, float& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValueByTime(nTime, val) : false;
}

bool ParaEngine::CVariable::GetValueByTime(int nTime, double& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValueByTime(nTime, val) : false;
}

bool ParaEngine::CVariable::GetValueByTime(int nTime, Vector3& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValueByTime(nTime, val) : false;
}

bool ParaEngine::CVariable::GetValueByTime(int nTime, Quaternion& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValueByTime(nTime, val) : false;
}

void ParaEngine::CVariable::SetValue(int nIndex, float val)
{
	if (m_pAnimated)
		m_pAnimated->SetValue(nIndex, val);
}

bool ParaEngine::CVariable::GetValue(int nIndex, float& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValue(nIndex, val) : false;
}

void ParaEngine::CVariable::SetValue(int nIndex, const Vector3& val)
{
	if (m_pAnimated)
		m_pAnimated->SetValue(nIndex, val);
	else
		operator=(val);
}

bool ParaEngine::CVariable::GetValue(int nIndex, Vector3& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValue(nIndex, val) : false;
}

bool ParaEngine::CVariable::GetValue(int nIndex, std::string& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValue(nIndex, val) : false;
}

bool ParaEngine::CVariable::GetValue(int nIndex, double& val)
{
	return (m_pAnimated) ? m_pAnimated->GetValue(nIndex, val) : false;
}

void ParaEngine::CVariable::SetValue(int nIndex, const Quaternion& val)
{
	if (m_pAnimated)
		m_pAnimated->SetValue(nIndex, val);
	else
		operator=(val);
}

void ParaEngine::CVariable::SetValue(int nIndex, const std::string& val)
{
	if (m_pAnimated)
		m_pAnimated->SetValue(nIndex, val);
	else
		operator=(val);
}

void ParaEngine::CVariable::SetValue(int nIndex, double val)
{
	if (m_pAnimated)
		m_pAnimated->SetValue(nIndex, val);
	else
		operator=(val);
}

int ParaEngine::CVariable::GetNextKeyIndex(int nTime)
{
	return m_pAnimated ? m_pAnimated->GetNextKeyIndex(nTime) : 0 ;
}
