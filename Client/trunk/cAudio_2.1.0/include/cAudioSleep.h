// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CAUDIOSLEEP_H
#define CAUDIOSLEEP_H

#include "cAudioDefines.h"

namespace cAudio
{
	//! Causes the current thread to give up control for a certain duration. 
	/** \param ms: amount of miliseconds to sleep */
	CAUDIO_API void cAudioSleep(unsigned int ms);
};

#endif //! CAUDIOSLEEP_H