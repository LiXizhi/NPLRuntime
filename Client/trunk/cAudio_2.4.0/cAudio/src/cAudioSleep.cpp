// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cAudioPlatform.h"

#ifdef CAUDIO_PLATFORM_WIN
#include <windows.h>	//Basic windows include for Sleep();
#else
#include <unistd.h>		//Assumed linux system, include for usleep()
#include <time.h>
#endif					//If you need to support another platform, simply add a define for it

#include "cAudioSleep.h"

namespace cAudio
{

void cAudioSleep(unsigned int ms)
{
#ifdef CAUDIO_PLATFORM_WIN
	Sleep(ms);
#else
	usleep(ms*1000); //convert from milliseconds to microseconds
#endif
}

};