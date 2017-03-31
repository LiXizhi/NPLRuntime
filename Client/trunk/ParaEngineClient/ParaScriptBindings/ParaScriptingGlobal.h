//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.4
// Description:	API for global objects
//-----------------------------------------------------------------------------
#pragma once
#include "util/LogService.h"
#include "IAttributeFields.h"
#include <map>
#include <string>

struct lua_State;


#pragma region StackObjectPtr

namespace ParaScripting
{
	class ParaObject;
	class ParaUIObject;
	class ParaAttributeObject;

	/**
	* this is for luabinding a pointer object on lua stack, instead of invoking new operator each time an object is created. 
	* Note: this class is not used, since we now uses a more universal template which treats all objects in ParaScripting namespace as stack object. 
	*/
	template <typename _Ty>
	class StackObjectPtr
	{
	public:
		StackObjectPtr(_Ty* obj){
			if(obj!=0)
			{
				_Myptr = *obj;
			}
		}
		StackObjectPtr(_Ty const& obj):_Myptr(obj){}

		operator bool () const
		{
			return _Myptr.IsValid();
		}

		bool operator! () const
		{
			return !(_Myptr.IsValid());
		}

		_Ty& operator*() const
		{
			return (*get());
		}

		_Ty *operator->() const
		{	
			return (get());
		}

		_Ty *get() const
		{
			return (_Ty *)&(_Myptr);
		}
	private:
		_Ty _Myptr;
	};

	template <typename T>
	T* get_pointer(StackObjectPtr<T> const& pointer){ 
		return pointer.get();
	}
	
	/** this defines a rule, so that every object defined in ParaScripting namespace will be treated as StackObject by luabind. 
	* i.e. all objects such as ParaObject, ParaUIObject, etc are passed by value on the stack, instead of using std::auto_ptr( new ParaObject(p)).
	* if one wants to pass by reference, overwrite this method. 
	* @see : see make_pointee_instance in luabind/policy.hpp and object_rep.hpp. 
	*/
	template <typename T>
	T* get_pointer(T const& pointer){ 
		return (T*)(&pointer);
	}
} // namespace ParaScripting

namespace luabind
{
	using ParaScripting::get_pointer; // <ParaScripting::ParaCharacter>;
}

#pragma endregion StackObjectPtr

namespace ParaScripting
{
	using namespace std;
	using namespace luabind;
	using namespace ParaEngine;
	/** 
	* @ingroup ParaGlobal
	* for Para Script global dictionary object
	*/
	class ParaObjectNode
	{
	public:
		enum objecttype{number, string}
		oType;
		union
		{
			double dNumber;
		};
		std::string str; 

		ParaObjectNode(double n){
			oType = number;
			dNumber = n;
		};
		ParaObjectNode(const char* newstr){
			oType = string;
			this->str = newstr;
		};
	};

	typedef struct
	{
		bool operator() (const std::string& first, const std::string& second) const
		{
			return first.compare(second)<0;
		}
	} CStringCompare;

	typedef pair <const char *, ParaObjectNode> ParaObject_Pair;
	

	/** 
	* @ingroup ParaGlobal
	* service logger 
	*/
	class ParaServiceLogger
	{
	public:
		ParaEngine::CServiceLogger_ptr m_logger_ptr;
		
		ParaServiceLogger(){};
		ParaServiceLogger(ParaEngine::CServiceLogger_ptr& logger);
		~ParaServiceLogger(){};
	public:
		/** log to the current logger */
		void log(int level, const object& message);

		/**
		Returns the assigned Level
		@return Level - the assigned Level
		*/
		int GetLevel();

		/** set level of this logger. */
		void SetLevel(const int level1);

		/**
		Check whether this logger is enabled for a given Level passed as parameter.
		@return bool True if this logger is enabled for level. It just checks (level>=this->m_level)
		*/
		bool IsEnabledFor(int level);

