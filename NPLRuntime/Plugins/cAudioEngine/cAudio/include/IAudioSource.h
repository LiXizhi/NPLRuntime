// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"
#include "IRefCounted.h"
#include "IAudioDecoder.h"
#include "cVector3.h"
#include "IEffect.h"
#include "IFilter.h"
#include "ISourceEventHandler.h"

namespace cAudio
{
	//! Interface for a single audio source, which allow you to manipulate sound sources (speakers) in 2D or 3D space.
    class IAudioSource : public IRefCounted
    {
    public:
		IAudioSource() {}
		virtual ~IAudioSource() {}

		//! Plays the source with the last set parameters.
		/**
		\return True if the source is playing, false if not. */
		virtual bool play() = 0;

		//! Plays the source in 2D mode.
		/** No automatic attenuation or panning will take place in this mode, but using setPosition will allow you to manually pan mono audio streams.
		\param toLoop: Whether to loop (restart) the audio when the end is reached.
		\return True if the source is playing, false if not. */
		virtual bool play2d(const bool& toLoop = false) = 0;

		//! Plays the source in 3D mode.
		/**
		\param position: Position to start the sound off at.
		\param soundstr: Affects how the source attenuates due to distance.  Higher values cause the source to stand out more over distance.
		\param toLoop: Whether to loop (restart) the audio when the end is reached.
		\return True if the source is playing, false if not. */
		virtual bool play3d(const cVector3& position, const float& soundstr = 1.0 , const bool& toLoop = false) = 0;
		
		//! Pauses playback of the sound source.
		virtual void pause() = 0;

		//! Stops playback of the sound source.    
		virtual void stop() = 0;

		//! Controls whether the source should loop or not.
		/** \param toLoop: Whether to loop (restart) the audio when the end is reached. */
		virtual void loop(const bool& toLoop) = 0;

		//! Seeks through the audio stream to a specific spot.
		/** Note: May not be supported by all codecs.
		\param seconds: Number of seconds to seek.
		\param relative: Whether to seek from the current position or the start of the stream.
		\return True on success, False if the codec does not support seeking. */
		virtual bool seek(const float& seconds, bool relative = false) = 0;

		//! Returns the total amount of time in the audio stream.  See IAudioDecoder for details.
		virtual float getTotalAudioTime() = 0;

		//! Returns the total decoded size of the audio stream.  See IAudioDecoder for details.
		virtual int getTotalAudioSize() = 0;

		//! Returns the original size of the audio stream.  See IAudioDecoder for details.
		virtual int getCompressedAudioSize() = 0;

		//! Returns the current position in the audio stream in seconds.  See IAudioDecoder for details.
		virtual float getCurrentAudioTime() = 0;

		//! Returns the current position in the decoded audio stream in bytes.  See IAudioDecoder for details.
		virtual int getCurrentAudioPosition() = 0;

		//! Returns the current position in the original audio stream in bytes.  See IAudioDecoder for details.
		virtual int getCurrentCompressedAudioPosition() = 0;

		//! Normally called every frame by the audio manager to update the internal buffers.  Note: For internal use only.
		virtual bool update() = 0;

		//! Returns if the source is ready to be used.
		virtual const bool isValid() const = 0;

		//! Returns if the source is playing.
		virtual const bool isPlaying() const = 0;

		//! Returns if the source is paused.
		virtual const bool isPaused() const = 0;

		//! Returns if the source is stopped.
		virtual const bool isStopped() const = 0;

		//! Returns if the source is looping.
		virtual const bool isLooping() const = 0;

		//! Sets the position of the source in 3D space.
		/**
		\param position: A 3D vector giving the new location to put this source. */
		virtual void setPosition(const cVector3& position) = 0;

		//! Sets the current velocity of the source for doppler effects.
		/**
		\param velocity: A 3D vector giving the speed and direction that the source is moving. */
		virtual void setVelocity(const cVector3& velocity) = 0;

		//! Sets the direction the source is facing.
		/**
		\param direction: A 3D vector giving the direction that the source is aiming. */
		virtual void setDirection(const cVector3& direction) = 0;

		//! Sets the factor used in attenuating the source over distance.
		/** Larger values make it attenuate faster, smaller values make the source carry better.
		Range: 0.0f to +inf (Default: 1.0f).
		\param rolloff: The rolloff factor to apply to the attenuation calculation. */
		virtual void setRolloffFactor(const float& rolloff) = 0;

		//! Sets how well the source carries over distance.
		/** Same as setRolloffFactor(1.0f/soundstrength).
		Range: 0.0f to +inf (Default: 1.0f).
		\param soundstrength: How well the sound carries over distance. */
		virtual void setStrength(const float& soundstrength) = 0;

		//! Sets the distance from the source where attenuation will begin.
		/** Range: 0.0f to +inf
		\param minDistance: Distance from the source where attenuation begins. */
		virtual void setMinDistance(const float& minDistance) = 0;

		//! Sets the distance from the source where attenuation will stop.
		/** Range: 0.0f to +inf
		\param maxDistance: Distance where attenuation will cease.  Normally the farthest range you can hear the source. */
		virtual void setMaxAttenuationDistance(const float& maxDistance) = 0;

		//! Sets the pitch of the source.
		/** Range: 0.0f to +inf (Default: 1.0f)
		\param pitch: New pitch level.  Note that higher values will speed up the playback of the sound. */
		virtual void setPitch(const float& pitch) = 0;

		//! Sets the source volume before attenuation and other effects.
		/** Range: 0.0f to +inf (Default: 1.0f).
		\param volume: New volume of the source. */
		virtual void setVolume(const float& volume) = 0;

