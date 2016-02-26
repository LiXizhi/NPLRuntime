#pragma once
#include "PEtypes.h"

namespace ParaEngine
{
	/** Enumeration of audio formats supported by the engine. */
	enum ParaAudioFormats
	{
		EAF_8BIT_MONO, 
		EAF_8BIT_STEREO,
		EAF_16BIT_MONO,
		EAF_16BIT_STEREO
	};

	/** Audio flags */
	enum ParaAudioFlagsEnum
	{
		AUDIO_FLAG_Uninitialized = 0,
		AUDIO_FLAG_STOP_RELEASE,
		AUDIO_FLAG_STOP_IMMEDIATE,
	};
	/** Audio flags */
	enum ParaAudioDistanceModelEnum
	{
		Audio_DistModel_NONE = 0, 
		Audio_DistModel_INVERSE_DISTANCE, 
		Audio_DistModel_INVERSE_DISTANCE_CLAMPED, 
		Audio_DistModel_LINEAR_DISTANCE, 
		Audio_DistModel_LINEAR_DISTANCE_CLAMPED, 
		Audio_DistModel_EXPONENT_DISTANCE, 
		Audio_DistModel_EXPONENT_DISTANCE_CLAMPED,
	};

	/** Interface for event handlers on Audio Sources. */
	class IAudioSourceEventHandler
	{
	public:
		/// This function is called when a source updates its buffers.
		virtual void onUpdate() = 0;

		/// This function is called when a source is released and soon to be deleted.
		virtual void onRelease() = 0;

		/// This function is called when a source starts playing.
		virtual void onPlay() = 0;

		/// This function is called when a source stopped playback.
		virtual void onStop() = 0;

		/// This function is called when a source is paused.
		virtual void onPause() = 0;
	};

	/** an audio source */
	class IParaAudioSource
	{
	public:
		/** Plays the source with the last set parameters.
		* @return True if the source is playing, false if not. */
		virtual bool play() = 0;

		/** Plays the source in 2D mode.
		No automatic attenuation or panning will take place in this mode, but using setPosition will allow you to manually pan mono audio streams.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@return True if the source is playing, false if not. */
		virtual bool play2d(const bool& toLoop = false) = 0;

		/** Plays the source in 3D mode.
		@param position: Position to start the sound off at.
		@param soundstr: Affects how the source attenuates due to distance.  Higher values cause the source to stand out more over distance.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@return True if the source is playing, false if not. */
		virtual bool play3d(const PARAVECTOR3& position, const float& soundstr = 1.0 , const bool& toLoop = false) = 0;
		
		/// Pauses playback of the sound source.
		virtual void pause() = 0;

		/// Stops playback of the sound source.    
		virtual void stop() = 0;

		/// Controls whether the source should loop or not.
		/** @param toLoop: Whether to loop (restart) the audio when the end is reached. */
		virtual void loop(const bool& toLoop) = 0;

		/// Seeks through the audio stream to a specific spot.
		/** Note: May not be supported by all codecs.
		@param seconds: Number of seconds to seek.
		@param relative: Whether to seek from the current position or the start of the stream.
		@return True on success, False if the codec does not support seeking. */
		virtual bool seek(const float& seconds, bool relative = false) = 0;

		/// Returns the total amount of time in the audio stream.  See IAudioDecoder for details.
		virtual float getTotalAudioTime() = 0;

		/// Returns the total decoded size of the audio stream.  See IAudioDecoder for details.
		virtual int getTotalAudioSize() = 0;

		/// Returns the original size of the audio stream.  See IAudioDecoder for details.
		virtual int getCompressedAudioSize() = 0;

		/// Returns the current position in the audio stream in seconds.  See IAudioDecoder for details.
		virtual float getCurrentAudioTime() = 0;

		/// Returns the current position in the decoded audio stream in bytes.  See IAudioDecoder for details.
		virtual int getCurrentAudioPosition() = 0;

		/// Returns the current position in the original audio stream in bytes.  See IAudioDecoder for details.
		virtual int getCurrentCompressedAudioPosition() = 0;

		/// Normally called every frame by the audio manager to update the internal buffers.  Note: For internal use only.
		virtual bool update() = 0;

