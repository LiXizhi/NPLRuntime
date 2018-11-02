
#pragma once

namespace ParaEngine
{
	class IObjectScriptingInterface
	{
	public:
		IObjectScriptingInterface();

		/** the game object will keep a list of script call backs.*/
		struct ScriptCallback 
		{
			/// function type 
			int func_type;
			/// the NPL file and function to be activated. Its format is similar to GUI events. 
			/// e.g. "(gl)script/NPC1.lua;NPC1.On_Click();"
			std::string script_func;
			std::string m_sFilename;
			std::string m_sCode;
			/// last time this function is called. 
			unsigned int m_nLastTick;
		public:
			ScriptCallback(int type, const std::string& script): func_type(type), m_nLastTick(0){
				SetScriptFunc(script);
			};
			ScriptCallback(): func_type(-1), m_nLastTick(0) {};

			/** set the script function */
			void SetScriptFunc(const std::string& script);

			const std::string& GetCode(){ return m_sCode;}
			const std::string& GetFileName(){ return m_sFilename;}
			const std::string& GetScriptFunc(){ return script_func;}

			unsigned int GetLastTick() const { return m_nLastTick;}
			void SetLastTick(unsigned int nTick) {m_nLastTick = nTick;}

			/** activate the script callback with the given code. */
			int ActivateAsync(const std::string&  code);
			/** activate the script callback locally now.  when function returns, the script has returned.*/
			int ActivateLocalNow(const std::string& script);
		};
		typedef std::vector<ScriptCallback> ObjectCallbackPool_Type;

#ifndef DEFINE_SCRIPT_EVENT
#define DEFINE_SCRIPT_EVENT(clsName, name) \
	ATTRIBUTE_METHOD1(clsName, Get##name##_s, const char**){\
	ScriptCallback* pCallBack = cls->GetScriptCallback(Type_##name);\
	*p1 = (pCallBack!=NULL) ? pCallBack->script_func.c_str() : CGlobals::GetString(G_STR_EMPTY).c_str();\
	return S_OK;\
		}\
		ATTRIBUTE_METHOD1(clsName, Set##name##_s, const char*)		{cls->AddScriptCallback(Type_##name, p1); return S_OK;}

#endif
#ifndef DEFINE_SCRIPT_EVENT_GET
#define DEFINE_SCRIPT_EVENT_GET(clsName, name) \
	ATTRIBUTE_METHOD1(clsName, Get##name##_s, const char**){\
	ScriptCallback* pCallBack = cls->GetScriptCallback(Type_##name);\
	*p1 = (pCallBack!=NULL) ? pCallBack->script_func.c_str() : CGlobals::GetString(G_STR_EMPTY).c_str();\
	return S_OK;\
		}
#endif
	public:
		/** add a new call back handler. it will override the previous one if any.
		@param script_func: format is [filename];[scode] where file name can be any NPL address, scode is a short code sequence to execute.
		it may has the same format as the GUI event handler. 
		e.g. ";function1()" : calling a global function 
		"(gl)script/character/npc.lua;npc.on_click()" : load script if not loaded before and then calling a global function 
		if this is "", RemoveScriptCallback() will be called. 
		*/
		virtual bool AddScriptCallback(int func_type, const string& script_func);
	
		/** return NULL if there is no associated script. */
		virtual ScriptCallback* GetScriptCallback(int func_type);
	
		/** remove a call back handler*/
		virtual bool RemoveScriptCallback(int func_type);

	protected:
		/** the NPL script handlers */
		ObjectCallbackPool_Type m_callback_scripts;
	};
}