		//! Sets the minimum volume that the source can be attenuated to.
		/** Range: 0.0f to +inf (Default: 0.0f).
		\param minVolume: New minimum volume of the source. */
		virtual void setMinVolume(const float& minVolume) = 0;

		//! Sets the maximum volume that the source can achieve.
		/** Range: 0.0f to +inf (Default: 1.0f).
		\param maxVolume: New maximum volume of the source. */
		virtual void setMaxVolume(const float& maxVolume) = 0;

		//! Sets the angle of the inner sound cone of the source.  The cone opens up in the direction of the source as set by setDirection(). 
		/** Note: This causes the sound to be loudest only if the listener is inside this cone.
		Range: 0.0f to 360.0f (Default: 360.0f).
		\param innerAngle: Inside angle of the cone. */
		virtual void setInnerConeAngle(const float& innerAngle) = 0;

		//! Sets the angle of the outer sound cone of the source. The cone opens up in the direction of the source as set by setDirection(). 
		/** Note: If the listener is outside of this cone, the sound cannot be heard.  Between the inner cone angle and this angle, the sound volume will fall off.
		Range: 0.0f to 360.0f (Default: 360.0f).
		\param outerAngle: Outside angle of the cone. */
		virtual void setOuterConeAngle(const float& outerAngle) = 0;

		//! Sets how much the volume of the source is scaled in the outer cone.
		/** Range: 0.0f to +inf (Default: 0.0f).
		\param outerVolume: Volume of the source in the outside cone. */
		virtual void setOuterConeVolume(const float& outerVolume) = 0;

		//! Sets the doppler strength, which enhances or diminishes the doppler effect.  Can be used to exaggerate the doppler for a special effect.
		/** Range: 0.0f to +inf (Default: 1.0f).
		\param dstrength: New strength for the doppler effect. */
		virtual void setDopplerStrength(const float& dstrength) = 0; 

		//! Overrides the doppler velocity vector.  It is usually better to let the engine take care of it automatically.
		/** Note: must be set every time you set the position, velocity, or direction.
		\param dvelocity: New doppler vector for the source. */
		virtual void setDopplerVelocity(const cVector3& dvelocity) = 0;

		//! Convenience function to automatically set the velocity and position for you in a single call.
		/** Velocity will be set to new position - last position.
		\param position: Position to move the source to. */
		virtual void move(const cVector3& position) = 0;

		//! Returns the audio objects position
		virtual const cVector3 getPosition() const = 0;

		//! Returns the audio objects velocity
		virtual const cVector3 getVelocity() const = 0;

		//! Returns the audio objects direction
		virtual const cVector3 getDirection() const = 0;

		//! Returns the factor used in attenuating the source over distance
		virtual const float getRolloffFactor() const = 0;

		//! Returns the strength of the source
		virtual const float getStrength() const = 0;

		//! Returns the distance from the source where attenuation will begin
		virtual const float getMinDistance() const = 0;

		//! Returns the distance from the source where attenuation will stop
		virtual const float getMaxDistance() const = 0;

		//! Returns the pitch of the source
		virtual const float getPitch() const = 0;

		//! Returns the source volume before attenuation and other effects
		virtual const float getVolume() const = 0;

		//! Returns the minimum volume that the source can be attenuated to
		virtual const float getMinVolume() const = 0;

		//! Returns the maximum volume that the source can achieve
		virtual const float getMaxVolume() const = 0;

		//! Returns the angle of the inner sound cone of the source
		virtual const float getInnerConeAngle() const = 0;

		//! Returns the angle of the outer sound cone of the source
		virtual const float getOuterConeAngle() const = 0;

		//! Returns how much the volume of the source is scaled in the outer cone
		virtual const float getOuterConeVolume() const = 0;

		//! Returns the doppler strength, which enhances or diminishes the doppler effect 
		virtual const float getDopplerStrength() const = 0;

		//! Returns the override for the doppler velocity vector
		virtual const cVector3 getDopplerVelocity() const = 0;

		//! Registers a new event handler to this source
		/**
		\param handler: Pointer to the event handler to register. */
		virtual void registerEventHandler(ISourceEventHandler* handler) = 0;

		//! Removes a specified event handler from this source
		/**
		\param handler: Pointer to the event handler to remove. */
		virtual void unRegisterEventHandler(ISourceEventHandler* handler) = 0;

		//! Removes all event handlers attached to this source
		virtual void unRegisterAllEventHandlers() = 0;

#if CAUDIO_EFX_ENABLED == 1
		//! Returns the number of effects at one time this source can support.
		virtual unsigned int getNumEffectSlotsAvailable() const = 0;

		//! Attaches an EFX audio effect to this sound source to a specific slot.
		/** 
		\param slot: Slot to attach this effect to. Range: 0 to getNumEffectSlotsAvailable().
		\param effect: Pointer to an effect object to attach. 
		\return Whether the effect was successfully attached. */
		virtual bool attachEffect(unsigned int slot, IEffect* effect) = 0;

		//! Removes an EFX audio effect from this sound source.
		/** 
		\param slot: Slot of the effect to remove. Range: 0 to getNumEffectSlotsAvailable().*/
		virtual void removeEffect(unsigned int slot) = 0;

		//! Attaches an audio filter to this sound source that will operate on the direct feed, separate from any effects.
		/**
		\param filter: Pointer to the filter to attach.  Any previous filter will be dropped. 
		\return Whether the filter was successfully attached. */
		virtual bool attachFilter(IFilter* filter) = 0;

		//! Removes a previously attached filter.
		virtual void removeFilter() = 0;
#endif

    protected:
    private:
    };
};