#pragma once
#include "PERef.h"
#include "AttributeClassIDTable.h"
#include "AttributeField.h"
#include "AttributeClass.h"

/** @def get the offset of a data member(x) from a class(cls) */
#define Member_Offset(x, cls) (int)&(((cls *)0)->x)

#define ATTRIBUTE_DEFINE_CLASS(clsName) \
	virtual int GetAttributeClassID(){return IAttributeFields::GetAttributeClassID();}\
	virtual const char* GetAttributeClassName(){ static const char name[] = #clsName; return name; }

/** @def define a property on class
* e.g. PROPERTY_FIELD(Test, IAttributeFields, m_nTest, int) defines SetTest and GetTest on IAttributeFields
*/
#define PROPERTY_FIELD(name, cls, member, datatype) \
	static void Set##name(cls* c, datatype v) {c->member = v;};\
	static datatype Get##name(cls* c) {return c->member;};

#define ATTRIBUTE_METHOD(clsName, name) static HRESULT name(clsName* cls)
#define ATTRIBUTE_METHOD1(clsName, name, dataType) static HRESULT name(clsName* cls, dataType p1)
#define ATTRIBUTE_METHOD2(clsName, name, dataType) static HRESULT name(clsName* cls, dataType p1,dataType p2)
#define ATTRIBUTE_METHOD3(clsName, name, dataType) static HRESULT name(clsName* cls, dataType p1,dataType p2,dataType p3)
#define ATTRIBUTE_METHOD4(clsName, name, dataType) static HRESULT name(clsName* cls, dataType p1,dataType p2,dataType p3,dataType p4)
#define ATTRIBUTE_METHOD1_1(clsName, name, dataType1, dataType2) static HRESULT name(clsName* cls, dataType1 p1,dataType2 p2)

/** @def define this to support factory object instantiation from attribute class. By default CAttributeClass::Create() does not support instantiation.
ATTRIBUTE_SUPPORT_CREATE_FACTORY(classname)
*/
#define ATTRIBUTE_SUPPORT_CREATE_FACTORY(classname)  \
	virtual CAttributeClass* CreateAttributeClass(){ \
		return new CAttributeClassImp<classname>(GetAttributeClassID(), GetAttributeClassName(), GetAttributeClassDescription()); \
	};\
	static classname * Create() { \
		classname * pObj = new classname(); \
		pObj->AddToAutoReleasePool();\
		return pObj;\
	};

namespace ParaEngine
{
	class CDynamicAttributesSet;
	class CDynamicAttributeField;

	/** A common interface for all classes implementing IAttributeFields
	By implementing this class's virtual functions, it enables a class to easily expose attributes
	to the NPL scripting interface. All standard attribute types are supported by the external editors, which
	makes displaying and editing object attributes an automatic process.

	This class has only one data member, hence there are little space penalties for implementing this class.
	The attribute information for each class is kept centrally in a global table by CAttributeClass.
	An example of using this class can be found at	AttributesManager.h and CAttributesManager::PrintManual()
	most objects in ParaEngine implement this class, such as CBaseObject, etc.
	The following virtual functions must be implemented: GetAttributeClassID(), GetAttributeClassName(), InstallFields()

	Use following macros to define new class or adding new attributes:
		ATTRIBUTE_DEFINE_CLASS(XXXX_classname)
		Class destructor should be a virtual function 
	*/
	class IAttributeFields : public IObject
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, IAttributeFields> WeakPtr_type;
		IAttributeFields(void);
		virtual ~IAttributeFields(void);

	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID();
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "IAttributeFields"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);
		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName);

		ATTRIBUTE_METHOD1(IAttributeFields, GetName_s, const char**)	{ *p1 = cls->GetIdentifier().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(IAttributeFields, SetName_s, const char*)	{ cls->SetIdentifier(p1); return S_OK; }
		ATTRIBUTE_METHOD(IAttributeFields, PrintObject_s){ cls->PrintObject(); return S_OK; }
		ATTRIBUTE_METHOD(IAttributeFields, AddRef_s){ cls->addref(); return S_OK; }
		ATTRIBUTE_METHOD1(IAttributeFields, GetRefCount_s, int*)	{ *p1 = cls->GetRefCount(); return S_OK; }

		ATTRIBUTE_METHOD1(IAttributeFields, SetTime_s, int)	{ cls->SetTime(p1); return S_OK; }
		ATTRIBUTE_METHOD1(IAttributeFields, GetTime_s, int*)	{ *p1 = cls->GetTime(); return S_OK; }

		ATTRIBUTE_METHOD(IAttributeFields, Release_s){ cls->Release(); return S_OK; }
	public:
		//////////////////////////////////////////////////////////////////////////
		//
		// implementation of the following virtual functions are optional
		//
		//////////////////////////////////////////////////////////////////////////

		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		/** whether some of the fields are modified.It is up to the implementation class to provide this functionality if necessary. */
		virtual bool IsModified(){ return false; };
		/** set whether any field has been modified. */
		virtual void SetModified(bool bModified){};

		/** validate all fields and return true if validation passed. */
		virtual bool ValidateFields(){ return true; };
		/** get the recent validation message due to the most recent call to ValidateFields() */
		virtual string GetValidationMessage(){ return ""; };

		/**
		* Reset the field to its initial or default value.
		* @param nFieldID : field ID
		* @return true if value is set; false if value not set.
		*/
		virtual bool ResetField(int nFieldID){ return false; };

		/**
		* Invoke an (external) editor for a given field. This is usually for NPL script field
		* @param nFieldID : field ID
		* @param sParameters : the parameter passed to the editor
		* @return true if editor is invoked, false if failed or field has no editor.
		*/
		virtual bool InvokeEditor(int nFieldID, const std::string& sParameters){ return false; };

		/** add child object. */
		virtual bool AddChildAttributeObject(IAttributeFields* pChild, int nRowIndex = -1, int nColumnIndex = 0);

		/** convert to object of a given type. 
		* @param sObjectType: 
		*/
		virtual void* QueryObjectByName(const std::string& sObjectType);

		/** convert to object of a given type.
		* @param nObjectType: such as values in ATTRIBUTE_CLASSID_TABLE, In most cases, we query ATTRIBUTE_CLASSID_CBaseObject
		*/
		virtual void* QueryObject(int nObjectType);

		/** get the current local time in case it is animated in milli seconds frames. */
		virtual int GetTime();
		virtual void SetTime(int nTime);

		//////////////////////////////////////////////////////////////////////////
		//
		// Dynamic field method
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* Get a dynamic field with a given name.
		* @param sName: name of the field to set
		* @return: return the field or NULL if does not exist
		*/
		virtual CDynamicAttributeField* GetDynamicField(const std::string& sName);

		/**
		* Get a dynamic field with a given index.
		* @param sName: name of the field to set
		* @return: return the field or NULL if does not exist
		*/
		virtual CDynamicAttributeField* GetDynamicField(int nIndex);

		/** get field name by index */
		virtual const char* GetDynamicFieldNameByIndex(int nIndex);

		/** how many dynamic field this object currently have. */
		virtual int GetDynamicFieldCount();

		/**
		* set a dynamic field with a given name.
		* @param sName: name of the field to set
		* @value: the value to add. if NULL, field will be removed.
		* @return : -1 failed, if 0 means modified, if 1 means a new key is added, if 2 means a key is removed.
		*/
		virtual int SetDynamicField(const std::string& sName, const CVariable& value);

		/** add dynamic field and return field index */
		virtual int AddDynamicField(const std::string& sName, ATTRIBUTE_FIELDTYPE dwType);

		/** remove all dynamic fields*/
		virtual void RemoveAllDynamicFields();

		/** save only text dynamic fields to fieldname = value text strings. one on each line. line separator is \n.
		* @return the number of fields saved. or -1 if failed.
		*/
		virtual int SaveDynamicFieldsToString(std::string& output);

		/** load only text dynamic fields from string
		* @return the number of fields loaded.
		*/
		virtual int LoadDynamicFieldsFromString(const std::string& input);
	public:
		/** get the main attribute class object. */
		virtual CAttributeClass* GetAttributeClass();
		/** print the content of this object to a text file at temp/doc/[ClassName].txt.
		This is usually used for dumping and testing object attributes.*/
		void PrintObject();

		static HRESULT GetAttributeClassID_s(IAttributeFields* cls, int* p1) { *p1 = cls->GetAttributeClassID(); return S_OK; }
		static HRESULT GetAttributeClassName_s(IAttributeFields* cls, const char** p1) { *p1 = cls->GetAttributeClassName(); return S_OK; }

		/**
		* Open a given file with the default registered editor in the game engine.
		* @param sFileName: file name to be opened by the default editor.
		* @param bWaitOnReturn: if false, the function returns immediately; otherwise it will wait for the editor to return.
		* @return true if opened.
		*/
		static bool OpenWithDefaultEditor(const char* sFilename, bool bWaitOnReturn = false);

	protected:
		/** initialize fields */
		virtual CAttributeClass* CreateAttributeClass();
		virtual CDynamicAttributesSet* GetDynamicAttributes(bool bCreateIfNotExist = false);
	private:
		CDynamicAttributesSet* m_pDynamicAttributes;
	};

	
}
