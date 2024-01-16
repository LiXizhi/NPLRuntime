//-----------------------------------------------------------------------------
// Class: para ble main
// Authors: big
// Date: 2024.1
// Desc: 
//-----------------------------------------------------------------------------

#include "PluginAPI.h"

#include "INPLRuntimeState.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"
#include "IAttributeFields.h"
#include "NPLInterface.hpp"

using namespace ParaEngine;

#pragma region PE_DLL 

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
    if (nType == ParaEngine::PluginActType_STATE)
    {
        std::cout << "from LibActivate!!!!" << std::endl;
        OUTPUT_LOG("from LibActivate!!!!!!!!");
    }
}