		/** by default, append mode is enabled in server mode, and disabled in client build.
		* @note: only call this function when no log is written before using the logger. 
		*/
		void SetAppendMode(bool bAppendToExistingFile);

		/** change the log file. 
		* @note: only call this function when no log is written before using the logger. 
		* @param sFileName: such as "log/test.log"
		*/
		void SetLogFile(const char* sFileName);

		/** if true we will flush the new log to file immediately. otherwise, flush operation is determined by the system. 
		* default to true for client log and false for service log. 
		* @note: only call this function when no log is written before using the logger. 
		*/
		void SetForceFlush(bool bForceFlush);
	};

	/**
	* @ingroup ParaGlobal
	* ParaGlobal namespace contains a list of HAPI functions to globally control the engine
	*/
	class PE_CORE_DECL ParaGlobal
	{
	public:
		/**
		* exit the applications. 
		*/
		static void ExitApp();

		/** This is same as ExitApp, except that it supports a return code. 
		* this is the recommended way of exiting application. 
		* this is mainly used for writing test cases. Where a return value of 0 means success, any other value means failure. 
		*/
		static void Exit(int nReturnCode);

		/** select a given attribute object. this is a static function*/
		static void SelectAttributeObject(const ParaAttributeObject& obj);

		/** 
		* write const char* to console, usually for debugging purposes.
		*/
		static void WriteToConsole(const char* strMessage);
		/** 
		* write const char* to log file, usually for debugging purposes.
		*/
		static void WriteToLogFile(const char* strMessage);

		/** get the current log file position. it is equivalent to the log file size in bytes. 
		one can later get log text between two Log positions. 
		*/
		static int GetLogPos();

		/**get log text between two Log positions. 
		* @param fromPos: position in bytes. if nil, it defaults to 0
		* @param nCount: count in bytes. if nil, it defaults to end of log file. 
		* @return string returned. 
		*/
		static const char* GetLog(int fromPos, int nCount);

		/** Get a service logger. Please see util/LogService.h for more information. */
		static ParaServiceLogger GetLogger(const object& name);
		static ParaServiceLogger GetLogger_(const char* name);

		/** 
		* write const char* to specific file.
		* obsolete
		*/
		static bool WriteToFile(const char* filename, const char* strMessage);
		/**
		* set the game status
		@param strState:
		- "disable"	disable the game
		- "enable"	enable the game
		- "pause"	pause the game
		- "resume"	resume the game
		*/
		static void SetGameStatus(const char* strState);
		/**
		* @return return the current game time in milliseconds.When game is paused, game time is also paused.
		* this is usually used for cinematic movies
		*/
		static double GetGameTime();


		/**
		* get the date in string
		[thread safe]
		* @param sFormat: can be NULL to use default.e.g. "ddd',' MMM dd yy"
		- d Day of month as digits with no leading zero for single-digit days. 
		- dd Day of month as digits with leading zero for single-digit days. 
		- ddd Day of week as a three-letter abbreviation. The function uses the LOCALE_SABBREVDAYNAME value associated with the specified locale. 
		- dddd Day of week as its full name. The function uses the LOCALE_SDAYNAME value associated with the specified locale. 
		- M Month as digits with no leading zero for single-digit months. 
		- MM Month as digits with leading zero for single-digit months. 
		- MMM Month as a three-letter abbreviation. The function uses the LOCALE_SABBREVMONTHNAME value associated with the specified locale. 
		- MMMM Month as its full name. The function uses the LOCALE_SMONTHNAME value associated with the specified locale. 
		- y Year as last two digits, but with no leading zero for years less than 10. 
		- yy Year as last two digits, but with leading zero for years less than 10. 
		- yyyy Year represented by full four digits. 
		- gg Period/era string. The function uses the CAL_SERASTRING value associated with the specified locale. This element is ignored if the date to be formatted does not have an associated era or period string. 
		*/
		static std::string GetDateFormat(const object&  sFormat);

