//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.3
// Description:	API for configuration manager. 
//-----------------------------------------------------------------------------
#pragma once
#include <map>
#include <vector>
#include "util/mutex.h"
using namespace std;
using namespace ParaEngine;
namespace ParaInfoCenter
{
	enum IC_CONFIG_TYPE{
		IC_CONFIG_INT,IC_CONFIG_FLOAT,IC_CONFIG_TEXT
	};
	/**
	* This class is not thread-safe. So use it only in a thread-safe class.
	* It's only accessible through CICConfigManager
	*/
	class PE_CORE_DECL CICConfigItem
	{
	public:
		~CICConfigItem();
		friend class CICConfigManager;
	protected:
		CICConfigItem(int value);
		CICConfigItem(double value);
		CICConfigItem(const char* value);
		int GetType(){return etype;}
		void SetValue(const char* value);
		union{
			int iData;
			double fData;
			char *sData;
		};
		int etype;
	};
	class CICConfigItems
	{
	public:
		~CICConfigItems();
		std::vector<CICConfigItem*> items;
	};
	/**
	*   The CICConfigManager manages the global configurations of the program.
	*   User can use LoadFromFile to load configurations from a file, SaveToFile to save the current configurations to file.
	*   To change the configurations at runtime, you can use SetIntValue, SetDoubleValue and SetTextValue to either add a new value
	* or change the old value. The values are all writable by default. 
	*   To get the configurations at runtime, you can use GetIntValue, GetDoubleValue and GetTextValue to get the value of a specified configuration.
	*   If you have multiple threads and the threads are all trying to access the configurations, the CICConfigManager provides a thread-safe 
	* operation. All the functions of the class can only be accessed by one thread at a time. If a thread tries to call a function when another thread 
	* is executing any of the class functions. The thread will block and wait until the other thread ends its current execution of its executing
	* function. 
	*   If you want to make sure that in a serial calling of the class functions, other threads cannot execute any of the class functions. Call Lock before
	* your calling and call UnLock after you finish the job. When one thread is in a Lock-UnLock pair, any other thread's calling to the class functions  
	* will return E_ACCESSDENIED. And any other thread's try to call Lock will block at the call until the lock owner release the lock. If you just want
	* to know if another thread is in the Lock-UnLock pair, use TryLock. This will gain a lock if no one is locking and return true. If any one have a lock,
	* it returns false immediately. 
	*/
	class CICConfigManager
	{
	public:
		CICConfigManager();
		~CICConfigManager();

		/** get the singleton instance of this object. */
		PE_CORE_DECL static CICConfigManager* GetInstance();
		/**
		* Load a configuration from a file. 
		* The file is a LUA file with correct scripts to write values to the configuration manager.
		* @param szFilename: the file name of the lua file. If it is NULL, it will load the default configuration file 
		*  "config.lua". The default value is NULL;
		* @param bOverwrite: Whether to overwrite the items if there is one in the configuration. The default value is true.
		* @return: S_OK if successful. 
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		*   If the file does not exist, the return value is E_FAIL.
		*/
		PE_CORE_DECL HRESULT LoadFromFile(const char* szFilename=NULL,bool bOverwrite=true);
		/**
		* Save the current configuration to a file. 
		* The file is a LUA file. The manager will write correct scripts.
		* @param szFilename: the file name of the lua file. If it is NULL, it will save to the default configuration file 
		*  "config.lua". The default value is NULL;
		* @param bOverwrite: Whether to overwrite the configuration file if it exists. The default value is true.
		* @return: S_OK if successful. 
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		*   If bOverwrite==false and the file exists, the return value is E_FAIL
		*   If the disk is full or cannot create file because of security or other reasons, the return value is E_FAIL
		*/
		PE_CORE_DECL HRESULT SaveToFile(const char* szFilename=NULL,bool bOverwrite=true);

		/**
		* Get size of a specified name.
		* @param szName: the value name
		* @param nSize: the pointer to the value that stores the size to get.
		* @return: S_OK if successful. 
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		*   If the szName is NULL or the specified name is not found or value==NULL, the return value is E_INVALIDARG.
		* @remark: If the specified name exists, nSize will the number of items under this name.
		*   If the specified name does not exist, nSize will zero.
		*/
		PE_CORE_DECL HRESULT GetSize(const char* szName, DWORD *nSize);
		/**
		* Get a specified integer value.
		* @param szName: the value name. 
		* @param value: a pointer to the variable to receive the value.
		* @return: return S_OK if successful.
		*   If the szName is NULL or the specified name is not found or value==NULL, the return value is E_INVALIDARG.
		*   If the type of the specified name is text, the return value is E_FAIL.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		* @remarks: If the type of the specified name is double, it will be auto-cast to integer.
		*/
		PE_CORE_DECL HRESULT GetIntValue(const char* szName,int *value, int index=0);

