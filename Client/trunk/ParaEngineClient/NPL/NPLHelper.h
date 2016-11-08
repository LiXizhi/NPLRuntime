#pragma once
#include "NPLWriter.h"
#include "NPLTable.h"

namespace luabind
{
	namespace adl{
		class object;

		template<class Next> 
		class index_proxy;
	}
	using adl::object;	
	typedef adl::index_proxy<adl::object> object_index_proxy;
}

namespace NPL
{
	using namespace std;

	class NPLLex;
	struct LexState;
	struct STableStack;
	
	/** a collection of helper functions. */
	class PE_CORE_DECL NPLHelper
	{
	public:
		NPLHelper(void);
		~NPLHelper(void);
		
	public:
		static void DevideString(const string& input,string& str1,string&str2,char separator=';');


		/** verify the script code. it returns true if the script code contains pure msg data or table. 
		* this function is used to verify scode received from the network. So that the execution of a pure data in the local runtime is harmless. 
		* @param sCode: the input string 
		* @param nCodeSize: number of bytes in string. if -1, strlen() is used to determine the size. 
		*/
		static bool IsSCodePureData(const char * sCode, int nCodeSize=-1);

		/** it will return true if input string is "false", "true", NUMBER, STRING, and {table} 
		* @param sCode: the input string 
		* @param nCodeSize: number of bytes in string. if -1, strlen() is used to determine the size. 
		*/
		static bool IsPureData(const char * sCode, int nCodeSize=-1);
		
		/** it will return true if input string is a {table} containing only "false", "true", NUMBER, STRING, and other such {table} 
		* @param sCode: the input string 
		* @param nCodeSize: number of bytes in string. if -1, strlen() is used to determine the size. 
		*/
		static bool IsPureTable(const char * sCode, int nCodeSize=-1);

		/**
		* serialize a luabind object into sCode. The object could be a value, string or a table without nested tables. 
		* [thread safe]
		* e.g. log(NPL.SerializeToSCode("var", {name = [[lxz" "lxz]], age = 24}));
		* @param sStorageVar if this is NULL or "", the scode contains only the object. otherwise an assignment is made,
		* by prefixing the scode with  "sStorageVar = "
		* @param input input luabind object
		* @param sCode the output scode. It can be StringBuilder or std::string. StringBuilder is the prefered way. 
		* @param nCodeOffset: offset in byte to begin writing to sCode[nCodeOffset]. Default to 0, which will write from the beginning of the sCode.
		* @return true if successful. 
		*/
		template <typename StringType>
		static bool SerializeToSCode(const char* sStorageVar, const luabind::object& input, StringType& sCode, int nCodeOffset = 0, STableStack* pRecursionTable = NULL);

		template <typename StringType>
		static bool SerializeToJson(const luabind::object& input, StringType& sCode, int nCodeOffset = 0, STableStack* pRecursionTable = NULL, bool bUseEmptyArray = false);

		static bool isControlCharacter(char ch);

		static bool containsControlCharacter(const char* str);

		/** safe convert the lua object to string. if the input is nil, NULL is returned. please note that the returned const char* has the same lifetime as the input object */
		static const char* LuaObjectToString(const luabind::object& input, int* pSize = NULL);

		/** safe convert the lua object to string. if the input is nil, output is not assigned. 
		* return true if input is a string object and value is written to output. */
		static bool LuaObjectToString(const luabind::object& input, string& output);

		/** safe convert the lua object to int */
		static int LuaObjectToInt(const luabind::object& input, int default_value=0);

		/** safe convert the lua object to float */
		static float LuaObjectToFloat (const luabind::object& input, float default_value = 0.f);

		/* test if input is a table. */
		static bool LuaObjectIsTable(const luabind::object& input);

		/** safe convert the lua object to double*/
		static double LuaObjectToDouble(const luabind::object& input, double default_value = 0.);
		
		/** safe convert the lua object to boolean*/
		static bool LuaObjectToBool(const luabind::object& input, bool default_value = false);

		/** safe convert the lua object to color */
		static DWORD LuaObjectToColor(const luabind::object& pen, DWORD default_value = 0);

