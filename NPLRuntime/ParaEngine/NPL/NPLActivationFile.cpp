//-----------------------------------------------------------------------------
// Class:	NPL activation file
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2015.3.3
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLTable.h"
#include "NPLHelper.h"
#include "INPLRuntimeState.h"
#include "util/os_calls.h"
#include "NPLActivationFile.h"

using namespace NPL;

extern "C"
{
	/** to load the lib, please call:
	NPL.call("protocol/pb.cpp", {});
	NPL.activate("protocol/pb.cpp");
	*/
	extern PE_CORE_DECL NPL::NPLReturnCode NPL_activate_protocol_pb_cpp(NPL::INPLRuntimeState* pState);
	extern PE_CORE_DECL NPL::NPLReturnCode NPL_activate_script_serialport_cpp(NPL::INPLRuntimeState* pState);

#ifdef ANDROID
	/** to load the lib, please call:
	NPL.call("LuaJavaBridge.cpp", {});
	NPL.activate("LuaJavaBridge.cpp");
	*/
	PE_CORE_DECL NPL::NPLReturnCode NPL_activate_LuaJavaBridge_cpp(NPL::INPLRuntimeState* pState);
#endif

#if TARGET_OS_IPHONE
	PE_CORE_DECL NPL::NPLReturnCode NPL_activate_LuaObjcBridge_cpp(NPL::INPLRuntimeState* pState);
#endif
}





NPL::NPL_C_Func_ActivationFile::NPL_C_Func_ActivationFile(const std::string& filename) : m_pFuncCallBack(0)
{
	// Load callback table
	m_callbackTable["NPL_activate_protocol_pb_cpp"] = (NPL_Activate_CallbackFunc)&NPL_activate_protocol_pb_cpp;
	
#ifdef ANDROID
	m_callbackTable["NPL_activate_LuaJavaBridge_cpp"] = (NPL_Activate_CallbackFunc)&NPL_activate_LuaJavaBridge_cpp;
#endif
	 
#if TARGET_OS_IPHONE
	m_callbackTable["NPL_activate_LuaObjcBridge_cpp"] = (NPL_Activate_CallbackFunc)&NPL_activate_LuaObjcBridge_cpp;
#endif

#ifdef EMSCRIPTEN
	m_callbackTable["NPL_activate_script_serialport_cpp"] = (NPL_Activate_CallbackFunc)&NPL_activate_script_serialport_cpp;
#endif
	SetFunctionByName(filename);
}

NPL::NPLReturnCode NPL::NPL_C_Func_ActivationFile::OnActivate(INPLRuntimeState* pState)
{
	if (m_pFuncCallBack){
		return m_pFuncCallBack(pState);
	}
	return NPL_OK;
}

void NPL::NPL_C_Func_ActivationFile::SetFunctionByName(const std::string& filename)
{
	std::string c_func_name = "NPL_activate_";
	c_func_name += filename;
	for (int i = 12; i < (int)c_func_name.size(); ++i)
	{
		char c = c_func_name[i];
		if (c == '/' || c == '\\' || c=='.')
			c_func_name[i] = '_';
	}

	if (m_callbackTable.find(c_func_name) != m_callbackTable.end())
	{
		m_pFuncCallBack = m_callbackTable[c_func_name];
	}

	if (!m_pFuncCallBack)
	{
		m_pFuncCallBack = (NPL_Activate_CallbackFunc)ParaEngine::GetProcAddress(CLIB_DEFHANDLE, c_func_name.c_str());
	}
	
	if (!m_pFuncCallBack){
		OUTPUT_LOG("warning: file %s not found, %s undefined\n", filename.c_str(), c_func_name.c_str());
	}
}

extern "C"
{
	/* example of IMPLICITLY define a function that is callable via NPL
	* NPL.activate("script/helloworld.cpp", {type="log", text="hello world C++ file!"})*/
	PE_CORE_DECL NPL::NPLReturnCode NPL_activate_script_helloworld_cpp(INPLRuntimeState* pState)
	{
		auto msg = NPLHelper::MsgStringToNPLTable(pState->GetCurrentMsg(), pState->GetCurrentMsgLength());
		std::string debug_str;
		NPL::NPLHelper::NPLTableToString("msg", msg, debug_str);
		OUTPUT_LOG("NPL_activate_nplcall_cpp: %s \n", debug_str.c_str());

		std::string sType = msg["type"];
		std::string sAction = msg["action"];
		if (sType == "log")
		{
			std::string text = msg["text"];
			OUTPUT_LOG("helloworld.cpp: %s \n", text.c_str());
		}
		return NPL_OK;
	};
}


/** example of EXPLICITLY define a function that is callable via NPL.
NPL side:
NPL.activate("stats.cpp", {type="log", text="hello world C++ file!"});
*/
NPL::NPLReturnCode CNPLFile_Stats::OnActivate(INPLRuntimeState* pState)
{
	return NPL_activate_script_helloworld_cpp(pState);
}
