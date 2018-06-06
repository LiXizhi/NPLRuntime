// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "../Headers/cListener.h"
#include <AL/al.h>
#include "../Headers/cEFXFunctions.h"

namespace cAudio
{
	void cListener::setPosition(const cVector3& pos)
	{
		cAudioMutexBasicLock lock(Mutex);
		Position = pos;
		alListener3f(AL_POSITION, Position.x, Position.y, Position.z);
	}
	void cListener::setDirection(const cVector3& dir)
	{
		cAudioMutexBasicLock lock(Mutex);
		Direction = dir;
		float orient[6] = {Direction[0], Direction[1], Direction[2], UpVector[0], UpVector[1], UpVector[2]};
		alListenerfv(AL_ORIENTATION, orient);
	}
	void cListener::setUpVector(const cVector3& up)
	{
		cAudioMutexBasicLock lock(Mutex);
		UpVector = up;
		float orient[6] = {Direction[0], Direction[1], Direction[2], UpVector[0], UpVector[1], UpVector[2]};
		alListenerfv(AL_ORIENTATION, orient);
	}
	void cListener::setVelocity(const cVector3& vel)
	{
		cAudioMutexBasicLock lock(Mutex);
		Velocity = vel;
		alListener3f(AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z);
	}
	void cListener::setMasterVolume(const float& volume)
	{
		cAudioMutexBasicLock lock(Mutex);
		MasterGain = volume;
		alListenerf(AL_GAIN, MasterGain);
	}
	void cListener::move(const cVector3& pos)
	{
		cAudioMutexBasicLock lock(Mutex);
		Velocity = pos - Position;
		Position = pos;
		
		alListener3f(AL_POSITION, Position.x, Position.y, Position.z);
		alListener3f(AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z);
	}
#ifdef CAUDIO_EFX_ENABLED
		void cListener::setMetersPerUnit(const float& meters)
		{
			cAudioMutexBasicLock lock(Mutex);
			alListenerf(AL_METERS_PER_UNIT, meters);
		}

		float cListener::getMetersPerUnit(void) const
		{
			float value = 1.0f;
			alGetListenerf(AL_METERS_PER_UNIT, &value);
			return value;
		}
#endif
};