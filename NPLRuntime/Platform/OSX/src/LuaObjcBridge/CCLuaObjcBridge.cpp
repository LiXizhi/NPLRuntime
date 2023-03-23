//-----------------------------------------------------------------------------
// Class: CCLuaObjcBridge.h
// Authors: big
// Emails: onedou@126.com
// CreateDate: 2023.3.23
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "NPLRuntime.h"

#include "CCLuaObjcBridge.h"

namespace ParaEngine
{
    void LuaObjcBridge::nplActivate(const std::string &msg, const std::string &strNPLFileName)
    {
        NPL::NPLRuntimeState_ptr rsptr = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(strNPLFileName);
        NPL::CNPLRuntime::GetInstance()->NPL_Activate(rsptr, strNPLFileName.c_str(), msg.c_str());
    }
}