		/**
		* get the time in string
		[thread safe]
		* @param sFormat: can be NULL to use default.
		* e.g. "hh':'mm':'ss tt"
			- h Hours with no leading zero for single-digit hours; 12-hour clock. 
			- hh Hours with leading zero for single-digit hours; 12-hour clock. 
			- H Hours with no leading zero for single-digit hours; 24-hour clock. 
			- HH Hours with leading zero for single-digit hours; 24-hour clock. 
			- m Minutes with no leading zero for single-digit minutes. 
			- mm Minutes with leading zero for single-digit minutes. 
			- s Seconds with no leading zero for single-digit seconds. 
			- ss Seconds with leading zero for single-digit seconds. 
			- t One character time-marker string, such as A or P. 
			- tt Multicharacter time-marker string, such as AM or PM. 
		*/
		static std::string GetTimeFormat(const object&  sFormat);

		/**
		* The timeGetTime function retrieves the system time, in milliseconds. The system time is the time elapsed since Windows was started.
		* Note that the value returned by the timeGetTime function is a DWORD value. The return value wraps around to 0 every 2^32 milliseconds, 
		* which is about 49.71 days. This can cause problems in code that directly uses the timeGetTime return value in computations,
		* particularly where the value is used to control code execution. You should always use the difference between two timeGetTime return values in computations. 
		*/
		static DWORD timeGetTime(); 

		/** get the elapsed time using high-resolution timing function in seconds. this function is mostly used for profiling on the NPL */
		static double getAccurateTime(); 

		/**
		* the random seed is set at application start
		* @return generate a random number between [0,1]
		*/
		static double random();

		/**
		* Get the system date and time in seconds. The system time is expressed in Coordinated Universal Time (UTC). 
		* Note: there is some trick to make the returned value a valid number in NPL. Only compare time with time returned by the same function. 
		* TODO: in the long run, a true unsigned int64 should be returned. 
		* [thread safe]
		*/
		static double GetSysDateTime(); 

		/**
		* generate a unique ID as a string. This is usually a string.
		* [thread safe]
		* @return
		*/
		static std::string GenerateUniqueID();

		/**
		* global object dictionary functions: this is a way for different script runtime to 
		* share some global information. Currently only value and const char* object can be saved.
		* one can save nil to a object name to delete the object.
		* @param objObject: object to save
		*/
		static void	SaveObject(const char* strObjectName, const object& objObject);

		/**
		* global object dictionary functions: this is a way for different script runtime to 
		* share some global information. Currently only value and const char* object can be saved.
		* return nil, if object is not found
		* @param strObjectName: the object name
		*/
		static object LoadObject(const object& strObjectName);
		/** reset the game loop script. the game loop script will be activated every 0.5 seconds 
		* see SetGameLoopInterval() to change the default interval
		* Please keep the game loop concise. The default game loop is ./script/gameinterface.lua
		*/
		static void SetGameLoop(const char* scriptName);
		/** set the game loop activation interval. The default value is 0.5 seconds. */
		static void SetGameLoopInterval(float fInterval);
		/**
		* run an external application. creates a new process and its primary thread. The new process runs the specified executable file in the security context of the calling process.
		* @remark: One can also use ParaEngine C++ or .Net API to write application plug-ins for the game engine, which can be loaded like any other script files.
		* e.g. To open a file in an external notepad editor use  ParaGlobal.CreateProcess("c:\\notepad.exe", "\"c:\\notepad.exe\" c:\\test.txt", true);
		* @param lpApplicationName:Pointer to a null-terminated string that specifies the module to execute. The specified module can be a Windows-based application. 
		* The string can specify the full path and file name of the module to execute or it can specify a partial name. In the case of a partial name, the function 
		* uses the current drive and current directory to complete the specification. The function will not use the search path. If the file name does not contain an extension, .exe is assumed. 
		* If the executable module is a 16-bit application, lpApplicationName should be NULL, and the string pointed to by lpCommandLine should specify the executable module as well as its arguments.
		* @param lpCommandLine:Pointer to a null-terminated string that specifies the command line to execute.
		* @param bWaitOnReturn: if false, the function returns immediately; otherwise it will wait for the editor to return. 
		* if this is true, the Child Process will have Redirected Input and Output to current log file. 
		* @return true if opened. 
		*/
		static bool CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn);

