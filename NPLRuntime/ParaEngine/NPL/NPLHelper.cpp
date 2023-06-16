//-----------------------------------------------------------------------------
// Class:	NPLHelper
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.10.23
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "NPLParser.h"
#include "NPLHelper.h"
#include "NPLTable.h"
#include "util/StringHelper.h"
#include "json/json.h"
#ifdef PARAENGINE_CLIENT
// #include "memdebug.h"
#endif
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include "ParaScriptingIO.h"

using namespace ParaEngine;
using namespace NPL;
using namespace luabind;

namespace NPL
{
	bool DeserializePureDataBlock(LexState *ls, luabind::object_index_proxy& objProxy);
	bool DeserializePureNPLDataBlock(LexState *ls, NPLObjectProxy& objProxy);
}

/** */
#define MAX_TABLE_STRING_LENGTH 512

NPLHelper::NPLHelper(void)
{
}

NPLHelper::~NPLHelper(void)
{
}

void NPLHelper::DevideString(const string& input, string& str1, string&str2, char separator)
{
	str1.clear();
	str2.clear();
	size_t pos = input.find(separator);
	if (pos != string::npos) {
		str1.assign(input.c_str(), pos);
		str2.assign(input.c_str(), pos + 1, input.size() - pos - 1);
	}
	else
		str1 = input;
}

bool NPLHelper::IsSCodePureData(const char * sCode, int nCodeSize)
{
	if (nCodeSize < 0)
		nCodeSize = (int)strlen(sCode);
	return NPL::NPLParser::IsMsgData(sCode, nCodeSize);
}

bool NPLHelper::IsPureData(const char * sCode, int nCodeSize)
{
	if (nCodeSize < 0)
		nCodeSize = (int)strlen(sCode);
	return NPL::NPLParser::IsPureData(sCode, nCodeSize);
}


bool NPL::NPLHelper::IsPureTable(const char * sCode, int nCodeSize)
{
	if (nCodeSize < 0)
		nCodeSize = (int)strlen(sCode);
	return NPL::NPLParser::IsPureTable(sCode, nCodeSize);
}

const char* NPLHelper::LuaObjectToString(const luabind::object& input, int* pSize)
{
	int nType = type(input);
	const char* output = NULL;
	if (nType == LUA_TSTRING)
	{
		if (pSize == 0)
			output = object_cast<const char*>(input);
		else
		{
			lua_State* L = input.interpreter();
			input.push(L);
			output = lua_tostring(L, -1);
			*pSize = (int)lua_strlen(L, -1);
			lua_pop(L, 1);
		}
	}
	return output;
}

bool NPLHelper::LuaObjectToString(const luabind::object& input, string& output)
{
	int nType = type(input);
	if (nType == LUA_TSTRING)
	{
		int nSize = 0;
		const char* pStr = LuaObjectToString(input, &nSize);
		output.assign(pStr, nSize);
		return true;
	}
	return false;
}

bool NPLHelper::NPLTableToString(const char* sStorageVar, NPLObjectProxy& input, std::string& sCode, int nCodeOffset)
{
	return SerializeNPLTableToString(sStorageVar, input, sCode, nCodeOffset);
}


