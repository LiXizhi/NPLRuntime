// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cVector3.h"
#include "cAudioDefines.h"

namespace cAudio
{
	//! Interface for the listener in cAudio.  This class provides abilities to move and orient where your camera or user is in the audio world.
    class IListener
    {
    public:
		IListener() {}
        virtual ~IListener() {}

		//! Sets the position of the listener.
		/** Note that you will still have to set velocity after this call for proper doppler effects.
		Use move() if you'd like to have cAudio automatically handle velocity for you. */
		/**
		\param pos: New position for the listener. */
		virtual void setPosition(const cVector3& pos) = 0;

		//! Sets the direction the listener is facing
		/**
		\param dir: New direction vector for the listener. */
		virtual void setDirection(const cVector3& dir) = 0;

		//! Sets the up vector to use for the listener
		/** Default up vector is Y+, same as OpenGL.
		\param up: New up vector for the listener. */
		virtual void setUpVector(const cVector3& up) = 0;

		//! Sets the current velocity of the listener for doppler effects
		/**
		\param vel: New velocity for the listener. */
		virtual void setVelocity(const cVector3& vel) = 0;

		//! Sets the global volume modifier (will effect all sources)
		/**
		\param volume: Volume to scale all sources by. Range: 0.0 to +inf. */
		virtual void setMasterVolume(const float& volume) = 0;

		//! Convenience function to automatically set the velocity and position for you in a single call
		/** Velocity will be set to new position - last position 
		\param pos: New position to move the listener to. */
		virtual void move(const cVector3& pos) = 0;

		//! Returns the current position of the listener
		virtual cVector3 getPosition(void) const = 0;

		//! Returns the current direction of the listener
		virtual cVector3 getDirection(void) const = 0;

		//! Returns the current up vector of the listener
		virtual cVector3 getUpVector(void) const = 0;

		//! Returns the current velocity of the listener
		virtual cVector3 getVelocity(void) const = 0;

		//! Returns the global volume modifier for all sources
		virtual float getMasterVolume(void) const = 0;

#if CAUDIO_EFX_ENABLED == 1
		//! Sets the meters per user world unit for use with sound effects
		/**
		\param meters: Number of meters per world unit in your simulation. */
		virtual void setMetersPerUnit(const float& meters) = 0;

		//! Returns the meters per user world unit for use with sound effects
		virtual float getMetersPerUnit(void) const = 0;
#endif
    };
}