		/// Releases all resources used by the audio source, normally used to clean up before deletion.  Note: For internal use only.
		virtual void release() = 0;

		/// Returns if the source is ready to be used.
		virtual const bool isValid() const = 0;

		/// Returns if the source is playing.
		virtual const bool isPlaying() const = 0;

		/// Returns if the source is paused.
		virtual const bool isPaused() const = 0;

		/// Returns if the source is stopped.
		virtual const bool isStopped() const = 0;

		/// Returns if the source is looping.
		virtual const bool isLooping() const = 0;

		/// Sets the position of the source in 3D space.
		/**
		@param position: A 3D vector giving the new location to put this source. */
		virtual void setPosition(const PARAVECTOR3& position) = 0;

		/// Sets the current velocity of the source for doppler effects.
		/**
		@param velocity: A 3D vector giving the speed and direction that the source is moving. */
		virtual void setVelocity(const PARAVECTOR3& velocity) = 0;

		/// Sets the direction the source is facing.
		/**
		@param direction: A 3D vector giving the direction that the source is aiming. */
		virtual void setDirection(const PARAVECTOR3& direction) = 0;

		/// Sets the factor used in attenuating the source over distance.
		/** Larger values make it attenuate faster, smaller values make the source carry better.
		Range: 0.0f to +inf (Default: 1.0f).
		@param rolloff: The rolloff factor to apply to the attenuation calculation. */
		virtual void setRolloffFactor(const float& rolloff) = 0;

		/// Sets how well the source carries over distance.
		/** Same as setRolloffFactor(1.0f/soundstrength).
		Range: 0.0f to +inf (Default: 1.0f).
		@param soundstrength: How well the sound carries over distance. */
		virtual void setStrength(const float& soundstrength) = 0;

		/// Sets the distance from the source where attenuation will begin.
		/** Range: 0.0f to +inf
		@param minDistance: Distance from the source where attenuation begins. */
		virtual void setMinDistance(const float& minDistance) = 0;

		/// Sets the distance from the source where attenuation will stop.
		/** Range: 0.0f to +inf
		@param maxDistance: Distance where attenuation will cease.  Normally the farthest range you can hear the source. */
		virtual void setMaxDistance(const float& maxDistance) = 0;

		/// Sets the pitch of the source.
		/** Range: 0.0f to +inf (Default: 1.0f)
		@param pitch: New pitch level.  Note that higher values will speed up the playback of the sound. */
		virtual void setPitch(const float& pitch) = 0;

		/// Sets the source volume before attenuation and other effects.
		/** Range: 0.0f to +inf (Default: 1.0f).
		@param volume: New volume of the source. */
		virtual void setVolume(const float& volume) = 0;

		/// Sets the minimum volume that the source can be attenuated to.
		/** Range: 0.0f to +inf (Default: 0.0f).
		@param minVolume: New minimum volume of the source. */
		virtual void setMinVolume(const float& minVolume) = 0;

		/// Sets the maximum volume that the source can achieve.
		/** Range: 0.0f to +inf (Default: 1.0f).
		@param maxVolume: New maximum volume of the source. */
		virtual void setMaxVolume(const float& maxVolume) = 0;

		/// Sets the angle of the inner sound cone of the source.  The cone opens up in the direction of the source as set by setDirection(). 
		/** Note: This causes the sound to be loudest only if the listener is inside this cone.
		Range: 0.0f to 360.0f (Default: 360.0f).
		@param innerAngle: Inside angle of the cone. */
		virtual void setInnerConeAngle(const float& innerAngle) = 0;

		/// Sets the angle of the outer sound cone of the source. The cone opens up in the direction of the source as set by setDirection(). 
		/** Note: If the listener is outside of this cone, the sound cannot be heard.  Between the inner cone angle and this angle, the sound volume will fall off.
		Range: 0.0f to 360.0f (Default: 360.0f).
		@param outerAngle: Outside angle of the cone. */
		virtual void setOuterConeAngle(const float& outerAngle) = 0;

		/// Sets how much the volume of the source is scaled in the outer cone.
		/** Range: 0.0f to +inf (Default: 0.0f).
		@param outerVolume: Volume of the source in the outside cone. */
		virtual void setOuterConeVolume(const float& outerVolume) = 0;

