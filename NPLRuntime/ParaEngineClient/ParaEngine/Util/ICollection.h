#pragma once
#include <map>
#include <vector>
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif
namespace ParaEngine
{
	using namespace std;
	template<class T>
	class ICollection
	{
	public:
		virtual T* GetAt(int index)=0;
		virtual const T* GetAt(int index)const =0;
		virtual int CopyInsert(T* obj, int index)=0;
		virtual int Insert(T* obj, int index)=0;
		virtual void Clear()=0;
		virtual int Remove(int index)=0;
		virtual int Remove()=0;
		virtual void CopyAdd(const T *obj)=0;
		virtual void Add(T *obj)=0;
		virtual int CopySetAt(const T *obj, int index)=0;
		virtual int SetAt(T* obj, int index)=0;
		virtual int IndexOf(const T *obj)const=0;
		virtual int Size()const=0;
		virtual int Resize(int newsize)=0;
	};
	/**
	* Default behavior of the collection adds only reference. But you can use the CopyXXX methods
	* to add a copy to the collection.
	* Template class T must inherited from IObject
	*/
	template<class T>
	class Collection_Vector :public ICollection<T>
	{
	public:
		typedef vector<T *> ConnectionVector_type;
		~Collection_Vector()
		{
			Clear();
		}
		T* GetAt(int index)
		{
			if ((DWORD)index>=m_items.size()) {
				return NULL;
			}
			return m_items[index];
		}
		const T* GetAt(int index)const
		{
			if ((DWORD)index>=m_items.size()) {
				return NULL;
			}
			return m_items[index];	
		}
		int CopyInsert(T* obj, int index)
		{
			if ((DWORD)index<=m_items.size()) {
				if (obj==NULL) {
					m_items.insert(m_items.begin()+index,obj);
				}else
					m_items.insert(m_items.begin()+index,(T*)obj->Clone());
				return index;
			}else
				return -1;
		}
		int Insert(T* obj, int index)
		{
			if ((DWORD)index<=m_items.size()) {
				if (obj) obj->addref();
				m_items.insert(m_items.begin()+index,obj);
				return index;
			}else 
				return -1;

		}
		void Clear()
		{
			typename ConnectionVector_type::iterator iter,iterend=m_items.end();
			for (iter=m_items.begin();iter!=iterend;iter++) {
				SAFE_RELEASE(*iter);
			}
			m_items.clear();
		}
		int Remove(int index)
		{
			if ((DWORD)index<m_items.size()) {
				SAFE_RELEASE(m_items[index]);
				m_items.erase(m_items.begin()+index);
				return (int)m_items.size();
			}
			return -1;
		}
		int Remove()
		{
			if (m_items.size()>0){
				SAFE_RELEASE(m_items.back());
				m_items.pop_back();
				return (int)m_items.size();
			}
			return -1;
		}
		void CopyAdd(const T* obj)
		{
			if (obj) {
				m_items.push_back((T*)obj->Clone());
			}else{
				T* pnull=NULL;
				m_items.push_back(pnull);
			}
		}
		void Add(T *obj)
		{
			if(obj) obj->addref();
			m_items.push_back(obj);
		}
		//copy an object to a place in the array
		int CopySetAt(const T *obj, int index)
		{
			if ((DWORD)index<m_items.size()) {
				SAFE_RELEASE(m_items[index]);
				if (obj) {
					m_items[index]=(T*)obj->Clone();
				}else{
					T* pnull=NULL;
					m_items[index]=pnull;
				}
				return index;
			}else
				return -1;

		}
		//only add a reference of an object to a place inthe array 
		//This will increase the reference count 
		int SetAt(T* obj, int index)
		{
			if ((DWORD)index<m_items.size()) {
				SAFE_RELEASE(m_items[index]);
				if (obj) obj->addref();
				m_items[index]=obj;
				return index;
			}else
				return -1;

		}
		int IndexOf(const T *obj)const
		{
			typename ConnectionVector_type::const_iterator iter, iterend = m_items.end();
			int a=0;
			for (a=0,iter=m_items.begin();iter!=iterend;iter++,a++) {
				if ((*iter)->Equals(obj)) {
					return a;
				};
			}
			return -1;
		}
		int Size()const{return (int)m_items.size();}
		int Resize(int newsize)
		{
			if (newsize<0) {
				return -1;
			}	
			int oldsize=(int)m_items.size();
			if (newsize<oldsize) {
				for (int a=newsize;a<oldsize;a++) {
					SAFE_RELEASE(m_items[a]);
				}

			}
			m_items.resize(newsize);
			if (newsize>oldsize) {
				for (int a=oldsize;a<newsize;a++) {
					m_items[a]=NULL;
				}
			}
			return newsize;
		}
	protected:
		ConnectionVector_type m_items;
	};

