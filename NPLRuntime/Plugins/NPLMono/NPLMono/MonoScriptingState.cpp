//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: Cross platformed NPL Mono plugin for ParaEngine
//-----------------------------------------------------------------------------
#include "NPLMono.h"

// ParaEngine includes
#include "PluginAPI.h"
#include "ParaScriptingGlobal.h"
#include "MonoScriptingState.h"
#include "util/os_calls.h"
#include <util/mutex.h>
#include "NPLMonoInterface.h"
#include "NPLMonoInterface_Bind.h"

using namespace ParaEngine;
ParaEngine::mutex g_mutex;
MonoDomain *	CMonoScriptingState::s_mono_domain = NULL;

//////////////////////////////////////////////////////////////////////////
//
// GlobalAssembliesPool
//
//////////////////////////////////////////////////////////////////////////

GlobalAssembliesPool::GlobalAssembliesPool()
:m_mono_domain(NULL)
{
	
}
GlobalAssembliesPool::~GlobalAssembliesPool()
{
	// safe delete all reference data
	Assembly_File_Map_Type::iterator itCur, itEnd = m_assemblies.end();
	for(itCur = m_assemblies.begin(); itCur != itEnd; ++itCur)
	{
		AssemblyFile* pAssembly = itCur->second;
		if(pAssembly)
		{
			Class_Map_Type::iterator itCur1, itEnd1 = pAssembly->m_classes.end();
			for(itCur1 = pAssembly->m_classes.begin(); itCur1 != itEnd1; ++itCur1)
			{
				ClassObject* pObject =  itCur1->second;
				if(pObject)
					delete pObject;
			}
			pAssembly->m_classes.clear();
			if(pAssembly)
				delete pAssembly;
		}
	}
	m_assemblies.clear();
}

GlobalAssembliesPool& GlobalAssembliesPool::GetSingleton()
{
	static GlobalAssembliesPool s_instance;
	return s_instance;
}

