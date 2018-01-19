#pragma once
#include "util/intrusive_ptr.h"
#include <string>
#include <map>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#pragma warning( disable:4251 )
#endif

namespace NPL
{
	class NPLObjectBase;
	class NPLNumberObject;
	class NPLBoolObject;
	class NPLTable;
	class NPLStringObject;
	class NPLObjectProxy;
	typedef ParaIntrusivePtr<NPLObjectBase> NPLObjectBase_ptr;
	typedef ParaIntrusivePtr<NPLTable> NPLTable_ptr;
	typedef ParaIntrusivePtr<NPLNumberObject> NPLNumberObject_ptr;
	typedef ParaIntrusivePtr<NPLBoolObject> NPLBoolObject_ptr;
	typedef ParaIntrusivePtr<NPLStringObject> NPLStringObject_ptr;

	/** base class for all NPL date members. Do not use this class directly. Use NPLObjectProxy*/
	class PE_CORE_DECL NPLObjectBase : public ParaEngine::intrusive_ptr_single_thread_base
	{
	public:
		typedef std::map<std::string, NPLObjectProxy> TableFieldMap_Type;
		typedef std::map<int, NPLObjectProxy> TableIntFieldMap_Type;
		typedef TableFieldMap_Type::iterator	Iterator_Type;
		typedef TableIntFieldMap_Type::iterator	IndexIterator_Type;

		enum NPLObjectType 
		{
			NPLObjectType_Nil, 
			NPLObjectType_Table, 
			NPLObjectType_Number, 
			NPLObjectType_String, 
			NPLObjectType_Bool,
		};
		NPLObjectBase():m_type(NPLObjectType_Nil){};
		virtual ~NPLObjectBase(){};

		inline NPLObjectType GetType(){return m_type;}

		bool isTable() { return GetType() == NPLObjectType_Table; }
		bool isNumber() { return GetType() == NPLObjectType_Number; }
		bool isString() { return GetType() == NPLObjectType_String; }
		bool isNil() { return GetType() == NPLObjectType_Nil; }
		bool isBool() { return GetType() == NPLObjectType_Bool; }

	protected:
		NPLObjectType m_type;
	};

	/** NPL object proxy. This is both a smart pointer and accessors
	// Example 1: Create NPLTable and serialize to string
	NPL::NPLObjectProxy msg;
	msg["nid"] = (double)10;
	msg["name"] = "value";
	msg["tab"]["name1"] = "value1";
	NPL::NPLHelper::SerializeNPLTableToString(NULL, msg, strOutput);

	// Example 2: deserialize NPLTable from string. 

	NPL::NPLObjectProxy tabMsg = NPL::NPLHelper::StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
	PE_ASSERT((double)tabMsg["nid"] == 10);
	PE_ASSERT((tabMsg["name"]) == "value");
	PE_ASSERT((tabMsg["tab"]["name1"]) == "value1");
	*/
	class PE_CORE_DECL NPLObjectProxy : public NPLObjectBase_ptr
	{
	public:
		NPLObjectProxy() {};
		NPLObjectProxy( NPLObjectBase* pObject);

		operator double ();

		void operator = (double value);

		operator bool ();

		void operator = (bool value);

		operator const string& ();
		const char* c_str();
		int toInt();

		bool operator == (const string& value);
		bool operator == (const char* value);

		void operator = (const std::string& value);
		void operator = (const char* value);

		/** get field by name. It will create an empty field if it does not exist. Use GetField() if you only want to find if a field exist of not. */
		NPLObjectProxy& operator [] (const string& sName);

		NPLObjectProxy& operator [] (const char* sName);

		NPLObjectProxy& operator [](int nIndex);

		/** get a field. It may return null if not found.*/
		NPLObjectProxy GetField(const string& sName);

		/** get a field. It may return null if not found.*/
		NPLObjectProxy GetField(const char* sName);

		
		/** Set the field.
		* @param sName: the field name
		* @param pObject: if NULL, it will remove the field, as if nil in lua.
		*/
		void SetField(const string& sName, const NPLObjectProxy& pObject);

		void SetField(int nIndex, const NPLObjectProxy& pObject);

		/** make the object invalid */
		void MakeNil();

		NPLObjectBase::Iterator_Type begin();
		NPLObjectBase::Iterator_Type end();

		NPLObjectBase::IndexIterator_Type index_begin();
		NPLObjectBase::IndexIterator_Type index_end();

		/** get the type */
		NPLObjectBase::NPLObjectType  GetType();
	};


	/** a floating point number */
	class PE_CORE_DECL NPLNumberObject : public NPLObjectBase
	{
	public:
		NPLNumberObject():m_value(0){m_type = NPLObjectType_Number;};
		/** create the table from a serialized string. */
		NPLNumberObject(double value):m_value(value){m_type = NPLObjectType_Number;};

