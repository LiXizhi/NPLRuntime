// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IMemoryProvider.h"
#include <cstring>

namespace cAudio
{
	//! Memory provider that wraps the standard memalloc and free
	class cStandardMemoryProvider : public IMemoryProvider
	{
	public:
		virtual void* Allocate(size_t size, const char* filename, int line, const char* function);
		virtual void Free(void* pointer);
		virtual size_t getMaxAllocationSize();
	};
};
