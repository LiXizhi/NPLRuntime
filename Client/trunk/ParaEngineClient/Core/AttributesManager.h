#pragma once
#include "IAttributeFields.h"
#include "ObjectFactory.h"
#include "unordered_ref_array.h"

namespace ParaEngine
{
	/** a central place for managing all attributes of all classes in ParaEngine. */
	class CAttributesManager : public IAttributeFields
	{
	public:
		CAttributesManager();
		~CAttributesManager();
		static CAttributesManager& GetSingleton();
	public:
		/** Get attribute class object by ID, return NULL if it does not exists.*/
		CAttributeClass* GetAttributeClassByID(int nClassID);
		CAttributeClass* GetAttributeClassByName(const std::string& sClassName);
		/** try to create object using registered factory attribute class or factory class*/
		IAttributeFields* CreateObject(const std::string& sClassName);

		/** register a factory class for creating object. e.g.
		* RegisterObjectFactory("CWeatherEffect", new CDefaultObjectFactory<CWeatherEffect>());
		*/
		bool RegisterObjectFactory(const std::string& sClassName, CObjectFactory* pObjectFactory);
		CObjectFactory* GetObjectFactory(const std::string& sClassName);

		/** print the manual of all classes in the manager to a text file. */
		void PrintManual(const string& filepath);
		/** print the content of a given object to a text file. This is usually used for dumping and testing object attribute.*/
		void PrintObject(const string& filepath, IAttributeFields* pObj);

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CAttributesManager;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "Attributes Manager"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD(CAttributesManager, PrintManual_s){cls->PrintManual("attr_manual.txt");return S_OK;}
	protected:
		map<int, CAttributeClass*>	m_classes;
		map<std::string, CAttributeClass*>	m_classNames;
		map<std::string, CObjectFactory*>	m_factory_map;
		unordered_ref_array<CObjectFactory*>  m_object_factories;

		/** nClassID must not exist prior to calling this function. */
		void AddAttributeClass(int nClassID, CAttributeClass* pClass);

	private:
		/** print a field in the manual format
		@param pObj: if not NULL, it will output value using get method. */
		void PrintField(CParaFile& file, CAttributeField* pField, void* pObj = NULL);
		/** print a class in the manual format
		@param pObj: if not NULL, it will output value using get method. */
		void PrintClass(CParaFile& file, CAttributeClass* pClass, void* pObj = NULL);
		
		friend class CAttributeField;
		friend class CAttributeClass;
		friend class IAttributeFields;
	};
}
