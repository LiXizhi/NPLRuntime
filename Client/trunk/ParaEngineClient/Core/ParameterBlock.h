#pragma once

#include <string>
#include <vector>
#include <list>

namespace ParaEngine
{
	using namespace std;
	struct TextureEntity;
	class CEffectFile;

	/**
	* a name, value pair. mainly used in Para effect parameter block.
	* value can be integer, float, vector3, vector4, matrix, TextureEntity, etc. 
	*/
	class CParameter
	{
	public:
		typedef TextureEntity* LPTextureEntity;

		/** current type of the parameter */
		enum PARAMETER_TYPE{
			PARAM_UNKNOWN,
			PARAM_INT,
			PARAM_FLOAT, 
			PARAM_BOOLEAN,
			PARAM_VECTOR2,
			PARAM_VECTOR3,
			PARAM_VECTOR4,
			PARAM_MATRIX,
			PARAM_RAWBYTES,
			PARAM_TEXTURE_ENTITY,
			PARAM_LAST,
		};

		CParameter() : m_type(PARAM_UNKNOWN){};
		CParameter(const string& sName):m_sName(sName), m_type(PARAM_UNKNOWN){};
		~CParameter(){};

		/** string name of the parameter */
		string m_sName;
		vector<char> m_data;
		PARAMETER_TYPE m_type;

		/** get the current type as a string, "unknown", "int", "boolean", "float", "vector2", "vector3","vector4","matrix43", "texture", "rawbytes" */
		const string& GetTypeAsString();
		/** set parameter type by string. if string is not recognized, the type will be unknown. 
		* @param sType: one of the "unknown", "int", "boolean", "float", "vector2", "vector3","vector4","matrix43", "texture", "rawbytes"
		* @return: the integer type is returned. 
		*/
		PARAMETER_TYPE SetTypeByString(const char* sType);
		/** set the value by string and type 
		* @param value: string presentation of the value. 
		* @param nType: if -1, the current type is used, otherwise the given type is used. 
		* @return: true if succeed
		*/
		bool SetValueByString(const char* value, int nType = -1);

		/** serialize the current value to a string according to the curren type. */
		string GetValueByString();

		/**  get parameter name. */
		const string& GetName() const {return m_sName;};
		/**  set parameter name. */
		void SetName(const string& sName) {m_sName = sName;};

		/** raw data bytes */
		char* GetRawData() { return &(m_data[0]); };

		/** raw data bytes */
		int GetRawDataLength() { return (int)(m_data.size());};

		/** set raw data bytes */
		void SetRawData(const char* data, int nSize){m_data.resize(nSize); memcpy(&(m_data[0]), data, nSize);};

		operator int() {return *((int*)GetRawData());};
		operator bool();
		operator DWORD() {return *((DWORD*)GetRawData());};
		operator float() {return *((float*)GetRawData());};
		operator Vector2() {return *((Vector2*)GetRawData());};
		operator Vector3() {return *((Vector3*)GetRawData());};
		operator Vector4() {return *((Vector4*)GetRawData());};
		operator Matrix4() {return *((Matrix4*)GetRawData());};
		operator LPTextureEntity() {return *((LPTextureEntity*)GetRawData());};

		//CParameter& operator =(const CParameter& r){m_data = r.m_data;};
		CParameter& operator =(const int& r) { m_type=PARAM_INT; m_data.resize(sizeof(int)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};
		CParameter& operator =(const bool& r) { m_type=PARAM_BOOLEAN;m_data.resize(sizeof(BOOL)); BOOL p = r; memcpy(GetRawData(), &p, (int)m_data.size()); return *this;};
		CParameter& operator =(const float& r) { m_type=PARAM_FLOAT;m_data.resize(sizeof(float)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};

		CParameter& operator =(const Vector2& r) { m_type=PARAM_VECTOR2;m_data.resize(sizeof(Vector2)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};
		CParameter& operator =(const Vector3& r) { m_type=PARAM_VECTOR3;m_data.resize(sizeof(Vector3)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};
		CParameter& operator =(const Vector4& r) { m_type=PARAM_VECTOR4;m_data.resize(sizeof(Vector4)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};
		CParameter& operator =(const Matrix4& r) { m_type=PARAM_MATRIX;m_data.resize(sizeof(Matrix4)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};
		CParameter& operator =(const LPTextureEntity& r) { m_type=PARAM_TEXTURE_ENTITY;m_data.resize(sizeof(LPTextureEntity)); memcpy(GetRawData(), &r, (int)m_data.size()); return *this;};

	};

	/**
	* a list of CParameter{name, value} pairs of anything.  
	*  usually used for DirectX effect parameter block. 
	*  value can be integer, float, vector3, vector4, matrix, TextureEntity, etc. 
	* Current the effect parameters can be set via ParaParamBlock interface from the scripting interface. 
	* we offer three levels of effect parameters: per effect file, per asset file, per mesh object. Effect parameters are also applied in that order. 
	* e.g. per effect file affects all object rendering with the effect file; per asset file affects all objects that use the mesh asset file; per mesh object affects only the mesh object. 
	*/
	class CParameterBlock
	{
	public:
		typedef map <string, CParameter>::iterator ParamIterator;
		map <string, CParameter> m_params;

		ParamIterator BeginIter() { return m_params.begin();};
		ParamIterator EndIter() { return m_params.end();};
		bool IsEmpty() { return m_params.empty();};

		/** if there is already a parameter with the same name, it will be overridden. */
		template <class t>
		void SetParameter(string sName, const t& value)
		{
			map <string, CParameter>::iterator it = m_params.find(sName);
			if(it!=m_params.end())
			{
				it->second = value;
			}
			else
			{
				CParameter p(sName);
				p = value;
				m_params[sName] = p;
			}
		}

		/** add a new parameter to block. if there is already a parameter with the same name, it will be overridden.*/
		bool AddParameter(const CParameter& p);

		/** get parameter by its name, NULL will be returned if parameter is not found. */
		CParameter* GetParameter(string sName)
		{
			map <string, CParameter>::iterator it = m_params.find(sName);
			if(it!=m_params.end())
			{
				return &(it->second);
			}
			else
			{
				return NULL;
			}
		}

		/**
		* @param sParamName: the name of the object to set
		* @param sValue_: internal name or value string
		*/
		void SetParamByStringValue(const char* sParamName, const char* sValue_);

		// clear all parameters
		void Clear()
		{
			m_params.clear();
		}

		/** apply to effect file. */
		bool ApplyToEffect( CEffectFile* pEffectFile );
	};
}