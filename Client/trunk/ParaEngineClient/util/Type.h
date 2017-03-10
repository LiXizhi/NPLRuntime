#pragma once
#include <string>
#include <map>
#include <float.h>
using namespace std;

namespace ParaEngine
{
	enum TYPE_ENUMERATION
	{
		Type_void, 
		Type_Int,
		Type_Bool,
		Type_Float,
		Type_Enum,
		Type_Double,
		Type_Vector2,
		Type_Vector3,
		Type_Vector4,
		Type_String,
		Type_GUIRoot,
		Type_GUIText,
		Type_GUIVideo,
		Type_GUIButton,
		Type_GUIEditBox,
		Type_GUIIMEEditBox,
		Type_GUIScrollBar,
		Type_GUIListBox,
		Type_GUIToolTip,
		Type_GUIPainter,
		Type_GUISlider,
		Type_GUIGrid,
		Type_GUICanvas,
		Type_GUIContainer,
		Type_GUIWebBrowser,
	};

	enum ITYPE_VERIFY_RESULT
	{
		Verify_Success=0,
		Verify_Overflow=1,
		Verify_Underflow=2,
		Verify_OutOfRange=3,
		Verify_Unknown=4,
		Verify_Failed=0xffffffff
	};
	class IType;
	//This is a singleton
	class CTypeManager
	{
	public:
		//initialize the types, adding void, int, bool, float, double, string etc. to the manager
		CTypeManager();
		//delete all instance
		~CTypeManager();
		//return a type pointer to the required type, null if not exists. Do NOT delete the return pointer.
		IType* GetType(const char* szTypeName)const;
		/**
		* Set a type in the manager. If the type already exists, return false. If setting operation successes, return true
		* The input pointer shall not be deleted 
		*/
		bool SetType(IType* pType);
		static void ReleaseType(IType* pType);
	protected:
		map<string,IType*> m_types;
	};

	/**
	* IType is for type information and validating
	* It contains type information of an object. It also provide methods to validate
	* if an input string can be change to the the type
	* May put the reflection support to type, but I will not do that now because it is not of high priority
	* You can create a type. But you can never delete it by using delete. You must use CTypeManager::ReleaseType() to delete it. 
	*/
	class IType
	{
	public:
		virtual const char* GetTypeName()const=0;
		virtual int Verify(const char* input)const=0;
		static IType* GetType(const char* szTypeName);
		IType* GetType()const;
		int GetTypeValue()const{return m_etype;}
		friend class CTypeManager;
		virtual bool operator ==(int value)const
		{
			return m_etype==value;
		}
		virtual bool operator ==(const char *szTypeName)const;
		virtual bool operator ==(const IType& value)const
		{
			return m_etype==value.m_etype;
		}
	protected:
		int m_etype;//TYPE_ENUMERATION
		virtual ~IType(){}
	};
	class CDataInt32;
	class CDataFloat;
	class CDataBool;

	class CVoid:public IType
	{
	public:
		static const char TypeName[];
		CVoid(){m_etype=Type_void;}
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
		virtual int Verify(const char* input)const
		{
			return input==NULL?Verify_Unknown:Verify_Success;
		}
	protected:
		virtual ~CVoid(){}
	};

	class CBool:public IType
	{
	public:
		static const char TypeName[];
		CBool(){m_etype=Type_Bool;}
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
		static bool Parse(const char* input);
		virtual int Verify(const char* input)const;
	protected:
		virtual ~CBool(){}
	};

	class CInt32:public IType
	{
	public:
		static const char TypeName[];
		CInt32(){m_etype=Type_Int;}
		static const int MaxValue;
		static const int MinValue;
		static int Parse(const char* input);
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
		virtual int Verify(const char* input)const;
	protected:
		virtual ~CInt32(){}
	};

	class CFloat:public IType
	{
	public:
		static const char TypeName[];
		/*static const float MaxValue;
		static const float MinValue;*/
		CFloat(){m_etype=Type_Float;}
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
		static float Parse(const char* input);
		virtual int Verify(const char* input)const;
	protected:
		virtual ~CFloat(){}
	};

