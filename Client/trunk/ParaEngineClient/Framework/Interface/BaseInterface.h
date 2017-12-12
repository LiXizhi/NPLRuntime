//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description:	API for interfaces. 
//-----------------------------------------------------------------------------
#pragma once

#include "PEtypes.h"

namespace ParaEngine
{
	class BaseInterface;

	/** @def ID for BaseInterfaces */
	#define BASEINTERFACE_ID Interface_ID(0, 1)

	/** Base class for those classes and interfaces in ParaEngine that can serve interfaces. */
	class InterfaceServer
	{
	public:
		virtual BaseInterface* GetInterface(Interface_ID id) { return NULL; }
	};

	/**
	*  The base class for interfaces in ParaEngine.  
	*  Provides basic identity, sub-interface access, lifetime management and 
	*  cloning methods.
	*/
	class BaseInterface : public InterfaceServer
	{
	public:
		// from InterfaceServer
		BaseInterface* GetInterface(Interface_ID id) { if (id == BASEINTERFACE_ID) return this; else return NULL; }

		// identification
		virtual Interface_ID	GetID() { return BASEINTERFACE_ID; }

		// interface lifetime management
		//   there is an implied Acquire() whenever an interface is served via a GetInterface()
		enum LifetimeType { noRelease, immediateRelease, wantsRelease, serverControlled }; 

		// LifetimeControl returns noRelease since 
		// AcquireInterface and ReleaseInterface do not perform 
		// any real acquiring and releasing (reference counting, etc.)
		// If the implementation of AcquireInterface and ReleaseInterface changes
		// in this class or derived classes, the return value of LifetimeControl 
		// needs to be updated accordingly.
		// RegisterNotifyCallback returns true if the callback will be called at or before deletion
		virtual LifetimeType	LifetimeControl() { return noRelease; }
		virtual BaseInterface*	AcquireInterface() { return (BaseInterface*)this; };  
		virtual void			ReleaseInterface() { };

		// direct interface delete request
		virtual void			DeleteInterface() { };

		// interface cloning
		virtual BaseInterface*	CloneInterface(void* remapDir = NULL) { return NULL; }
	};
}

