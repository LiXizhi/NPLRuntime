#pragma once
#include "Variable.h"

namespace ParaEngine
{
	using namespace std;

	enum SIMPLE_SCHEMA
	{
		SCHEMA_RGB = 0,
		SCHEMA_FILE,
		SCHEMA_SCRIPT,
		SCHEMA_INTEGER,
		SCHEMA_FLOAT,
		SCHEMA_DIALOG,
	};

	/** for a single attribute field */
	class CAttributeField
	{
	public:
		CAttributeField();
		~CAttributeField();
	public:
		union any_offset{
			void* ptr_fun;
			int offset_data;
		};
		any_offset m_offsetSetFunc;
		any_offset m_offsetGetFunc;

		
		/** see ATTRIBUTE_FIELDTYPE */
		DWORD		m_type;

		/** additional schematics for describing the display format of the data. Different attribute type have different schematics.
		@see GetSimpleSchema() */
		string		m_sSchematics;
		/** a help string.*/
		string		m_sHelpString;

	protected:
		/** field name: e.g. "base.position" */
		string		m_sFieldname;
		/* hash of filed name, used to fast query*/
		size_t		m_hash;
	public:
		void SetFieldname(const string& sFieldname);

		const string& GetFieldname() const;

		size_t GetHash() const;

		static std::hash<string> HashFunc;
		/**
		* get the field type as string
		* @return one of the following type may be returned
		* "void" "bool" "string" "int" "float" "float_float" "float_float_float" "double" "vector2" "vector3" "vector4" "enum" "deprecated" ""
		*/
		const char* GetTypeAsString();

		/** whether has get function. */
		inline bool HasGetFunction() { return (m_offsetGetFunc.ptr_fun != 0); }
		/** whether has set function. */
		inline bool HasSetFunction() { return (m_offsetSetFunc.ptr_fun != 0); }

		inline HRESULT Call(void* obj)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj))m_offsetSetFunc.ptr_fun)(obj);
			else if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj))m_offsetSetFunc.ptr_fun)(obj);
			else
				return E_FAIL;
		};
		inline HRESULT Get(void* obj)
		{
			if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj))m_offsetSetFunc.ptr_fun)(obj);
			else
				return E_FAIL;
		};
		/* attention should be paid: alway explicitly pass the parameter type to the function. e.g. Set(obj, (bool)bValue)*/
		template <class datatype>
		inline HRESULT Set(void* obj, datatype p1)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype p1))m_offsetSetFunc.ptr_fun)(obj, p1);
			else
				return E_FAIL;
		};
		// this ensure that Matrix4 is always passed by reference. when defining attribute, one should always use const Matrix4&
		inline HRESULT Set(void* obj, const Matrix4& p1)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, const Matrix4& p1))m_offsetSetFunc.ptr_fun)(obj, p1);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Set(void* obj, datatype p1, datatype p2)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype p1, datatype p2))m_offsetSetFunc.ptr_fun)(obj, p1, p2);
			else
				return E_FAIL;
		};
		template <class datatype1, class datatype2>
		inline HRESULT Set(void* obj, datatype1 p1, datatype2 p2)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype1 p1, datatype2 p2))m_offsetSetFunc.ptr_fun)(obj, p1, p2);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Set(void* obj, datatype p1, datatype p2, datatype p3)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype p1, datatype p2, datatype p3))m_offsetSetFunc.ptr_fun)(obj, p1, p2, p3);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Set(void* obj, datatype p1, datatype p2, datatype p3, datatype p4)
		{
			if (m_offsetSetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype p1, datatype p2, datatype p3, datatype p4))m_offsetSetFunc.ptr_fun)(obj, p1, p2, p3, p4);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Get(void* obj, datatype* p1)
		{
			if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype* p1))m_offsetGetFunc.ptr_fun)(obj, p1);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Get(void* obj, datatype* p1, datatype* p2)
		{
			if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype* p1, datatype* p2))m_offsetGetFunc.ptr_fun)(obj, p1, p2);
			else
				return E_FAIL;
		};
		template <class datatype1, class datatype2>
		inline HRESULT Get(void* obj, datatype1* p1, datatype2* p2)
		{
			if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype1* p1, datatype2* p2))m_offsetGetFunc.ptr_fun)(obj, p1, p2);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Get(void* obj, datatype* p1, datatype* p2, datatype* p3)
		{
			if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype* p1, datatype* p2, datatype* p3))m_offsetGetFunc.ptr_fun)(obj, p1, p2, p3);
			else
				return E_FAIL;
		};
		template <class datatype>
		inline HRESULT Get(void* obj, datatype* p1, datatype* p2, datatype* p3, datatype* p4)
		{
			if (m_offsetGetFunc.ptr_fun != 0)
				return ((HRESULT(*)(void* obj, datatype* p1, datatype* p2, datatype* p3, datatype* p4))m_offsetGetFunc.ptr_fun)(obj, p1, p2, p3, p4);
			else
				return E_FAIL;
		};
	public:
		/**
		* attribute schematics:  All simple schematics start with ':', they are mostly used in script.
		color3	":rgb"
		file	":file"
		script	":script"
		integer	":int{min, max}"
		float	":float{min, max}"
		* @param schema
		* @return
		*/
		static const char* GetSimpleSchema(SIMPLE_SCHEMA schema);
		static const char* GetSimpleSchemaOfRGB(){ return GetSimpleSchema(SCHEMA_RGB); };
		static const char* GetSimpleSchemaOfFile(){ return GetSimpleSchema(SCHEMA_FILE); };
		static const char* GetSimpleSchemaOfScript(){ return GetSimpleSchema(SCHEMA_SCRIPT); };
		static const char* GetSimpleSchemaOfInt(int nMin, int nMax);
		static const char* GetSimpleSchemaOfFloat(float nMin, float nMax);

		/**
		* parse the schema type from the schema string.
		* @return : simple schema type. it may be any of the following value.
		unspecified: ""
		color3	":rgb"
		file	":file"
		script	":script"
		integer	":int"
		float	":float"
		*/
		const char* GetSchematicsType();

		/**
		* parse the schema min max value from the schema string.
		* @param fMin : [out]if the schema does not contain a min value, the default value which is smallest float.
		* @param fMax : [out]if the schema does not contain a max value, the default value which is largest float.
		* @return true if found min max.
		*/
		bool GetSchematicsMinMax(float& fMin, float& fMax);
	};
}