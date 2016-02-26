#ifndef cow_ptr_H_HEADER_GUARD_
#define cow_ptr_H_HEADER_GUARD_

/*
// cow_ptr class by David Maisonave (Axter)
// Copyright (C) 2005
// David Maisonave (Axter) (609-345-1007) (www.axter.com)
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  David Maisonave (Axter) makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.
Description: COW = Copy On Write
The cow_ptr class is a smart pointer class that can be used
with an STL container to create a container of smart pointers.
The main purpose of the cow_ptr, is to make it easier to create
a container of abstract based objects with a smart pointer that has
lazy construction logic.
The cow_ptr will share the pointer until one of the shared objects tries
to access the non-constant pointer.  At that point, the object is cloned,
and it releases the reference count to the original pointer.
The main difference between cow_ptr and other similar smart pointers
is that the cow_ptr has a clone function pointer, which is used to create
a copy of the derived object.  The clone function pointer is used in
the cow_ptr copy constructor and in the assignment operator.

The cow_ptr can also be used with sorted containers (std::map, std::set).
When used with sorted containers, the base class must have an operator<() function. (See example code.)

****** For more detailed description and example usage see following links: *******
Example Program:
http://code.axter.com/clone_ptr_demo.zip

See example program for example usage.

cow_ptr<std::string> test_ptr1(new std::string("some value"));
cow_ptr<std::string> test_ptr2 = test_ptr1;
std::string readonly_op = test_ptr2.c_ref().c_str();
(*test_ptr2) = "some value2";

*/

#include <algorithm>

// The clone function: (Thanks to Kai-Uwe Bux)
template < typename T, typename DT> T *  cow_ptr_ConstructAndDestruct_default_allocator_(T*, bool, DT* , void*);

/*! @class cow_ptr
@brief cow_ptr is a Copy On Write smart pointer.
It does not require the pointee to have clone function logic.
*/
template<typename T>
class cow_ptr
{
	typedef T * ( *clone_fct_Type ) (const T *, bool, void*, void*);//Last two values are not used (See note#1 at end of file)
	struct ref_link
	{
		ref_link():m_back(NULL), m_front(NULL){}
		ref_link *m_back;
		ref_link *m_front;
	private:  //Disallow copy constructor and assignment op
		ref_link(const ref_link&);
		ref_link& operator=(const ref_link&);
	};
	mutable ref_link m_ref_link;
	T* m_type;
	clone_fct_Type m_clone_fct;
public:
	//cow_ptr will only clone type that is pass to the constructor
	template<typename T_obj>
		cow_ptr(T_obj* type): m_type(type), m_clone_fct(get_alloc_func(type))
	{
#ifdef BOOST_ASSERT
		BOOST_ASSERT(type != NULL);
		BOOST_ASSERT(typeid(*type) == typeid(T_obj));
#endif //BOOST_ASSERT
	}
	//Destructor
	~cow_ptr() throw(){release();}
	//Copy constructor
	cow_ptr(const cow_ptr& Src):m_type(Src.m_type), m_clone_fct(Src.m_clone_fct){insert_link(&Src.m_ref_link);}
#if !defined(_MSC_VER) || (_MSC_VER > 1200)
	//Constructor for smart pointer derived type cow_ptr<DerivedT>
	template<class CompatibleDerivedT>
		cow_ptr(const cow_ptr<CompatibleDerivedT>& Src):m_type(NULL), m_clone_fct(NULL)
	{//No clean way to do this via COW, so just clone it
		Src.make_clone(m_type, m_clone_fct);
	}
	template<class CompatibleDerivedT>
		cow_ptr& operator=(const cow_ptr<CompatibleDerivedT>& Src)
	{//No clean way to do this via COW, so just clone it
		release();
		Src.make_clone(m_type, m_clone_fct);
		return *this;
	}
#endif //_MSC_VER != 1200
	//Assignment operator
	cow_ptr& operator=(const cow_ptr& Src){return assign(Src);}
	enum implement_default_object{eYes, eNo};
	//Default constructor needed for std::map
	cow_ptr(implement_default_object use_default_obj = eYes):m_type(NULL), m_clone_fct(NULL)
	{
		if (use_default_obj == eYes)
			assign(GetSetDefaultObject());
	}
	//For added safety, call SetDefaultObject to set default object before
	//using this class as the second type in a std::map
	static void SetDefaultObject(const cow_ptr<T>& NewValue){GetSetDefaultObject(&NewValue);}
	typedef T* pointer;
	typedef T& reference;
	bool operator! () const{return c_ptr() == 0;}
	template<class T2>
		cow_ptr& equal(const T2& Src){
		(*get_ptr()) = (Src);
		return *this;
	}
	//Operators that will force exclusive pointer
	inline T* operator->() {return get_ptr();}
	inline T& operator*() {return *get_ptr();}
	inline const T* operator->()const {return m_type;}
	inline const T& operator*() const {return *m_type;}
	cow_ptr& operator+=(const cow_ptr& Src){
		get_ptr()->operator+=(*Src.c_ptr());
		return *this;
	}
	template<class T2>
	cow_ptr& operator+=(const T2& Src){
		get_ptr()->operator+=(Src);
		return *this;
	}
	cow_ptr& operator+(const cow_ptr& Src){
		get_ptr()->operator+(*Src.c_ptr());
		return *this;
	}
	cow_ptr& operator-=(const cow_ptr& Src){
		get_ptr()->operator-=(*Src.c_ptr());
		return *this;
	}
	cow_ptr& operator-(const cow_ptr& Src){
		get_ptr()->operator-(*Src.c_ptr());
		return *this;
	}
	clone_fct_Type get_function_ptr()const{return m_clone_fct;}
	T* get_ptr() //get exclusive pointer
	{
		if (is_ref_linked())
		{
			T* tmp = m_clone_fct(m_type, true, NULL, NULL);
			release();
			m_type = tmp;
		}
		return m_type;
	}
	inline const T* c_ptr()const{return  m_type;}
	inline const T& c_ref()const{return  *m_type;}
	//Other Misc methods
	void swap(cow_ptr<T> & other)throw(){std::swap(m_ref_link, other.m_ref_link);std::swap(m_type, other.m_type);std::swap(m_clone_fct, other.m_clone_fct);}
	//Needed for operator=(const cow_ptr<CompatibleDerivedT>& 
	bool is_ref_linked()const{return (m_ref_link.m_back || m_ref_link.m_front);}
	//Allows cloning
	template<class PT, class FPT>	void make_clone(PT*& ptr, FPT& func_ptr) const
	{
		if (m_type)
		{
			ptr = m_clone_fct(m_type, true, NULL, NULL);
			func_ptr = (FPT)m_clone_fct;
		}
		else
			ptr = NULL;
	}
private:
	template<typename T_obj>
		static	clone_fct_Type get_alloc_func(T_obj*)
	{
		T * ( *tmp ) (T *, bool, T_obj*, void*) = cow_ptr_ConstructAndDestruct_default_allocator_<T,T_obj>;
		return (clone_fct_Type)tmp;
	}
	void release() throw()
	{
		if (!is_ref_linked())
		{
			if(m_type != NULL)
			{
				m_clone_fct(m_type, false, NULL, NULL);
				m_type = NULL;
			}
		}
		else
		{
			pop_link();
		}
	}
	void pop_link()
	{
		if (m_ref_link.m_back)
		{
			m_ref_link.m_back->m_front = m_ref_link.m_front;
		}
		if (m_ref_link.m_front)
		{
			m_ref_link.m_front->m_back = m_ref_link.m_back;
		}
		m_ref_link.m_front = NULL;
		m_ref_link.m_back = NULL;
	}
	void insert_link(ref_link *Target)
	{
		m_ref_link.m_back = Target;
		m_ref_link.m_front = Target->m_front;
		if (m_ref_link.m_front)
		{
			m_ref_link.m_front->m_back = &m_ref_link;
		}
		if (m_ref_link.m_back)
		{
			m_ref_link.m_back->m_front = &m_ref_link;
		}
	}
	template<class CompatibleSmartPtr>
		cow_ptr& assign(CompatibleSmartPtr& Src)
	{
		if (!m_type || Src.is_ref_linked() || m_type != Src.c_ptr())
		{
			release();
			m_type = Src.m_type;
			m_clone_fct = Src.m_clone_fct;
			insert_link(&Src.m_ref_link);
		}
		return *this;
	}
	static cow_ptr<T>& GetSetDefaultObject(const cow_ptr<T>* NewValue = NULL)
	{
		static cow_ptr<T> DefaultObj(eNo);
		if (NewValue && NewValue->m_type)
			DefaultObj = *NewValue;
		return DefaultObj;
	}
};