		/**
		* Performs an operation on a specified file.
		* e.g. ParaGlobal.ShellExecute("open", "iexplore.exe", "http://www.paraengine.com", nil, 1); 
		* 
		* @param lpOperation:[in] Pointer to a null-terminated string, 
		*  - "wait" this is a special one that uses ShellExecuteEx to wait on the process to terminate before return
		*  - "edit" Launches an editor and opens the document for editing. If lpFile is not a document file, the function will fail.
		*  - "explore"	Explores the folder specified by lpFile.
		*  - "find"		Initiates a search starting from the specified directory.
		*  - "open"		Opens the file specified by the lpFile parameter. The file can be an executable file, a document file, or a folder.
		*  - "print"		Prints the document file specified by lpFile. If lpFile is not a document file, the function will fail.
		*  - NULL		For systems prior to Microsoft Windows 2000, the default verb is used if it is valid and available in the registry. If not, the "open" verb is used.
		* @param lpFile [in] Pointer to a null-terminated string that specifies the file or object on which to execute the specified verb. To specify a Shell namespace object, pass the fully qualified parse name. Note that not all verbs are supported on all objects. For example, not all document types support the "print" verb.
		* @param lpParameters[in] If the lpFile parameter specifies an executable file, lpParameters is a pointer to a null-terminated string that specifies the parameters to be passed to the application. The format of this string is determined by the verb that is to be invoked. If lpFile specifies a document file, lpParameters should be NULL.
		* @param lpDirectory [in] Pointer to a null-terminated string that specifies the default directory.
		* @param nShowCmd: 	[in] Flags that specify how an application is to be displayed when it is opened. If lpFile specifies a document file, the flag is simply passed to the associated application. It is up to the application to decide how to handle it.
		*  - 	#define SW_HIDE             0
		*  - 	#define SW_NORMAL           1
		*  - 	#define SW_MAXIMIZE         3
		*  - 	#define SW_SHOW             5
		*  - 	#define SW_MINIMIZE         6
		*  - 	#define SW_RESTORE          9
		*/
		static bool ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd);

		/**
		* create a open file dialog. This function does not return until the user selects a dialog. 
		* @param inout: 
			input table:{filter="All Files (*.*);*.*;", filterindex, initialdir, flags, }
				t.filter="All Files (*.*)\0*.*\0"
			output :	{filename, result=true}
				t.filename: the full path and file name specified by the user
				t.result: boolean if user clicks the OK button
		* @return: true if user clicks ok. and the inout.filename contains the result. 
		*/
		static bool OpenFileDialog(const object& inout);

		/**
		* Write a string to the registry. e.g. WriteRegStr("HKLM", "Software\My Company\My Software", "string Value", "string Name");
		* @param root_key: must be 
		HKCR or HKEY_CLASSES_ROOT
		HKLM or HKEY_LOCAL_MACHINE
		HKCU or HKEY_CURRENT_USER
		HKU or HKEY_USERS
		*/
		static bool WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value);
		
		/** Read string from the registry. 
		* Valid values for root_key are listed under WriteRegStr. NULL will be returned if the string is not present. 
		* If the value is present, but is of type REG_DWORD, it will be read and converted to a string.
		* @param root_key: must be 
		HKCR or HKEY_CLASSES_ROOT
		HKLM or HKEY_LOCAL_MACHINE
		HKCU or HKEY_CURRENT_USER
		HKU or HKEY_USERS
		*/
		static const char* ReadRegStr(const string& root_key, const string& sSubKey, const string& name);

		/**
		* Write a DWORD to the registry. see WriteRegStr() for more info
		* @param root_key: must be 
		HKCR or HKEY_CLASSES_ROOT
		HKLM or HKEY_LOCAL_MACHINE
		HKCU or HKEY_CURRENT_USER
		HKU or HKEY_USERS
		*/
		static bool WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value);

		/** Read DWORD from the registry. 
		* Valid values for root_key are listed under WriteRegStr. NULL will be returned if the DWORD is not present or type is a string. 
		* @param root_key: must be 
		HKCR or HKEY_CLASSES_ROOT
		HKLM or HKEY_LOCAL_MACHINE
		HKCU or HKEY_CURRENT_USER
		HKU or HKEY_USERS
		*/
		static DWORD ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name);
	};

	/**
	* @ingroup ObjectAttributes
	* 
	* it represents an attribute object associated with an object. 
	* Call ParaObject::GetAttributeObject() or ParaObject::GetAttributeObject() to get an instance of this object. 
	e.g. In NPL, one can write
		local att = player:GetAttributeObject();
		local bGloble = att:GetField("global", true);
		local facing = att:GetField("facing", 0);
		att:SetField("facing", facing+3.14);
		local pos = att:GetField("position", {0,0,0});
		pos[1] = pos[1]+100;pos[2] = 0;pos[3] = 10;
		att:SetField("position", pos);
		att:PrintObject("test.txt");
	*
	* the following shows objects and their supported attributes. 
	* 
	* @include CAutoCamera.txt
	* @include CGlobalTerrain.txt
	* @include COceanManager.txt
	* @include CSceneObject.txt
	* @include CSkyMesh.txt
	* @include CTerrain.txt
	* @include ParaEngineSettings.txt
	* @include "RPG Character.txt"
	*/
	class PE_CORE_DECL ParaAttributeObject
	{
	public:
		IAttributeFields::WeakPtr_type m_pAttribute;
		CAttributeClass* m_pAttClass;

		ParaAttributeObject():m_pAttribute(),m_pAttClass(NULL){};
		ParaAttributeObject(IAttributeFields * pAttribute, CAttributeClass* pAttClass);
		ParaAttributeObject(IAttributeFields * pAttribute);

		/** get the attribute object. This function return a clone of this object. */
		ParaAttributeObject GetAttributeObject();


		/**
		* return true, if this object is the same as the given object.
		*/
		bool equals(const ParaAttributeObject& obj) const;
		
		/** get child attribute object. this can be regarded as an intrusive data model of a given object. 
		* once you get an attribute object, you can use this model class to access all data in the hierarchy.
		*/
		ParaAttributeObject GetChild(const std::string& sName);
		ParaAttributeObject GetChildAt(int nIndex);
		ParaAttributeObject GetChildAt2(int nRowIndex, int nColumnIndex);
		int GetChildCount();
		int GetChildCount2(int nColumnIndex);
		/** we support multi-dimensional child object. by default objects have only one column. */
		int GetColumnCount();

		/** add a child object*/
		bool AddChild(ParaAttributeObject& obj);

		/** query object */
		const ParaObject& QueryObject();
		const ParaUIObject& QueryUIObject();

		/**
		* check if the object is valid
		*/
		bool IsValid() const;

		/** class ID */
		int  GetClassID() const;
		/** class name */
		const char* GetClassName() const;
		const char* GetClassName1() const;
		/** class description */
		const char* GetClassDescription() const;

		/** Set which order fields are saved. 
		enum Field_Order
		{
		Sort_ByName,
		Sort_ByCategory,
		Sort_ByInstallOrder,
		}; */
		void SetOrder(int order);
		/** get which order fields are saved. */
		int GetOrder();

		/** get the total number of field. */
		int GetFieldNum();
		/** get field at the specified index. "" will be returned if index is out of range. */
		const char* GetFieldName(int nIndex);

		/**
		* get field index of a given field name. -1 will be returned if name not found. 
		* @param sFieldname 
		* @return 
		*/
		int GetFieldIndex(const char*  sFieldname);

		/**
		* get the field type as string
		* @param nIndex : index of the field
		* @return one of the following type may be returned 
		* "void" "bool" "string" "int" "float" "float_float" "float_float_float" "double" "vector2" "vector3" "vector4" "enum" "deprecated" ""
		*/
		const char* GetFieldType(int nIndex);
		/**
		* whether the field is read only. a field is ready only if and only if it has only a get method.
		* @param nIndex : index of the field
		* @return true if it is ready only or field does not exist
		*/
		bool IsFieldReadOnly(int nIndex);

		/**
		* Get Field Schematics string 
		* @param nIndex: index of the field
		* @return 
		*	"" will be returned if index is out of range
		*/
		const char* GetFieldSchematics(int nIndex);
		/**
		* parse the schema type from the schema string.
		* @return : simple schema type. it may be any of the following value. 
			unspecified: ""
			color3	":rgb" 
			file	":file" 
			script	":script"
			integer	":int"
		*/
		const char* GetSchematicsType(int nIndex);

		/**
		* parse the schema min max value from the schema string.
		* @param nIndex: index of the field
		* @param fMin : [in|out] default value
		* @param fMax : [in|out] default value
		* @return true if found min max.
		*/
		void GetSchematicsMinMax(int nIndex, float fMinIn, float fMaxIn, float& fMin, float& fMax);

		/** get field by name.
		e.g. suppose att is the attribute object.
			local bGloble = att:GetField("global", true);
			local facing = att:GetField("facing", 0);
			local pos = att:GetField("position", {0,0,0});
			pos[1] = pos[1]+100;pos[2] = 0;pos[3] = 10;

		@param sFieldname: field name
		@param output: default value. if field type is vectorN, output is a table with N items.
		@return: return the field result. If field not found, output will be returned. 
			if field type is vectorN, return a table with N items.Please note table index start from 1
		*/
		object GetField(const char*  sFieldname, const object& output);
		/** similar to GetField(). except that the output is a string. 
		Used for API exporting. not thread safe. */
		const char* GetStringField(const char*  sFieldname);
		/** similar to GetField(). except that the output is a value. 
		Used for API exporting. not thread safe. 
		@param nIndex: if the value has multiple component, such as a vector3. this is the index of the componet.*/
		double GetValueField(const char*  sFieldname, int nIndex=0);
		
		/** set field by name 
		e.g. suppose att is the attribute object.
			att:SetField("facing", 3.14);
			att:SetField("position", {100,0,0});
		@param sFieldname: field name
		@param input: input value. if field type is vectorN, input is a table with N items.*/
		void SetField(const char*  sFieldname, const object& input);
		/** similar to SetField(). except that the input is a string. 
		Used for API exporting. not thread safe.*/
		void SetStringField(const char*  sFieldname, const char* input);
		/** similar to SetField(). except that the input is a string. 
		Used for API exporting. not thread safe. 
		@param nIndex: if the value has multiple component, such as a vector3. this is the index of the component.*/
		void SetValueField(const char*  sFieldname, int nIndex, double value);
		
		/** call field by name. This function is only valid when The field type is void. 
		It simply calls the function associated with the field name. */
		void CallField(const char*  sFieldname);

		/** print attribute to file 
		@param file: file name to save the manual to.
		*/
		void PrintObject(const char* file);

		/**
		* Reset the field to its initial or default value. 
		* @param nFieldID : field ID
		* @return true if value is set; false if value not set. 
		*/
		bool ResetField(int nFieldID);

		/**
		* Invoke an (external) editor for a given field. This is usually for NPL script field
		* @param nFieldID : field ID
		* @param sParameters : the parameter passed to the editor
		* @return true if editor is invoked, false if failed or field has no editor. 
		*/
		bool InvokeEditor(int nFieldID, const char* sParameters);


		//////////////////////////////////////////////////////////////////////
		// dynamic fields
		//////////////////////////////////////////////////////////////////////
		/** get field by name.
		e.g. suppose att is the attribute object.
		local bGloble = att:GetField("URL", nil);
		local facing = att:GetField("Title", "default one");
		
		@param sFieldname: field name
		@param output: default value. if field type is vectorN, output is a table with N items.
		@return: return the field result. If field not found, output will be returned. 
		if field type is vectorN, return a table with N items.Please note table index start from 1
		*/
		object GetDynamicField(const char*  sFieldname, const object& output);
		
		/**
		* Get a dynamic field with a given index. 
		*/
		object GetDynamicField_(int nIndex, const object& output);

		object GetDynamicFieldImp(CDynamicAttributeField* pField, const object &output);

		/** get field name by index */
		const char* GetDynamicFieldNameByIndex(int nIndex);

		/** how many dynamic field this object currently have. */
		int GetDynamicFieldCount();

		/** set field by name 
		* e.g. suppose att is the attribute object.
		* att:SetDynamicField("URL", 3.14);
		* att:SetDynamicField("Title", {100,0,0});
		* @param sFieldname: field name
		* @param input: input value. can be value or string type
		* @return : -1 failed, if 0 means modified, if 1 means a new key is added, if 2 means a key is removed.
		*/
		int SetDynamicField(const char*  sFieldname, const object& input);
		int SetDynamicField_(int nFieldIndex, const object& input);

		int GetFieldKeyNums(const char*  sFieldname);
		void SetFieldKeyNums(const char*  sFieldname, int nKeyNum);

		void SetFieldKeyTime(const char*  sFieldname, int nKeyIndex, int nTime);
		int GetFieldKeyTime(const char*  sFieldname, int nKeyIndex);

		void SetFieldKeyValue(const char*  sFieldname, int nKeyIndex, const object& input);
		object GetFieldKeyValue(const char*  sFieldname, int nKeyIndex, const object& output);

		/** remove all dynamic fields*/
		void RemoveAllDynamicFields();

		/** add dynamic field and return field index 
		* @param dwType: type of ATTRIBUTE_FIELDTYPE
		*/
		int AddDynamicField(const std::string& sName, int dwType);
	};

	/**
	* @ingroup ParaBootStrapper
	* ParaGlobal namespace contains a list of HAPI functions to access the boot strapper functionality.
	*
	* Bootstrapper file is a xml file to be executed at the very beginning of the game engine. 
	* Its main function is to specify the main game loop file to be activated. When the ParaIDE set a solution to be active, 
	* it actually modifies the bootstrapper file to load the main file of that application solution.
	*/
	class PE_CORE_DECL ParaBootStrapper
	{
	public:
		/**
		* load from a given XML file. 
		* @param sXMLfile the path of the file, if this is "", the config/bootstrapper.xml will be used.
		* @return true if success
		*/
		static bool LoadFromFile(const char*  sXMLfile);
		/**
		* save to a given XML file. 
		* @param sXMLfile the path of the file, if this is "", the config/bootstrapper.xml will be used.
		* @return true if success
		*/
		static bool SaveToFile(const char* sXMLfile);

		/** load the default setting. this function is called at the constructor.*/
		static void LoadDefault();

		/** get the game loop file. the game loop file provides the heart beat of the application. 
		* It is also the very first(entry) script to be activated when the application starts up.
		* The default game loop is ./script/gameinterface.lua
		*/
		static const char* GetMainLoopFile();
		/** Set the game loop file. the game loop file provides the heart beat of the application. 
		* It is also the very first(entry) script to be activated when the application starts up.
		* The default game loop is ./script/gameinterface.lua
		*/
		static void SetMainLoopFile(const char* sMainFile);
	};
}