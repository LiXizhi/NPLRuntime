#pragma once

#include "PluginManager.h"
#include "PluginAPI.h"

namespace ParaEngine
{
#ifdef WIN32
#ifdef _DEBUG
	static constexpr const char* OCCPlugin_DLL_FILE_PATH = "OCCPlugin_d.dll";
#else
	static constexpr const char* OCCPlugin_DLL_FILE_PATH = "OCCPlugin.dll";
#endif
#elif defined(__APPLE__)
	static constexpr const char* OCCPlugin_DLL_FILE_PATH = "libOCCPlugin.dylib";
#else
	static constexpr const char* OCCPlugin_DLL_FILE_PATH = "libOCCPlugin.so";
#endif

	class CadImporterFactory
	{
		inline static ICadModelImporter* g_pImproter = nullptr;

	public:
		static ICadModelImporter* GetImporter()
		{
			if (g_pImproter)
				return g_pImproter;

			DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(OCCPlugin_DLL_FILE_PATH);
			if (pPluginEntity == 0)
			{
				pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", OCCPlugin_DLL_FILE_PATH);
			}

			if (pPluginEntity != 0)
			{
				for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
				{
					ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

					if (pClassDesc && (strcmp(pClassDesc->ClassName(), "ICadModelImporter") == 0))
					{
						g_pImproter = (ICadModelImporter*)pClassDesc->Create();
					}
				}
			}
			return g_pImproter;
		}
	};
}
