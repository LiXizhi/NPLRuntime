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
	CAttributeField field;
	field.m_sFieldname = sFieldname;
	field.m_type = Type;

	field.m_offsetSetFunc.ptr_fun = offsetSetFunc;
	field.m_offsetGetFunc.ptr_fun = offsetGetFunc;

	if (sSchematics != NULL)
		field.m_sSchematics = sSchematics;
	if (sHelpString != NULL)
		field.m_sHelpString = sHelpString;

	InsertField(field, bOverride);
}
void CAttributeClass::AddField_Deprecated(const char *sFieldname, bool bOverride)
{
	CAttributeField field;
	field.m_sFieldname = sFieldname;
	field.m_type = FieldType_Deprecated;
	m_attributes.push_back(field);
}

bool CAttributeClass::RemoveField(const char* sFieldname)
{
	vector<CAttributeField>::iterator itCur;
	for (itCur = m_attributes.begin(); itCur != m_attributes.end();)
	{
		if ((*itCur).m_sFieldname == sFieldname){
			itCur = m_attributes.erase(itCur);
			return true;
		}
		else
			++itCur;
	}
	return false; // not found matching field
}

void CAttributeClass::RemoveAllFields()
{
	m_attributes.clear();
}

bool CAttributeClass::InsertField(CAttributeField& item, bool bOverride)
{
	CAttributeField* pField = GetField(item.m_sFieldname.c_str());
	if (pField != 0)
	{
		if (bOverride)
			*pField = item;
		else
			return false;
	}
	else
		m_attributes.push_back(item);
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
int CAttributeClass::GetFieldIndex(const char*  sFieldname)
{
	int i = 0;
	vector<CAttributeField>::iterator itCur, itEnd = m_attributes.end();
	for (itCur = m_attributes.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if ((*itCur).m_sFieldname == sFieldname)
			return i;
	}
	return -1;
}

CAttributeField* CAttributeClass::GetField(const char*  sFieldname)
{
	vector<CAttributeField>::iterator itCur, itEnd = m_attributes.end();
	for (itCur = m_attributes.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur).m_sFieldname == sFieldname)
			return &(*itCur);
	}
	return NULL;
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

