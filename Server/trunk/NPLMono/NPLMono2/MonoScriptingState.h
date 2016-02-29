#pragma once

#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <INPLScriptingState.h>
#include <set>
#include <util/Mutex.hpp>

// forward declare
struct AssemblyFile;
struct ClassObject;
class GlobalAssembliesPool;

typedef std::map <std::string, AssemblyFile*>	Assembly_File_Map_Type;
typedef std::map <std::string, ClassObject*>	Class_Map_Type;

/** this represent a class (with the same name as the file name) inside an assembly */
struct ClassObject
{
	/** name space name of the class */
	std::string m_namespace_name;

	/** keep a duplicate copy of class file name, like "HelloWorld" */
	std::string m_class_name;

	/** the Mono Class object of the class */
	MonoClass * m_class;
	/** the static void activate() function inside the mono class.*/
	MonoMethod * m_activate_function;
};

/** the .Net assembly file, which may contain many C# classes */
struct AssemblyFile
{
	MonoAssembly* m_assembly;
	MonoImage* m_image;

	Class_Map_Type m_classes;
};

/** a singleton class that keeps dynamically loaded meta data. 
* this class is thread safe
* Must call SetMonoDomain() just once with the root app domain, before calling GetClass() method.  
*/
class GlobalAssembliesPool
{
public:
	GlobalAssembliesPool();
	~GlobalAssembliesPool();

	static GlobalAssembliesPool& GetSingleton();

	// Get mono domain
	inline MonoDomain* GetMonoDomain() {return m_mono_domain;};
	// Set root mono domain in to which all assemblies are loaded. 
	void SetMonoDomain(MonoDomain* domain) {m_mono_domain = domain;};

	/**
	* Get class object of a given CS script file without running it. It will load the file is not loaded. 
	* If class is already loaded, it will not be loaded again.
	* [thread safe]
	* @param filePath: the local CS script file path, such as "test.dll/test.cs", "test.dll/ParaMono.test.cs", where ParaMono can be a namespace. 
	* @return: ClassObject * object or NULL. 
	*/
	ClassObject* GetClass(const string& filePath);

private:

	/** mapping from dll (exe) file name to all loaded assemblities */
	Assembly_File_Map_Type m_assemblies;

	/** The root domain into which assemblies are loaded. */
	MonoDomain *	m_mono_domain;

	// mutex for function GetClass().
	ParaEngine::Mutex m_mutex;
};

/** Mono scripting state */
class CMonoScriptingState : public NPL::IMonoScriptingState
{
public:
	CMonoScriptingState();
	~CMonoScriptingState();

	/** call this function before calling anything else. It will load all NPL modules into the runtime state. */
	void Init();

	/** return the name of this runtime state. if "", it is considered an anonymous name */
	const char* GetName() const;

	/// get the mono domain. 
	inline MonoDomain* GetMonoDomain() {return s_mono_domain;};

	/// load only NPL related functions.
	void LoadNPLLib();

	/// load only Para related functions.
	void LoadParaLib();

public:
	/** this is something like delete this*/
	virtual void Release();

	/**get the Mono Domain state. 
	@return MonoDomain *
	*/
	virtual void* GetState(){return (void*)GetMonoDomain();};

	/// return true if the runtime state is valid
	virtual bool IsValid() {return s_mono_domain != 0;}

	/**
	* whether a given script file is loaded. 
	* @param filePath: the local file path in the following format:
	*		[dir0]/[dir1]/[dir2]/[filename.lua]
	* @return: return true if file is already loaded in the current state.
	*/
	virtual bool IsScriptFileLoaded(const string& filepath);

	/**
	* load a new CS script file without running it. If the file is already loaded,
	* it will not be loaded again.
	* @param filePath: the local CS script file path, such as "test.dll/test.cs", "test.dll/ParaMono.test.cs", where ParaMono can be a namespace. 
	* @param bReload: if true, the file will be reloaded even if it is already loaded.
	*    otherwise, the file will only be loaded if it is not loaded yet. 
	* @return: return true, if file is loaded. 
	*/
	virtual bool LoadFile(const string& filePath, bool bReload);

	/** do string in the current state. This function is usually called from the scripting interface.
	* @param sCall: the string to executed. 
	* @param nLength: length in bytes.if this is 0, length will be calculated, but must be smaller than a predefined safe length. If this is positive. any string length is allowed. 
	*/
	virtual int DoString(const char* sCall, int nLength = 0);

	/**
	* Activate a local file. The file should be loaded already.
	* @param filepath: pointer to the file path.
	* @return: NPLReturnCode
	*/
	virtual NPL::NPLReturnCode ActivateFile(const string& filepath, const char * code = NULL, int nLength=0);

	/**
	* load or restore the runtime state 
	@param name: any runtime state name. 
	@param pState: the NPL runtime state that this mono scripting state belongs to.
	*/
	virtual bool CreateState(const char* name, NPL::INPLRuntimeState* pState = 0);

	/** destroy the runtime state
	* careful with this function, it will make the state invalid.
	*/
	virtual void DestroyState();

	/** set the NPL runtime state that this mono scripting state belongs to. 
	* This function is used internally by NPLRuntimeState. */
	virtual void SetNPLRuntimeState(NPL::INPLRuntimeState* pState);

	/** Get the NPL runtime state that this mono scripting state belongs to */
	virtual NPL::INPLRuntimeState* GetNPLRuntimeState();

protected:
	/** it is safe to call this multiple times. however only the first time does the init. 
	* it will set the mono dir and load the mono configuration for all mono domains. 
	*/
	void CreateAppDomain();

	static const char *GetStringProperty(const char *propertyName, MonoClass *classType, MonoObject *classObject);
private:
	/// the name of this runtime state. if "", it is considered an anonymous name
	string m_name;

	/** each assembly is loaded and run in a MonoDomain.*/
	static MonoDomain *	s_mono_domain;
	
	/** this one is set to GlobalAssembliesPool::GetSingleton() at constructor. */
	GlobalAssembliesPool* m_global_assemblies_pool;

	/** keeping the npl runtime state interface that this mono state belongs to.*/
	NPL::INPLRuntimeState* m_npl_runtime_state;
};


