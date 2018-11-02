//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description:	API for object modifier plugable class. 
//-----------------------------------------------------------------------------
#pragma once
#include "baseinterface.h"
namespace ParaEngine
{
	/**
	Each plugin is derived from a base class provided by ParaEngine. 
	For example, if you want to develop a modifier object plugin you might derive your class from class CModifier.
	It is the plugin developer's responsibility to provide implementations of some of the required methods.
	Plugins can also call methods provided by ParaEngine using a pointer to an IParaEngine Class. 
	*/
	class CModifier
	{
	public:
		PE_CORE_DECL CModifier(void);
		PE_CORE_DECL virtual ~CModifier(void);
	public:
		
		/**
		* This is how things are actually deleted. 
		* Since they are created with the ClassDesc::Create()  function, and 
		* deleted via this function, they can use a different memory allocator
		* than the core code. (theoretically)
		*/
		PE_CORE_DECL virtual void DeleteThis();
	};
}

