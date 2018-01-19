#pragma once
#include "NPLCommon.h"
#include "INPLAcitvationFile.h"

namespace NPL
{
	class INPLRuntimeState;
	
	/** for automatic C function NPL.activate() without registration. 
	* for example, you have a static C function named as below. 
	* must be qualified as extern "C" to avoid the symbol name being mangled.
	* extern "C"{
	*	NPL::NPLReturnCode NPL_activate_script_helloworld_cpp(INPLRuntimeState* pState);
	* }
	* NPL.activate("script/helloworld.cpp", {}) or NPL.activate("script_helloworld.cpp", {}) will both activate it, 
	* however 2 instances of this class object is created.
	*
	* @note: for security reasons, function name must match NPL_activate_XXXX_cpp. 
	*/
	class NPL_C_Func_ActivationFile : public INPLActivationFile
	{
	public:
		typedef NPL::NPLReturnCode(*NPL_Activate_CallbackFunc) (INPLRuntimeState* pState);

		NPL_C_Func_ActivationFile(const std::string& filename);
		virtual ~NPL_C_Func_ActivationFile(){};

		virtual NPL::NPLReturnCode OnActivate(INPLRuntimeState* pState);
	protected:
		void SetFunctionByName(const std::string& filename);
	protected:
		NPL_Activate_CallbackFunc m_pFuncCallBack;
	};

	/** this is a sample code of how to explicitly define a INPLActivationFile handler in C++. 
	*/
	class CNPLFile_Stats : public INPLActivationFile
	{
	public:
		virtual NPL::NPLReturnCode OnActivate(INPLRuntimeState* pState);
	};
}