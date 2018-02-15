//-----------------------------------------------------------------------------
// Class: DynamicAttributesSet
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.9, refactored 2015.9.2
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DynamicAttributesSet.h"

using namespace ParaEngine;

ParaEngine::CDynamicAttributesSet::CDynamicAttributesSet()
{
}

ParaEngine::CDynamicAttributesSet::~CDynamicAttributesSet()
{
}

int ParaEngine::CDynamicAttributesSet::GetDynamicFieldIndex(const std::string& sName)
{
	auto it = m_name_to_index.find(sName);
	if (it != m_name_to_index.end())
		return it->second;
	else
		return -1;
}

CDynamicAttributeField* ParaEngine::CDynamicAttributesSet::GetDynamicField(const std::string& sName)
{
	int nIndex = GetDynamicFieldIndex(sName);
	return (nIndex >= 0) ? GetDynamicField(nIndex) : NULL;
}

CDynamicAttributeField* ParaEngine::CDynamicAttributesSet::GetDynamicField(int nIndex)
{
	return (nIndex < (int)m_fields.size()) ? (m_fields[nIndex]).get() : 0;
}

const char* ParaEngine::CDynamicAttributesSet::GetDynamicFieldNameByIndex(int nIndex)
{
	auto pField = GetDynamicField(nIndex);
	return pField ? pField->GetName().c_str() : NULL;
}


int ParaEngine::CDynamicAttributesSet::GetDynamicFieldCount()
{
	return (int)m_fields.size();
}

int ParaEngine::CDynamicAttributesSet::SetDynamicField(const std::string& sName, const CVariable& value)
{
	int nResult = 0;
	if (!value.IsNil())
	{
		CDynamicAttributeField* pValue = GetDynamicField(sName);
		if (pValue)
			*pValue = value;
		else
		{
			// add new field
			int nIndex = AddDynamicField(sName, value.GetType());
			if (nIndex >= 0){
				nResult = 1;
				pValue = GetDynamicField(nIndex);
				*pValue = value;
			}
			else
				nResult = -1;
		}
	}
	else
	{
		// remove field
		auto it = m_name_to_index.find(sName);
		if (it != m_name_to_index.end())
		{
			nResult = 2;
			int nIndex = it->second;
			m_name_to_index.erase(it);
			m_fields.erase(m_fields.begin()+nIndex);
		}
	}
	return nResult;
}

int ParaEngine::CDynamicAttributesSet::AddDynamicField(const std::string& sName, ATTRIBUTE_FIELDTYPE dwType)
{
	int nIndex = GetDynamicFieldIndex(sName);
	if (nIndex<0)
	{
		// add new field
		nIndex = (int)m_fields.size();
		m_name_to_index[sName] = nIndex;
		DynamicFieldPtr field(new CDynamicAttributeField(sName, dwType));
		m_fields.push_back(field);
	}
	return nIndex;
}

void ParaEngine::CDynamicAttributesSet::RemoveAllDynamicFields()
{
	m_name_to_index.clear();
	m_fields.clear();
}