		/// Sets the doppler strength, which enhances or diminishes the doppler effect.  Can be used to exaggerate the doppler for a special effect.
		/** Range: 0.0f to +inf (Default: 1.0f).
		@param dstrength: New strength for the doppler effect. */
		virtual void setDopplerStrength(const float& dstrength) = 0; 

		/// Overrides the doppler velocity vector.  It is usually better to let the engine take care of it automatically.
		/** Note: must be set every time you set the position, velocity, or direction.
		@param dvelocity: New doppler vector for the source. */
		virtual void setDopplerVelocity(const PARAVECTOR3& dvelocity) = 0;

		/// Convenience function to automatically set the velocity and position for you in a single call.
		/** Velocity will be set to new position - last position.
		@param position: Position to move the source to. */
		virtual void move(const PARAVECTOR3& position) = 0;

		/// Returns the audio objects position
		virtual const PARAVECTOR3 getPosition() const = 0;

		/// Returns the audio objects velocity
		virtual const PARAVECTOR3 getVelocity() const = 0;

		/// Returns the audio objects direction
		virtual const PARAVECTOR3 getDirection() const = 0;

		/// Returns the factor used in attenuating the source over distance
		virtual const float getRolloffFactor() const = 0;

		/// Returns the strength of the source
		virtual const float getStrength() const = 0;

		/// Returns the distance from the source where attenuation will begin
		virtual const float getMinDistance() const = 0;

		/// Returns the distance from the source where attenuation will stop
		virtual const float getMaxDistance() const = 0;

		/// Returns the pitch of the source
		virtual const float getPitch() const = 0;

		/// Returns the source volume before attenuation and other effects
		virtual const float getVolume() const = 0;

		/// Returns the minimum volume that the source can be attenuated to
		virtual const float getMinVolume() const = 0;

		/// Returns the maximum volume that the source can achieve
		virtual const float getMaxVolume() const = 0;

		/// Returns the angle of the inner sound cone of the source
		virtual const float getInnerConeAngle() const = 0;

		/// Returns the angle of the outer sound cone of the source
		virtual const float getOuterConeAngle() const = 0;

		/// Returns how much the volume of the source is scaled in the outer cone
		virtual const float getOuterConeVolume() const = 0;

		/// Returns the doppler strength, which enhances or diminishes the doppler effect 
		virtual const float getDopplerStrength() const = 0;

		/// Returns the override for the doppler velocity vector
		virtual const PARAVECTOR3 getDopplerVelocity() const = 0;

		/** Registers a new event handler to this source
		* @param handler: Pointer to the event handler to register. */
		virtual void registerEventHandler(IAudioSourceEventHandler* handler) = 0;

		/// Removes all event handlers attached to this source
		virtual void unRegisterAllEventHandlers() = 0;
	};

	/** ParaAudioEngine core interface. 
	*/
	class IParaAudioEngine
	{
	public:
		virtual void Release() = 0;

		/// Initializes the manager.
		/**
		@param deviceName: Name of the device to create this manager for.
		@param outputFrequency: Frequency of the output audio or -1 for the device default.
		@param eaxEffectSlots: Number of EFX effect slots to request.  Has no effect if EFX is not supported or compiled out.
		@return True on success, False if initialization of OpenAL failed. */
		virtual bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4) = 0;

		/// Shuts the manager down, cleaning up audio sources in the process.  Does not clean up decoders, data sources, or event handlers.
		virtual void shutDown() = 0;

		/// If threading is disabled, you must call this function every frame to update the playback buffers of audio sources.  Otherwise it should not be called.
		virtual void update() = 0;

		/// Returns an Audio Source by its "name" and NULL if the name is not found
		/**
		@param name: Name of the audio source to retrieve.
		@return Pointer to the audio source object or NULL if it could not be found.*/
		virtual IParaAudioSource* getSoundByName(const char* name) = 0;

		/// Releases ALL Audio Sources (but does not shutdown the manager)
		virtual void releaseAllSources() = 0;

		/// Releases a single Audio Source, removing it from the manager.
		/** @param source: Pointer to the source to release. */
		virtual void release(IParaAudioSource* source) = 0;

