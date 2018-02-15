#pragma once
#include "PERefPtr.h"
#include <string>

namespace ParaEngine
{
	class IType;
	struct ObjectEvent;

	/* base class for reference counted objects. */
	class PE_CORE_DECL CRefCounted
	{
	public:
		/** add reference count of the object. */
		void addref()const
		{
			++m_refcount;
		}

		/** decrease reference count of the object. 
		* @return : return true if the the reference count is zero after decreasing it*/
		bool delref()const
		{
			return --m_refcount<=0;
		}
		/** get the reference count */
		int GetRefCount()const 
		{
			return m_refcount;
		}
		//all overridden functions should call this function in order to use the "delete when 
		//reference is zero" scheme
		virtual int Release();
		
		/**
		* addref and releases the ownership sometime soon automatically (usually at the end of the current frame).
		* @returns The CRefCounted itself.
		* @see CObjectAutoReleasePool
		*/
		CRefCounted* AddToAutoReleasePool();

	protected:
		CRefCounted();
		virtual ~CRefCounted();
	protected:
		mutable int m_refcount;
	};

	/**
	* Base class for a reference counted asset.
	* This class differs from IRef that it has 0 reference count, instead of 1 during creation.
	* such its derived class usually use asset_ptr<CRefCountedOne> to store a newly allocated object. 
	* and can use traditional new, delete pairs if not managed.
	*/
	class CRefCountedOne : public CRefCounted
	{
	protected:
		CRefCountedOne();
	};

	/** DONOT use this class directly, use weak_ptr instead. 
	* single threaded weak reference class to work with intrusive reference count class T.
	* T is usually CRefCounted derived class like CBaseObject.
	*/
	template <typename T>
	class PE_CORE_DECL weak_ref_object : public CRefCounted
	{
	public:
		weak_ref_object() : m_watched_object(0){};
		weak_ref_object(T* pWatchedObject) : m_watched_object(pWatchedObject){};

		virtual ~weak_ref_object(){};

		inline bool expired() const
		{
			return m_watched_object == nullptr;
		}
		inline bool IsValid() const
		{
			return m_watched_object != nullptr;
		}

		/** get the watched real object pointer */
		inline T* get() const { return m_watched_object; }
		inline operator T* () { return m_watched_object; }
		/** this is usually called in the destructor of type T, so that we know that the pointer is invalid. 
		*/
		inline void UnWatch(){
			m_watched_object = nullptr;
		}

	protected:
		T* m_watched_object;
	};

	/** weak reference ptr
	* use in a class like this: weak_ptr<CRefCounted> p;
	* e.g.
	* IObject* pObj = NULL;
	* WeakPtr a;
	* WeakPtr b(pObj);
	* PE_ASSERT(a == pObj && pObj == a && a == b);
	*/
	template<class RealT, class DefaultPointerClass = RealT> 
	struct PE_CORE_DECL weak_ptr
	{
	public:
		typedef weak_ref_object<RealT> WeakRefObject_type;

		weak_ptr() : px(0)
		{
		}

		~weak_ptr()
		{
			if (px != 0)
				px->Release();
		}

		weak_ptr(WeakRefObject_type * p)
			:px(p)
		{
			if (px != 0)
				px->addref();
		}

		/** Y must be derived class of RealT. */
		template<class Y>
		explicit weak_ptr(Y * p) :px(0)
		{
			static_assert(std::is_convertible<Y*, RealT*>::value, "Invalid Type for ParaEngine::weak_ptr!");
			if (p)
				px = p->GetWeakReference().get_weak_ref_object();
			
			if (px != 0)
				px->addref();
		}

		weak_ptr(const weak_ptr & r) : px(r.get_weak_ref_object()) // never throws
		{
			if (px != 0)
				px->addref();
		}

		weak_ptr & operator=(const weak_ptr & r) // never throws
		{
			if (px != r.get_weak_ref_object())
			{
				if (px != 0)
					px->Release();
				px = r.get_weak_ref_object();
				if (px != 0)
					px->addref();
			}
			return *this;
		}

		weak_ptr& operator=(WeakRefObject_type * r)
		{
			if(px != r)
			{
				if (px != 0)
					px->Release();
				px = r;
				if (px != 0)
					px->addref();
			}
			return *this;
		}

