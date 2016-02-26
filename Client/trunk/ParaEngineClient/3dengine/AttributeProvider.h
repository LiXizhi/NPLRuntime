#pragma once
#include "ic/ParaDatabase.h"
#include "IAttributeFields.h"
#include <vector>
#include <string>
namespace ParaEngine
{
	using namespace std;
	/** a database record in attribute db
	*/
	struct CAttributeDbItem
	{
		string m_sName;
		//string m_sInstanceName;
		int m_nType;
		string m_sValue;
		string m_sDescription;
		string m_sSchematics;
		CAttributeDbItem():m_nType(0){};
	};

	/** 
	Attribute fields data provider: it is a group of attribute stored in {name, value} value pair fasion in a single database table. 
	The db table does not define the name, hence a table in the database is very similar to an intance of Iattributefields in ParaEngine. 
	To be extensible, the table record has the following format: {name(prim key), InstanceName(indexed), value, type, description, schematics}
	in other word, the database table provides a serialization of IAttributeField (all fields are persistent except get/set function address).
	All instances sharing the same ClassName are usually stored in the same db table. In case of a singleton, we can default InstanceName field to NULL. 
	e.g. game world class may store global information for a game in the database, such as the spawn points, last player position, etc. 
	Besides serializing IAttributeField class, the data provider should also expose methods to add and remove dynamic fields.
	@remark: Currently, instanced object is not supported. One table only matches to a single object. 
	*/
	class AttributeProvider
	{
	public:
		AttributeProvider(void);
		~AttributeProvider(void);
	public:
		//////////////////////////////////////////////////////////////////////////
		// common function 

		/** whether db is opened. */
		bool IsValid() {return m_pDataBase.get() !=0;};

		/** delete the database and set everything to NULL*/
		void Cleanup();
		/** get the database object associated with this provider*/
		DBEntity* GetDBEntity(){return m_pDataBase.get();};
		/**
		* replace the current database with current one.  the old one is closed and the new once will be opened. 
		* @param sConnectionstring: the file path
		*/
		void SetDBEntity(const string& sConnectionstring);

		/** ensure that the database has been set up properly. If not, ResetDatabase() is called to reset the database to blank */
		void ValidateDatabase();

#ifdef _DEBUG
		/** test data base*/
		void TestDB();
#endif

	public:
		//////////////////////////////////////////////////////////////////////////
		// query functions 

		/**
		* save the attribute object to the given table
		* @param sTableName  if table name is empty,  the current table is used. 
		* @param pAttributeObject [in]
		* @return true if succeeds
		*/
		bool SaveAttributeObject(const char*  sTableName, IAttributeFields* pAttributeObject);

		/**
		* load the attribute object from a given table.
		* @param sTableName  if table name is empty,  the current table is used. 
		* @param pAttributeObject [out]
		* @return true if succeeds
		*/
		bool LoadAttributeObject(const char*  sTableName, IAttributeFields* pAttributeObject);

		/**
		* current attribute table name, if the table does not exists it will be created. 
		* @param sName 
		*/
		void SetTableName(const char*  sName);

		/**
		* current attribute table name
		* @return current attribute table name
		*/
		const string& GetTableName(){return m_sCurrentTableName;};

		/**
		* reset a given table to blank. if the table does not exists a blank one will be created. 
		* @param sName table name
		* @return true if succeeds
		*/
		bool  ResetTable(const char*  sName);

		/**
		* current object name . a table may save attribute for multiple objects. 
		* this may be empty string, in which case the table is assumed to be storing a singleton object.
		* @param sName 
		*/
		//void SetObjectName(const char*  sName) { m_sObjectName = sName;}
		//const string& GetObjectName() { return m_sObjectName;}

		/**
		* whether a given attribute exists. This function is usually used internally. 
		* One common use of this function is to test if a field exists, so that we know if a object has been saved before or not.
		* @param sName : name of the attribute field
		* @return 
		*/
		bool DoesAttributeExists(const char*  sName);

		/**
		* get the attribute of the current attribute instance in the current table
		* @param sName : name of the attribute field
		* @param sOut : value of the field as a string. 
		* @return true if succeed. 
		*/
		bool GetAttribute(const char*  sName, string& sOut);

		/**
		* get the complete attribute information in the current table
		* @param inout : [in|out] get the complete database information. CAttributeDbItem::m_sName and CAttributeDbItem::m_sInstanceName must be filled first.
		*	the rest of CAttributeDbItem will be filled when the function returns. Please note that if CAttributeDbItem::m_sInstanceName is empty 
		*	then the current object name will used instead. 
		* @return true if succeed. 
		*/
		bool GetAttribute(CAttributeDbItem& inout);

		/**
		* insert a new attribute field in the current table
		* @param sIn : [in] CAttributeDbItem::m_sName is always ignored. 
		* @return true if succeed. 
		*/
		bool InsertAttribute(CAttributeDbItem& sIn);

		/**
		* insert by simple name value pair
		* @param sName 
		* @param sValue 
		* @return 
		*/
		bool InsertAttribute(const char*  sName, const char*  sValue);

		/**
		* update the attribute of the current attribute instance in the current table
		* insert the attribute if it is not created before.
		* @param sName : name of the attribute field
		* @param sIn: value of the attribute field
		* @return true if succeed. 
		*/
		bool UpdateAttribute(const char*  sName, const char*  sIn);

		/**
		* delete the attribute of the current attribute instance in the current table
		* @param sName : name of the attribute field
		* @return true if succeed. 
		*/
		bool DeleteAttribute(const char*  sName);

		/** create the current table, if it does not exists. This function is mostly 
		* called automatically when the SetTableName() function is called. 
		* @return true if the table exists either created or just the old one. */
		bool CreateTableIfNotExists();

	private:
		/** base database interface */
		ParaEngine::asset_ptr<DBEntity> m_pDataBase;
		CDBRecordSet* m_pProcSelectAttribute;
		CDBRecordSet* m_pProcAttributeExists;
		CDBRecordSet* m_pProcUpdateAttribute;
		CDBRecordSet* m_pProcInsertAttribute;
		CDBRecordSet* m_pProcDeleteAttribute;

		/**
		* called whenever the table changes
		*/
		void ReleaseStoredProcedures();
		/** current attribute table name */
		string m_sCurrentTableName;
		/** current object name . a table may save attribute for multiple objects. 
		this may be empty string, in which case the table is assumed to be storing a singleton object. */
		//string m_sObjectName;
	};
}
