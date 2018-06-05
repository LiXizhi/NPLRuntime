// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IListener.h"
#include "cMutex.h"

namespace cAudio
{
	class cListener : public IListener
    {
    public:
		cListener() :	Direction(cVector3(0.f, 0.f, -1.f)),
						UpVector(cVector3(0.f, 1.f, 0.f)),
						MasterGain(1.f) {}
		virtual ~cListener() {}

		virtual void setPosition(const cVector3& pos);
		virtual void setDirection(const cVector3& dir);
		virtual void setUpVector(const cVector3& up);
		virtual void setVelocity(const cVector3& vel);
		virtual void setMasterVolume(const float& volume);
		virtual void move(const cVector3& pos);

		virtual cVector3 getPosition(void) const { return Position; }
		virtual cVector3 getDirection(void) const { return Direction; }
		virtual cVector3 getUpVector(void) const { return UpVector; }
		virtual cVector3 getVelocity(void) const { return Velocity; }
		virtual float getMasterVolume(void) const { return MasterGain; }

#if CAUDIO_EFX_ENABLED == 1
		virtual void setMetersPerUnit(const float& meters);
		virtual float getMetersPerUnit(void) const;
#endif

	protected:
		cAudioMutex Mutex;

		cVector3 Position;
		cVector3 Direction;
		cVector3 UpVector;
		cVector3 Velocity;
		float MasterGain;
	private:
    };
}
