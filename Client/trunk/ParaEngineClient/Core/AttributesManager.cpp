//-----------------------------------------------------------------------------
// Class: CAttributesManager
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.9
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AttributesManager.h"
#include "memdebug.h"

using namespace ParaEngine;



CAttributesManager::CAttributesManager()
{

}

CAttributesManager::~CAttributesManager()
{
	// delete all classes.
	map<int, CAttributeClass*>::iterator itCur, itEnd = m_classes.end();
	for (itCur = m_classes.begin(); itCur!=itEnd; ++itCur)
	{
		delete ((*itCur).second);
	}
	m_classes.clear();

	m_classNames.clear();
}

int CAttributesManager::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass!=NULL);
	pClass->AddField("Print Manual",FieldType_void, NULL, (void*)PrintManual_s, NULL, NULL, bOverride);
	
	return S_OK;
}

CAttributesManager& CAttributesManager::GetSingleton()
{
	static CAttributesManager g_instance;
	return g_instance;
}

void CAttributesManager::AddAttributeClass(int nClassID, CAttributeClass* pClass)
{
	if (pClass)
	{
		m_classes[nClassID] = pClass;
		m_classNames[pClass->GetClassName()] = pClass;
	}
}

CAttributeClass* CAttributesManager::GetAttributeClassByID(int nClassID)
{
	auto iter = m_classes.find(nClassID);
	return (iter != m_classes.end()) ? iter->second : NULL;
}

CAttributeClass* ParaEngine::CAttributesManager::GetAttributeClassByName(const std::string& sClassName)
{
	auto iter = m_classNames.find(sClassName);
	return (iter != m_classNames.end()) ? iter->second : NULL;
}

IAttributeFields* ParaEngine::CAttributesManager::CreateObject(const std::string& sClassName)
{
	CAttributeClass* pClass = GetAttributeClassByName(sClassName);
	if (pClass)
	{
		return pClass->Create();
	}
	else
	{
		CObjectFactory* pFactory = GetObjectFactory(sClassName);
		if (pFactory)
		{
			return pFactory->Create();
		}
	}
	return NULL;
}

bool ParaEngine::CAttributesManager::RegisterObjectFactory(const std::string& sClassName, CObjectFactory* pObjectFactory)
{
	auto iter = m_factory_map.find(sClassName);
	if (iter == m_factory_map.end())
	{
		m_factory_map[sClassName] = pObjectFactory;
		m_object_factories.push_back(pObjectFactory);
		return true;
	}
	else
	{
		OUTPUT_LOG("warn: duplicated call RegisterObjectFactory for class: %s \n", sClassName.c_str());
		return false;
	}
}

CObjectFactory* ParaEngine::CAttributesManager::GetObjectFactory(const std::string& sClassName)
{
	auto iter = m_factory_map.find(sClassName);
	return (iter != m_factory_map.end()) ? (iter->second) : NULL;
}

void CAttributesManager::PrintManual(const string& filepath)
{
	CParaFile file;
	if(file.CreateNewFile(filepath.c_str()))
	{
		OUTPUT_LOG("Attribute manual created at %s", filepath.c_str());

		//////////////////////////////////////////////////////////////////////////
		//
		// output the attribute manager itself (i.e. this pointer).
		//
		//////////////////////////////////////////////////////////////////////////
		CAttributeClass* pClass =  GetAttributeClass();
		if(pClass!=0)
			PrintClass(file, pClass, this);
		
	}
}

void CAttributesManager::PrintObject(const string& filepath, IAttributeFields* pObj)
{
	if(pObj!=NULL)
	{
		CParaFile file;
		if(file.CreateNewFile(filepath.c_str()))
		{
			OUTPUT_LOG("object attribute created at %s", filepath.c_str());
			CAttributeClass* pClass =  pObj->GetAttributeClass();
			if(pClass!=0)
				PrintClass(file, pClass, pObj);
		}
	}
}

