//-----------------------------------------------------------------------------
// Class: IAttributeFields
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.9
// Notes:  
/**
@page ClassAttribute Class Attribute
There are two choices when implementing class properties: 
- one is using __stdcall callback functor (this is the one I used in ParaEngine)
- another is to use class member function pointers with template programming.
Because C++ member function uses __thiscall convention instead of __stdcall, there is no way
to store a pointer to it without explicitly specifying its class type at compile time. In other words,
we can only achieve the same functionality with template programming which is not suitable for our situation.
In case I change my opinion: the following template class may be used std::mem_fun, boost::function, 
also there is open source game engine called delta3D engine who uses the template programming method for its object proxy properties.
For the moment, I use static member function (__stdcall) with the first parameter explicitly declared as the class' this pointer
to emulate the non-static member function. Although this is cumbersome sometimes(programmers need to add two more lines of code in the header file for each property), 
but it works well with overloaded functions, multiple inheritance and virtual functions in case they are used in the attribute fields. 
Most importantly:
- property information only need to be built once(automatically built the first time they are used), and can be used for all instances of the same class type afterwards.
- It also gives programmers individual control over the behaviors of each property.
e.g. we can make the behavior of the property's set/get methods slightly different than the corresponding class member functions, such as 
(1) adding more robust data validation and error reporting functionalities
(2) secretly changing the class's this pointer, so that we can expose properties from both a class' member class and base class.
- It makes the executable smaller than using template programming. 

@note: if multiple inheritance is used, make sure IAttributeFields is the first one in the derived class.
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "IAttributeFields.h"
#include "AttributesManager.h"
#include "util/regularexpression.h"
#include "ParaEngineSettings.h"
#include "DynamicAttributeField.h"
#include "DynamicAttributesSet.h"
#ifdef PARAENGINE_CLIENT
#include "EditorHelper.h"
#endif
#include "AttributeModelProxy.h"

using namespace ParaEngine;

IAttributeFields::IAttributeFields(void)
	:m_pDynamicAttributes(NULL)
{
}

IAttributeFields::~IAttributeFields(void)
{
	SAFE_DELETE(m_pDynamicAttributes);
}

int IAttributeFields::SaveDynamicFieldsToString(string& output)
{
	int nCount = 0;
	CAttributeClass* pClass =  GetAttributeClass();
	if(pClass!=0)
	{
		int nCount = GetDynamicFieldCount();
		try
		{
			for (int i=0;i<nCount;++i)
			{
				CDynamicAttributeField* pField = GetDynamicField(i);
				const char* sFieldName = GetDynamicFieldNameByIndex(i);
				if(pField)
				{
					output.append(sFieldName);
					output.append(" = ");
					output.append((const string&)(*pField));
					output.append("\n");
					nCount++;
				}
			}
		}
		catch (...)
		{
			OUTPUT_LOG("error SaveDynamicFieldsToString for %s\r\n", pClass->GetClassName());
			nCount = -1;
		}
	}
	return nCount;
}

int IAttributeFields::LoadDynamicFieldsFromString(const string& input)
{
	int nCount = 0;
	string field_name;
	string field_value;
	string line;
	
	regex re_line("([^\r\n]+)\r?\n?");
	regex re("(\\w+)\\s*=\\s*(.*)");
	sregex_iterator iter(input.begin(), input.end(), re_line), iterEnd;
	for (auto it = iter; it != iterEnd; it++)
	{
		line = (*it).str(1);
		smatch result;
		if(regex_search(line, result, re))
		{
			field_name = result.str(1);
			field_value = result.str(2);
			CVariable value;
			value = field_value;
			SetDynamicField(field_name, value);
			nCount++;
		}
	}

	return nCount;
}

void IAttributeFields::PrintObject()
{
	string filepath = "temp/doc/";
	filepath +=GetAttributeClassName();
	filepath +=".txt";
	CParaFile file;
	if(file.CreateNewFile(filepath.c_str()))
	{
		OUTPUT_LOG("object attribute created at %s\r\n", filepath.c_str());
		CAttributeClass* pClass =  GetAttributeClass();
		if(pClass!=0)
			CGlobals::GetAttributesManager()->PrintClass(file, pClass, this);
	}
}

CAttributeClass* ParaEngine::IAttributeFields::CreateAttributeClass()
{
	return new CAttributeClass(GetAttributeClassID(), GetAttributeClassName(), GetAttributeClassDescription());
}

CDynamicAttributesSet* ParaEngine::IAttributeFields::GetDynamicAttributes(bool bCreateIfNotExist)
{
	if (m_pDynamicAttributes)
		return m_pDynamicAttributes;
	else if (bCreateIfNotExist)
	{
		m_pDynamicAttributes= new CDynamicAttributesSet();
		return m_pDynamicAttributes;
	}
	return NULL;
}

CAttributeClass* IAttributeFields::GetAttributeClass()
{
	auto nClassId = GetAttributeClassID();
	CAttributeClass* pClass = CAttributesManager::GetSingleton().GetAttributeClassByID(nClassId);
	if(pClass==NULL)
	{
		pClass = CreateAttributeClass();
		if (pClass != 0)
		{
			CAttributesManager::GetSingleton().AddAttributeClass(nClassId, pClass);
			InstallFields(pClass, true);
		}
		else
		{
			OUTPUT_LOG("error: attribute type creation failed.\r\n");
		}
	}
	PE_ASSERT(pClass!=NULL);
	return pClass;
}

bool IAttributeFields::OpenWithDefaultEditor( const char* sScriptFile, bool bWaitOnReturn)
{
#ifdef PARAENGINE_CLIENT
	return CEditorHelper::OpenWithDefaultEditor(sScriptFile, bWaitOnReturn);
#else
	return true;
#endif
}

CDynamicAttributeField* ParaEngine::IAttributeFields::GetDynamicField(const std::string& sName)
{
	auto pAttrs = GetDynamicAttributes();
	return (pAttrs) ? pAttrs->GetDynamicField(sName) : NULL;
}

CDynamicAttributeField* ParaEngine::IAttributeFields::GetDynamicField( int nIndex )
{
	auto pAttrs = GetDynamicAttributes();
	return (pAttrs) ? pAttrs->GetDynamicField(nIndex) : NULL;
}

const char* ParaEngine::IAttributeFields::GetDynamicFieldNameByIndex(int nIndex)
{
	auto pAttrs = GetDynamicAttributes();
	return (pAttrs) ? pAttrs->GetDynamicFieldNameByIndex(nIndex) : NULL;
}

int ParaEngine::IAttributeFields::GetDynamicFieldCount()
{
	auto pAttrs = GetDynamicAttributes();
	return (pAttrs) ? pAttrs->GetDynamicFieldCount() : 0;
}

int ParaEngine::IAttributeFields::SetDynamicField(const std::string& sName, const CVariable& value)
{
	auto pAttrs = GetDynamicAttributes(true);
	if (pAttrs)
	{
		return pAttrs->SetDynamicField(sName, value);
	}
	return -1;
}

int ParaEngine::IAttributeFields::AddDynamicField(const std::string& sName, ATTRIBUTE_FIELDTYPE dwType)
{
	auto pAttrs = GetDynamicAttributes(true);
	if (pAttrs)
	{
		return pAttrs->AddDynamicField(sName, dwType);
	}
	return -1;
}

void ParaEngine::IAttributeFields::RemoveAllDynamicFields()
{
	auto pAttrs = GetDynamicAttributes();
	if (pAttrs)
		pAttrs->RemoveAllDynamicFields();
}

int ParaEngine::IAttributeFields::GetAttributeClassID()
{
	static int nLastID = ATTRIBUTE_CLASSID_Last + 1000;
	static std::map<std::string, int> s_auto_class_id;

	auto it = s_auto_class_id.find(GetAttributeClassName());
	if (it != s_auto_class_id.end())
		return it->second;
	else
	{
		s_auto_class_id[GetAttributeClassName()] = nLastID;
		int nID = nLastID;
		nLastID++;
		return nID;
	}
}

int ParaEngine::IAttributeFields::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return 0;
}

IAttributeFields* ParaEngine::IAttributeFields::GetChildAttributeObject(const std::string& sName)
{
	return NULL;
}

IAttributeFields* ParaEngine::IAttributeFields::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	return NULL;
}

int ParaEngine::IAttributeFields::GetChildAttributeColumnCount()
{
	return 1;
}

const std::string& ParaEngine::IAttributeFields::GetIdentifier()
{
	return CGlobals::GetString(0);
}

void ParaEngine::IAttributeFields::SetIdentifier(const std::string& sID)
{
}

bool ParaEngine::IAttributeFields::AddChildAttributeObject(IAttributeFields* pChild, int nRowIndex /*= -1*/, int nColumnIndex /*= 0*/)
{
	return false;
}