		/// Returns the name of an available playback device.
		/** @param index: Specify which name to retrieve ( Range: 0 to getAvailableDeviceCount()-1 ) 
		@return Name of the selected device. */
		virtual const char* getAvailableDeviceName(unsigned int index) = 0;
		
		/// Returns the number of playback devices available for use.
		/** @return Number of playback devices available. */
		virtual unsigned int getAvailableDeviceCount() = 0;
		
		/// Returns the name of the default system playback device.
		/** @return Name of the default playback device. */
		virtual const char* getDefaultDeviceName() = 0;

		/// Creates an Audio Source object using the highest priority data source that has the referenced filename
		/**
		@param name: Name of the audio source.
		@param filename: Path to the file to load audio data from.
		@param stream: Whether to stream the audio data or load it all into a memory buffer at the start.  You should consider using streaming for really large sound files.
		@return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IParaAudioSource* create(const char* name, const char* filename, bool stream = false) = 0;
		
		/// Creates an Audio Source from a memory buffer using a specific audio codec.
		/**
		@param name: Name of the audio source.
		@param data: Pointer to a buffer in memory to load the data from.
		@param length: Length of the data buffer.
		@param extension: Extension for the audio codec of the data in the memory buffer.
		@return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IParaAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension) = 0;
		
		/// Creates an Audio Source from raw audio data in a memory buffer.
		/**
		@param name: Name of the audio source.
		@param data: Pointer to a buffer in memory to load the data from.
		@param length: Length of the data buffer.
		@param frequency: Frequency (or sample rate) of the audio data.
		@param format: Format of the audio data.
		@return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IParaAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, ParaAudioFormats format) = 0;

	public:
		/**
		* set the audio distance model. 
		* see: http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.htm
		*/
		virtual void SetDistanceModel(ParaAudioDistanceModelEnum eDistModel) = 0;

		//////////////////////////////////////////////////////////////////////////
		/// Interface for the listener in cAudio.  This class provides abilities to move and orient where your camera or user is in the audio world.
		//////////////////////////////////////////////////////////////////////////

		/** Sets the position of the listener.
		Note that you will still have to set velocity after this call for proper doppler effects.
		Use move() if you'd like to have cAudio automatically handle velocity for you.
		@param pos: New position for the listener. */
		virtual void setPosition(const PARAVECTOR3& pos) = 0;

		/** Sets the direction the listener is facing
		@param dir: New direction vector for the listener. */
		virtual void setDirection(const PARAVECTOR3& dir) = 0;

		/** Sets the up vector to use for the listener
		Default up vector is Y+, same as OpenGL.
		@param up: New up vector for the listener. */
		virtual void setUpVector(const PARAVECTOR3& up) = 0;

		/** Sets the current velocity of the listener for doppler effects
		@param vel: New velocity for the listener. */
		virtual void setVelocity(const PARAVECTOR3& vel) = 0;

		/** Sets the global volume modifier (will effect all sources)
		@param volume: Volume to scale all sources by. Range: 0.0 to +inf. */
		virtual void setMasterVolume(const float& volume) = 0;

		/// Convenience function to automatically set the velocity and position for you in a single call
		/** Velocity will be set to new position - last position 
		@param pos: New position to move the listener to. */
		virtual void move(const PARAVECTOR3& pos) = 0;

		/// Returns the current position of the listener
		virtual PARAVECTOR3 getPosition(void) const = 0;

		/// Returns the current direction of the listener
		virtual PARAVECTOR3 getDirection(void) const = 0;

		/// Returns the current up vector of the listener
		virtual PARAVECTOR3 getUpVector(void) const = 0;

		/// Returns the current velocity of the listener
		virtual PARAVECTOR3 getVelocity(void) const = 0;

		/// Returns the global volume modifier for all sources
		virtual float getMasterVolume(void) const = 0;

		/** 
		* set the default coordinate system used. 
		* @param nLeftHand: 0 if it is left handed coordinate system, which is the one used by DirectX.
		* and 1, if it is the right handed coordinate system which is OpenAL(and OpenGL) uses.
		*/
		virtual void SetCoordinateSystem(int nLeftHand) = 0;
	};
}