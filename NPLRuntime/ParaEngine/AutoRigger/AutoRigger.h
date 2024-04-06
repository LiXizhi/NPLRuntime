#pragma once

#include "BaseObject.h"

#ifdef ENABLE_AUTO_RIGGER

#include <thread>
class Mesh;

namespace ParaEngine
{
	struct ParaXEntity;
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

		/* callback on scripting side.
		* msg = {count=[0-1], filenames=sFilenames, msg=sMsg}
		* @param sFilenames: colon separated list of filenames
		* @param sMsg: additional message like error message or the filename hint.
		*/
		void On_AddRiggedFile(int nResultCount = 0, const char* sFilenames = 0, const char* sMsg = 0);
		
		
		ATTRIBUTE_METHOD1(CAutoRigger, AddModelTemplate_s, char*) { cls->AddModelTemplate(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, RemoveModelTemplate_s, const char*) { cls->RemoveModelTemplate(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, SetTargetModel_s, const char*) { cls->SetTargetModel(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CAutoRigger, SetOutputFilePath_s, const char*) { cls->SetOutputFilePath(p1); return S_OK; }
		ATTRIBUTE_METHOD(CAutoRigger, AutoRigModel_s) { cls->AutoRigModel(); return S_OK; }

		DEFINE_SCRIPT_EVENT(CAutoRigger, AddRiggedFile);
		
		int InstallFields(CAttributeClass* pClass, bool bOverride);

	private:
		typedef std::map<std::string, ParaXEntity*> ModelTemplateMap;
		ModelTemplateMap::iterator FindBestMatch2(Mesh* targetModel);
		void AutoRigThreadFunc();
		/** bind the target model to the default template which simply has one bone when the system failed to match the target with any given templates. */
		void BindTargetModelDefault();

		ModelTemplateMap* m_ModelTemplates;
		ParaXEntity* m_pTargetModel;
		std::string m_OutputFilePath;
		std::thread m_workerThread;

		std::atomic_bool m_bIsRunnging;
	};
}
#else
namespace ParaEngine
{
	/* empty class 
	*/
	class CAutoRigger : public CBaseObject
	{
	public:
		ATTRIBUTE_DEFINE_CLASS(CAutoRigger);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CAutoRigger);
		CAutoRigger(){};
	};
}
#endif