	class CDouble:public IType
	{
	public:
		static const char TypeName[];
		static const double MaxValue;
		static const double MinValue;
		CDouble(){m_etype=Type_Double;}
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
		static double Parse(const char* input);
		virtual int Verify(const char* input)const;
	protected:
		virtual ~CDouble(){}
	};

	class CStr:public IType
	{
	public:
		static const char TypeName[];
		CStr(){m_etype=Type_String;}
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
		virtual int Verify(const char* input)const;
	protected:
		virtual ~CStr(){}
	};

	class CDataInt32 : public IObject
	{
	public:
		CDataInt32():m_data(0)
		{
			if (!m_type){
				m_type=IType::GetType("int");
			}
		};
		CDataInt32(int data):m_data(data)
		{
			if (!m_type){
				m_type=IType::GetType("int");
			}
		}
		virtual ~CDataInt32(){};
		IObject* Clone()const{return new CDataInt32(m_data);}
		void Clone(IObject* obj)const{((CDataInt32*)obj)->m_data=m_data;}
		operator int(){return m_data;}
		CDataInt32& operator =(int data){m_data=data;return *this;}
		virtual bool operator ==(int value){return m_data==value;}
		virtual string ToString()const;
		virtual const IType* GetType()const{return m_type;}
	protected:
		static const IType* m_type;
		int m_data;
	};

	class CDataBool:public IObject
	{
	public:
		CDataBool():m_data(0)
		{
			if (!m_type){
				m_type=IType::GetType("bool");
			}
		};
		CDataBool(bool data):m_data(data)
		{
			if (!m_type){
				m_type=IType::GetType("bool");
			}
		};
		virtual ~CDataBool(){};
		IObject* Clone()const{return new CDataBool(m_data);}
		void Clone(IObject* obj)const{((CDataBool*)obj)->m_data=m_data;}
		operator bool(){return m_data;}
		CDataBool& operator =(bool data){m_data=data;return *this;}
		virtual bool operator ==(bool value)const{return m_data==value;}
		virtual string ToString()const;
		virtual const IType* GetType()const{return m_type;}
	protected:
		static const IType* m_type;
		bool m_data;
	};

	class CDataFloat:public IObject
	{
	public:
		CDataFloat():m_data(0)
		{
			if (!m_type){
				m_type=IType::GetType("float");
			}
		};
		CDataFloat(float data):m_data(data)
		{
			if (!m_type){
				m_type=IType::GetType("float");
			}
		};
		virtual ~CDataFloat(){};
		IObject* Clone()const{return new CDataFloat(m_data);}
		void Clone(IObject* obj)const{((CDataFloat*)obj)->m_data=m_data;}
		operator float(){return m_data;}
		CDataFloat& operator =(float data){m_data=data;return *this;}
		virtual bool operator ==(float value)const{return m_data==value;}
		virtual string ToString()const;
		virtual const IType* GetType()const{return m_type;}
	protected:
		static const IType* m_type;
		float m_data;
	};

	class CDataDouble:public IObject
	{
	public:
		CDataDouble():m_data(0)
		{
			if (!m_type){
				m_type=IType::GetType("double");
			}
		};
		CDataDouble(double data):m_data(data)
		{
			if (!m_type){
				m_type=IType::GetType("double");
			}
		};
		virtual ~CDataDouble(){};
		IObject* Clone()const{return new CDataDouble(m_data);}
		void Clone(IObject* obj)const{((CDataDouble*)obj)->m_data=m_data;}
		operator double(){return m_data;}
		CDataDouble& operator =(double data){m_data=data;return *this;}
		virtual bool operator ==(double value)const{return m_data==value;}
		virtual string ToString()const;
		virtual const IType* GetType()const{return m_type;}
	protected:
		static const IType* m_type;
		double m_data;
	};

