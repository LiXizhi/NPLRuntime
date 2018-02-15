#pragma once
#include "DynamicAttributeField.h"

namespace ParaEngine
{
	/** a group of dynamic attributes */
	class CDynamicAttributesSet : public CRefCounted
	{
	public:
		CDynamicAttributesSet();
		virtual ~CDynamicAttributesSet();

		/**
		* Get a dynamic field with a given name.
		* @param sName: name of the field to set
		* @return: return the field or NULL if does not exist
		*/
		CDynamicAttributeField* GetDynamicField(const std::string& sName);

		/**
		* Get a dynamic field with a given index.
		* @param sName: name of the field to set
		* @return: return the field or NULL if does not exist
		*/
		CDynamicAttributeField* GetDynamicField(int nIndex);

		/** get field name by index */
		const char* GetDynamicFieldNameByIndex(int nIndex);

		/** get field index 
		* @return -1 if not found. 
		*/
		int GetDynamicFieldIndex(const std::string& sName);

		/** how many dynamic field this object currently have. */
		int GetDynamicFieldCount();

		/**
		* set a dynamic field with a given name. if field not exist, it will create one. 
		* @param sName: name of the field to set
		* @value: the value to add. if NULL, field will be removed.
		* @return : -1 failed, if 0 means modified, if 1 means a new key is added, if 2 means a key is removed. 
		*/
		int SetDynamicField(const std::string& sName, const CVariable& value);

		/** add dynamic field and return field index */
		int AddDynamicField(const std::string& sName, ATTRIBUTE_FIELDTYPE dwType);

		/** remove all dynamic fields*/
		void RemoveAllDynamicFields();

		/** save only text dynamic fields to fieldname = value text strings. one on each line. line separator is \n.
		* @return the number of fields saved. or -1 if failed.
		*/
		int SaveDynamicFieldsToString(string& output);

		/** load only text dynamic fields from string
		* @return the number of fields loaded.
		*/
		int LoadDynamicFieldsFromString(const string& input);

	private:
		map<string, int> m_name_to_index;
		vector<DynamicFieldPtr> m_fields;
	};
}