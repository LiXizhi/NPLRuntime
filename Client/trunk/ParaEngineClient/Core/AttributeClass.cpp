//-----------------------------------------------------------------------------
// Class: AttributeClass
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.9
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AttributeClass.h"

using namespace ParaEngine;

CAttributeClass::CAttributeClass(int nClassID, const char* sClassName, const char* sClassDescription)
	:m_nClassID(nClassID), m_sClassName(sClassName), m_sClassDescription(sClassDescription)
	, m_bSort(false)
{

}
int  CAttributeClass::GetClassID() const
{
	return m_nClassID;
}

const char* CAttributeClass::GetClassName() const
{
	return m_sClassName;
}

const char* CAttributeClass::GetClassDescription() const
{
	return m_sClassDescription;
}

void CAttributeClass::AddField(const char*  sFieldname, DWORD Type, void* offsetSetFunc, void* offsetGetFunc, const char* sSchematics, const char* sHelpString, bool bOverride)
{
	auto index = this->GetFieldIndex(sFieldname);
	CAttributeField* pField = nullptr;
	if (index == -1)
	{
		auto size = m_attributes.size();
		m_attributes.resize(size + 1);
		pField = &m_attributes[size];
	}
	else
	{
		if (bOverride)
		{
			pField = &m_attributes[index];
		}
	}

	if (pField)
	{
		pField->SetFieldname(sFieldname);
		pField->m_type = Type;

		pField->m_offsetSetFunc.ptr_fun = offsetSetFunc;
		pField->m_offsetGetFunc.ptr_fun = offsetGetFunc;

		if (sSchematics != NULL)
			pField->m_sSchematics = sSchematics;
		if (sHelpString != NULL)
			pField->m_sHelpString = sHelpString;

		m_bSort = false;
	}

}
void CAttributeClass::AddField_Deprecated(const char *sFieldname, bool bOverride)
{
	CAttributeField field;
	field.SetFieldname(sFieldname);
	field.m_type = FieldType_Deprecated;
	m_attributes.push_back(field);
	m_bSort = false;
}

bool CAttributeClass::RemoveField(const char* sFieldname)
{
	auto index = this->GetFieldIndex(sFieldname);
	if (index == -1)
		return false;

	m_attributes.erase(m_attributes.begin() + index);
	m_bSort = false;
	return true;
}

void CAttributeClass::RemoveAllFields()
{
	m_attributes.clear();
	m_bSort = false;
}

bool CAttributeClass::InsertField(CAttributeField& item, bool bOverride)
{
	CAttributeField* pField = GetField(item.GetFieldname());
	if (pField != 0)
	{
		if (bOverride)
		{
			*pField = item;
			m_bSort = false;
		}
		else
			return false;
	}
	else
	{
		m_attributes.push_back(item);
		m_bSort = false;
	}
	return true;
}

void CAttributeClass::SetOrder(Field_Order order)
{
	if (m_nCurrentOrder != order)
	{
		m_nCurrentOrder = order;
		// TODO: sort by order.
	}
}


void CAttributeClass::SortField()
{
	if (m_bSort && m_attributes.size() > 2)
		return;

	std::sort(m_attributes.begin(), m_attributes.end(), [](const CAttributeField& a, const CAttributeField& b)
	{
		return a.GetHash() < b.GetHash();
	});

	m_bSort = true;
}

int CAttributeClass::GetFieldIndex(const std::string& sFieldname)
{
	SortField();

	auto hash = CAttributeField::HashFunc(sFieldname);

	auto it = std::lower_bound(m_attributes.begin(), m_attributes.end(), hash, [](const CAttributeField& a, size_t hash)
	{
		return a.GetHash() < hash;
	});

	for (; it != m_attributes.end() && it->GetHash() == hash; it++)
	{
		if (it->GetFieldname() == sFieldname)
			return it - m_attributes.begin();
	}

	return -1;
}

int CAttributeClass::GetFieldIndex(const char*  sFieldname)
{
	return GetFieldIndex(std::string(sFieldname));
}

CAttributeField* CAttributeClass::GetField(const std::string& sFieldname)
{
	int index = GetFieldIndex(sFieldname);
	if (index == -1)
		return nullptr;

	return &m_attributes[index];
}

CAttributeField* CAttributeClass::GetField(const char*  sFieldname)
{
	return GetField(std::string(sFieldname));
}
CAttributeClass::Field_Order CAttributeClass::GetOrder()
{
	return m_nCurrentOrder;
}

int CAttributeClass::GetFieldNum()
{
	return (int)m_attributes.size();
}

CAttributeField* CAttributeClass::GetField(int nIndex)
{
	if (nIndex >= 0 && nIndex < GetFieldNum())
	{
		return &(m_attributes[nIndex]);
	}
	else
		return NULL;
}

IAttributeFields* ParaEngine::CAttributeClass::Create()
{
	OUTPUT_LOG("error: The class %s does not support instantiation from attribute class. \n Define ATTRIBUTE_SUPPORT_CREATE_FACTORY(%s); to support it \n", GetClassName(), GetClassName());
	PE_ASSERT(false);
	return NULL;
}