		/** Currently it will only convert pure data table from string to object. 
		* @param input: input string such as {name=value,}
		* @param nLen: number of char in input. 
		* @param output: the table object . it needs to be an empty table object. 
		* @param pState: if NULL, output.interpreter() is used. 
		* @return true if succeed. 
		*/
		static bool StringToLuaObject(const char* input,int nLen, luabind::object& output, lua_State* pState=NULL);
		static bool StringToAnyObject(const char* input, int nLen, luabind::object& output, lua_State* pState = NULL);

		/** the input must be a string that begins with "msg={", msg={name=value, ...} 
		* it will replace the global msg table with a new table. 
		*/
		static bool MsgStringToLuaObject(const char* input,int nLen, lua_State* pState);

		/** converting string to NPL table object 
		* @param input: such as "{nid=10, name=\"value\", tab={name1=\"value1\"}}"
		*/
		static NPLObjectProxy StringToNPLTable(const char* input,int nLen=-1);

		/** same as StringToNPLTable(), except that it begins with "msg={...}" 
		* @param input: such as "msg={nid=10, name=\"value\", tab={name1=\"value1\"}}"
		*/
		static NPLObjectProxy MsgStringToNPLTable(const char* input,int nLen=-1);

		/** lua object to npl object. */
		static bool LuaObjectToNPLObject(const luabind::object& input, NPLObjectProxy& out);

		/** safe convert the lua object to string. if the input is nil, output is not assigned. 
		* return true if input is a string object and value is written to output. 
		* @param bUseMsg: if true, the table will be inside msg={...}, it default to false. 
		*/
		template <typename StringType>
		static bool SerializeNPLTableToString(const char* sStorageVar, NPLObjectProxy& input, StringType& sCode, int nCodeOffset=0);

		/** same as SerializeNPLTableToString. Except that it is used via DLL interface. */
		static bool NPLTableToString(const char* sStorageVar, NPLObjectProxy& input, std::string& sCode, int nCodeOffset=0);

		/** 
		* this is something like string.format("%q") in NPL.
		* [thread-safe]
		* replace quotation mark in string. e.g. "\" will be converted to \"\\\"
		* @param output: into which buffer to write the output
		* @param input: the input string to be encoded
		* @param nOutputOffset: the output will be written to output[nOutputOffset]. default to 0,which is the beginning
		* @param nInputSize: if -1, strlen is used on buffer
		*/
		template <typename StringType>
		static void EncodeStringInQuotation(StringType& output, int nOutputOffset, const char* input, int nInputSize);

		template <typename StringType>
		static void EncodeStringInQuotation(StringType& output, int nOutputOffset, const std::string& input)
		{
			EncodeStringInQuotation(output, nOutputOffset, input.c_str(), (int)input.size());
		}
		template <typename StringType>
		static void EncodeStringInQuotation(StringType& output, int nOutputOffset, const char* input)
		{
			EncodeStringInQuotation(output, nOutputOffset, input, (int)strlen(input));
		}
		

		template <typename StringType>
		static void EncodeJsonStringInQuotation(StringType& output, int nOutputOffset, const char* input, int nInputSize);

		template <typename StringType>
		static void EncodeJsonStringInQuotation(StringType& output, int nOutputOffset, const std::string& input)
		{
			EncodeJsonStringInQuotation(output, nOutputOffset, input.c_str(), (int)input.size());
		}
		template <typename StringType>
		static void EncodeJsonStringInQuotation(StringType& output, int nOutputOffset, const char* input)
		{
			EncodeJsonStringInQuotation(output, nOutputOffset, input, (int)strlen(input));
		}

		/** this function is thread-safe.
		* Check if the given string can be encoded using lua long string, i.e.  [[]]
		* json string usually can not be encoded in double brackets.
		*/
		static bool CanEncodeStringInDoubleBrackets(const char*	buffer, int nLength);
	};

	/** only used internally*/
	struct STableStack
	{
		const luabind::object* m_pTableObj;
		const STableStack* m_pParent;
	};
}


