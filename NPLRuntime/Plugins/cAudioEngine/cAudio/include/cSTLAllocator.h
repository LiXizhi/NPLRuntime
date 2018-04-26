// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"
#include "cAudioMemory.h"
//#include "cAudioString.h"

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <cstddef>

namespace cAudio
{
#if CAUDIO_REROUTE_STL_ALLOCATIONS == 1
	//! Reroutes allocations from STL containers into cAudio's memory system
	template <typename T> class cSTLAllocator
	{
	public:
		typedef T					value_type;
		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;
		typedef value_type&			reference;
		typedef const value_type&	const_reference;
		typedef std::size_t			size_type;
		typedef std::ptrdiff_t		difference_type;

		template<typename U>
		struct rebind
		{
			typedef cSTLAllocator<U> other;
		};

		cSTLAllocator()
		{ }

		~cSTLAllocator() throw()
		{ }

		cSTLAllocator( const cSTLAllocator& ) throw()
		{ }

		template <typename U>
		cSTLAllocator( const cSTLAllocator<U>& ) throw()
		{ }

		pointer address(reference x) const
		{
			return &x;
		}

		const_pointer address(const_reference x) const
		{
			return &x;
		}

		pointer allocate( size_type count, typename std::allocator<void>::const_pointer ptr = 0 )
		{
            (void)ptr;
			register size_type size = count*sizeof( T );
			pointer p  = static_cast<pointer>(CAUDIO_MALLOC(size));
			return p;
		}

		void deallocate( pointer p, size_type size )
		{
			CAUDIO_FREE(p);
		}

		size_type max_size() const throw()
		{
			return cAudio::getMemoryProvider()->getMaxAllocationSize();
		}

		void construct(pointer p, const T& val)
		{
			// call placement new
			new(static_cast<void*>(p)) T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}
	};

	template<> class cSTLAllocator<void>
	{
    public:
		typedef size_t      size_type;
		typedef ptrdiff_t   difference_type;
		typedef void*       pointer;
		typedef const void* const_pointer;
		typedef void        value_type;

		template<typename U>
		struct rebind
		{
			typedef cSTLAllocator<U> other;
		};
    };

	template <typename T>
	inline bool operator==(const cSTLAllocator<T>&, const cSTLAllocator<T>&)
	{
		return true;
	}

	template <typename T>
	inline bool operator!=(const cSTLAllocator<T>&, const cSTLAllocator<T>&)
	{
		return false;
	}
#endif

#if CAUDIO_REROUTE_STL_ALLOCATIONS == 1
	//typedef std::basic_string< cAudioChar, std::char_traits<cAudioChar>, cSTLAllocator<cAudioChar> > cAudioString;
	template<typename T1, typename T2> struct cAudioMap { typedef std::map< T1, T2, std::less< T1 >, cSTLAllocator< std::pair< T1, T2 > > > Type; };
	template<typename T> struct cAudioSet { typedef std::set< T, std::less< T >, cSTLAllocator< T > > Type; };
	template<typename T> struct cAudioList { typedef std::list< T, cSTLAllocator< T > > Type; };
	template<typename T> struct cAudioVector { typedef std::vector< T, cSTLAllocator< T > > Type; };
#else
	//typedef std::string cAudioString;
	//typedef std::basic_string<cAudioChar> cAudioString;
	template<typename T1, typename T2> struct cAudioMap { typedef std::map< T1, T2> Type; };
	template<typename T> struct cAudioSet { typedef std::set< T > Type; };
	template<typename T> struct cAudioList { typedef std::list< T > Type; };
	template<typename T> struct cAudioVector { typedef std::vector< T > Type; };
#endif

};
