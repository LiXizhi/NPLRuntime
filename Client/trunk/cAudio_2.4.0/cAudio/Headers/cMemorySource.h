// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IDataSource.h"
#include "cMemoryOverride.h"

namespace cAudio
{
	//!Class used to read from a memory buffer.
	class cMemorySource : public IDataSource, public cMemoryOverride
	{
		public:
			/** Default Constructor
			\param data: Pointer to a data buffer to use.
			\param size: Size of the target buffer.
			\param copy: Whether to copy the buffer or use the provided pointer.  On destruct, that pointer will be deleted unless copy is true.
			*/
			cMemorySource(const void* data, int size, bool copy);
			~cMemorySource();

			virtual bool isValid();
			virtual int getCurrentPos();
			virtual int getSize();
			virtual int read(void* output, int size);
			virtual bool seek(int amount, bool relative);
		protected:
			char* Data;
			int Size;
			bool Valid;
			int Pos;
	};
};