		/** Y must be derived class of RealT. */
		template<class Y>
		weak_ptr& operator=(Y* r) // never throws
		{
			static_assert(std::is_convertible<Y*, RealT*>::value, "Invalid Type for ParaEngine::weak_ptr!");

			if (get() != r)
			{
				if (px != 0)
					px->Release();
				
				px = r ? r->GetWeakReference().get_weak_ref_object() : 0;

				if (px != 0)
					px->addref();
			}
			return *this;
		}

		/** Y must be derived class of RealT. 
		* support casting to any derived class of RealT, such as CBaseObject, IAttributeObject, IGameObject, etc. 
		*/
		template<class Y>
		operator Y* () const // never throws
		{
			static_assert(std::is_convertible<Y*, RealT*>::value, "Invalid Type for ParaEngine::weak_ptr!");
			return (Y*)get();
		}

		DefaultPointerClass& operator* () const // never throws
		{
			PE_ASSERT(px != 0 && (*px).get()!=0);
			return *((*px).get());
		}

		DefaultPointerClass * operator-> () const // never throws
		{
			PE_ASSERT(px != 0);
			return (DefaultPointerClass*) ((*px).get());
		}

		DefaultPointerClass * get() const // never throws
		{
			return (DefaultPointerClass*) ((px != 0) ? (*px).get() : nullptr);
		}

		inline WeakRefObject_type * get_weak_ref_object() const // never throws
		{
			return px;
		}

		operator bool() const
		{
			return px != 0 && (*px).get()!=nullptr;
		}

		bool operator! () const // never throws
		{
			return px == 0 || (*px).get() == nullptr;
		}

		int use_count() const // never throws
		{
			return (px != 0 && (*px).get()) ? ((*px).get())->GetRefCount() : 0;
		}

		bool unique() const // nothrow
		{
			return use_count() == 1;
		}

		void reset()
		{
			if (px != 0)
			{
				px->Release();
				px = 0;
			}
		}

		inline bool operator==(weak_ptr const & b) const
		{
			return this->px == b.get_weak_ref_object();
		}

		inline bool operator!=(weak_ptr const & b) const
		{
			return this->px != b.get_weak_ref_object();
		}

		/* this conflicts with implicit cast operator
		
		template<class Y>
		inline bool operator==(Y const * b) const
		{
			return this->get() == b;
		}

		template<class Y>
		inline bool operator!=(Y const * b) const
		{
			return this->get() != b;
		}*/

		inline void UnWatch(){
			if (px)
				px->UnWatch();
		}
	private:
		// contained pointer
		WeakRefObject_type * px;
	};

	/** base class for object, such as CBaseObject, IAttributeObject, GUI object.  
	* this class support weak reference
	*/
	class PE_CORE_DECL IObject :public CRefCounted
	{
	public:
		typedef ParaEngine::weak_ptr<IObject> WeakPtr_type;

		/**
		* Clone the object's contains to a pointer.
		* The caller should allocate the memory and pass the pointer to this function..
		* Inheritance should implement this function
		* void Clone(IObject*) and IObject* Clone() should have the same behavior
		* In some cases, if you want to avoid Dead Reference( one object is being referred 
		* to by many objects and you can't update all the references). It is possible to use 
		* this function to replace the contains at a given pointer. But this may result in 
		* memory leak if you do not implement this function carefully or misuse it.
		**/
		virtual void Clone(IObject* obj)const;
		/**
		* Clone the object's contains and return a pointer to the newly created object.
		* The caller should free the memory of the return object.
		* Inheritance should implement this function
		* void Clone(IObject*) and IObject* Clone() should have the same behavior
		**/
		virtual IObject* Clone()const;
		/**
		* Compare the object with another object.
		* Inheritance should implement this function
		**/
		virtual bool Equals(const IObject *obj)const;
		virtual const IType* GetType()const { return nullptr; };
		virtual std::string ToString()const;

		/** get weak reference object. */
		WeakPtr_type& GetWeakReference();
		
		/** this function is only used to backward compatibility of ParaObject:AddEvent() function.  */
		virtual int ProcessObjectEvent(const ObjectEvent& event) { return 0; };
	protected:
		//One should not delete the object, use Release() instead
		//all overridden destructor should follow this rule
		virtual ~IObject();

		WeakPtr_type m_weak_reference;
	};

	typedef ParaEngine::weak_ptr<IObject> WeakPtr;
}