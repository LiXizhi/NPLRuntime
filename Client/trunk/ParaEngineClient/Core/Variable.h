#pragma once

namespace ParaEngine
{
	class IAnimated;

	/** a list of all attribute type
	* this is also used for all variable type supported. 
	*/
	enum ATTRIBUTE_FIELDTYPE
	{
		// unknown
		FieldType_unknown,
		// get(), set()
		FieldType_void,
		// get(int*) set(int)
		FieldType_Int,
		// get(bool*) set(bool)
		FieldType_Bool,
		// get(float*) set(float)
		FieldType_Float,
		// get(float*,float* ) set(float, float)
		FieldType_Float_Float,
		// get(float*,float*,float*) set(float, float, float)
		FieldType_Float_Float_Float,
		// get(int*) set(int)
		FieldType_Enum,
		// get(double*) set(double)
		FieldType_Double,
		// get(Vector2*) set(Vector2)
		FieldType_Vector2,
		// get(Vector3*) set(Vector3)
		FieldType_Vector3,
		// get(Vector4*) set(Vector4)
		FieldType_Vector4,
		// get(Quaternion*) set(Quaternion)
		FieldType_Quaternion,
		// get(const char**) set(const char*)
		FieldType_String,
		// get(DWORD*) set(DWORD)
		FieldType_DWORD,
		// get(DVector3*) set(DVector3), double precision vector3
		FieldType_DVector3,
		// get(float*,float*,float*,float*) set(float, float, float, float)
		FieldType_Float_Float_Float_Float,
		// get(Matrix4*) set(const Matrix4&)
		FieldType_Matrix4,
		// get(double*,double* ) set(double, double)
		FieldType_Double_Double,
		// get(double*,double*,double*) set(double, double, double)
		FieldType_Double_Double_Double,
		//////////////////////////////////////////////
		// following types are all animated
		//////////////////////////////////////////////
		FieldType_Animated,
		FieldType_AnimatedInt,
		FieldType_AnimatedFloat,
		FieldType_AnimatedDouble,
		FieldType_AnimatedVector2,
		FieldType_AnimatedVector3,
		FieldType_AnimatedDVector3,
		FieldType_AnimatedVector4,
		FieldType_AnimatedQuaternion,

		// this is for custom void pointer
		FieldType_void_pointer,
		// get(const char**,float*) set(const char*, float)
		FieldType_String_Float,
		// get(const char**,const char**) set(const char*, const char*)
		FieldType_String_String,
		FieldType_Deprecated = 0xffffffff
	};



	/** represent any kind of static or animated variable, such as:
	* string, double, float, int, bool, Vector3, Quaternion, or animated data with multiple time, value keys. 
	*/
	class CVariable
	{
	public:
		CVariable(DWORD dwType = FieldType_unknown);
		~CVariable();
		
		/** clone from given variable */
		void Clone(const CVariable& value);

		/** Get Type as string
		* @param dwType: type of ATTRIBUTE_FIELDTYPE
		*/
		static const char* GetTypeAsString(DWORD dwType);
		const char* GetTypeName() const;
		inline ATTRIBUTE_FIELDTYPE GetType() const { return (ATTRIBUTE_FIELDTYPE)m_type; }

		bool IsNil() const;
		bool IsStringType() const;
		/** whether the key is animated or not */
		bool IsAnimated();

		/** change type to given type*/
		void ChangeType(ATTRIBUTE_FIELDTYPE newType);

		/**
		* In case of nil, it is "nil", in case of string, it is in quotation mark.
		* @param output: it will append result to output
		*/
		void ToNPLString(std::string& output);

		void operator = (const CVariable& val);

		operator double();
		void operator = (double value);

		operator float();
		void operator = (float value);

		operator int();
		operator DWORD();
		void operator = (int value);
		
		operator Vector3();
		void operator = (const Vector3& value);

		operator Quaternion();
		void operator = (const Quaternion& value);

		operator bool();
		void operator = (bool value);

		operator const string& ();
		operator const char* ();
		bool operator == (const string& value);
		bool operator == (const char* value);
		void operator = (const std::string& value);
		void operator = (const char* value);


		/** get total number of animated keys. For non-animated attribute, this will return 1. */
		int GetNumKeys();
		/** get set the total number of animated keys. */
		void SetNumKeys(int nKeyCount);

		/** add a given key with default value at specified time
		* if there is already a key at the index, we will return the old index. 
		* @param isKeyExist: if not NULL, it will return whether the given key already exist at the index.
		* return the key index. */
		int AddKey(int nTime, bool* isKeyExist = NULL);

		/** only applied to Animated attribute
		* @param nIndex: key index.
		* @param nTime: in milliseconds
		*/
		void SetTime(int nIndex, int nTime);
		int GetTime(int nIndex);
		
		void SetValue(int nIndex, const Quaternion& val);
		bool GetValue(int nIndex, Quaternion& val);
		/** get interpolated value at the given time. */
		bool GetValueByTime(int nTime, Quaternion& val);

		void SetValue(int nIndex, const Vector3& val);
		bool GetValue(int nIndex, Vector3& val);
		bool GetValueByTime(int nTime, Vector3& val);

		void SetValue(int nIndex, double val);
		bool GetValue(int nIndex, double& val);
		bool GetValueByTime(int nTime, double& val);

		void SetValue(int nIndex, float val);
		bool GetValue(int nIndex, float& val);
		bool GetValueByTime(int nTime, float& val);

		void SetValue(int nIndex, const std::string& val);
		bool GetValue(int nIndex, std::string& val);
	protected:

		/** return the index of the first key whose time is larger than or equal to time.
		* return -1 if no suitable index is found.
		*/
		int GetNextKeyIndex(int nTime);

	protected:
		DWORD m_type;
		union{
			double m_doubleVal;
			float m_floatVal;
			int m_intVal;
			bool m_boolVal;
			float m_vector3[3];
			float m_vector4[4];
		};
		IAnimated* m_pAnimated;
		std::string m_strVal;
	};


}