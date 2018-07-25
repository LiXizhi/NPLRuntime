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
		void SetThreshold();
		void AutoRigModel();
		void Clear();
		
		
		ATTRIBUTE_METHOD1(CAutoRigger, AddModelTemplate_s, char*) { cls->AddModelTemplate(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, RemoveModelTemplate_s, const char*) { cls->RemoveModelTemplate(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, SetTargetModel_s, const char*) { cls->SetTargetModel(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, AutoRigModel_s, const char*) { cls->AutoRigModel(); return S_OK; }
		

		int CAutoRigger::InstallFields(CAttributeClass* pClass, bool bOverride);

	private:
		void AutoRigThreadFunc();

		void Rigging(CParaXModel* targetModel, CParaXModel* skeletonModel, Mesh& newMesh, void* tester);

	private:
		typedef std::map<std::string, ParaXEntity*> ModelTemplateMap;
		ModelTemplateMap* m_ModelTemplates;
		ParaXEntity* m_pTargetModel;
	};
}