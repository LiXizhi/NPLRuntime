// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IListener.h"
#include "cAudioDefines.h"
#include "EAudioFormats.h"
#include "IAudioEffects.h"
#include "IDataSourceFactory.h"
#include "IManagerEventHandler.h"
#include <string>

namespace cAudio
{
	class IAudioSource;
	class IAudioDecoderFactory;
	class AudioCaptureBuffer;

	//! Interface for the playback capabilities of cAudio.
    class IAudioManager
    {
    public:
		IAudioManager() { }
		virtual ~IAudioManager() { }

		//! Initializes the manager.
		/**
		\param deviceName: Name of the device to create this manager for.
		\param outputFrequency: Frequency of the output audio or -1 for the device default.
		\param eaxEffectSlots: Number of EFX effect slots to request.  Has no effect if EFX is not supported or compiled out.
		\return True on success, False if initialization of OpenAL failed. */
		virtual bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4) = 0;

		//! Shuts the manager down, cleaning up audio sources in the process.  Does not clean up decoders, data sources, or event handlers.
		virtual void shutDown() = 0;

		//! If threading is disabled, you must call this function every frame to update the playback buffers of audio sources.  Otherwise it should not be called.
		virtual void update() = 0;

		//! Returns if the thread used to update all Audio Managers is running.
		/** Note: Will always return false if threading is disabled.
		The library automatically shuts down the thread if no Audio Managers exist and will restart the thread on creation of a new manager.
		\return True if the thread is currently running, false otherwise.
		*/
		virtual bool isUpdateThreadRunning() = 0;

		//! Returns an Audio Source by its "name" and NULL if the name is not found
		/**
		\param name: Name of the audio source to retrieve.
		\return Pointer to the audio source object or NULL if it could not be found.*/
		virtual IAudioSource* getSoundByName(const char* name) = 0;

		//! Releases ALL Audio Sources (but does not shutdown the manager)
		virtual void releaseAllSources() = 0;

		//! Releases a single Audio Source, removing it from the manager.
		/** \param source: Pointer to the source to release. */
		virtual void release(IAudioSource* source) = 0;

		//! Creates an Audio Source object using the highest priority data source that has the referenced filename
		/**
		\param name: Name of the audio source.
		\param playLooped: if the sound is looped a IAudioSource is returned
		\param startPaused: if false the sound plays asap
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IAudioSource* play2D(const char* filename, 
									 bool playLooped = false,
									 bool startPaused = false) = 0;

		//! Creates an Audio Source object using the highest priority data source that has the referenced filename
		/**
		\param name: Name of the audio source.
		\param position: The start position of the sound.
		\param playLooped: if the sound is looped a IAudioSource is returned
		\param startPaused: if false the sound plays asap
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IAudioSource* play3D(const char* filename, 
									 cVector3 position,
									 bool playLooped = false,
									 bool startPaused = false) = 0;

		//! Sets master volume. (valid range [0 - 1.0])
		virtual void setMasterVolume(float vol) = 0;

		//! Get the master volume.
		virtual float getMasterVolume() const = 0;

		//! Stops all playing sounds.
		virtual void stopAllSounds() = 0;

		//! Creates an Audio Source object using the highest priority data source that has the referenced filename
		/**
		\param name: Name of the audio source.
		\param filename: Path to the file to load audio data from.
		\param stream: Whether to stream the audio data or load it all into a memory buffer at the start.  You should consider using streaming for really large sound files.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IAudioSource* create(const char* name, const char* filename, bool stream = false) = 0;
		
		//! Creates an Audio Source from a memory buffer using a specific audio codec.
		/**
		\param name: Name of the audio source.
		\param data: Pointer to a buffer in memory to load the data from.
		\param length: Length of the data buffer.
		\param extension: Extension for the audio codec of the data in the memory buffer.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension) = 0;
		
		//! Creates an Audio Source from raw audio data in a memory buffer.
		/**
		\param name: Name of the audio source.
		\param data: Pointer to a buffer in memory to load the data from.
		\param length: Length of the data buffer.
		\param frequency: Frequency (or sample rate) of the audio data.
		\param format: Format of the audio data.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, AudioFormats format) = 0;


		//! Creates an Audio Source from AudioCaptureBuffer in a memory buffer.
		/**
		\param name: Name of the audio source.
		\param AudioCaptureBuffer: Pointer to a AudioCaptureBuffer in memory to load the data from.
		\param frequency: Frequency (or sample rate) of the audio data.
		\param format: Format of the audio data.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IAudioSource* createFromAudioBuffer(const char* name, AudioCaptureBuffer* pBiffer, unsigned int frequency, AudioFormats format) = 0;

		//! Register an Audio Decoder.
		/**
		\param factory: Pointer to the factory instance to use.
		\param extension: Extension of the audio codec to register this decoder under.  For example, .wav for a RIFF/wav decoder.
		\return True on success, False if registration failed. */
		virtual bool registerAudioDecoder(IAudioDecoderFactory* factory, const char* extension) = 0;

