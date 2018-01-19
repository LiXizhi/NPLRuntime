#pragma once
//-----------------------------------------------------------------------------
// Class:	FixedString on stack, based on boost fixed_string (which is rejected by boost). 
// Authors:	Modified by LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2008.6.20
// Desc: 
// This class manages a fixed-size, null-terminated string buffer.  It is meant
// to be allocated on the stack, and it makes no use of the heap internally.  In
// most cases you'll just want to use a std::string, but when you need to
// avoid the heap, you can use this class instead.
//
// Methods are provided to read the null-terminated buffer and to append data
// to the buffer, and once the buffer fills-up, it simply discards any extra
// append calls.
//
// Since this object clips if the internal fixed buffer is exceeded, it is
// appropriate for exception handlers where the heap may be corrupted. Fixed
// buffers that overflow onto the heap are provided by StackString. (see stack_container.hpp).
//-----------------------------------------------------------------------------
// (C) Copyright 2003-2004: Reece H. Dunn
//
//	Safe string operations
//
//	Original by Reece H. Dunn.
//	Modified by J. Nagle.
//
//
//	The templated class fixed_string<size> is provided for
//	declaring fixed size strings that do not use "new", and
//	the abstract base class fixed_string_base is provided
//	for references to fixed_string of any size.
//
//	These classes are intended to be safe against store-type buffer
//	overflows.
//	All storing subscripts must be checked.
//	Non-const char* pointers must not be returned.
//
//	The usual STL operations are to be provided for fixed_string,
//	along with the classic C string operations.
//
//	Direct replacement of C strings with fixed_string should
//	either work correctly or result in compile errors.
//
#include <cstring>
#include <stddef.h>
#include <stdarg.h>
#include <stdexcept>
#include <string>

namespace ParaEngine
{
	//	Abstract base class for templated strings.
	//	This can be passed to functions, regardless of size.
	template <class T>
	class fixed_string_base
	{
	public:
		virtual std::size_t capacity() const = 0;
		virtual std::size_t size() = 0;
		virtual const T* c_str() const = 0;
		virtual T* data() = 0;
		virtual void copy(const T * s ) = 0;
		virtual void clear() = 0;
		virtual void append(const T* s) = 0;
		virtual T& operator[](std::size_t ix) = 0;
		virtual const T& operator[](std::size_t ix) const = 0;

		typedef T* iterator;
		typedef const T* const_iterator;
		//	Implemented functions without direct equivalents at the fixed_string level.
		fixed_string_base<T>& operator=(const fixed_string_base<T>& s) 
		{	copy(s.c_str()); return(*this); }
		fixed_string_base<T>& operator=(const T* s)
		{	copy(s); return(*this); }
		int comp(fixed_string_base<T>& s1, 
			const fixed_string_base<T>& s2)
		{	s1.comp(s2.c_str()); return(s1); }
		bool operator==(const fixed_string_base<T>& s)
		{	return(comp(s) == 0); }
		bool operator==(const T* s)
		{	return(comp(s) == 0); }
	};
	//
	//	class fixed_string
	//
	//	Generic template class for fixed size strings.
	//
	//	One extra space is always allocated for a trailing null.
	//	But only calls to c_str guarantee that the trailing
	//	null will be present.
	//
	template< class T, std::size_t N>
	class fixed_string
		: public fixed_string_base<T>
	{
	public:
		typedef typename fixed_string_base<T>::iterator	iterator;
		typedef typename fixed_string_base<T>::const_iterator	const_iterator;

	private:
		T m_str[N+1];		// the data
		int m_len;			// length, not including trailing null
		// -1 if unknown
	private:
		void update_len()	// update length if necessary
		{	if (m_len >= 0) return;	// length needs no update
		for (m_len = 0; m_len < N && m_str[m_len]; m_len++)
		{} // count
		m_str[m_len] = 0;
		}

		void invalidate_len()	// invalidate length. String has changed
		{	m_len = -1;
		}

	public:
		fixed_string()			// default constructor
			:
		m_len(0)
		{
			m_str[0] = 0;		// initially null
			m_str[N] = 0;		// last must always be a null
		}

		fixed_string(const T* s)// constructor from string
			:
		m_len(0)
		{
			m_str[0] = 0;		// initially null
			m_str[N] = 0;		// last must always be a null
			copy(s);			// copy input
		}


		fixed_string(std::size_t cnt, T ch)	// constructor from char
			:
		m_len(0)
		{
			m_str[0] = 0;		// initially null
			m_str[N] = 0;		// last must always be a null
			while (cnt--) 
				this->operator +=(ch);	// append N copies of char
		}

	public: // access
		T* data() 				// access to raw data - unsafe
		{	invalidate_len();	// invalidates length count
		return(m_str);
		}
		const T* c_str() const
		{	//	"Virtual constness" - must update length info and trailing null
			fixed_string<T,N>* s =
				const_cast<fixed_string<T,N>* > (this);
			s->update_len();
			s->m_str[m_len] = 0;
			return( m_str );
		}

		std::size_t capacity() const
		{
			return(N);
		}

		std::size_t size()
		{
			update_len();
			return(m_len);
		}

		void clear()
		{
			m_len = 0;
			m_str[0] = 0;
		}

		void operator+=(const T& ch)
		{
			update_len();
			if (m_len >= N-1)
				throw(std::length_error("fixed_string +="));		// fails
			m_str[m_len] = ch;
			m_len++;
			m_str[m_len] = 0;
		}
		//	Subscripting.  
		//	Note that you can address the extra space for the trailing
		//	null, so that the classic "for (int i=0; s[i]; i++) will work.
	public: 
		T& operator[](std::size_t ix)
		{
			invalidate_len();
			if (ix <= N)
				return(m_str[ix]);		// normal case
			throw(std::length_error("fixed_string []"));		// fails
		}

		const T& operator[](std::size_t ix) const
		{
			if (ix <= N)
				return(m_str[ix]);		// normal case
			throw(std::length_error("fixed_string []"));		// fails
		}

	public:	// iterators, unchecked
		iterator begin()	{	return(&m_str[0]); }
		const iterator begin() const	{	return(&m_str[0]); }
		iterator end()		{	return(begin() + size() + 1); }
		const iterator end() const	{	return(begin() + size() + 1); }

	public: // copy
		void copy(const fixed_string_base<T>& s)
		{
			copy(s.c_str());
		}

		void copy(const T* s)
		{
			clear();
			append(s);		// inefficient, test only
		}


	public:  // append
		void append(const fixed_string_base<T>& s)
		{
			append(s.c_str());
		}

		void append(const T* s)
		{
			for (int i=0; s[i] !=0; i++)
			{
				*this += s[i];
			}
		}
	public: // comparison
		inline int comp(const char* s) const
		{
			return(compare(c_str(),s));		// from char_traits
		}
		inline int comp(const fixed_string_base<T>& s) const
		{
			return(compare(c_str(),s,c_str())); // from char_traits
		}
	};
} // ParaEngine

