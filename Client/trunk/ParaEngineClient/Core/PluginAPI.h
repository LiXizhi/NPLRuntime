//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description:	API for plug in. 
//-----------------------------------------------------------------------------
#pragma once
#include "coreexport.h"
#include "PEtypes.h"
#include "IParaEngineCore.h"

namespace ParaEngine
{
	class IParaEngineCore;
}

namespace ParaEngine
{
/** define PE_MODULE_STANDALONE if the plugin does not link to Core lib under win32 */
#ifndef PE_MODULE_STANDALONE
	/** Returns the state of the VERSION_PARAENGINE */ 
	PE_CORE_DECL DWORD GetParaEngineVersion();

	/** get IParaEngineCore interface. IParaEngineCore interface contains everything that a plug-in need to access the game engine. */
	PE_CORE_DECL ParaEngine::IParaEngineCore* GetCOREInterface();
#endif

	//-------------------------------------------
	// Super-classes that are plugable.
	/**@def basic 3d object */
	#define  BASE_OBJECT_CLASS_ID					0x00000001
	/**@def object modifier */
	#define  OBJECT_MODIFIER_CLASS_ID				0x00000002
	/**@def NPL file object */
	#define  NPL_FILE_CLASS_ID						0x00000003
	/**@def basic 2D GUI object */
	#define  GUI_OBJECT_CLASS_ID					0x00000004
	/**@def basic game object which perceptions */
	#define  GAME_OBJECT_CLASS_ID					0x00000005
	/**@def ParaEngine extension class ID */
	#define  EXTENSION_CLASS_ID						0x00000006

	/**
	System keeps a list of the DLL's found on startup. This is the interface to a single class
	A class descriptor object does the following: 
	- Describes the properties of each plugin class. 
	- Creates the plugin class. 
	- Destroys the plugin class. 
	
	It has several important purposes, but the two main ones relate to classifying the type of object the plugin is, 
	and allocating the memory for instances of the plugin class objects. 

	A plugin DLL can contain many plugin classes. At runtime, ParaEngine calls your DLL through the LibNumberClasses function 
	and asks how many plugin class it contains. Then with that number, it loops enters a for loop and calls each of 
	your static class descriptors through the LibClassDesc(i) function. After the game engine gets your class descriptor, 
	it then calls the descriptors Create Method to allocate an instance of your plugin.
	
	Each of these plugin classes must have a Class ID.There are two IDs associated with each plugin system object. These are:
	- Class ID uniquely identifies a specific plugin class. 
	- Super Class ID specifies the superclass of the plugin's class.
	*/
	class ClassDescriptor 
	{
	public:
		virtual					~ClassDescriptor() {}
		/**
		* return a pointer to an instance of the class.When the system needs to delete an instance of a plugin class, 
		* it usually calls a method named DeleteThis(). plugin developers must implement this method as well. 
		* Since a developer uses the new operator to allocate memory, he or she should use the delete operator to de-allocate it.
		*/
		virtual void *			Create(bool loading=false)=0;
		
		/**
		* class name
		*/
		virtual const char* 	ClassName()=0;
		/**
		* This method returns a system-defined constant describing the class that this plugin class was derived from. 
		*/
		virtual SClass_ID		SuperClassID()=0;
		/**
		* uniquely identifies a specific plugin class. 
		*/
		virtual Class_ID		ClassID()=0;
		/**
		* it may be some predefined category name. 
		*/
		virtual const char* 	Category()=0;
		/**
		* returns owning module handle (DLL handle)
		*/
		virtual HINSTANCE		HInstance() { return NULL; }


		/** whether the class is accessible from the UI.*/
		virtual int				IsPublic(){return TRUE;};
		/** whether the class uses windows GDI to draw controls.*/
		virtual int				UseGDI(){return FALSE;};

		
		/** genetic activation function as the one in the NPL script file.*/
		virtual void* activate(int cmd, void* arg1=0, void* arg2=0, void* arg3=0) { return 0; } 
	};
}

#if !defined(PE_CORE_EXPORTING) && !defined(PARAENGINE_MOBILE)

namespace ParaEngine
{
	typedef IParaEngineCore* (STDCALL* funcPtrGetCOREInterface_type)();

	/** this class is only used in ParaEngine dll plugins to call main exe functions from the loaded DLL
	The dll no need to actually link to the executable. It uses GetProcAddress in win32.  Under linux, it just calls GetCOREInterface() directly.
	usage:
	ParaEngine::CParaEngineCore::GetParaEngineCOREInterface()->WriteToLog("hello %s\n", "world");
	*/
	class CParaEngineCore
	{
	public:
		static IParaEngineCore* GetParaEngineCOREInterface()
		{
			static IParaEngineCore* s_pCore = NULL;
			if (s_pCore == NULL)
			{
#ifdef WIN32
#ifdef _DEBUG
				HINSTANCE hDLL = (HINSTANCE)::LoadLibrary(_T("ParaEngineClient_d.dll"));
#else
				HINSTANCE hDLL = (HINSTANCE)::LoadLibrary(_T("ParaEngineClient.dll"));
#endif
				if(hDLL != INVALID_HANDLE_VALUE)
				{
					funcPtrGetCOREInterface_type pFuncPtr = (funcPtrGetCOREInterface_type)::GetProcAddress(hDLL, "GetParaEngineCOREInterface");
					if(pFuncPtr){
						s_pCore = pFuncPtr();
					}
					else{
						throw "can not GetCOREInterface";
					}
					::FreeLibrary(hDLL);
				}
#else
				s_pCore = GetCOREInterface();
#endif
			}
			return s_pCore;
		}
	};
}

#endif