void* ParaEngine::IAttributeFields::QueryObjectByName(const std::string& sObjectType)
{
	if (sObjectType == "IAttributeFields")
		return this;
	else
		return NULL;
}

void* ParaEngine::IAttributeFields::QueryObject(int nObjectType)
{
	if (nObjectType == ATTRIBUTE_CLASSID_IAttributeFields)
		return this;
	else
		return NULL;
}

int ParaEngine::IAttributeFields::GetTime()
{
	return 0;
}

void ParaEngine::IAttributeFields::SetTime(int nTime)
{
}

int IAttributeFields::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// TODO: install parent fields if there are any. Please replace __super with your parent class name.
	//__super::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass != NULL);
	pClass->AddField("ClassID", FieldType_Int, NULL, (void*)IAttributeFields::GetAttributeClassID_s, NULL, NULL, bOverride);
	pClass->AddField("PrintMe", FieldType_void, (void*)PrintObject_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ClassName", FieldType_String, NULL, (void*)IAttributeFields::GetAttributeClassName_s, NULL, NULL, bOverride);
	pClass->AddField("name", FieldType_String, (void*)SetName_s, (void*)GetName_s, NULL, "identifier of the object", bOverride);
	pClass->AddField("addref", FieldType_void, (void*)AddRef_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("RefCount", FieldType_Int, NULL, (void*)IAttributeFields::GetRefCount_s, NULL, NULL, bOverride);
	pClass->AddField("release", FieldType_void, (void*)Release_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("Time", FieldType_Int, (void*)SetTime_s, (void*)GetTime_s, NULL, NULL, bOverride);
	return S_OK;
}