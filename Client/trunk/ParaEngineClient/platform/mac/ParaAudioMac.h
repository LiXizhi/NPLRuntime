#ifndef AUDIO_ENGINE_MAC_H
#define AUDIO_ENGINE_MAC_H

#ifdef WIN32
#include "ParaEngine.h"
#else
#include "IParaAudioEngine.h"
#endif


namespace ParaEngine
{

    /** Interface for event handlers on Audio Sources. */
	class MacAudioSourceEventHandler : public IAudioSourceEventHandler
	{
	public:
		/// This function is called when a source updates its buffers.
		void onUpdate() ;

		/// This function is called when a source is released and soon to be deleted.
		void onRelease() ;

		/// This function is called when a source starts playing.
		void onPlay() ;

		/// This function is called when a source stopped playback.
		void onStop() ;

		/// This function is called when a source is paused.
		void onPause() ;
	};

	/** an audio source */
	class MacParaAudioSource : public IParaAudioSource
	{
	public:
		/** Plays the source with the last set parameters.
		* @return True if the source is playing, false if not. */
		bool play() ;

		/** Plays the source in 2D mode.
		No automatic attenuation or panning will take place in this mode, but using setPosition will allow you to manually pan mono audio streams.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@return True if the source is playing, false if not. */
		bool play2d(const bool& toLoop = false) ;

		/** Plays the source in 3D mode.
		@param position: Position to start the sound off at.
		@param soundstr: Affects how the source attenuates due to distance.  Higher values cause the source to stand out more over distance.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@return True if the source is playing, false if not. */
		bool play3d(const PARAVECTOR3& position, const float& soundstr = 1.0 , const bool& toLoop = false) ;

		/// Pauses playback of the sound source.
		void pause() ;

		/// Stops playback of the sound source.
		void stop() ;

		/// Controls whether the source should loop or not.
		/** @param toLoop: Whether to loop (restart) the audio when the end is reached. */
		void loop(const bool& toLoop) ;

		/// Seeks through the audio stream to a specific spot.
		/** Note: May not be supported by all codecs.
		@param seconds: Number of seconds to seek.
		@param relative: Whether to seek from the current position or the start of the stream.
		@return True on success, False if the codec does not support seeking. */
		bool seek(const float& seconds, bool relative = false) ;

		/// Returns the total amount of time in the audio stream.  See IAudioDecoder for details.
		float getTotalAudioTime() ;

		/// Returns the total decoded size of the audio stream.  See IAudioDecoder for details.
		int getTotalAudioSize() ;

		/// Returns the original size of the audio stream.  See IAudioDecoder for details.
		int getCompressedAudioSize() ;

		/// Returns the current position in the audio stream in seconds.  See IAudioDecoder for details.
		float getCurrentAudioTime() ;

		/// Returns the current position in the decoded audio stream in bytes.  See IAudioDecoder for details.
		int getCurrentAudioPosition() ;

		/// Returns the current position in the original audio stream in bytes.  See IAudioDecoder for details.
		int getCurrentCompressedAudioPosition() ;

		/// Normally called every frame by the audio manager to update the internal buffers.  Note: For internal use only.
		bool update() ;

		/// Releases all resources used by the audio source, normally used to clean up before deletion.  Note: For internal use only.
		void release() ;

		/// Returns if the source is ready to be used.
		const bool isValid() const ;

		/// Returns if the source is playing.
		const bool isPlaying() const ;

		/// Returns if the source is paused.
		const bool isPaused() const ;

		/// Returns if the source is stopped.
		const bool isStopped() const ;

		/// Returns if the source is looping.
		const bool isLooping() const ;

		/// Sets the position of the source in 3D space.
		/**
		@param position: A 3D vector giving the new location to put this source. */
		void setPosition(const PARAVECTOR3& position) ;

		/// Sets the current velocity of the source for doppler effects.
		/**
		@param velocity: A 3D vector giving the speed and direction that the source is moving. */
		void setVelocity(const PARAVECTOR3& velocity) ;

		/// Sets the direction the source is facing.
		/**
		@param direction: A 3D vector giving the direction that the source is aiming. */
		void setDirection(const PARAVECTOR3& direction) ;

		/// Sets the factor used in attenuating the source over distance.
		/** Larger values make it attenuate faster, smaller values make the source carry better.
		Range: 0.0f to +inf (Default: 1.0f).
		@param rolloff: The rolloff factor to apply to the attenuation calculation. */
		void setRolloffFactor(const float& rolloff) ;

		/// Sets how well the source carries over distance.
		/** Same as setRolloffFactor(1.0f/soundstrength).
		Range: 0.0f to +inf (Default: 1.0f).
		@param soundstrength: How well the sound carries over distance. */
		void setStrength(const float& soundstrength) ;

		/// Sets the distance from the source where attenuation will begin.
		/** Range: 0.0f to +inf
		@param minDistance: Distance from the source where attenuation begins. */
		void setMinDistance(const float& minDistance) ;