		virtual ~NPLNumberObject(){};

		void SetValue(double value){m_value = value;}
		double GetValue() {return m_value;}

		NPLNumberObject& operator = (double value) { SetValue(value); return *this;}

	private:
		double m_value;
	};


	/** a boolean */
	class PE_CORE_DECL NPLBoolObject : public NPLObjectBase
	{
	public:
		NPLBoolObject():m_value(false){m_type = NPLObjectType_Bool;};
		/** create the table from a serialized string. */
		NPLBoolObject(bool value):m_value(value){m_type = NPLObjectType_Bool;};

		virtual ~NPLBoolObject(){};

		inline void SetValue(bool value){m_value = value;}
		bool GetValue() {return m_value;}

		NPLBoolObject& operator = (bool value) { SetValue(value); return *this;}

	private:
		bool m_value;
	};


	/** a floating point number */
	class PE_CORE_DECL NPLStringObject : public NPLObjectBase
	{
	public:
		NPLStringObject(){m_type = NPLObjectType_String;};
		/** create the table from a serialized string. */
		NPLStringObject(const std::string& value):m_value(value){m_type = NPLObjectType_String;};

		virtual ~NPLStringObject(){};

		void SetValue(const std::string& value){m_value = value;}
		std::string& GetValue(){return m_value;}

		NPLStringObject& operator = (const std::string& value) { SetValue(value); return *this;}

	private:
		std::string m_value;
	};


	/** this is a pure c++ implementation of lua table. it can convert back and force from string. 
	* only data members are supported. This class is mostly used by C++ plugin modules. 
	* @remark: Use NPLObjectProxy instead of this class. 
	* Example: 
	// Example 1: Create NPLTable and serialize to string
	NPL::NPLObjectProxy msg;
	msg["nid"] = (double)10;
	msg["name"] = "value";
	msg["tab"]["name1"] = "value1";
	NPL::NPLHelper::SerializeNPLTableToString(NULL, msg, strOutput);

	// Example 2: deserialize NPLTable from string. 

	NPL::NPLObjectProxy tabMsg = NPL::NPLHelper::StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
	PE_ASSERT((double)tabMsg["nid"] == 10);
	PE_ASSERT((tabMsg["name"]) == "value");
	PE_ASSERT((tabMsg["tab"]["name1"]) == "value1");

	* @note: no cyclic link is allowed. 
	*/
	class PE_CORE_DECL NPLTable : public NPLObjectBase
	{
	public:
		/** this is an empty table*/
		NPLTable(){m_type = NPLObjectType_Table;};

		virtual ~NPLTable();
	public:
		/** output table to serialized string. */
		void ToString(std::string& str);

		/** clear the table object */
		void Clear();

		/** Set the field.
		* @param sName: the field name
		* @param pObject: if NULL, it will remove the field, as if nil in lua. 
		*/
		void SetField(const string& sName, const NPLObjectProxy& pObject);

		void SetField(int nIndex, const NPLObjectProxy& pObject);       

		/** get a field. It may return null if not found.*/
		NPLObjectProxy GetField(const string& sName);
		NPLObjectProxy GetField(int nIndex);

		/** create or get a field. It may return null proxy */
		NPLObjectProxy& CreateGetField(const string& sName);
		NPLObjectProxy& CreateGetField(int nIndex);

		Iterator_Type begin() {return m_fields.begin();}; 
		Iterator_Type end() {return m_fields.end();}; 

		IndexIterator_Type index_begin() { return m_index_fields.begin(); };
		IndexIterator_Type index_end() { return m_index_fields.end(); };

		/** this will create get field. */
		NPLObjectProxy& operator [](const string& sName) {return CreateGetField(sName);};
		NPLObjectProxy& operator [](const char* sName) {return CreateGetField(sName);};
		NPLObjectProxy& operator [](int nIndex) {return CreateGetField(nIndex);};
	private:
		TableFieldMap_Type	m_fields;
		TableIntFieldMap_Type m_index_fields;
	};
}

// required by DLL interface
//EXPIMP_TEMPLATE template class PE_CORE_DECL ParaIntrusivePtr<NPL::NPLObjectBase>;
//EXPIMP_TEMPLATE template class PE_CORE_DECL ParaIntrusivePtr<NPL::NPLTable>;
//EXPIMP_TEMPLATE template class PE_CORE_DECL ParaIntrusivePtr<NPL::NPLNumberObject>;
//EXPIMP_TEMPLATE template class PE_CORE_DECL ParaIntrusivePtr<NPL::NPLBoolObject>;
//EXPIMP_TEMPLATE template class PE_CORE_DECL ParaIntrusivePtr<NPL::NPLStringObject>;

#ifdef _MSC_VER
#pragma warning( pop ) 
#endif