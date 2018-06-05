// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

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
		virtual void grab()
		{
			++RefCount; 
		}

		//! Decrements the reference count by one.  If it hits zero, this object is deleted.
		virtual bool drop()
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

	protected:
		int RefCount;
	};	
}
