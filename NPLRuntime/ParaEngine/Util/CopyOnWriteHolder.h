//----------------------------------------------------------------------
// Class:	CopyOnWriteHolder
// Authors:	Liu Weili
// Date:	2006.4.17
// Revised: 2006.4.17
//
// desc: 
// This class 
// 
//-----------------------------------------------------------------------
#pragma once
#define COW_UNSHARABLE 0xf0000000
namespace Policy{
	template<class X>
	class  CopyOnWriteItem
	{
	public:
		CopyOnWriteItem(): RefCount(1) , Data(new X){};
		~CopyOnWriteItem()
		{
			delete Data;
		}
		CopyOnWriteItem(const X& data): RefCount(1)
		{
			Data = new X(data);
		}
		int		RefCount;
		X*		Data;
	private:
		CopyOnWriteItem (const CopyOnWriteItem&); 
		CopyOnWriteItem& operator=(const CopyOnWriteItem&);

	};

	template<class X>
	class CopyOnWriteHolder
	{
	public:
		CopyOnWriteHolder()
		{
			Data=NULL;
		}
		CopyOnWriteHolder(const X& data)
		{
			Data=new CopyOnWriteItem<X>(data);
		}
		CopyOnWriteHolder (const CopyOnWriteHolder<X>& holder)
		{
			if (this != &holder && holder.Data!=NULL){
				//if the current thread does not own the old holder, a new copy must be allocated in this thread
				//This avoids further multi-threading problems
				if (holder.Data->RefCount==COW_UNSHARABLE){
					Data=new CopyOnWriteItem<X>(*holder.Data->Data);
				}else{
					Data=holder.Data;
					++Data->RefCount;
				}
			}
		}

		~CopyOnWriteHolder()
		{
			if (Data!=NULL&&(Data->RefCount==COW_UNSHARABLE||(--Data->RefCount)<1)){
				delete Data;
			}
		}
		CopyOnWriteHolder& operator=(const CopyOnWriteHolder<X>& holder)
		{
			if (this != &holder && holder.Data!=NULL){
				this->~CopyOnWriteHolder();
				//if the current thread does not own the old holder, a new copy must be allocated in this thread
				//This avoids further multi-threading problems
				if (holder.Data->RefCount==COW_UNSHARABLE){
					Data=new CopyOnWriteItem<X>(*holder.Data->Data);
				}else{
// 					if (Data!=NULL&&(Data->RefCount==COW_UNSHARABLE||(--Data->RefCount)<1)){
// 						delete Data;
// 					}
					Data=holder.Data;
					++Data->RefCount;
				}
			}
			return *this;
		}

		//the function gets a const reference to the data, this will not make the object unsharable.
		const X* GetConstData()const{return Data!=NULL?Data->Data:NULL;}

		//the function gets a writable reference to the data, this will make the object unsharable
		X* GetData()
		{
			if (!Data){
				return NULL;
			}
			AboutToModify(false);
			return Data->Data;
		}
		void AboutToModify(bool bSharable)
		{
			if (Data->RefCount>1&&Data->RefCount!=COW_UNSHARABLE){
				CopyOnWriteItem<X>* newdata=new CopyOnWriteItem<X>(*Data->Data);
				--Data->RefCount;
				Data=newdata;
			}
			Data->RefCount=bSharable?1:COW_UNSHARABLE;

		}

	protected:
		CopyOnWriteItem<X>* Data;
	};
}