		/// Sets the distance from the source where attenuation will stop.
		/** Range: 0.0f to +inf
		@param maxDistance: Distance where attenuation will cease.  Normally the farthest range you can hear the source. */
		void setMaxDistance(const float& maxDistance) ;

		/// Sets the pitch of the source.
		/** Range: 0.0f to +inf (Default: 1.0f)
		@param pitch: New pitch level.  Note that higher values will speed up the playback of the sound. */
		void setPitch(const float& pitch) ;

		/// Sets the source volume before attenuation and other effects.
		/** Range: 0.0f to +inf (Default: 1.0f).
		@param volume: New volume of the source. */
		void setVolume(const float& volume) ;

		/// Sets the minimum volume that the source can be attenuated to.
		/** Range: 0.0f to +inf (Default: 0.0f).
		@param minVolume: New minimum volume of the source. */
		void setMinVolume(const float& minVolume) ;

		/// Sets the maximum volume that the source can achieve.
		/** Range: 0.0f to +inf (Default: 1.0f).
		@param maxVolume: New maximum volume of the source. */
		void setMaxVolume(const float& maxVolume) ;

		/// Sets the angle of the inner sound cone of the source.  The cone opens up in the direction of the source as set by setDirection().
		/** Note: This causes the sound to be loudest only if the listener is inside this cone.
		Range: 0.0f to 360.0f (Default: 360.0f).
		@param innerAngle: Inside angle of the cone. */
		void setInnerConeAngle(const float& innerAngle) ;

		/// Sets the angle of the outer sound cone of the source. The cone opens up in the direction of the source as set by setDirection().
		/** Note: If the listener is outside of this cone, the sound cannot be heard.  Between the inner cone angle and this angle, the sound volume will fall off.
		Range: 0.0f to 360.0f (Default: 360.0f).
		@param outerAngle: Outside angle of the cone. */
		void setOuterConeAngle(const float& outerAngle) ;

		/// Sets how much the volume of the source is scaled in the outer cone.
		/** Range: 0.0f to +inf (Default: 0.0f).
		@param outerVolume: Volume of the source in the outside cone. */
		void setOuterConeVolume(const float& outerVolume) ;

		/// Sets the doppler strength, which enhances or diminishes the doppler effect.  Can be used to exaggerate the doppler for a special effect.
		/** Range: 0.0f to +inf (Default: 1.0f).
		@param dstrength: New strength for the doppler effect. */
		void setDopplerStrength(const float& dstrength) ;

		/// Overrides the doppler velocity vector.  It is usually better to let the engine take care of it automatically.
		/** Note: must be set every time you set the position, velocity, or direction.
		@param dvelocity: New doppler vector for the source. */
		void setDopplerVelocity(const PARAVECTOR3& dvelocity) ;

		/// Convenience function to automatically set the velocity and position for you in a single call.
		/** Velocity will be set to new position - last position.
		@param position: Position to move the source to. */
		void move(const PARAVECTOR3& position) ;

		/// Returns the audio objects position
		const PARAVECTOR3 getPosition() const ;

		/// Returns the audio objects velocity
		const PARAVECTOR3 getVelocity() const ;

		/// Returns the audio objects direction
		const PARAVECTOR3 getDirection() const ;

		/// Returns the factor used in attenuating the source over distance
		const float getRolloffFactor() const ;

		/// Returns the strength of the source
		const float getStrength() const ;

		/// Returns the distance from the source where attenuation will begin
		const float getMinDistance() const ;

		/// Returns the distance from the source where attenuation will stop
		const float getMaxDistance() const ;

		/// Returns the pitch of the source
		const float getPitch() const ;

		/// Returns the source volume before attenuation and other effects
		const float getVolume() const ;

		/// Returns the minimum volume that the source can be attenuated to
		const float getMinVolume() const ;

		/// Returns the maximum volume that the source can achieve
		const float getMaxVolume() const ;

		/// Returns the angle of the inner sound cone of the source
		const float getInnerConeAngle() const ;

		/// Returns the angle of the outer sound cone of the source
		const float getOuterConeAngle() const ;

		/// Returns how much the volume of the source is scaled in the outer cone
		const float getOuterConeVolume() const ;

		/// Returns the doppler strength, which enhances or diminishes the doppler effect
		const float getDopplerStrength() const ;

		/// Returns the override for the doppler velocity vector
		const PARAVECTOR3 getDopplerVelocity() const ;

		/** Registers a new event handler to this source
		* @param handler: Pointer to the event handler to register. */
		void registerEventHandler(IAudioSourceEventHandler* handler) ;

		/// Removes all event handlers attached to this source
		void unRegisterAllEventHandlers() ;
	};

	/** ParaAudioEngine core interface.
	*/
	class MacParaAudioEngine : public IParaAudioEngine
	{
	public:
		void Release() ;

		/// Initializes the manager.
		/**
		@param deviceName: Name of the device to create this manager for.
		@param outputFrequency: Frequency of the output audio or -1 for the device default.
		@param eaxEffectSlots: Number of EFX effect slots to request.  Has no effect if EFX is not supported or compiled out.
		@return True on success, False if initialization of OpenAL failed. */
		bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4) ;