		//! Unregister a previously registered Audio Decoder.
		/** Note that all current sound sources will still continue to use any currently allocated decoders. Will NOT delete any user added factory instance, you must do that yourself.
		\param extension: Extension for the audio decoder to remove. */
		virtual void unRegisterAudioDecoder(const char* extension) = 0;

		//! Returns whether an audio decoder is currently registered for this file type
		/**
		\param extension: Extension for the audio decoder to check for.
		\return True if the specified decoder is registered or False if not. */
		virtual bool isAudioDecoderRegistered(const char* extension) = 0;

		//! Returns a registered audio decoder factory.
		/** 
		\param extension: Extension for the audio decoder to return.
		\return A pointer to the found factory or NULL if it could not be found. */
		virtual IAudioDecoderFactory* getAudioDecoderFactory(const char* extension) = 0;

		//! Unregisters all attached Audio Decoders.
		/** Note that all current sound sources will still continue to use any currently allocated decoders. Will NOT delete any user added factory instance, you must do that yourself. */
		virtual void unRegisterAllAudioDecoders() = 0;

		//! Registers a data source with this manager
		/** Use this function to provide access to a custom resource manager, zip archive, vfs, or any other place you'd like to get audio files from.
		\param factory: Pointer to the data source factory to register.
		\param name: Name for the data source (ie. Zip Archive or FileSystem)
		\param priority: Determines what order data sources are asked for a file.  The higher the priority, the sooner they are asked.  cAudio stops looking for a file as soon as any data source returns it.
		\return True on success, False on failure to register. */
		virtual bool registerDataSource(IDataSourceFactory* factory, const char* name, int priority) = 0;
		
		//! Removes a previously registered data source.
		/** Note that sound sources will still continue to use any currently allocated sources, so you may not be able to close an open zip/archive file handle until they are done. Will NOT delete any user added factory instance, you must do that yourself.
		\param name: Name of the data source to unregister. */
		virtual void unRegisterDataSource(const char* name) = 0;

		//! Returns whether a data source is currently registered under a certain name.
		/**
		\param name: Name of the data source to check for.
		\return True if the specified data source is registered or False if not. */
		virtual bool isDataSourceRegistered(const char* name) = 0;

		//! Returns a previously registered data source factory.
		/** 
		\param name: Name of the data source to return.
		\return A pointer to the found factory or NULL if it could not be found. */
		virtual IDataSourceFactory* getDataSourceFactory(const char* name) = 0;

		//! Removes all previously registered data sources.
		/** Note that sound sources will still continue to use any currently allocated sources, so you may not be able to close an open zip/archive file handle until they are done. Will NOT delete any user added factory instance, you must do that yourself. */
		virtual void unRegisterAllDataSources() = 0;

		//! Registers a new event handler with the manager.
		/**
		\param handler: Pointer to an event handler instance to register. */
		virtual void registerEventHandler(IManagerEventHandler* handler) = 0;

		//! Unregisters a previously registered event handler from the manager.
		/** Note: Will not delete your event handler, you must take care of that yourself.
		\param handler: Pointer to the event handler to remove. */
		virtual void unRegisterEventHandler(IManagerEventHandler* handler) = 0;

		//! Unregisters all previously registered event handlers from the manager.
		/** Note: Will not delete your event handler, you must take care of that yourself. */
		virtual void unRegisterAllEventHandlers() = 0;

		//! Returns the interface for the listener.
		virtual IListener* getListener() = 0;

#if CAUDIO_EFX_ENABLED == 1
		//! Returns the interface for audio effects.
		virtual IAudioEffects* getEffects() = 0;
#endif

    protected:
    private:
    };
}
