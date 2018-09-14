#pragma once

#include "ParaEngine.h"
#include "BaseObject.h"

class Mesh;

namespace ParaEngine
{

	class ParaXEntity;
	class CParaXModel;
	class CAutoRigger : public CBaseObject
	{
	public:
		ATTRIBUTE_DEFINE_CLASS(CAutoRigger);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CAutoRigger);

		CAutoRigger();
		~CAutoRigger();

		void AddModelTemplate( const char* fileName);
		void RemoveModelTemplate(const char* fileName);
		void SetTargetModel(const char* fileName);
		void SetOutputFilePath(const char* filePath);
		void SetThreshold();
		void AutoRigModel();
		void Clear();

		void On_AddRiggedFile();
		
		
		ATTRIBUTE_METHOD1(CAutoRigger, AddModelTemplate_s, char*) { cls->AddModelTemplate(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, RemoveModelTemplate_s, const char*) { cls->RemoveModelTemplate(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, SetTargetModel_s, const char*) { cls->SetTargetModel(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, SetOutputFilePath_s, const char*) { cls->SetOutputFilePath(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, AutoRigModel_s, const char*) { cls->AutoRigModel(); return S_OK; }

		DEFINE_SCRIPT_EVENT(CAutoRigger, AddRiggedFile);
		
		int InstallFields(CAttributeClass* pClass, bool bOverride);

	private:
		typedef std::map<std::string, ParaXEntity*> ModelTemplateMap;
		ModelTemplateMap::iterator FindBestMatch(Mesh* targetMesh);
		ModelTemplateMap::iterator FindBestMatch2(Mesh* targetModel);
		void AutoRigThreadFunc();

		ModelTemplateMap* m_ModelTemplates;
		ParaXEntity* m_pTargetModel;
		std::string m_OutputFilePath;
	};
}