ClassObject* GlobalAssembliesPool::GetClass(const string& filePath)
{
	ParaEngine::Lock lock_(m_mutex);

	if(GetMonoDomain() == NULL)
	{
		APP_LOG("warning: NPLMono domain is not initialized");
		return NULL;
	}

	int nLastSlash = filePath.find_last_of('/');
	if(nLastSlash!=string::npos)
	{
		string assembly_file_name = filePath.substr(0, nLastSlash);
		int nLastDot = filePath.find_last_of('.');
		if(nLastDot!=string::npos && nLastSlash<nLastDot)
		{
			string full_class_name = filePath.substr(nLastSlash+1, nLastDot - nLastSlash-1);

			// Load the assembly_file_name if not loaded. 
			AssemblyFile * pFile = NULL;
			Assembly_File_Map_Type::iterator iter = m_assemblies.find(assembly_file_name);
			if(iter != m_assemblies.end())
			{
				pFile = iter->second;
			}
			else
			{
				MonoAssembly * assembly = mono_domain_assembly_open (GetMonoDomain(), assembly_file_name.c_str());
				MonoImage * image = NULL;
				if (!assembly)
				{
					OUTPUT_LOG("error: failed loading .Net assemby file %s\n", assembly_file_name.c_str());
				}
				else
				{
					image = mono_assembly_get_image (assembly);
					OUTPUT_LOG("NPLMono: assemby file %s is loaded\n", assembly_file_name.c_str());
				}
				pFile = new AssemblyFile();
				pFile->m_assembly = assembly;
				pFile->m_image = image;
				m_assemblies[assembly_file_name] = pFile;
			}

			if(pFile && pFile->m_image)
			{
				// Load the class if not loaded.
				ClassObject* pClass = NULL;
				Class_Map_Type::iterator iter1 = pFile->m_classes.find(full_class_name);
				if(iter1 != pFile->m_classes.end())
				{
					pClass = iter1->second;
				}
				else
				{
					// get the namespace and class name
					// if file is "file.dll/MyNameSpace.MyClassName.cs"
					string namespace_name, class_name;
					nLastDot = full_class_name.find_last_of('.');
					if(nLastDot!=string::npos)
					{
						namespace_name = full_class_name.substr(0, nLastDot);
						class_name = full_class_name.substr(nLastDot+1);
					}
					else
					{
						class_name = full_class_name;
					}

					MonoClass * klass = mono_class_from_name (pFile->m_image, namespace_name.c_str(), class_name.c_str());
					MonoMethod * method_activate = NULL;
					if (!klass) 
					{
						OUTPUT_LOG("error: can't find %s{%s} in assembly %s\n", namespace_name.c_str(), class_name.c_str(), assembly_file_name.c_str());
					}
					else
					{
						OUTPUT_LOG("Class loaded: %s{%s} in assembly %s \n", namespace_name.c_str(), class_name.c_str(), assembly_file_name.c_str());
						gpointer iter3 = NULL;
						MonoMethod *method = NULL, *m = NULL;

						/* retrieve all the methods we need */
						while ((m = mono_class_get_methods (klass, &iter3)))
						{
							if (strcmp (mono_method_get_name (m), "activate") == 0) 
							{
								method_activate = m;
							}
						}
					}

					pClass = new ClassObject();
					pClass->m_namespace_name = namespace_name;
					pClass->m_class_name = class_name;
					pClass->m_class = klass;
					pClass->m_activate_function = method_activate;
					pFile->m_classes[full_class_name] = pClass;
				}

				return pClass;
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//
// CMonoScriptingState
//
//////////////////////////////////////////////////////////////////////////

CMonoScriptingState::CMonoScriptingState()
:m_npl_runtime_state(NULL)
{
	m_global_assemblies_pool = &(GlobalAssembliesPool::GetSingleton());
}

CMonoScriptingState::~CMonoScriptingState()
{
	DestroyState();
}

void CMonoScriptingState::Release()
{
	delete this;
}

bool CMonoScriptingState::CreateState(const char* name, NPL::INPLRuntimeState* pState)
{
	ParaEngine::Lock lock_(g_mutex);
	if(name!=0)
		m_name = name;

	SetNPLRuntimeState(pState);

	// create the mono domain
	CreateAppDomain();
	
	/** The official doc says:
	* If your applications has threads that will access Mono, access Mono variables, point to Mono objects, 
	* be called back by Mono, these threads must be registered with the Mono runtime using the mono_thread_attach
	*/
	mono_thread_attach(GetMonoDomain());
	OUTPUT_LOG("mono_thread_attach(thread id:%d) for state %s\n", ParaEngine::GetThisThreadID(), m_name.c_str());

	return true;
}

void CMonoScriptingState::DestroyState()
{
	ParaEngine::Lock lock_(g_mutex);
	// delete mono domain. 
	if(GetMonoDomain()!=0)
	{
		mono_jit_cleanup(GetMonoDomain());
		OUTPUT_LOG("mono jit cleaned up(thread id:%d) for state %s\n", ParaEngine::GetThisThreadID(), m_name.c_str());
		s_mono_domain = NULL;
	}
}

void CMonoScriptingState::CreateAppDomain()
{
	/*
	* Load the default Mono configuration file, this is needed
	* if you are planning on using the dllmaps defined on the
	* system configuration
	*/
	if(s_mono_domain == NULL)
	{
#ifdef WIN32
		// In windows, mono .net library cannot be automatically found. 
		string mono_lib_path = MONO_ROOT;
		mono_lib_path += "\\lib\\";
		for(int i=0;i<mono_lib_path.size(); i++)
		{
			if(mono_lib_path[i] == '/')
				mono_lib_path[i] = '\\';
		}

		string mono_etc_path = MONO_ROOT;
		mono_etc_path += "\\etc";
		for(int i=0;i<mono_etc_path.size(); i++)
		{
			if(mono_etc_path[i] == '/')
				mono_etc_path[i] = '\\';
		}

		OUTPUT_LOG("mono_set_dirs: \n   lib:%s\n   etc:%s\n", mono_lib_path.c_str(), mono_etc_path.c_str());
		mono_set_dirs(mono_lib_path.c_str(), mono_etc_path.c_str());
#else
		// for linux, mono/lib/ and mono/etc directories can be found automatically. 
#endif
		mono_config_parse(NULL);

		const char* mono_jit_version_ = "v2.0.50727";
		/*
		* mono_jit_init() creates a domain: each assembly is
		* loaded and run in a MonoDomain with version 2.0.
		*/
		s_mono_domain = mono_jit_init_version ("NPLMono_Root", mono_jit_version_);

		// Set root mono domain in to which all assemblies are loaded. 
		m_global_assemblies_pool->SetMonoDomain(s_mono_domain);

		// add all internal calls 
		Init();

		// mono jit init version
		OUTPUT_LOG("mono_jit_init_version %s\n", mono_jit_version_);
	}
}

const char* CMonoScriptingState::GetName() const 
{
	return m_name.c_str();
}

void CMonoScriptingState::SetNPLRuntimeState(NPL::INPLRuntimeState* pState)
{
	m_npl_runtime_state = pState;
}

NPL::INPLRuntimeState* CMonoScriptingState::GetNPLRuntimeState()
{
	return m_npl_runtime_state;
}

void CMonoScriptingState::Init()
{
	LoadNPLLib();
	LoadParaLib();
}

bool CMonoScriptingState::IsScriptFileLoaded(const string& filepath)
{
	return false;
}

bool CMonoScriptingState::LoadFile(const string& filePath, bool bReload)
{
	// never load files, we will only load at activation time. 
	return true;
}

int CMonoScriptingState::DoString(const char* sCall, int nLength)
{
	// TODO: JIT for any code is not supported at the moment. 
	return 0;
}

NPL::NPLReturnCode CMonoScriptingState::ActivateFile(const string& filepath, const char * code, int nLength)
{
	ClassObject* pClass = m_global_assemblies_pool->GetClass(filepath);
	
	if(pClass && pClass->m_activate_function)
	{
		// NOW we will call the activate function. 
		MonoObject* exception = NULL;
		gpointer args [2];
		int nType = ParaEngine::PluginActType_STATE;
		/*MonoString* sCode = NULL;
		if(nLength > 0)
			sCode = mono_string_new_len(GetMonoDomain(), code, nLength);
		else
			sCode = mono_string_new (GetMonoDomain(), (code!=0) ? code : "");*/
		void* npl_runtime_state = GetNPLRuntimeState();
		args [0] = &nType;
		args [1] = &npl_runtime_state;

		MonoObject* result = mono_runtime_invoke (pClass->m_activate_function, NULL, args, &exception);
		if (exception) 
		{
			OUTPUT_LOG("warning: An exception was thrown when activating %s\n", filepath.c_str());
		}
		else
		{
			/* mono_runtime_invoke () always boxes the return value if it's a valuetype */
			// int nResult = *(int*)mono_object_unbox (result);
			return NPL::NPL_OK;
		}
	}
	return NPL::NPL_Error;
}

//////////////////////////////////////////////////////////////////////////
//
// Note: add all wrapper internal calls here
//
//////////////////////////////////////////////////////////////////////////
void CMonoScriptingState::LoadNPLLib()
{
	Add_NPL_Internal_Call();
}

void CMonoScriptingState::LoadParaLib()
{
	/*
	* We add our API internal call, so that C# code can call us back.
	*/
	// TODO:
}