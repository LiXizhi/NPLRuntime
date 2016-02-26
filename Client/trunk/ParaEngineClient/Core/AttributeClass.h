#pragma once
#include "AttributeField.h"

namespace ParaEngine
{
	/** an attribute class is a collection of attribute fields. 
	* the same attribute class is shared by all instances of a given IAttributeField derived class. 
	* see also, CAttributesManager
	*/
	class CAttributeClass
	{
	public:
		CAttributeClass(int nClassID, const char* sClassName, const char* sClassDescription);
		virtual ~CAttributeClass(){}
		enum Field_Order
		{
			Sort_ByName,
			Sort_ByCategory,
			Sort_ByInstallOrder,
		};

	public:
		/** add a new field.
		@param sFieldname: field name
		@param Type: the field type. it may be one of the ATTRIBUTE_FIELDTYPE.
		@param offsetSetFunc: must be __stdcall function pointer or NULL. The function prototype should match that of the Type.
		@param offsetGetFunc: must be __stdcall function pointer or NULL. The function prototype should match that of the Type.
		@param sSchematics: a string or NULL. The string pattern should match that of the Type.
		@param sHelpString: a help string or NULL.
		@param bOverride: true to override existing field if any. This is usually set to true, so that inherited class can
		override the fields installed previously by the base class.
		*/
		void AddField(const char*  sFieldname, DWORD Type, void* offsetSetFunc, void* offsetGetFunc, const char* sSchematics, const char* sHelpString, bool bOverride);
		/** use of deprecated field takes no effect and will output warning in the log. */
		void AddField_Deprecated(const char *fieldName, bool bOverride = true);
		/** remove a field, return true if moved. false if field not found. */
		bool RemoveField(const char* sFieldname);
		void RemoveAllFields();

		/** class ID */
		int  GetClassID() const;
		/** class name */
		const char* GetClassName() const;
		/** class description */
		const char* GetClassDescription() const;

		/** Set which order fields are saved. */
		void SetOrder(Field_Order order);
		/** get which order fields are saved. */
		Field_Order GetOrder();

		/** get the total number of field. */
		int GetFieldNum();
		/** get field at the specified index. NULL will be returned if index is out of range. */
		CAttributeField* GetField(int nIndex);

		/**
		* get field index of a given field name. -1 will be returned if name not found.
		* @param sFieldname
		* @return
		*/
		int GetFieldIndex(const char*  sFieldname);

		/** return NULL, if the field does not exists */
		CAttributeField* GetField(const char*  sFieldname);

		/** create an instance of this class object */
		virtual IAttributeFields* Create();
	protected:
		int m_nClassID;
		const char* m_sClassName;
		const char* m_sClassDescription;
		vector<CAttributeField> m_attributes;
		Field_Order m_nCurrentOrder;
	private:
		/** insert a new field. return true if succeeded.
		@param bOverride: true to override existing field if any. This is usually set to true, so that inherited class can
		override the fields installed previously by the base class.
		*/
		bool InsertField(CAttributeField& item, bool bOverride);
	};

	/** derived attribute class.
	* Class T should add following macro to its header in order to use this factory class
	* ATTRIBUTE_SUPPORT_CREATE_FACTORY(CWeatherEffect);
	* or T should have Create() method manually defined.
	*/
	template<typename T>
	class CAttributeClassImp : public CAttributeClass
	{
	public:
		typedef T classType;
		CAttributeClassImp(int nClassID, const char* sClassName, const char* sClassDescription)
			: CAttributeClass(nClassID, sClassName, sClassDescription) {};
		virtual ~CAttributeClassImp(){};

		/** see class reference if classType::Create is not defined. */
		virtual IAttributeFields* Create()
		{
			return classType::Create();
		}
	};
}