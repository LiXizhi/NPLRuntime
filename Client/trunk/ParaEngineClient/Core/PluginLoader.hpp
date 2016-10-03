#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Corporation, All Rights Reserved.
// Author:	LiXizhi
// Date:	2010.2
// Desc:	Use this file to load dll, if one wants to link to core lib at runtime. 
//			PE_MODULE_STANDALONE is usually defined with delayed core lib. 
//-----------------------------------------------------------------------------
#include <vector>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

#include "PluginAPI.h"

#ifndef OUTPUT_LOG
#define OUTPUT_LOG printf
#endif
namespace ParaEngine 
{
	using std::vector;
	// forward declarations.
	class ClassDescriptor;
	class IParaEngineCore;

	//////////////////////////////////////////////////////////////////////////
	// plug-in library functions to be explicitly imported to the game engine. 

	/** "LibDescription": must be implemented in a plug-in. */
	typedef const char* (*lpFnLibDescription)();
	/** "LibVersion": must be implemented in a plug-in. */
	typedef unsigned long (*lpFnLibVersion)();
	/** "LibNumberClasses": must be implemented in a plug-in. */
	typedef int (*lpFnLibNumberClasses)();
	/** "LibClassDesc": must be implemented in a plug-in. */
	typedef ClassDescriptor* (*lpFnLibClassDesc)(int i);
	/** "LibInit": this is optional in a plug-in */
	typedef void (*lpFnLibInit)();
	/** "LibInitParaEngine": this is optional in a plug-in */
	typedef void (*lpFnLibInitParaEngine)(IParaEngineCore* pIParaEngineCore);
	/** "LibActivate": this is optional in a plug-in */
	typedef int (*lpFnLibActivate)(int nType, void* pVoid);
		
	typedef void (STDCALL *pfnEnsureInit)(void);
	typedef void (STDCALL *pfnForceTerm)(void);

	/**
	*  a DLL/so plug-in loaded explicitly at runtime. This is a header only file. 
	*/
	class CPluginLoader
	{
	public:
		/** the dll to load, it does not load it immediately. */
		CPluginLoader(const char* filename = NULL)
			: m_pFuncActivate(NULL), m_hDLL(NULL), m_bIsValid(false), m_bIsInitialized(false)
		{
			if(filename)
			{
				m_sDllFilePath = filename;
			}
		}
		virtual ~CPluginLoader()
		{
			// free the library.
			if(IsValid())
			{
				UnLoad();
			}
		}

		/** whether the plugin loaded is valid. */
		bool IsValid()
		{
			return (m_hDLL!=NULL);
		}

		/**
		* @return dll file path. this is the same as the asset entity key.
		*/
		const char* GetDLLFilePath()
		{
			return m_sDllFilePath.c_str();
		}

		/**
		* The plugin must provide the system with a way to retrieve the Class Descriptors defined by the plugin. 
		* Class descriptors provide the system with information about the plugin classes in the DLL. 
		* The function LibClassDesc(i) allows the system to access the class descriptors. A DLL may have several class descriptors, 
		* one for each plugin class. The function should return a pointer to the 'i-th' class descriptor. 
		*/
		ClassDescriptor* GetClassDescriptor(int i)
		{
			if(IsValid() && i>=0 && i<GetNumberOfClasses())
				return m_listClassDesc[i];
			else
				return NULL;
		}

		/**
		* When a plugin file is loaded that contains an entity that the system does not have access to (i.e. the DLL is not available), 
		* a message is presented to the user. The system requires that each DLL return a text string to present to the user if it is unavailable.
		* @return return the dll description string.
		*/
		const char* GetLibDescription()
		{
			return m_sLibDescription.c_str();
		}

		/**
		* return the number of plugin classes inside the DLL
		*/
		int GetNumberOfClasses()
		{
			return (int)m_listClassDesc.size();
		}