	class CDataString:public IObject
	{
	public:
		CDataString():m_data(0)
		{
			if (!m_type){
				m_type=IType::GetType("string");
			}
		};
		CDataString(const string& data):m_data(data)
		{
			if (!m_type){
				m_type=IType::GetType("string");
			}
		};
		virtual ~CDataString(){};
		IObject* Clone()const{return new CDataString(m_data);}
		void Clone(IObject* obj)const{((CDataString*)obj)->m_data=m_data;}
		operator const string()const{return m_data;}
		operator const char*()const {return m_data.c_str();}
		operator const wstring()const;
		CDataString& operator =(const string& data){m_data=data;return *this;}
		CDataString& operator =(const char* data);
		bool operator ==(const string& value)const{return m_data==value;}
		bool operator ==(const char* value)const{return m_data==value;}
		CDataString& operator =(const wstring& data);
		CDataString& operator =(const wchar_t* data);
		bool operator ==(const wstring& value)const;
		virtual string ToString()const;
		virtual const IType* GetType()const{return m_type;}
	protected:
		static const IType* m_type;
		string m_data;
		
	};
	//////////////////////////////////////////////////////////////////////////
	// GUI types
	//////////////////////////////////////////////////////////////////////////
	class CGUIType:public IType
	{
	public :
		virtual bool IsContainer()const=0;
		virtual int Verify(const char* input)const{return Verify_Success;}
	};

	class CGUIRootType:public CGUIType
	{
	public:
		CGUIRootType(){m_etype=Type_GUIRoot;}
		virtual bool IsContainer()const{return true;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIRootType(){}
	};

	class CGUIButtonType:public CGUIType
	{
	public:
		CGUIButtonType(){m_etype=Type_GUIButton;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIButtonType(){}
	};

	class CGUIWebBrowserType:public CGUIType
	{
	public:
		CGUIWebBrowserType(){m_etype=Type_GUIWebBrowser;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIWebBrowserType(){}
	};

	class CGUITextType:public CGUIType
	{
	public:
		CGUITextType(){m_etype=Type_GUIText;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUITextType(){}
	};

	class CGUIVideoType:public CGUIType
	{
	public:
		CGUIVideoType(){m_etype=Type_GUIVideo;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIVideoType(){}
	};

	class CGUIEditBoxType:public CGUIType
	{
	public:
		CGUIEditBoxType(){m_etype=Type_GUIEditBox;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIEditBoxType(){}
	};

	class CGUIIMEEditBoxType:public CGUIType
	{
	public:
		CGUIIMEEditBoxType(){m_etype=Type_GUIIMEEditBox;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIIMEEditBoxType(){}
	};

	class CGUIScrollBarType:public CGUIType
	{
	public:
		CGUIScrollBarType(){m_etype=Type_GUIScrollBar;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIScrollBarType(){}
	};

	class CGUIListBoxType:public CGUIType
	{
	public:
		CGUIListBoxType(){m_etype=Type_GUIListBox;}
		virtual bool IsContainer()const{return true;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIListBoxType(){}
	};

	class CGUIToolTipType:public CGUIType
	{
	public:
		CGUIToolTipType(){m_etype=Type_GUIToolTip;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIToolTipType(){}
	};

	class CGUIPainterType:public CGUIType
	{
	public:
		CGUIPainterType(){m_etype=Type_GUIPainter;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIPainterType(){}
	};

	class CGUISliderType:public CGUIType
	{
	public:
		CGUISliderType(){m_etype=Type_GUISlider;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUISliderType(){}
	};

	class CGUIGridType:public CGUIType
	{
	public:
		CGUIGridType(){m_etype=Type_GUIGrid;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIGridType(){}
	};

	class CGUIContainerType:public CGUIType
	{
	public:
		CGUIContainerType(){m_etype=Type_GUIContainer;}
		virtual bool IsContainer()const{return true;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUIContainerType(){}
	};
	class CGUICanvasType:public CGUIType
	{
	public:
		CGUICanvasType(){m_etype=Type_GUICanvas;}
		virtual bool IsContainer()const{return false;}
		static const char TypeName[];
		virtual const char* GetTypeName()const
		{
			return TypeName;
		}
	protected:
		virtual ~CGUICanvasType(){}
	};

}