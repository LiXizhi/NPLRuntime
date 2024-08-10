#pragma once

namespace ParaEngine
{
	/** only used for editing and created on demand. suitable for list, array objects
	* e.g.
	* auto proxy = new CArrayAttributeProxy< std::list<CBaseObject*> >(pList);
	*/
	template<typename T>
	class CArrayAttributeProxy : public IAttributeFields
	{
	public:
		typedef T* ArrayPtr_type;
		CArrayAttributeProxy(ArrayPtr_type pArray, const char* sName = NULL) : m_pArray(pArray){ 
			if (sName)
				SetIdentifier(sName);
		};
		virtual ~CArrayAttributeProxy(){};

		ATTRIBUTE_DEFINE_CLASS(CArrayAttributeProxy);

		virtual const std::string& GetIdentifier() {
			return m_sName;
		}
		virtual void SetIdentifier(const std::string& sName){
			m_sName = sName;
		}

		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0) {
			return (m_pArray != nullptr) ? (int)m_pArray->size() : 0;
		};
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex=0){
			if (m_pArray){
				auto iter = m_pArray->begin();
				std::advance(iter, nRowIndex);
				if (iter != m_pArray->end())
					return *iter;
			}
			return NULL;
		}
	public:
		ArrayPtr_type m_pArray;
		std::string m_sName;
	};

	/** only used for editing and created on demand. suitable for map objects
	* e.g.
	* auto proxy = new CMapAttributeProxy< map<string, CBaseObject*> >(pList);
	*/
	template<typename T>
	class CMapAttributeProxy : public IAttributeFields
	{
	public:
		typedef T* MapPtr_type;
		CMapAttributeProxy(MapPtr_type pArray, const char* sName = NULL) : m_pArray(pArray){
			if (sName)
				SetIdentifier(sName);
		};
		virtual ~CMapAttributeProxy(){};

		ATTRIBUTE_DEFINE_CLASS(CMapAttributeProxy);

		virtual int GetChildAttributeObjectCount() {
			return (m_pArray != nullptr) ? (int)m_pArray->size() : 0;
		};
		virtual IAttributeFields* GetChildAttributeObject(int nIndex){
			if (m_pArray){
				auto iter = m_pArray->begin();
				std::advance(iter, nIndex);
				if (iter != m_pArray->end())
					return iter->second;
			}
			return NULL;
		}

		virtual const std::string& GetIdentifier() {
			return m_sName;
		}
		virtual void SetIdentifier(const std::string& sName){
			m_sName = sName;
		}
	public:
		MapPtr_type m_pArray;
		std::string m_sName;
	};
}