		/**
		* init the asset entity object.
		* @param sFilename : dll file path
		*/
		void Init(const char* sFilename)
		{
			m_bIsInitialized = true;
			m_bIsValid = true;

			if(sFilename)
			{
				m_sDllFilePath = sFilename;
			}
			string sDLLPath = m_sDllFilePath;

			// load the library.
#ifdef WIN32
			m_hDLL = (HINSTANCE)LoadLibraryS(sDLLPath.c_str());
#else
			m_hDLL = LoadLibraryS(sDLLPath.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif

			if (m_hDLL != NULL)
			{
#ifdef WIN32
				{
					// see http://support.microsoft.com/default.aspx?scid=kb;en-us;814472

					// ... initialization code
					pfnEnsureInit pfnDll= (pfnEnsureInit) GetProcAddressS(m_hDLL, "DllEnsureInit");
					if(pfnDll!=0)
					{
						// Exit, return; there is nothing else to do.
						pfnDll();
					}
				}
#endif

				lpFnLibDescription pLibDescription = (lpFnLibDescription) GetProcAddressS(m_hDLL, "LibDescription");

				if (pLibDescription != 0)
				{
					// call the function
					m_sLibDescription = pLibDescription();
					//OUTPUT_LOG("lib desc %s \r\n", m_sLibDescription.c_str());
				}
				else
				{
#ifdef	WIN32
					OUTPUT_LOG("failed loading %s : because it does not expose the LibDescription method\r\n", sDLLPath.c_str());
#else
					const char* sErrorMsg = dlerror();
					if(sErrorMsg == 0)
						sErrorMsg = "unknown error";
					OUTPUT_LOG("warning: GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
					// handle the error
					UnLoad();
				}

				lpFnLibVersion pLibVersion = (lpFnLibVersion)GetProcAddressS(m_hDLL, "LibVersion");
				if (pLibVersion  != 0)
				{
					// call the function
					m_nParaEngineVersion = pLibVersion();
				}
				else
				{
#ifdef	WIN32
					OUTPUT_LOG("failed loading %s : because it does not expose the LibVersion method\r\n", sDLLPath.c_str());
#else
					const char* sErrorMsg = dlerror();
					if(sErrorMsg == 0)
						sErrorMsg = "unknown error";
					OUTPUT_LOG("warning: GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
					// handle the error
					UnLoad();
				}
				int nClassNum=0;

				lpFnLibNumberClasses pLibNumberClasses = (lpFnLibNumberClasses)GetProcAddressS(m_hDLL, "LibNumberClasses");
				if (pLibNumberClasses != 0)
				{
					// call the function
					nClassNum= pLibNumberClasses();
					//OUTPUT_LOG("lib classes count %d \r\n", nClassNum);
					m_listClassDesc.reserve(nClassNum);
				}
				else
				{
#ifdef	WIN32
					OUTPUT_LOG("failed loading %s : because it does not expose the LibNumberClasses method\r\n", sDLLPath.c_str());
#else
					const char* sErrorMsg = dlerror();
					if(sErrorMsg == 0)
						sErrorMsg = "unknown error";
					OUTPUT_LOG("warning: GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
					// handle the error
					UnLoad();
				}
				lpFnLibClassDesc pLibClassDesc = (lpFnLibClassDesc)GetProcAddressS(m_hDLL, "LibClassDesc");
				if (pLibDescription != 0)
				{
					// call the function
					for (int i=0; i<nClassNum; ++i)
					{
						ClassDescriptor* pClassDesc = pLibClassDesc(i);
						if(pClassDesc!=0)
						{
							m_listClassDesc.push_back(pClassDesc);
						}
						else
						{
							OUTPUT_LOG("the %d th class in %s is not loaded \r\n", i, sDLLPath.c_str());
						}
					}
				}
				else
				{
#ifdef	WIN32
					OUTPUT_LOG("failed loading %s : because it does not expose the LibClassDesc method\r\n", sDLLPath.c_str());
#else
					const char* sErrorMsg = dlerror();
					if(sErrorMsg == 0)
						sErrorMsg = "unknown error";
					OUTPUT_LOG("warning: GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
					// handle the error
					UnLoad();
				}

#ifdef WIN32
				lpFnLibInit pLibInit = (lpFnLibInit)GetProcAddressS(m_hDLL, "LibInit");
				if (pLibDescription != 0)
				{
					// call the function
					pLibInit();
				}
#endif

				m_pFuncActivate = (lpFnLibActivate)GetProcAddressS(m_hDLL, "LibActivate");
			}
			else
			{
				m_bIsValid = false;
			}

			if(IsValid())
			{
				// rule of silence, do not pollute std output
				// OUTPUT_LOG("Plug-in loaded: %s\r\n", sDLLPath.c_str());
			}
			else
			{
				OUTPUT_LOG("Failed loading plug-in: %s\r\n", sDLLPath.c_str());
			}
		}

		/** Free the library. */
		void UnLoad()
		{
			if(m_hDLL!=0)
			{
#ifdef WIN32
				{
					// see http://support.microsoft.com/default.aspx?scid=kb;en-us;814472
					pfnForceTerm pfnDll=( pfnForceTerm) GetProcAddressS(m_hDLL, "DllForceTerm");
					if(pfnDll!=0)
					{
						pfnDll();	
					}
				}
#endif
				FreeLibraryS(m_hDLL);
				m_hDLL = 0;
			}
			m_bIsValid = false;
		}

		/**
		* Activate the DLL.
		* @param nType type of the activation
		*	@see PluginActivationType
		* @param pVoid any data associated with the type
		* @return: E_FAIL if not succeeded.
		*/
		int Activate(int nType=0, void* pVoid=NULL)
		{
			if(IsValid() && m_pFuncActivate!=0)
			{
				return m_pFuncActivate(nType, pVoid);
			}
			return -1;
		}

		/* same as LoadLibrary in windows or dlopen() in linux
		#define RTLD_LAZY   1
		#define RTLD_NOW    2
		#define RTLD_GLOBAL 4
		*/
		static void* LoadLibraryS(const char *pcDllname, int iMode=2)
		{
			std::string sDllName = pcDllname;
#ifdef WIN32 // Microsoft compiler
			if(sDllName.find(".") == string::npos)
				sDllName += ".dll";
			return (void*)::LoadLibraryA(pcDllname);
#else
			if(sDllName.find(".") == string::npos)
				sDllName += ".so";
			return dlopen(sDllName.c_str(),iMode);
#endif

		}
		static void * GetProcAddressS(void *Lib, const char *Fnname)
		{
#ifdef WIN32 // Microsoft compiler
			return (void*)::GetProcAddress((HINSTANCE)Lib,Fnname);
#else
			return dlsym(Lib,Fnname);
#endif
		}

		static bool FreeLibraryS(void *hDLL)
		{
#ifdef WIN32 // Microsoft compiler
			return ::FreeLibrary((HINSTANCE)hDLL);
#else
			return dlclose(hDLL);
#endif
		}

	private:
		/** dll file path */
		string m_sDllFilePath;
		/** list of class descriptors defined in the DLL file*/
		vector<ClassDescriptor*> m_listClassDesc;
		/**
		When a plugin file is loaded that contains an entity that the system does not have access to (i.e. the DLL is not available), 
		a message is presented to the user. The system requires that each DLL return a text string to present to the user if it is unavailable.
		*/
		string m_sLibDescription;
		/** for which ParaEngine version that this lib is compiled. 
		This allows the system to deal with obsolete versions of ParaEngine plugin DLLs. 
		Because the the game engine architecture supports such a close relationship to its plugins, 
		the system may at some point need to prevent older plugin DLLs from being loaded. 
		*/
		unsigned long m_nParaEngineVersion;

		/** Handle to DLL */
		void* m_hDLL;

		bool m_bIsInitialized;

		bool m_bIsValid;

		/** activate function pointer */
		lpFnLibActivate m_pFuncActivate;
	};
}