template <typename StringType>
bool NPL::NPLHelper::SerializeToJson(const luabind::object& input, StringType& sCode, int nCodeOffset /*= 0*/, STableStack* pRecursionTable /*= NULL*/, bool bUseEmptyArray)
{
	sCode.resize(nCodeOffset);

	int nType = type(input);
	switch (nType)
	{
	case LUA_TNIL:
	{
		sCode.append("null");
		break;
	}
	case LUA_TNUMBER:
	{
		double value = object_cast<double>(input);
		char buff[40];
		int nLen = ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
		sCode.append(buff, nLen);
		break;
	}
	case LUA_TBOOLEAN:
	{
		bool bValue = object_cast<bool>(input);
		sCode.append(bValue ? "true" : "false");
		break;
	}
	case LUA_TSTRING:
	{
		// this is something like string.format("%q") in NPL.
		int nSize = 0;
		const char* pStr = LuaObjectToString(input, &nSize);
		EncodeJsonStringInQuotation(sCode, (int)(sCode.size()), pStr, nSize);
		break;
	}
	case LUA_TTABLE:
	{
		if (pRecursionTable)
		{
			// check for recursive tables
			const STableStack* pCheckTable = pRecursionTable;
			while (pCheckTable)
			{
				if ((*pCheckTable->m_pTableObj) == input)
				{
					return false;
				}
				pCheckTable = pCheckTable->m_pParent;
			}
		}
		STableStack thisRecursionTable = { &input, pRecursionTable };

		int nTableStartIndex = sCode.size();
		sCode.append("{");
		int nNumberIndex = 1;
		// -1 unset, 0 object, 1 array
		int nIsArrayTable = -1; 
		for (luabind::iterator itCur(input), itEnd; itCur != itEnd; ++itCur)
		{
			// we only serialize item with a string key
			const object& key = itCur.key();
			if (type(key) == LUA_TSTRING && nIsArrayTable!=1)
			{
				nIsArrayTable = 0;
				const char* sKey = object_cast<const char*>(key);
				const object& value = *itCur;
				int nOldSize = (int)(sCode.size());
				// if sKey contains only alphabetic letters, we will use sKey=data,otherwise, we go the safer one ["sKey"]=data.
				// the first is more efficient in disk space. 
				int nSKeyCount = (int)strlen(sKey);
				EncodeJsonStringInQuotation(sCode, (int)(sCode.size()), sKey, nSKeyCount);
				sCode.append(":");
				if (SerializeToJson(value, sCode, (int)(sCode.size()), &thisRecursionTable, bUseEmptyArray))
				{
					sCode.append(",");
				}
				else
				{
					sCode.resize(nOldSize);
				}
			}
			else if (type(key) == LUA_TNUMBER)
			{
				double dKey = object_cast<double>(key);
				int64_t nKey = (int64_t)(dKey);
				const object& value = *itCur;
				int nOldSize = (int)(sCode.size());

				// for number index, we will serialize without square brackets. 
				if (nIsArrayTable != 0 && nNumberIndex == nKey && dKey == nKey)
				{
					nIsArrayTable = 1;
					++nNumberIndex;
				}
				else if (nIsArrayTable != 1)
				{
					nIsArrayTable = 0;
					char buff[40];
					sCode.append("\"");
					int nLen = 0;
					if (dKey == nKey)
					{
						nLen = ParaEngine::StringHelper::fast_itoa(nKey, buff, 40);
					}
					else
					{
						nLen = ParaEngine::StringHelper::fast_dtoa(dKey, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
					}
					sCode.append(buff, nLen);
					sCode.append("\":");
				}
				else
				{
					// mixing array with string key will be skipped 
					sCode.resize(nOldSize);
					break;
				}

				if (SerializeToJson(value, sCode, (int)(sCode.size()), &thisRecursionTable, bUseEmptyArray))
				{
					sCode.append(",");
				}
				else
				{
					nNumberIndex = -1;
					sCode.resize(nOldSize);
				}
			}
		}
		if (nIsArrayTable == -1)
		{
			// for empty table
			if (bUseEmptyArray)
			{
				sCode[nTableStartIndex] = '[';
				sCode.append("]");
			}
			else
				sCode.append("}");
		}
		else
		{
			if (nIsArrayTable == 1)
			{
				sCode[nTableStartIndex] = '[';
				sCode[sCode.size() - 1] = ']';
			}
			else
				sCode[sCode.size() - 1] = '}';
		}
		break;
	}
	default:
		// we will escape any functions, etc. 
		return false;
		break;
	}
	return true;
}

bool NPL::NPLHelper::isControlCharacter(char ch)
{
	return ch > 0 && ch <= 0x1F;
}

bool NPL::NPLHelper::containsControlCharacter(const char* str)
{
	while (*str)
	{
		if (isControlCharacter(*(str++)))
			return true;
	}
	return false;
}

template <typename StringType>
bool NPLHelper::SerializeToSCode(const char* sStorageVar, const luabind::object& input, StringType& sCode, int nCodeOffset, STableStack* pRecursionTable, bool sort /*= false*/)
{
	sCode.resize(nCodeOffset);

	int nStorageVarLen = 0;
	if (sStorageVar != NULL)
	{
		nStorageVarLen = (int)strlen(sStorageVar);
		if (nStorageVarLen > 0)
		{
			sCode.append(sStorageVar, nStorageVarLen);
			sCode.append("=");
		}
	}

	int nType = type(input);
	switch (nType)
	{
	case LUA_TNUMBER:
	{
		double value = object_cast<double>(input);
		char buff[40];
		int nLen = ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
		sCode.append(buff, nLen);
		break;
	}
	case LUA_TBOOLEAN:
	{
		bool bValue = object_cast<bool>(input);
		sCode.append(bValue ? "true" : "false");
		break;
	}
	case LUA_TSTRING:
	{
		// this is something like string.format("%q") in NPL.
		int nSize = 0;
		const char* pStr = LuaObjectToString(input, &nSize);
		EncodeStringInQuotation(sCode, (int)(sCode.size()), pStr, nSize);
		break;
	}
	case LUA_TTABLE:
	{
		if (pRecursionTable)
		{
			// check for recursive tables
			const STableStack* pCheckTable = pRecursionTable;
			while (pCheckTable){
				if ((*pCheckTable->m_pTableObj) == input)
				{
					if (nStorageVarLen > 0)
					{
						sCode.resize(nCodeOffset);
					}
					return false;
				}
				pCheckTable = pCheckTable->m_pParent;
			}
		}
		STableStack thisRecursionTable = { &input, pRecursionTable};
		
		sCode.append("{");

		if (sort)
		{
			struct sortItem
			{
				sortItem(const object& key_, const object& value_) : key(key_), value(value_) {}
				sortItem() {};
				object key;
				object value;
			};

			std::vector<sortItem> sortTable;

			for (luabind::iterator itCur(input), itEnd; itCur != itEnd; ++itCur)
			{
				sortTable.push_back(sortItem(itCur.key(), *itCur));
			}

			std::sort(sortTable.begin(), sortTable.end(), [](const sortItem& a, const sortItem& b) {
				const object& key1 = a.key;
				const object& key2 = b.key;

				auto type1 = type(key1);
				auto type2 = type(key2);

				if (type1 == LUA_TNUMBER && type2 == LUA_TNUMBER)
				{
					return object_cast<double>(key1) < object_cast<double>(key2);
				}
				else if (type1 == LUA_TNUMBER && type2 == LUA_TSTRING)
				{
					return false;
				}
				else if (type1 == LUA_TSTRING && type2 == LUA_TNUMBER)
				{
					return true;
				}
				else if (type1 == LUA_TSTRING && type2 == LUA_TSTRING)
				{
					//return object_cast<std::string>(key1).compare(object_cast<std::string>(key2)) < 0;
					key1.push(key1.interpreter());
					key2.push(key2.interpreter());

					auto ret = lua_lessthan(key1.interpreter(), -2, -1);
					lua_pop(key2.interpreter(), 2);

					return ret > 0;

				}
				else if (type1 == type2)
				{
					key1.push(key1.interpreter());
					auto p1 = lua_topointer(key1.interpreter(), -1);
					lua_pop(key1.interpreter(), 1);

					key2.push(key2.interpreter());
					auto p2 = lua_topointer(key2.interpreter(), -1);
					lua_pop(key2.interpreter(), 1);

					return p1 < p2;
				}
				else
				{
					return type1 < type2;
				}
			});


			int nNumberIndex = 1;
			//for (luabind::iterator itCur(input), itEnd; itCur != itEnd; ++itCur)
			for (size_t i = 0; i < sortTable.size(); i++)
			{

				// we only serialize item with a string key
				const object& key = sortTable[i].key;
				const object& value = sortTable[i].value;
				if (type(key) == LUA_TSTRING)
				{
					const char* sKey = object_cast<const char*>(key);
						int nOldSize = (int)(sCode.size());
					// if sKey contains only alphabetic letters, we will use sKey=data,otherwise, we go the safer one ["sKey"]=data.
					// the first is more efficient in disk space. 
					int nSKeyCount = (int)strlen(sKey);
					bool bIsIdentifier = NPLParser::IsIdentifier(sKey, nSKeyCount);
					if (bIsIdentifier && nSKeyCount > 0)
					{
						sCode.append(sKey, nSKeyCount);
						sCode.append("=");
					}
					else
					{
						sCode.append("[");
						EncodeStringInQuotation(sCode, (int)(sCode.size()), sKey, nSKeyCount);
						sCode.append("]=");
					}
					if (SerializeToSCode(NULL, value, sCode, (int)(sCode.size()), &thisRecursionTable, sort))
					{
						sCode.append(",");
					}
					else
					{
						sCode.resize(nOldSize);
					}
				}
				else if (type(key) == LUA_TNUMBER)
				{
					double dKey = object_cast<double>(key);
					int64_t nKey = (int64_t)(dKey);
					int nOldSize = (int)(sCode.size());

					// for number index, we will serialize without square brackets. 
					if (nNumberIndex == nKey && dKey == nKey)
					{
						++nNumberIndex;
					}
					else
					{
						char buff[40];
						sCode.append("[");
						int nLen = 0;
						if (dKey == nKey)
						{
							nLen = ParaEngine::StringHelper::fast_itoa(nKey, buff, 40);
						}
						else
						{
							nLen = ParaEngine::StringHelper::fast_dtoa(dKey, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
						}
						sCode.append(buff, nLen);
						sCode.append("]=");
					}

					if (SerializeToSCode(NULL, value, sCode, (int)(sCode.size()), &thisRecursionTable, sort))
					{
						sCode.append(",");
					}
					else
					{
						nNumberIndex = -1;
						sCode.resize(nOldSize);
					}
				}
			}
		}
		else
		{
			int nNumberIndex = 1;
			for (luabind::iterator itCur(input), itEnd; itCur != itEnd; ++itCur)
			{
				// we only serialize item with a string key
				const object& key = itCur.key();
				if (type(key) == LUA_TSTRING)
				{
					const char* sKey = object_cast<const char*>(key);
					const object& value = *itCur;
					int nOldSize = (int)(sCode.size());
					// if sKey contains only alphabetic letters, we will use sKey=data,otherwise, we go the safer one ["sKey"]=data.
					// the first is more efficient in disk space. 
					int nSKeyCount = (int)strlen(sKey);
					bool bIsIdentifier = NPLParser::IsIdentifier(sKey, nSKeyCount);
					if (bIsIdentifier && nSKeyCount > 0)
					{
						sCode.append(sKey, nSKeyCount);
						sCode.append("=");
					}
					else
					{
						sCode.append("[");
						EncodeStringInQuotation(sCode, (int)(sCode.size()), sKey, nSKeyCount);
						sCode.append("]=");
					}
					if (SerializeToSCode(NULL, value, sCode, (int)(sCode.size()), &thisRecursionTable, sort))
					{
						sCode.append(",");
					}
					else
					{
						sCode.resize(nOldSize);
					}
				}
				else if (type(key) == LUA_TNUMBER)
				{
					double dKey = object_cast<double>(key);
					int64_t nKey = (int64_t)(dKey);
					const object& value = *itCur;
					int nOldSize = (int)(sCode.size());

					// for number index, we will serialize without square brackets. 
					if (nNumberIndex == nKey && dKey == nKey)
					{
						++nNumberIndex;
					}
					else
					{
						char buff[40];
						sCode.append("[");
						int nLen = 0;
						if (dKey == nKey)
						{
							nLen = ParaEngine::StringHelper::fast_itoa(nKey, buff, 40);
						}
						else
						{
							nLen = ParaEngine::StringHelper::fast_dtoa(dKey, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
						}
						sCode.append(buff, nLen);
						sCode.append("]=");
					}

					if (SerializeToSCode(NULL, value, sCode, (int)(sCode.size()), &thisRecursionTable, sort))
					{
						sCode.append(",");
					}
					else
					{
						nNumberIndex = -1;
						sCode.resize(nOldSize);
					}
				}
			}
		}
		
		
		sCode.append("}");
		break;
	}
	case LUA_TUSERDATA:
	{
		sCode.append("\"");
		try
		{
			ParaScripting::ParaFileObject file = object_cast<ParaScripting::ParaFileObject>(input);
			const char* pStr = file.GetBase64String();
			if (pStr)
			{
				sCode.append(pStr);
			}
			// TODO: LXZ: figure out a way to check user data type, instead of throwing exceptions
			// the following two may be possible ways, but I have not figured out how.

			//ParaScripting::ParaFileObject* pFile = (ParaScripting::ParaFileObject*)luaL_checkudata(input.interpreter(), 0, "ParaIO.ParaFileObject");

			//ParaScripting::ParaFileObject * pFile = object_cast<ParaScripting::ParaFileObject*>(input);
			//if(pFile)
			//{
			//	//OUTPUT_LOG("file name:%s\n", pFile->readline());
			//}
		}
		catch (...)
		{

		}
		sCode.append("\"");
		break;
	}
	default:
		// we will escape any functions, etc. 
		if (nStorageVarLen > 0)
		{
			sCode.resize(nCodeOffset);
		}
		return false;
		break;
	}
	return true;
}

template <typename StringType>
bool NPLHelper::SerializeNPLTableToString(const char* sStorageVar, NPLObjectProxy& input, StringType& sCode, int nCodeOffset)
{
	sCode.resize(nCodeOffset);

	int nStorageVarLen = 0;
	if (sStorageVar != NULL)
	{
		nStorageVarLen = (int)strlen(sStorageVar);
		if (nStorageVarLen > 0)
		{
			sCode.append(sStorageVar, nStorageVarLen);
			sCode.append("=");
		}
	}

	NPLObjectBase::NPLObjectType nType = input.GetType();
	switch (nType)
	{
	case NPLObjectBase::NPLObjectType_Number:
	{
		double value = input;
		char buff[40];
		int nLen = ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
		sCode.append(buff, nLen);
		break;
	}
	case NPLObjectBase::NPLObjectType_Bool:
	{
		bool bValue = input;
		sCode.append(bValue ? "true" : "false");
		break;
	}
	case NPLObjectBase::NPLObjectType_String:
	{
		// this is something like string.format("%q") in NPL.
		const string& str = input;
		EncodeStringInQuotation(sCode, (int)(sCode.size()), str.c_str(), (int)(str.size()));
		break;
	}
	case NPLObjectBase::NPLObjectType_Table:
	{
		sCode.append("{");
		int nNextIndex = 1;
		// serialize item with number key
		for (NPLTable::IndexIterator_Type itCur = input.index_begin(), itEnd = input.index_end(); itCur != itEnd; ++itCur)
		{
			int nIndex = itCur->first;
			NPLObjectProxy& value = itCur->second;
			int nOldSize = (int)(sCode.size());
			if (nIndex != nNextIndex)
			{
				sCode.append("[");
				char buff[40];
				int nLen = snprintf(buff, 40, "%d", nIndex);
				sCode.append(buff, nLen);
				sCode.append("]=");
			}
			if (SerializeNPLTableToString(NULL, value, sCode, (int)(sCode.size())))
				sCode.append(",");
			else
				sCode.resize(nOldSize);
			nNextIndex = nIndex + 1;
		}
		// serialize item with a string key
		for (NPLTable::Iterator_Type itCur = input.begin(), itEnd = input.end(); itCur != itEnd; ++itCur)
		{
			const string& key = itCur->first;
			const char* sKey = key.c_str();
			NPLObjectProxy& value = itCur->second;
			int nOldSize = (int)(sCode.size());
			// if sKey contains only alphabetic letters, we will use sKey=data,otherwise, we go the safer one ["sKey"]=data.
			// the first is more efficient in disk space. 
			int nSKeyCount = (int)(key.size());
			bool bIsIdentifier = NPLParser::IsIdentifier(sKey, nSKeyCount);
			if (bIsIdentifier)
			{
				sCode.append(sKey, nSKeyCount);
				sCode.append("=");
			}
			else
			{
				sCode.append("[");
				EncodeStringInQuotation(sCode, (int)(sCode.size()), sKey, nSKeyCount);
				sCode.append("]=");
			}
			if (SerializeNPLTableToString(NULL, value, sCode, (int)(sCode.size())))
			{
				sCode.append(",");
			}
			else
			{
				sCode.resize(nOldSize);
			}
		}
		sCode.append("}");
		break;
	}
	default:
		// we will escape any functions or nil, etc. 
		if (nStorageVarLen > 0)
		{
			sCode.resize(nCodeOffset);
		}
		return false;
		break;
	}
	return true;
}

// Thread-safe version
template <typename StringType>
void NPL::NPLHelper::EncodeJsonStringInQuotation(StringType& buff, int nOutputOffset, const char* str, int nSize)
{
	// this is something like string.format("%q") in NPL.
	// estimate the size. 
	if (nSize < 0)
		nSize = (int)strlen(str);
	int nFinalSize = nOutputOffset + nSize + 2;
	buff.resize(nFinalSize);

	// replace quotation mark in string. 
	int nPos = nOutputOffset;
	buff[nPos++] = '"';
	for (int i = 0; i < nSize; ++i)
	{
		char c = str[i];
		switch (c)
		{
		case '\"': case '\\': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = c;

			break;
		}
		case '\b': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 'b';
			break;
		}
		case '\n': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 'n';
			break;
		}
		case '\r': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 'r';
			break;
		}
		case '\t': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 't';
			break;
		}
		case '\0': {
			nFinalSize += 5;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 'u';
			buff[nPos++] = '0';
			buff[nPos++] = '0';
			buff[nPos++] = '0';
			buff[nPos++] = '0';
			break;
		}
		default: 
		{
			if (isControlCharacter(c))
			{
				nFinalSize += 5;
				buff.resize(nFinalSize);
				buff[nPos++] = '\\';
				buff[nPos++] = 'u';
				buff[nPos++] = '0';
				buff[nPos++] = '0';
				char buff_[5];
				int nLen = ParaEngine::StringHelper::fast_itoa((WORD)c, buff_, 5, 16);
				buff[nPos++] = nLen > 1 ? buff_[nLen - 2] : '0';
				buff[nPos++] = nLen > 0 ? buff_[nLen - 1] : '0';
			}
			else
			{
				buff[nPos++] = c;
			}
			break;
		}
		}
	}
	buff[nPos++] = '"';
	PE_ASSERT(nPos == nFinalSize && (int)(buff.size()) == nFinalSize);
}

