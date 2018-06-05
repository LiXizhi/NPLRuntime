// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"
#include "IMemoryProvider.h"

#ifdef CAUDIO_DEBUG
	#define CAUDIO_NEW new (__FILE__, __LINE__, __FUNCTION__)
	#define CAUDIO_DELETE delete
	#define CAUDIO_MALLOC(size) cAudio::getMemoryProvider()->Allocate(size, __FILE__, __LINE__, __FUNCTION__)
	#define CAUDIO_FREE(pointer) cAudio::getMemoryProvider()->Free((void*)pointer)
#else
	#define CAUDIO_NEW new 
	#define CAUDIO_DELETE delete
	#define CAUDIO_MALLOC(size) cAudio::getMemoryProvider()->Allocate(size, NULL, -1, NULL)
	#define CAUDIO_FREE(pointer) cAudio::getMemoryProvider()->Free((void*)pointer)
#endif

namespace cAudio
{
	//! Returns a pointer to the memory provider of cAudio
	/** Used by cAudio for all allocations of memory
	\return A pointer to the memory provider */
	CAUDIO_API IMemoryProvider* getMemoryProvider();
};