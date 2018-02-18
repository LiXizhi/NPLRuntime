//----------------------------------------------------------------------
// Class:	effect file base class
// Authors:	LiXizhi
// Date:	2014.10.3
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "effect_file.h"

using namespace ParaEngine;

void CEffectFileBase::onDrawPass(CParameterBlock* pMaterialParams, int passIndex)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_DrawPass);
	if (pCallback)
	{
		thread_local static string code;
		code.clear();
		if (pMaterialParams && pMaterialParams->GetParameter("CallbackKey"))
		{
			code += "callback_key=";
			code += pMaterialParams->GetParameter("CallbackKey")->GetValueByString();
			code += ";";
		}
		else
			code += "callback_key=-1;";
		code += "pass_index=";
		thread_local static string pass_index_str;
		thread_local static stringstream ss;
		ss.clear();
		ss << passIndex;
		ss >> pass_index_str;
		code += pass_index_str;
		code += ";";
		code += pCallback->GetCode();
		pCallback->ActivateLocalNow(code);
	}
}