// Thread-safe version
template <typename StringType>
void NPL::NPLHelper::EncodeStringInQuotation(StringType& buff, int nOutputOffset, const char* str, int nSize)
{
	// this is something like string.format("%q") in NPL.
	// estimate the size. 
	if (nSize < 0)
		nSize = (int)strlen(str);
	int nFinalSize = nOutputOffset + nSize + 2;
	buff.resize(nFinalSize);

	// replace quotation mark in string. 
	int nPos = nOutputOffset;
	buff[nPos++] = '"';
	for (int i = 0; i < nSize; ++i)
	{
		char c = str[i];
		switch (c)
		{
		case '"': case '\\': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = c;

			break;
		}
		case '\n': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 'n';
			break;
		}
		case '\r': {
			nFinalSize += 1;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = 'r';
			break;
		}
		case '\0': {
			nFinalSize += 3;
			buff.resize(nFinalSize);

			buff[nPos++] = '\\';
			buff[nPos++] = '0';
			buff[nPos++] = '0';
			buff[nPos++] = '0';
			break;
		}
		default: {
			buff[nPos++] = c;
			break;
		}
		}
	}
	buff[nPos++] = '"';
	PE_ASSERT(nPos == nFinalSize && (int)(buff.size()) == nFinalSize);
}

bool NPL::NPLHelper::CanEncodeStringInDoubleBrackets(const char* buffer, int nLength)
{
	char c = 0; 
	int cont = 0;

	for (int i = 0; i < nLength; i++)
	{
		c = buffer[i];
		if (c == '[')
		{
			if (buffer[i + 1] == '[')
			{
				cont++;
#ifndef ALLOW_NESTED_LUA_COMPAT_LSTR
				return false;
#endif
				i++;
			}
		}
		else if(c== ']')
		{
#ifdef ALLOW_NESTED_LUA_COMPAT_LSTR
			if (cont == 0 && (i + 1) == nLength)
				return false;
#endif
			if (buffer[i + 1] == ']')
			{
#ifndef ALLOW_NESTED_LUA_COMPAT_LSTR
				return false;
#endif
				if (cont == 0)
					return false;
				i++;
				cont--;
			}
		}
		else if (c == '\0')
		{
			return false;
		}
	}
#if defined ALLOW_NESTED_LUA_COMPAT_LSTR
	return cont == 0;
#else
	return true;
#endif
}