	/**
	* Default behavior of the collection adds only reference. But you can use the CopyXXX methods
	* to add a copy to the collection.
	* Template class T must inherited from IObject
	*/
	template<class T>
	class Collection_Map:public ICollection<T>
	{
	public:
		typedef map<int, T *> CollectionMap_type;
		Collection_Map():m_nSize(0){}
		~Collection_Map()
		{
			Clear();
		}
		T* GetAt(int index)
		{
			typename CollectionMap_type::iterator iter;
			if ((iter=m_items.find(index))==m_items.end()) {
				return NULL;
			}
			return iter->second;
		}
		const T* GetAt(int index)const
		{
			typename CollectionMap_type::const_iterator iter;
			if ((iter=m_items.find(index))==m_items.end()) {
				return NULL;
			}
			return iter->second;
		}
		int CopyInsert(T* obj, int index)
		{
			if (index>m_nSize){
				return -1;
			}
			typename CollectionMap_type::iterator iter1, iter2;
			T* pobj=NULL;
			if (obj!=NULL) {
				pobj=(T*)obj->Clone();
			}
			T* temp;
			//delete the item if it is NULL
			for (int a=index;a<=m_nSize;a++){
				if ((iter1=m_items.find(a))!=m_items.end()){
					temp=iter1->second;
				}else
					temp=NULL;
				if (pobj==NULL){
					m_items.erase(a);
				}else{
					m_items[a]=pobj;
				}
				pobj=temp;
			}
			m_nSize++;
			return index;
		}
		int Insert(T* obj, int index)
		{
			if (index>m_nSize){
				return -1;
			}
			if (obj) obj->addref();
			typename CollectionMap_type::iterator iter;
			T* pobj=obj;
			T* temp;
			//delete the item if it is NULL
			for (int a=index;a<=m_nSize;a++){
				if ((iter=m_items.find(a))!=m_items.end()){
					temp=iter->second;
				}else
					temp=NULL;
				if (pobj==NULL){
					m_items.erase(a);
				}else{
					m_items[a]=pobj;
				}
				pobj=temp;
			}
			m_nSize++;
			return index;
		}
		void Clear()
		{
			typename CollectionMap_type::iterator iter, iterend = m_items.end();
			for (iter=m_items.begin();iter!=iterend;iter++) {
				SAFE_RELEASE(iter->second);
			}
			m_items.clear();
		}
		int Remove(int index)
		{
			if (m_items.find(index)!=m_items.end()) {
				SAFE_RELEASE(m_items[index]);
				m_items.erase(index);
				typename CollectionMap_type::iterator iter;
				//delete the item if it is NULL
				for (int a=index;a<m_nSize;a++){
					if ((iter=m_items.find(a+1))!=m_items.end()){
						m_items[a]=m_items[a+1];
					}else
						m_items.erase(a);
				}
				m_nSize--;
				return m_nSize;
			}
			return -1;
		}
		int Remove()
		{
			if (m_items.find(m_nSize-1)!=m_items.end()){
				SAFE_RELEASE(m_items[--m_nSize]);
				m_items.erase(m_nSize);
				return m_nSize;
			}
			return -1;
		}
		void CopyAdd(const T* obj)
		{
			if (obj) {
				m_items[m_nSize]=((T*)obj->Clone());
			}
			m_nSize++;
		}
		void Add(T *obj)
		{
			if(obj){
				obj->addref();
				m_items[m_nSize]=obj;

			}
			m_nSize++;
		}
		//copy an object to a place in the array
		int CopySetAt(const T *obj, int index)
		{
			typename CollectionMap_type::iterator iter;
			if ((DWORD)index<(DWORD)m_nSize) {
				if ((iter=m_items.find(index))!=m_items.end()){
					SAFE_RELEASE(iter->second);
				}
				if (obj) {
					m_items[index]=(T*)obj->Clone();
				}
				return index;
			}else
				return -1;

		}
		//only add a reference of an object to a place inthe array 
		//This will increase the reference count 
		int SetAt(T* obj, int index)
		{
			if ((DWORD)index<(DWORD)m_nSize) {
				if (m_items.find(index)!=m_items.end()){
					SAFE_RELEASE(m_items[index]);
				}
				if (obj){
					obj->addref();
					m_items[index]=obj;
				}
				return index;
			}else
				return -1;

		}
		int IndexOf(const T *obj)const
		{
			typename CollectionMap_type::const_iterator iter, iterend = m_items.end();
			int a=0;
			for (a=0,iter=m_items.begin();iter!=iterend;iter++,a++) {
				if ((iter->second)->Equals(obj)) {
					return a;
				};
			}
			return -1;
		}
		int Size()const{return m_nSize;}
		int Resize(int newsize)
		{
			if (newsize<0) {
				return -1;
			}	
			if (newsize<m_nSize) {
				typename CollectionMap_type::iterator iter;
				for (int a=newsize;a<m_nSize;a++) {
					if ((iter=m_items.find(a))!=m_items.end()){
						SAFE_RELEASE(iter->second);
					}
				}

			}
			m_nSize=newsize;
			return newsize;
		}
	protected:
		CollectionMap_type m_items;
		int m_nSize;
	};

}