void CAttributesManager::PrintClass(CParaFile& file, CAttributeClass* pClass, void* pObj)
{
	if(pClass==0)
		return;
	int nCount = pClass->GetFieldNum();
	file.WriteFormated("\
================================\n\
%s\n\
================================\n\
# field num:%d\n", pClass->GetClassName(), nCount);
	try
	{
		for (int i=0;i<nCount;++i)
		{
			CAttributeField* pField = pClass->GetField(i);
			PrintField(file, pField, pObj);
		}
	}
	catch (...)
	{
		OUTPUT_LOG("error exposing attributes for %s\n", pClass->GetClassName());
	}
}
void CAttributesManager::PrintField(CParaFile& file, CAttributeField* pField, void* pObj)
{
	if(pField==0)
		return;
	file.WriteFormated("%-20s%s\n\t", pField->GetFieldname().c_str(), pField->m_sHelpString.c_str());
	switch(pField->m_type)
	{
	case FieldType_void:
		{
			file.WriteFormated("%-10s\t", "void");
			file.WriteFormated("action");
			break;
		}
	case FieldType_Int:
	case FieldType_DWORD:
	case FieldType_Enum:
		{
			file.WriteFormated("%-10s\t", "int");
			if(pObj!=NULL)
			{
				int value;
				pField->Get(pObj, &value);
				file.WriteFormated("%d", value);
			}
			break;
		}
	case FieldType_Bool:
		{
			file.WriteFormated("%-10s\t", "bool");
			if(pObj!=NULL)
			{
				bool value;
				pField->Get(pObj, &value);
				file.WriteFormated("%s", value?"true":"false");
			}
			break;
		}
	case FieldType_Float:
		{
			file.WriteFormated("%-10s\t", "float");
			if(pObj!=NULL)
			{
				float value;
				pField->Get(pObj, &value);
				file.WriteFormated("%f", value);
			}
			break;
		}
	case FieldType_Float_Float:
		{
			file.WriteFormated("%-10s\t", "float*2");
			if(pObj!=NULL)
			{
				float value[2];
				pField->Get(pObj, &value[0], &value[1]);
				file.WriteFormated("(%f,%f)", value[0], value[1]);
			}
			break;
		}
	case FieldType_Float_Float_Float:
		{
			file.WriteFormated("%-10s\t", "float*3");
			if(pObj!=NULL)
			{
				float value[3];
				pField->Get(pObj, &value[0], &value[1], &value[2]);
				file.WriteFormated("(%f,%f)", value[0], value[1], value[2]);
			}
			break;
		}
	case FieldType_Float_Float_Float_Float:
	{
		file.WriteFormated("%-10s\t", "float*4");
		if (pObj != NULL)
		{
			float value[4];
			pField->Get(pObj, &value[0], &value[1], &value[2], &value[3]);
			file.WriteFormated("(%f,%f)", value[0], value[1], value[2], value[3]);
		}
		break;
	}
	case FieldType_Double:
		{
			file.WriteFormated("%-10s\t", "double");
			if(pObj!=NULL)
			{
				double value;
				pField->Get(pObj, &value);
				file.WriteFormated("%f", value);
			}
			break;
		}
	case FieldType_Vector2:
		{
			file.WriteFormated("%-10s\t", "vector2");
			if(pObj!=NULL)
			{
				Vector2 value;
				pField->Get(pObj, &value);
				file.WriteFormated("<%f,%f>", value.x, value.y);
			}
			break;
		}
	case FieldType_Vector3:
		{
			file.WriteFormated("%-10s\t", "vector3");
			if(pObj!=NULL)
			{
				Vector3 value;
				pField->Get(pObj, &value);
				file.WriteFormated("<%f,%f,%f>", value.x, value.y, value.z);
			}
			break;
		}
	case FieldType_Vector4:
		{
			file.WriteFormated("%-10s\t", "vector4");
			if(pObj!=NULL)
			{
				Vector4 value;
				pField->Get(pObj, &value);
				file.WriteFormated("<%f,%f,%f,%f>", value.x, value.y,value.z, value.w);
			}
			break;
		}
	case FieldType_String:
		{
			file.WriteFormated("%-10s\t", "string");
			if(pObj!=NULL)
			{
				const char* value;
				pField->Get(pObj, &value);
				file.WriteFormated("%s", value);
			}
			break;
		}
	case FieldType_String_Float:
		{
			file.WriteFormated("%-10s\t", "string,float");
			if(pObj!=NULL)
			{
				const char* value1;
				float value2;
				pField->Get(pObj, &value1, &value2);
				file.WriteFormated("%s,%f", value1, value2);
			}
			break;
		}
	case FieldType_String_String:
		{
			file.WriteFormated("%-10s\t", "string,string");
			if(pObj!=NULL)
			{
				const char* value1;
				const char* value2;
				pField->Get(pObj, &value1, &value2);
				file.WriteFormated("%s,%s", value1, value2);
			}
			break;
		}
	case FieldType_Deprecated:
		{
			file.WriteFormated("deprecated");
			break;
		}
	default:
		break;
	}
	
	file.WriteString("\n");
}