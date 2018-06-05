// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include <new>
#include "cAudioMemory.h"

namespace cAudio
{
	//! Overrides the memory allocations for classes derived from it and makes them use the cAudio memory system
	class cMemoryOverride
	{
	public:
		void* operator new(size_t size, const char* file, int line, const char* function)
		{
			return cAudio::getMemoryProvider()->Allocate(size, file, line, function);
		}

		void* operator new(size_t size)
		{
			return cAudio::getMemoryProvider()->Allocate(size, NULL, -1, NULL);
		}

		void* operator new(size_t size, void* pointer)
		{
			(void) size;
			return pointer;
		}

		void* operator new[] ( size_t size, const char* file, int line, const char* function )
		{
			return cAudio::getMemoryProvider()->Allocate(size, file, line, function);
		}

		void* operator new[] ( size_t size )
		{
			return cAudio::getMemoryProvider()->Allocate(size, NULL, -1, NULL);
		}

		void operator delete( void* pointer )
		{
			cAudio::getMemoryProvider()->Free(pointer);
		}

		void operator delete( void* pointer, void* )
		{
			cAudio::getMemoryProvider()->Free(pointer);
		}

		void operator delete( void* pointer, const char* , int , const char*  )
		{
			cAudio::getMemoryProvider()->Free(pointer);
		}

		void operator delete[] ( void* pointer )
		{
			cAudio::getMemoryProvider()->Free(pointer);
		}

		void operator delete[] ( void* pointer, const char* , int , const char*  )
		{
			cAudio::getMemoryProvider()->Free(pointer);
		}
	};
};

