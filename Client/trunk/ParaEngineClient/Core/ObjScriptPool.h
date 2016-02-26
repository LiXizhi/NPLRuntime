#pragma once
#include <string>
#include <list>
#include "util/ParaMemPool.h"

namespace ParaEngine
{
	using namespace std;
	
	template <class OBJ_TYPE>
	class Object_Script {
	public:
		int m_objType;
		string m_srcFile;
		string m_code;
		OBJ_TYPE* m_pObj;
		bool m_bDelete;
		Object_Script()
		{
			m_objType=0;m_pObj=NULL;m_bDelete=false;
		}
	};

	template <class OBJ_TYPE>
	class ObjectScriptPool
	{
	public:
		typedef Object_Script<OBJ_TYPE> OBJ_SCRIPT;
		typedef FixedSizedAllocator<OBJ_SCRIPT>	DL_Allocator_OBJ_SCRIPT;
		typedef std::list<OBJ_SCRIPT, DL_Allocator_OBJ_SCRIPT> OBJ_SCRIPT_List_Type;

		enum GUI_SCRIPT_TYPE{
			NONE=0,
			ONLOAD,ONMOUSEDOWN,ONMOUSEUP,ONMOUSEWHEEL,ONMOUSEHOVER,ONMOUSEMOVE,
			ONMOUSEENTER,ONMOUSELEAVE,ONCLICK,ONDOUBLECLICK,ONDRAGBEGIN,ONDRAGMOVE,
			ONDRAGEND,ONCHAR,ONKEYDOWN,ONKEYUP,ONCHANGE,ONSTRING,ONFOCUSIN,ONFOCUSOUT,
			ONSELECT,
			ONEVENT
		};

		void init(){m_pScripts.clear();	};
		ObjectScriptPool(){init();};
		~ObjectScriptPool(){};
		void AddScript(const string& srcFile,int etype){
			m_pScripts.push_back(OBJ_SCRIPT());
			OBJ_SCRIPT& script = m_pScripts.back();
			script.m_code="";
			script.m_objType=etype;
			script.m_pObj=NULL;
			script.m_srcFile=srcFile;
		};
		
		void AddScript(const string&  srcFile, int etype, const string&  code){
			m_pScripts.push_back(OBJ_SCRIPT());
			OBJ_SCRIPT& script = m_pScripts.back();
			script.m_code=code;
			script.m_objType=etype;
			script.m_pObj=NULL;
			script.m_srcFile=srcFile;
		};
		
		void AddScript(const string&  srcFile, int etype, const string&  code, OBJ_TYPE* obj){
			m_pScripts.push_back(OBJ_SCRIPT());
			OBJ_SCRIPT& script = m_pScripts.back();
			script.m_code=code;
			script.m_objType=etype;
			script.m_pObj=obj;
			script.m_srcFile=srcFile;
		};

		void AddScript(OBJ_SCRIPT* script){
			AddScript(script->m_srcFile,script->m_objType,script->m_code,script->m_pObj);
		};

		OBJ_SCRIPT*	GetScript(int etype){	
			for (m_iter=m_pScripts.begin();m_iter!=m_pScripts.end();m_iter++) {
				if ((*m_iter).m_objType==etype) {
					return &(*m_iter);
				}
			}
			return NULL;
		};

		OBJ_SCRIPT*	GetScript(int etype, OBJ_TYPE* obj){	
			for (m_iter=m_pScripts.begin();m_iter!=m_pScripts.end();m_iter++) {
				if ((*m_iter).m_objType==etype && (*m_iter).m_pObj == obj) 
				{
					return &(*m_iter);
				}
			}
			return NULL;
		};

		OBJ_SCRIPT* GetNextScript(int etype){	
			for (;m_iter!=m_pScripts.end();m_iter++) {
				if ((*m_iter).m_objType==etype) {
					return &(*m_iter);
				}
			}
			return NULL;
		};

		OBJ_SCRIPT*	GetScript(OBJ_TYPE* pObj){	
			for (m_iter=m_pScripts.begin();m_iter!=m_pScripts.end();m_iter++) {
				if ((*m_iter).m_pObj==pObj) {
					return &(*m_iter);
				}
			}
			return NULL;
		};

		OBJ_SCRIPT* GetNextScript(OBJ_TYPE* pObj){	
			for (;m_iter!=m_pScripts.end();m_iter++) {
				if ((*m_iter).m_pObj==pObj) {
					return &(*m_iter);
				}
			}
			return NULL;
		};

		OBJ_SCRIPT&	operator[](DWORD index){
			if (index<0||index>=m_pScripts.size()) {
				return m_pScripts.front();
			}
			int a=0;
			typename OBJ_SCRIPT_List_Type::iterator iter;
			for (iter=m_pScripts.begin();(iter!=m_pScripts.end())&&(a!=index);a++,iter++);
			return (*iter);
		};

		void		DeleteScript(OBJ_TYPE* obj){
			typename OBJ_SCRIPT_List_Type::iterator iter;
			for (iter=m_pScripts.begin();iter!=m_pScripts.end();) {
				if ((*iter).m_pObj==obj) {
					iter=m_pScripts.erase(iter);
				}else iter++;
			}
		};

		void		DeleteScript(int etype){
			typename OBJ_SCRIPT_List_Type::iterator iter;
			for (iter=m_pScripts.begin();iter!=m_pScripts.end();) {
				if ((*iter).m_objType==etype) {
					iter=m_pScripts.erase(iter);
				}else iter++;
			}

		};

		void		DeleteMarkedScript(){
			typename OBJ_SCRIPT_List_Type::iterator iter;
			for (iter=m_pScripts.begin();iter!=m_pScripts.end();) {
				if ((*iter).m_bDelete) {
					iter=m_pScripts.erase(iter);
				}else iter++;
			}
		};

		void		ActivateScript(const string& code){};
		size_t		size(){
			return m_pScripts.size();
		};
		void		ClearAll(){m_pScripts.clear();};

	private:
		OBJ_SCRIPT_List_Type m_pScripts;
		typename OBJ_SCRIPT_List_Type::iterator m_iter;
	};
}