bool NPL::NPLHelper::MsgStringToLuaObject(const char* input, int nLen, lua_State* pState)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;

	try
	{
		NPLParser::next(ls);  /* read first token */

		if (ls->t.token == NPLLex::TK_NAME && ls->t.seminfo.ts == "msg")
		{
			// create a new msg table. 
			luabind::object tabGlobal = luabind::globals(pState);
			luabind::object_index_proxy tabMsg = tabGlobal["msg"];

			NPLParser::next(ls);
			if (ls->t.token == '=')
			{
				NPLParser::next(ls);
				if (DeserializePureDataBlock(ls, tabMsg))
				{
					NPLParser::testnext(ls, ';');
					if (ls->t.token == NPLLex::TK_EOS)
					{
						return true;
					}
				}
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_LOG("error: %s in NPLHelper::MsgStringToLuaObject()\n", err);
		return false;
	}
	catch (...)
	{
		OUTPUT_LOG("error: unknown error in NPLHelper::MsgStringToLuaObject()\n");
		return false;
	}
	return false;
}

bool NPL::NPLHelper::StringToLuaObject(const char* input, int nLen, luabind::object& output, lua_State* pState)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;

	try
	{
		NPLParser::next(ls);  /* read first token */

		if (ls->t.token == '{')
		{
			if (pState == 0)
				pState = output.interpreter();
			luabind::object tabGlobal = luabind::globals(pState);
			luabind::object_index_proxy tabProxy = tabGlobal["__tmp"];

			if (DeserializePureDataBlock(ls, tabProxy))
			{
				NPLParser::testnext(ls, ';');
				if (ls->t.token == NPLLex::TK_EOS)
				{
					output = tabProxy;
					// this is not necessary, the next call will overwrite this. 
					// tabGlobal["__tmp"] = luabind::detail::nil_type();
					return true;
				}
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_LOG("error: %s in NPLHelper::StringToLuaObject()\n", err);
		return false;
	}
	catch (...)
	{
		OUTPUT_LOG("error: unknown error in NPLHelper::StringToLuaObject()\n");
		return false;
	}
	return false;
}

bool NPL::NPLHelper::StringToAnyObject(const char* input, int nLen, luabind::object& output, lua_State* pState)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;

	try
	{
		NPLParser::next(ls);  /* read first token */

		if (pState == 0)
			pState = output.interpreter();
		luabind::object tabGlobal = luabind::globals(pState);
		luabind::object_index_proxy tabProxy = tabGlobal["__tmp"];
		if (DeserializePureDataBlock(ls, tabProxy))
		{
			NPLParser::testnext(ls, ';');
			if (ls->t.token == NPLLex::TK_EOS)
			{
				output = tabProxy;
				// this is not necessary, the next call will overwrite this. 
				// tabGlobal["__tmp"] = luabind::detail::nil_type();
				return true;
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_LOG("error: %s in NPLHelper::StringToAnyObject()\n", err);
		return false;
	}
	catch (...)
	{
		OUTPUT_LOG("error: unknown error in NPLHelper::StringToAnyObject()\n");
		return false;
	}
	return false;
}


NPLObjectProxy NPL::NPLHelper::MsgStringToNPLTable(const char* input, int nLen)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;

	try
	{
		NPLParser::next(ls);  /* read first token */

		if (ls->t.token == NPLLex::TK_NAME && ls->t.seminfo.ts == "msg")
		{
			NPLParser::next(ls);
			if (ls->t.token == '=')
			{
				NPLParser::next(ls);
				NPLObjectProxy output;
				if (DeserializePureNPLDataBlock(ls, output))
				{
					NPLParser::testnext(ls, ';');
					if (ls->t.token == NPLLex::TK_EOS)
					{
						return output;
					}
				}
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_LOG("error: %s in NPLHelper::StringToNPLTable()\n", err);
		return NPLObjectProxy();
	}
	catch (...)
	{
		OUTPUT_LOG("error: unknown error in NPLHelper::StringToNPLTable()\n");
		return NPLObjectProxy();
	}
	return NPLObjectProxy();
}

bool NPL::NPLHelper::LuaObjectToNPLObject(const luabind::object& inputObj, NPLObjectProxy& out)
{
	if (type(inputObj) == LUA_TTABLE)
	{
		for (luabind::iterator itCur(inputObj), itEnd; itCur != itEnd; ++itCur)
		{
			// we only serialize item with a string key
			const object& key = itCur.key();
			const object& input = *itCur;
			if (type(key) == LUA_TSTRING)
			{
				NPLObjectProxy v;
				if (LuaObjectToNPLObject(input, v))
				{
					out[object_cast<std::string>(key)] = v;
				}
			}
			else if (type(key) == LUA_TNUMBER)
			{
				NPLObjectProxy v;
				if (LuaObjectToNPLObject(input, v))
				{
					out[object_cast<int>(key)] = v;
				}
			}
		}
	}
	else if (type(inputObj) == LUA_TSTRING)
	{
		out = object_cast<std::string>(inputObj);
	}
	else if (type(inputObj) == LUA_TNUMBER)
	{
		out = object_cast<double>(inputObj);
	}
	else if (type(inputObj) == LUA_TBOOLEAN)
	{
		out = object_cast<bool>(inputObj);
	}
	else
	{
		return false;
	}
	return true;
}

NPLObjectProxy NPL::NPLHelper::StringToNPLTable(const char* input, int nLen)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;

	try
	{
		NPLParser::next(ls);  /* read first token */

		if (ls->t.token == '{')
		{
			NPLObjectProxy output;
			if (DeserializePureNPLDataBlock(ls, output))
			{
				NPLParser::testnext(ls, ';');
				if (ls->t.token == NPLLex::TK_EOS)
				{
					return output;
				}
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_LOG("error: %s in NPLHelper::StringToNPLTable()\n", err);
		return NPLObjectProxy();
	}
	catch (...)
	{
		OUTPUT_LOG("error: unknown error in NPLHelper::StringToNPLTable()\n");
		return NPLObjectProxy();
	}
	return NPLObjectProxy();
}

int NPL::NPLHelper::LuaObjectToInt(const luabind::object& input, int default_value/*=0*/)
{
	int output = default_value;
	if (type(input) == LUA_TNUMBER)
	{
		output = object_cast<int>(input);
	}
	return output;
}

float NPL::NPLHelper::LuaObjectToFloat(const luabind::object& input, float default_value /*= 0.f*/)
{
	float output = default_value;
	if (type(input) == LUA_TNUMBER)
	{
		output = object_cast<float>(input);
	}
	return output;
}

double NPL::NPLHelper::LuaObjectToDouble(const luabind::object& input, double default_value /*= 0.*/)
{
	double output = default_value;
	if (type(input) == LUA_TNUMBER)
	{
		output = object_cast<double>(input);
	}
	return output;
}

bool NPL::NPLHelper::LuaObjectToBool(const luabind::object& input, bool default_value /*= false*/)
{
	auto input_type = type(input);
	bool output = default_value;
	if (input_type == LUA_TBOOLEAN)
	{
		output = object_cast<bool>(input);
	}
	else if (input_type == LUA_TSTRING)
	{
		const char * value = object_cast<const char*>(input);
		output = (strcmp(value, "true") == 0);
	}
	return output;
}

bool NPL::NPLHelper::LuaObjectIsTable(const luabind::object& input)
{
	return type(input) == LUA_TTABLE;
}

DWORD NPL::NPLHelper::LuaObjectToColor(const luabind::object& input, DWORD default_value)
{
	auto input_type = luabind::type(input);
	if (input_type == LUA_TSTRING)
	{
		return Color::FromString(object_cast<const char*>(input));
	}
	else if (input_type == LUA_TNUMBER)
	{
		return Color(object_cast<DWORD>(input));
	}
	return default_value;
}

bool NPL::DeserializePureDataBlock(LexState *ls, luabind::object_index_proxy&  objProxy)
{
	// data
	int c = ls->t.token;
	switch (c)
	{
	case NPLLex::TK_TRUE:
		objProxy = true;
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_FALSE:
		objProxy = false;
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_NIL:
		objProxy = luabind::detail::nil_type();
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_NUMBER:
		objProxy = ls->t.seminfo.r;
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_STRING:
		objProxy = ls->t.seminfo.ts;
		NPLParser::next(ls);
		return true;
	case '-':
	{
		// negative number
		NPLParser::next(ls);
		if (ls->t.token == NPLLex::TK_NUMBER)
		{
			objProxy = -ls->t.seminfo.r;
			NPLParser::next(ls);
			return true;
		}
		else
			return false;
	}
	case '{':
	{
		// table object
		luabind::object tabMsg = luabind::newtable(objProxy.interpreter());

		NPLParser::enterlevel(ls);
		bool bBreak = false;
		NPLParser::next(ls);
		// auto table index that begins with 1. 
		int nTableAutoIndex = 1;
		while (!bBreak)
		{
			c = ls->t.token;
			if (c == '}')
			{
				// end of table
				NPLParser::leavelevel(ls);
				NPLParser::next(ls);
				bBreak = true;
				objProxy = tabMsg;
			}
			else if (c == NPLLex::TK_NAME)
			{
				luabind::object_index_proxy proxy_ = tabMsg[ls->t.seminfo.ts];
				// by name assignment, such as name = data|table
				NPLParser::next(ls);
				if (ls->t.token == '=')
				{
					NPLParser::next(ls);
					if (!DeserializePureDataBlock(ls, proxy_))
						return false;
					NPLParser::testnext(ls, ',');
				}
				else
					return false;
			}
			else if (c == '[')
			{
				// by integer or string key assignment, such as [number|string] = data|table
				NPLParser::next(ls);
				int token_ = ls->t.token;
				if (token_ == NPLLex::TK_NUMBER)
				{
					// positive number: 
					// Note: convert to integer
					// Removed: verify that it is an integer, instead of a floating value.
					luabind::object_index_proxy proxy_ = tabMsg[(double)(ls->t.seminfo.r)];

					NPLParser::next(ls);
					if (ls->t.token == ']')
					{
						NPLParser::next(ls);
						if (ls->t.token == '=')
						{
							NPLParser::next(ls);
							if (!DeserializePureDataBlock(ls, proxy_))
								return false;
							NPLParser::testnext(ls, ',');
						}
						else
							return false;
					}
				}
				else if (token_ == '-')
				{
					// negative number
					NPLParser::next(ls);
					if (ls->t.token == NPLLex::TK_NUMBER)
					{
						// Note: convert to integer
						// Removed:verify that it is an integer, instead of a floating value.
						luabind::object_index_proxy proxy_ = tabMsg[-(int)(ls->t.seminfo.r)];

						NPLParser::next(ls);
						if (ls->t.token == ']')
						{
							NPLParser::next(ls);
							if (ls->t.token == '=')
							{
								NPLParser::next(ls);
								if (!DeserializePureDataBlock(ls, proxy_))
									return false;
								NPLParser::testnext(ls, ',');
							}
							else
								return false;
						}
					}
					else
						return false;
				}
				else if (token_ == NPLLex::TK_STRING)
				{
					// verify that the string is a value key(non-empty);
					// fixed by Xizhi 2015.10.20, allow empty string as key, such that echo(NPL.LoadTableFromString("{[\"\"]=1}")) is Fine.
					/*if (ls->t.seminfo.ts.empty())
						return false;*/

					luabind::object_index_proxy proxy_ = tabMsg[ls->t.seminfo.ts];
					NPLParser::next(ls);
					if (ls->t.token == ']')
					{
						NPLParser::next(ls);
						if (ls->t.token == '=')
						{
							NPLParser::next(ls);
							if (!DeserializePureDataBlock(ls, proxy_))
								return false;
							NPLParser::testnext(ls, ',');
						}
						else
							return false;
					}
				}
				else
					return false;
			}
			/// Fixed: 2008.6.3 LiXizhi
			/// the following is for auto indexed table items {"string1", "string2\r\n", 213, nil,["A"]="B", true, false, {"another table", "field1"}}
			else
			{
				luabind::object_index_proxy proxy_ = tabMsg[nTableAutoIndex++];
				if (!DeserializePureDataBlock(ls, proxy_))
					return false;
				NPLParser::testnext(ls, ',');
			}
		}
		return true;
	}
	default:
		break;
	}
	return false;
}

bool NPL::DeserializePureNPLDataBlock(LexState *ls, NPLObjectProxy& objProxy)
{
	// data
	int c = ls->t.token;
	switch (c)
	{
	case NPLLex::TK_TRUE:
		objProxy = true;
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_FALSE:
		objProxy = false;
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_NIL:
		objProxy.MakeNil();
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_NUMBER:
		objProxy = ls->t.seminfo.r;
		NPLParser::next(ls);
		return true;
	case NPLLex::TK_STRING:
		objProxy = ls->t.seminfo.ts;
		NPLParser::next(ls);
		return true;
	case '-':
	{
		// negative number
		NPLParser::next(ls);
		if (ls->t.token == NPLLex::TK_NUMBER)
		{
			objProxy = -ls->t.seminfo.r;
			NPLParser::next(ls);
			return true;
		}
		else
			return false;
	}
	case '{':
	{
		// table object
		NPLObjectProxy tabMsg;

		NPLParser::enterlevel(ls);
		bool bBreak = false;
		NPLParser::next(ls);
		// auto table index that begins with 1. 
		int nTableAutoIndex = 1;
		while (!bBreak)
		{
			c = ls->t.token;
			if (c == '}')
			{
				// end of table
				NPLParser::leavelevel(ls);
				NPLParser::next(ls);
				bBreak = true;
				objProxy = tabMsg;
			}
			else if (c == NPLLex::TK_NAME)
			{
				NPLObjectProxy& proxy_ = tabMsg[ls->t.seminfo.ts];
				// by name assignment, such as name = data|table
				NPLParser::next(ls);
				if (ls->t.token == '=')
				{
					NPLParser::next(ls);
					if (!DeserializePureNPLDataBlock(ls, proxy_))
						return false;
					NPLParser::testnext(ls, ',');
				}
				else
					return false;
			}
			else if (c == '[')
			{
				// by integer or string key assignment, such as [number|string] = data|table
				NPLParser::next(ls);
				int token_ = ls->t.token;
				if (token_ == NPLLex::TK_NUMBER)
				{
					// verify that it is an integer, instead of a floating value.
					NPLObjectProxy& proxy_ = tabMsg[(int)(ls->t.seminfo.r)];

					NPLParser::next(ls);
					if (ls->t.token == ']')
					{
						NPLParser::next(ls);
						if (ls->t.token == '=')
						{
							NPLParser::next(ls);
							if (!DeserializePureNPLDataBlock(ls, proxy_))
								return false;
							NPLParser::testnext(ls, ',');
						}
						else
							return false;
					}
				}
				else if (token_ == '-')
				{
					// negative number
					NPLParser::next(ls);
					if (ls->t.token == NPLLex::TK_NUMBER)
					{
						// Note: convert to integer
						// Removed:verify that it is an integer, instead of a floating value.
						NPLObjectProxy& proxy_ = tabMsg[-(int)(ls->t.seminfo.r)];

						NPLParser::next(ls);
						if (ls->t.token == ']')
						{
							NPLParser::next(ls);
							if (ls->t.token == '=')
							{
								NPLParser::next(ls);
								if (!DeserializePureNPLDataBlock(ls, proxy_))
									return false;
								NPLParser::testnext(ls, ',');
							}
							else
								return false;
						}
					}
					else
						return false;
				}
				else if (token_ == NPLLex::TK_STRING)
				{
					// verify that the string is a value key(non-empty);
					if (ls->t.seminfo.ts.empty())
						return false;

					NPLObjectProxy& proxy_ = tabMsg[ls->t.seminfo.ts];
					NPLParser::next(ls);
					if (ls->t.token == ']')
					{
						NPLParser::next(ls);
						if (ls->t.token == '=')
						{
							NPLParser::next(ls);
							if (!DeserializePureNPLDataBlock(ls, proxy_))
								return false;
							NPLParser::testnext(ls, ',');
						}
						else
							return false;
					}
				}
				else
					return false;
			}
			/// Fixed: 2008.6.3 LiXizhi
			/// the following is for auto indexed table items {"string1", "string2\r\n", 213, nil,["A"]="B", true, false, {"another table", "field1"}}
			else
			{
				NPLObjectProxy& proxy_ = tabMsg[nTableAutoIndex++];
				if (!DeserializePureNPLDataBlock(ls, proxy_))
					return false;
				NPLParser::testnext(ls, ',');
			}
		}
		return true;
	}
	default:
		break;
	}
	return false;
}

// Note: Instantiate function with the explicitly specified template. 
// This allows us to put template implementation code in cpp file. 
template void NPL::NPLHelper::EncodeStringInQuotation(std::string& output, int nOutputOffset, const char* input, int nInputSize);
template void NPL::NPLHelper::EncodeStringInQuotation(ParaEngine::StringBuilder& output, int nOutputOffset, const char* input, int nInputSize);

template void NPL::NPLHelper::EncodeJsonStringInQuotation(std::string& output, int nOutputOffset, const char* input, int nInputSize);
template void NPL::NPLHelper::EncodeJsonStringInQuotation(ParaEngine::StringBuilder& output, int nOutputOffset, const char* input, int nInputSize);

template bool NPL::NPLHelper::SerializeToSCode(const char* sStorageVar, const luabind::object& input, std::string& sCode, int nCodeOffset, STableStack* pRecursionTable, bool sort);
template bool NPL::NPLHelper::SerializeToSCode(const char* sStorageVar, const luabind::object& input, ParaEngine::StringBuilder& sCode, int nCodeOffset, STableStack* pRecursionTable, bool sort);

template bool NPL::NPLHelper::SerializeToJson(const luabind::object& input, std::string& sCode, int nCodeOffset, STableStack* pRecursionTable, bool bUseEmptyArray);
template bool NPL::NPLHelper::SerializeToJson(const luabind::object& input, ParaEngine::StringBuilder& sCode, int nCodeOffset, STableStack* pRecursionTable, bool bUseEmptyArray);

template bool NPL::NPLHelper::SerializeNPLTableToString(const char* sStorageVar, NPL::NPLObjectProxy& input, std::string& sCode, int nCodeOffset);
template bool NPL::NPLHelper::SerializeNPLTableToString(const char* sStorageVar, NPL::NPLObjectProxy& input, ParaEngine::StringBuilder& sCode, int nCodeOffset);


#ifdef _DEBUG
void Test_NPLTable()
{
	// Example 2: deserialize NPLTable from string. 
	NPL::NPLObjectProxy tabMsg = NPL::NPLHelper::StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
	PE_ASSERT((double)tabMsg["nid"] == 10);
	PE_ASSERT((tabMsg["name"]) == "value");
	PE_ASSERT((tabMsg["tab"]["name1"]) == "value1");

	// Example 1: Create NPLTable and serialize to string
	NPL::NPLObjectProxy msg;
	msg["nid"] = (double)10;
	msg["name"] = "value";
	msg["tab"]["name1"] = "value1";
	PE_ASSERT((double)msg["nid"] == 10);
	PE_ASSERT((msg["name"]) == "value");
	PE_ASSERT((msg["tab"]["name1"]) == "value1");
	std::string strOutput;
	NPL::NPLHelper::SerializeNPLTableToString(NULL, msg, strOutput);
	OUTPUT_LOG("NPL TABLE test succeeded: %s\n", strOutput.c_str());

	// Test Case: MsgStringToNPLTable
	strOutput = std::string("msg=") + strOutput;

	tabMsg = NPL::NPLHelper::MsgStringToNPLTable(strOutput.c_str(), (int)(strOutput.size()));
	PE_ASSERT((double)tabMsg["nid"] == 10);
	PE_ASSERT((tabMsg["name"]) == "value");
	PE_ASSERT((tabMsg["tab"]["name1"]) == "value1");
	OUTPUT_LOG("test MsgStringToNPLTable succeed\n");
}
#endif