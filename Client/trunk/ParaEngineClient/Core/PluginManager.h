#pragma once
#include "PEtypes.h"
#include "AssetEntity.h"
#include "AssetManager.h"
#include "util/mutex.h"
#include <vector>

namespace ParaEngine
{
	using namespace std;
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
	typedef void(*lpFnLibInitParaEngine)(IParaEngineCore* pIParaEngineCore);
	/** "LibActivate": this is optional in a plug-in */
	typedef int (*lpFnLibActivate)(int nType, void* pVoid);

	typedef void (STDCALL *pfnEnsureInit)(void);
	typedef void (STDCALL *pfnForceTerm)(void);

	/**
	* a DLL plug-in loaded explicitly at runtime. 
	*/
	struct DLLPlugInEntity : public AssetEntity
	{
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
		/** library version. 
		This allows the system to deal with obsolete versions of ParaEngine plugin DLLs. 
		Because the the game engine architecture supports such a close relationship to its plugins, 
		the system may at some point need to prevent older plugin DLLs from being loaded. 
		*/
		unsigned long m_nLibVersion;
		
		/** Handle to DLL */
		void* m_hDLL;

		/** activate function pointer */
		lpFnLibActivate m_pFuncActivate;
		
	public:
		DLLPlugInEntity(const AssetKey& key);
		virtual ~DLLPlugInEntity();
		virtual AssetType GetType(){return dllplugin;};
		
		/** library version.*/
		unsigned long GetLibVersion() const { return m_nLibVersion; }

		/**
		* @return dll file path. this is the same as the asset entity key.
		*/
		const char* GetDLLFilePath();
		/**
		* The plugin must provide the system with a way to retrieve the Class Descriptors defined by the plugin. 
		* Class descriptors provide the system with information about the plugin classes in the DLL. 
		* The function LibClassDesc(i) allows the system to access the class descriptors. A DLL may have several class descriptors, 
		* one for each plugin class. The function should return a pointer to the 'i-th' class descriptor. 
		*/
		ClassDescriptor* GetClassDescriptor(int i);
		
		/**
		* When a plugin file is loaded that contains an entity that the system does not have access to (i.e. the DLL is not available), 
		* a message is presented to the user. The system requires that each DLL return a text string to present to the user if it is unavailable.
		* @return return the dll description string.
		*/
		const char* GetLibDescription();
		/**
		* return the number of plugin classes inside the DLL
		*/
		int GetNumberOfClasses();

		/**
		* init the asset entity object.
		* @param sFilename : dll file path
		*/
		void Init(const char* sFilename);

		/** Free the library. */
		void FreeLibrary();

		/**
		* Activate the DLL.
		* @param nType type of the activation
		*	@see PluginActivationType
		* @param pVoid any data associated with the type
		* @return: E_FAIL if not succeeded.
		*/
		int Activate(int nType=0, void* pVoid=NULL);
	};
	
	/**
	* manage plug-ins 
	*/
	class CPluginManager : public AssetManager<DLLPlugInEntity>
	{
	public:
		CPluginManager(void);
		~CPluginManager(void);
		/** singleton*/
		static CPluginManager* GetInstance();
	public:

		/**
		* Load a DLL at the given path
		* [thread safe]
		* @param sIdentifier: this may be ""
		* @param fileName: Dll Path 
		* @return true, if loaded. If the dll is already loaded it will return true immediately.
		*/
		DLLPlugInEntity* LoadDLL(const string&  sIdentifier, const string&  fileName);
		
		/**
		* get the plug-in entity by its short name.
		* [thread safe]
		* @param sFilePath this is the dll path.
		* @return dll entity.
		*/
		DLLPlugInEntity* GetPluginEntity(const string& sFilePath);

		/**
		* load and activate the given dll
		* [thread safe]
		* @param sDllPath 
		* @param nType type of the activation 
		*	@see PluginActivationType
		* @param pVoid any data associated with the type
		* @return: E_FAIL if not succeeded.
		*/
		int ActivateDLL(const string& sDllPath,int nType=0, void* pVoid=NULL);

		/**
		* load all dll under the given directory.
		* [thread safe]
		* @param sDirectory the directory to load dll. e.g. "plugin/*.dll"
		* @return the number of dll loaded. 
		*/
		int LoadAllDLLinDirectory(const string& sDirectory);

		/** get mutex */
		ParaEngine::mutex& GetMutex() {return m_mutex;};

	private: 
		/** global mutex */
		ParaEngine::mutex m_mutex;
	};



}