		/// Shuts the manager down, cleaning up audio sources in the process.  Does not clean up decoders, data sources, or event handlers.
		void shutDown() ;

		/// If threading is disabled, you must call this function every frame to update the playback buffers of audio sources.  Otherwise it should not be called.
		void update() ;

		/// Returns an Audio Source by its "name" and NULL if the name is not found
		/**
		@param name: Name of the audio source to retrieve.
		@return Pointer to the audio source object or NULL if it could not be found.*/
		IParaAudioSource* getSoundByName(const char* name) ;

		/// Releases ALL Audio Sources (but does not shutdown the manager)
		void releaseAllSources() ;

		/// Releases a single Audio Source, removing it from the manager.
		/** @param source: Pointer to the source to release. */
		void release(IParaAudioSource* source) ;

		/// Returns the name of an available playback device.
		/** @param index: Specify which name to retrieve ( Range: 0 to getAvailableDeviceCount()-1 )
		@return Name of the selected device. */
		const char* getAvailableDeviceName(unsigned int index) ;

		/// Returns the number of playback devices available for use.
		/** @return Number of playback devices available. */
		unsigned int getAvailableDeviceCount() ;

		/// Returns the name of the default system playback device.
		/** @return Name of the default playback device. */
		const char* getDefaultDeviceName() ;

		/// Creates an Audio Source object using the highest priority data source that has the referenced filename
		/**
		@param name: Name of the audio source.
		@param filename: Path to the file to load audio data from.
		@param stream: Whether to stream the audio data or load it all into a memory buffer at the start.  You should consider using streaming for really large sound files.
		@return A pointer to an Audio Source or NULL if creation failed.
		*/
		IParaAudioSource* create(const char* name, const char* filename, bool stream = false) ;

		/// Creates an Audio Source from a memory buffer using a specific audio codec.
		/**
		@param name: Name of the audio source.
		@param data: Pointer to a buffer in memory to load the data from.
		@param length: Length of the data buffer.
		@param extension: Extension for the audio codec of the data in the memory buffer.
		@return A pointer to an Audio Source or NULL if creation failed.
		*/
		IParaAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension) ;

		/// Creates an Audio Source from raw audio data in a memory buffer.
		/**
		@param name: Name of the audio source.
		@param data: Pointer to a buffer in memory to load the data from.
		@param length: Length of the data buffer.
		@param frequency: Frequency (or sample rate) of the audio data.
		@param format: Format of the audio data.
		@return A pointer to an Audio Source or NULL if creation failed.
		*/
		IParaAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, ParaAudioFormats format) ;

	public:
		/**
		* set the audio distance model.
		* see: http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.htm
		*/
		void SetDistanceModel(ParaAudioDistanceModelEnum eDistModel) ;

		//////////////////////////////////////////////////////////////////////////
		/// Interface for the listener in cAudio.  This class provides abilities to move and orient where your camera or user is in the audio world.
		//////////////////////////////////////////////////////////////////////////

		/** Sets the position of the listener.
		Note that you will still have to set velocity after this call for proper doppler effects.
		Use move() if you'd like to have cAudio automatically handle velocity for you.
		@param pos: New position for the listener. */
		void setPosition(const PARAVECTOR3& pos) ;

		/** Sets the direction the listener is facing
		@param dir: New direction vector for the listener. */
		void setDirection(const PARAVECTOR3& dir) ;

		/** Sets the up vector to use for the listener
		Default up vector is Y+, same as OpenGL.
		@param up: New up vector for the listener. */
		void setUpVector(const PARAVECTOR3& up) ;

		/** Sets the current velocity of the listener for doppler effects
		@param vel: New velocity for the listener. */
		void setVelocity(const PARAVECTOR3& vel) ;

		/** Sets the global volume modifier (will effect all sources)
		@param volume: Volume to scale all sources by. Range: 0.0 to +inf. */
		void setMasterVolume(const float& volume) ;

		/// Convenience function to automatically set the velocity and position for you in a single call
		/** Velocity will be set to new position - last position
		@param pos: New position to move the listener to. */
		void move(const PARAVECTOR3& pos) ;

		/// Returns the current position of the listener
		PARAVECTOR3 getPosition(void) const ;

		/// Returns the current direction of the listener
		PARAVECTOR3 getDirection(void) const ;

		/// Returns the current up vector of the listener
		PARAVECTOR3 getUpVector(void) const ;

		/// Returns the current velocity of the listener
		PARAVECTOR3 getVelocity(void) const ;

		/// Returns the global volume modifier for all sources
		float getMasterVolume(void) const ;

		/**
		* set the default coordinate system used.
		* @param nLeftHand: 0 if it is left handed coordinate system, which is the one used by DirectX.
		* and 1, if it is the right handed coordinate system which is OpenAL(and OpenGL) uses.
		*/
		void SetCoordinateSystem(int nLeftHand) ;

    public:
        static MacParaAudioEngine* GetInstance();
    };
}

#endif
