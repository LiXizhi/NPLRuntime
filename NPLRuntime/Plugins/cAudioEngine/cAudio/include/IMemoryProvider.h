// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"
#include <cstring>
namespace cAudio
{
	//! Interface for a class that allocates and frees memory used by cAudio.
	class IMemoryProvider
	{
	public:
		//! Allocates memory and returns a pointer to it.
		/**
		\param size: Size of the memory chunk to allocate in bytes.
		\param filename: Filename of the source file that this allocation took place in (in Debug) or NULL otherwise.
		\param line: Line of the source file where this allocation took place (in Debug) or -1 otherwise.
		\param function: Function that this allocation took place in (in Debug) or NULL otherwise.
		\return Pointer to the allocated memory or NULL if allocation failed. */
		virtual void* Allocate(size_t size, const char* filename, int line, const char* function) = 0;
		
		//! Frees memory previously allocated.
		/**
		\param pointer: Pointer to the memory location to free. */
		virtual void Free(void* pointer) = 0;

		//! Returns the largest possible single allocation that can be made.
		virtual size_t getMaxAllocationSize() = 0;
	};
};
