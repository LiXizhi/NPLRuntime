// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef IREFCOUNTED_H
#define IREFCOUNTED_H

#include "cAudioMemory.h"

namespace cAudio
{
	//! Applies reference counting to certain cAudio objects.
	class IRefCounted
	{
	public:
		IRefCounted() : RefCount(1) { }
		virtual ~IRefCounted() { }

		//! Increments the reference count by one.
		void grab()
		{
			++RefCount; 
		}

		//! Decrements the reference count by one.  If it hits zero, this object is deleted.
		bool drop()
		{
			--RefCount;
			if (RefCount < 1)
			{
				CAUDIO_DELETE this;
				return true;
			}
			return false;
		}

		//! Returns the current reference count of this object.
		int getReferenceCount() const
		{
			return RefCount;
		}

	private:
		int RefCount;
	};	
}
#endif //! IREFCOUNTED_H