//Operators that can work with the shared pointer
template<class T, class U> bool operator<(cow_ptr<T> const & a, cow_ptr<U> const & b){return (*a.c_ptr()) < (*b.c_ptr());}
template<class T, class U> bool operator>(cow_ptr<T> const & a, cow_ptr<U> const & b){return (*a.c_ptr()) > (*b.c_ptr());}
template<class T, class U> bool operator<=(cow_ptr<T> const & a, cow_ptr<U> const & b){return (*a.c_ptr()) <= (*b.c_ptr());}
template<class T, class U> bool operator>=(cow_ptr<T> const & a, cow_ptr<U> const & b){return (*a.c_ptr()) >= (*b.c_ptr());}
template<class T, class U> bool operator==(cow_ptr<T> const & a, cow_ptr<U> const & b){return (*a.c_ptr()) == (*b.c_ptr());}
template<class T, class U> bool operator!=(cow_ptr<T> const & a, cow_ptr<U> const & b){return (*a.c_ptr()) != (*b.c_ptr());}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96
// Resolve the ambiguity between our op!= and the one in rel_ops
template<class T> bool operator!=(cow_ptr<T> const & a, cow_ptr<T> const & b){return (*a.c_ptr()) != (*b.c_ptr());}
#endif


/*
Note#1:
The ConstructAndDestruct functions have two extra arguments that are not used.  These arguments are not required at all in more compliant compilers
like VC++ 7.x and GNU 3.x.  However, for none compliant (pre-standard) compilers like VC++ 6.0 and BCC55, the extra argument declaration is required
so as to be able to fully qualify the function template type.  The argument declarations are needed for these compilers, but the actual argument variables
are not needed.  Anything pass to these last two arguments will be discarded.
*/

// The clone function: (Thanks to Kai-Uwe Bux)
template < typename T, typename DT> T *  cow_ptr_ConstructAndDestruct_default_allocator_(T *  ptr, bool bConstruct, DT* , void*) {
	if (bConstruct){return new DT(*static_cast<const DT*>(ptr));} //Should add assert like the following  assert(typeid(*tmp_ptr) == typeid(*Obj));
	delete const_cast<T*>(ptr); //This cast is needed for VC++ 6.0
	return NULL;
} 


#endif //!cow_ptr_H_HEADER_GUARD_