		/**
		* Get a specified double value.
		* @param szName: the value name. 
		* @param value: a pointer to the variable to receive the value.
		* @return: return S_OK if successful. 
		*   If the szName is NULL or the specified name is not found or value==NULL, the return value is E_INVALIDARG.
		*   If the type of the specified name is text, the return value is E_FAIL.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		* @remarks: If the type of the specified name is integer, it will be auto-cast to double.
		*/
		PE_CORE_DECL HRESULT GetDoubleValue(const char* szName,double *value, int index=0);

		/**
		* Get a specified text value.
		* @param szName: the value name. 
		* @param value: a pointer to the variable to receive the value.
		* @return: return S_OK if successful. 
		*   If the szName is NULL or the specified name is not found or value==NULL, the return value is E_INVALIDARG.
		*   If the type of the specified name is not text, the data will be casted to a string.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		*/
		PE_CORE_DECL HRESULT GetTextValue(const char* szName,string &value, int index=0);

		/**
		* Set a specified integer value.
		* @param szName: the value name. 
		* @param value: the new value.
		* @param index: where, in the chain, you want to set the value.
		* @return: return S_OK if successful. If the szName is NULL, the return value is E_INVALIDARG.
		*   If the index is not a valid place, the return value is E_INVALIDARG.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		*   If the overwrite is false and the value already exists, the return value is E_FAIL.
		* @remarks: If the type of the specified name is double, the new value will be auto-cast to double and then assigns to the item.
		*   If the specified name is text, the new value will be casted to a string and then assigns to the item.
		*   If the specified name is not found, a new item will be created. In this case, index will be ignored. 
		*/
		PE_CORE_DECL HRESULT SetIntValue(const char* szName,int value, int index=0);

		/**
		* Appends a specified integer value.
		* @param szName: the value name. 
		* @param value: the new value.
		* @return: return S_OK if successful. If the szName is NULL, the return value is E_INVALIDARG.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		* @remarks: If the specified name is not exist, this functions the same as SetIntValue(szName, value, 0);
		*   If there is already an instance of the specified name, it will create a new item and append it to the tail of the chain
		*   of this specified name.
		*/
		PE_CORE_DECL HRESULT AppendIntValue(const char* szName,int value);
		/**
		* Set a specified double value.
		* @param szName: the value name. 
		* @param value: the new value.
		* @return: return S_OK if successful. If the szName is NULL, the return value is E_INVALIDARG.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		*   If the overwrite is false and the value already exists, the return value is E_FAIL.
		* @remarks: If the type of the specified name is integer, the new value will be auto-cast to integer and then assigns to the item.
		*   If the specified name is text, the new value will be casted to a string and then assigns to the item.
		*   If the specified name is not found, a new item will be created. In this case, index will be ignored. 
		*/
		PE_CORE_DECL HRESULT SetDoubleValue(const char* szName,double value, int index=0);
		/**
		* Appends a specified double value.
		* @param szName: the value name. 
		* @param value: the new value.
		* @return: return S_OK if successful. If the szName is NULL, the return value is E_INVALIDARG.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		* @remarks: If the specified name is not exist, this functions the same as SetDoubleValue(szName, value, 0);
		*   If there is already an instance of the specified name, it will create a new item and append it to the tail of the chain
		*   of this specified name.
		*/
		PE_CORE_DECL HRESULT AppendDoubleValue(const char* szName,double value);
		/**
		* Set a specified string value.
		* @param szName: the value name. 
		* @param value: the new value.
		* @return: return S_OK if successful. If the szName is NULL, the return value is E_INVALIDARG.
		*   If the type of the specified name is integer or double, the return value is E_FAIL.
		*   If the overwrite is false and the value already exists, the return value is E_FAIL.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		* @remarks: If the specified name is not found, a new item will be created. In this case, index will be ignored. 
		*/
		PE_CORE_DECL HRESULT SetTextValue(const char* szName,const char *value, int index=0);
		/**
		* Appends a specified text value.
		* @param szName: the value name. 
		* @param value: the new value.
		* @return: return S_OK if successful. If the szName is NULL, the return value is E_INVALIDARG.
		*   If the configuration manager is locked, the return value is E_ACCESSDENIED.
		* @remarks: If the specified name is not exist, this functions the same as SetTextValue(szName, value, 0);
		*   If there is already an instance of the specified name, it will create a new item and append it to the tail of the chain
		*   of this specified name.
		*/
		PE_CORE_DECL HRESULT AppendTextValue(const char* szName,const char* value);

		/**
		* Lock the configuration manager so that no other thread can access it. 
		* If the configuration manager is already locked by other thread. It will block until the configuration manager is unlocked.
		* Then it will acquire a lock on the configuration manager.
		*/
		PE_CORE_DECL void Lock();

		/**
		* Unlock the configuration manager so that other thread can access it.
		*/
		PE_CORE_DECL void UnLock();
#ifdef _DEBUG
		static void test();
#endif
	private:
		map<string, CICConfigItems*> m_items;
		bool m_bLocked;
		bool m_bOverwrite;
		ParaEngine::mutex m_csLock;
		ParaEngine::mutex m_csFunc;
	};
}
