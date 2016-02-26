

#pragma once

#include "PluginManager.h"
#include "PluginAPI.h"

namespace ParaEngine
{
#ifdef _DEBUG
	const char* OCCPlugin_DLL_FILE_PATH = "OCCPlugin_d.dll";
#else
	const char* OCCPlugin_DLL_FILE_PATH = "OCCPlugin.dll";
#endif

	class CadImporterFactory
	{
		static ICadModelImporter* g_pImproter;

	public:

		static ICadModelImporter* GetImporter()
		{
			if(g_pImproter)
				return g_pImproter;

			DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(OCCPlugin_DLL_FILE_PATH);
			if(pPluginEntity == 0)
			{
				pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", OCCPlugin_DLL_FILE_PATH);
			}

			if(pPluginEntity != 0)
			{
				for (int i=0; i < pPluginEntity->GetNumberOfClasses(); ++i)
				{
					ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

					if(pClassDesc && (strcmp(pClassDesc->ClassName(), "ICadModelImporter") == 0))
					{
						g_pImproter = (ICadModelImporter*) pClassDesc->Create();
					}
				}
			}
			return g_pImproter;
		}
	};

	ICadModelImporter* CadImporterFactory::g_pImproter = NULL;

}