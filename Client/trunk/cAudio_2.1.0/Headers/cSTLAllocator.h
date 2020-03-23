#ifndef CSTLALLOCATOR_H_INCLUDED
#define CSTLALLOCATOR_H_INCLUDED

#include "../include/cAudioDefines.h"
#include "../include/cAudioMemory.h"

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>

namespace cAudio
{
#ifdef CAUDIO_REROUTE_STL_ALLOCATIONS
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

#ifdef CAUDIO_REROUTE_STL_ALLOCATIONS
	typedef std::basic_string< char, std::char_traits<char>, cSTLAllocator<char> > cAudioString;
	template<typename T1, typename T2> struct cAudioMap { typedef std::map< T1, T2, std::less< T1 >, cSTLAllocator< std::pair< T1, T2 > > > Type; };
	template<typename T> struct cAudioSet { typedef std::set< T, std::less< T >, cSTLAllocator< T > > Type; };
	template<typename T> struct cAudioList { typedef std::list< T, cSTLAllocator< T > > Type; };
	template<typename T> struct cAudioVector { typedef std::vector< T, cSTLAllocator< T > > Type; };
#else
	typedef std::string cAudioString;
	template<typename T1, typename T2> struct cAudioMap { typedef std::map< T1, T2> Type; };
	template<typename T> struct cAudioSet { typedef std::set< T > Type; };
	template<typename T> struct cAudioList { typedef std::list< T > Type; };
	template<typename T> struct cAudioVector { typedef std::vector< T > Type; };
#endif

};

#endif //! CSTLALLOCATOR_H_INCLUDED