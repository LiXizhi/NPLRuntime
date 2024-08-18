//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Co., All Rights Reserved.
// Date:	2010.6
// Description:	API for Audio Engine. 
//-----------------------------------------------------------------------------
#pragma once
#include "AudioEngine2.h"

using namespace std;

namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

namespace ParaScripting
{
	using namespace std;
	using namespace ParaEngine;
	using namespace luabind;

	/** @ingroup ParaAudio
	* It represents a 2D or 3D audio source object. 
	*/
	class ParaAudioSource
	{
	public:
		CAudioSource2_ptr m_pAudioSource;
	public:
		ParaAudioSource() {};
		ParaAudioSource(CAudioSource2_ptr& pSource) : m_pAudioSource(pSource) {};

		/** true if valid*/
		bool IsValid() const;

		/** stop and unload this audio source from memory. It will make the sound source invalid after calling this function.  
		* it is good practice to unload unused sound. 
		*/
		void release();

		/** Plays the source with the last set parameters.
		@return True if the source is playing, false if not. 
		*/
		bool play();

		/** get the source name. (this may not be the file name) */
		const char* GetName();

		/** Plays the source in 2D mode. 
		No automatic attenuation or panning will take place in this mode, but using setPosition will allow you to manually pan mono audio streams.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@return True if the source is playing, false if not. */
		bool play2d(bool toLoop);

		/// Plays the source in 3D mode.
		/**
		@param position: Position to start the sound off at.
		@param soundstr: Affects how the source attenuates due to distance.  Higher values cause the source to stand out more over distance.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		\return True if the source is playing, false if not. */
		bool play3d(float x, float y, float z, float soundstr, bool toLoop);
		
		/// Pauses playback of the sound source.
		void pause();

		/// Stops playback of the sound source.    
		void stop();

		/** Controls whether the source should loop or not.
		@param toLoop: Whether to loop (restart) the audio when the end is reached. */
		void loop(bool toLoop);

		/** Seeks through the audio stream to a specific spot.
		Note: May not be supported by all codecs.
		@param seconds: Number of seconds to seek.
		@param relative: Whether to seek from the current position or the start of the stream.
		@return True on success, False if the codec does not support seeking. */
		bool seek(float seconds, bool relative);

		///@return the total amount of time in the audio stream.  See IAudioDecoder for details.
		float getTotalAudioTime();

		///@return the total decoded size of the audio stream.  See IAudioDecoder for details.
		int getTotalAudioSize();

		///@return the current position in the audio stream in seconds.  See IAudioDecoder for details.
		float getCurrentAudioTime();

		///@return the current position in the decoded audio stream in bytes.  See IAudioDecoder for details.
		int getCurrentAudioPosition();


		///@return if the source is ready to be used.
		bool isValid() const;

		///@return if the source is playing.
		bool isPlaying() const;

		///@return if the source is paused.
		bool isPaused() const;

		///@return if the source is stopped.
		bool isStopped() const;

		///@return if the source is looping.
		bool isLooping() const;

		/** Sets the position of the source in 3D space.
		@param position: A 3D vector giving the new location to put this source. */
		void setPosition(float x, float y, float z);

		/** Sets the current velocity of the source for doppler effects.
		@param velocity: A 3D vector giving the speed and direction that the source is moving. */
		void setVelocity(float x, float y, float z);

		/// Sets the direction the source is facing.
		/**
		@param direction: A 3D vector giving the direction that the source is aiming. */
		void setDirection(float x, float y, float z);

		/** Sets the factor used in attenuating the source over distance.
		Larger values make it attenuate faster, smaller values make the source carry better.
		Range: 0.0f to +inf (Default: 1.0f).
		@param rolloff: The rolloff factor to apply to the attenuation calculation. */
		void setRolloffFactor(float rolloff);

		/// Sets how well the source carries over distance.
		/** Same as setRolloffFactor(1.0f/soundstrength).
		Range: 0.0f to +inf (Default: 1.0f).
		@param soundstrength: How well the sound carries over distance. */
		void setStrength(float soundstrength);

		/** Sets the distance from the source where attenuation will begin.
		Range: 0.0f to +inf
		@param minDistance: Distance from the source where attenuation begins. */
		void setMinDistance(float minDistance);

		/** Sets the distance from the source where attenuation will stop.
		Range: 0.0f to +inf
		@param maxDistance: Distance where attenuation will cease.  Normally the farthest range you can hear the source. */
		void setMaxDistance(float maxDistance);

		/** Sets the pitch of the source.
		Range: 0.0f to +inf (Default: 1.0f)
		@param pitch: New pitch level.  Note that higher values will speed up the playback of the sound. */
		void setPitch(float pitch);

		/** Sets the source volume before attenuation and other effects.
		Range: 0.0f to +inf (Default: 1.0f).
		@param volume: New volume of the source. */
		void setVolume(float volume);

		/** Sets the minimum volume that the source can be attenuated to.
		Range: 0.0f to +inf (Default: 0.0f).
		@param minVolume: New minimum volume of the source. */
		void setMinVolume(float minVolume);

		/** Sets the maximum volume that the source can achieve.
		Range: 0.0f to +inf (Default: 1.0f).
		@param maxVolume: New maximum volume of the source. */
		void setMaxVolume(float maxVolume);

		/** Sets the angle of the inner sound cone of the source.  The cone opens up in the direction of the source as set by setDirection(). 
		Note: This causes the sound to be loudest only if the listener is inside this cone.
		Range: 0.0f to 360.0f (Default: 360.0f).
		@param innerAngle: Inside angle of the cone. */
		void setInnerConeAngle(float innerAngle);

		/** Sets the angle of the outer sound cone of the source. The cone opens up in the direction of the source as set by setDirection(). 
		Note: If the listener is outside of this cone, the sound cannot be heard.  Between the inner cone angle and this angle, the sound volume will fall off.
		Range: 0.0f to 360.0f (Default: 360.0f).
		@param outerAngle: Outside angle of the cone. */
		void setOuterConeAngle(float outerAngle);

		/** Sets how much the volume of the source is scaled in the outer cone.
		Range: 0.0f to +inf (Default: 0.0f).
		@param outerVolume: Volume of the source in the outside cone. */
		void setOuterConeVolume(float outerVolume);
		
		/** Convenience function to automatically set the velocity and position for you in a single call.
		Velocity will be set to new position - last position.
		* @param position: Position to move the source to. */
		void move(float x, float y, float z);

		///@return the audio objects position
		void getPosition(float& x, float& y, float& z) const;

		///@return the audio objects velocity
		void getVelocity(float& x, float& y, float& z) const;

		///@return the audio objects direction
		void getDirection(float& x, float& y, float& z) const;

		///@return the factor used in attenuating the source over distance
		float getRolloffFactor() const;

		///@return the strength of the source
		float getStrength() const;

		///@return the distance from the source where attenuation will begin
		float getMinDistance() const;

		///@return the distance from the source where attenuation will stop
		float getMaxDistance() const;

		///@return the pitch of the source
		float getPitch() const;

		///@return the source volume before attenuation and other effects
		float getVolume() const;

		///@return the minimum volume that the source can be attenuated to
		float getMinVolume() const;

		///@return the maximum volume that the source can achieve
		float getMaxVolume() const;

		///@return the angle of the inner sound cone of the source
		float getInnerConeAngle() const;

		///@return the angle of the outer sound cone of the source
		float getOuterConeAngle() const;

		///@return how much the volume of the source is scaled in the outer cone
		float getOuterConeVolume() const;
	};

	/**
	* @ingroup ParaAudio
	* Audio Engine functions
	*/
	class PE_CORE_DECL ParaAudio
	{
	public:
		/// ---------------- Audio Engine Functions ----------------
		/// get is audio engine enabled
		static bool IsAudioEngineEnabled();

		/// enable Audio Engine
		static void EnableAudioEngine(bool bEnable);
		

		/** Set the volume of all categories and all currently playing wave files. 
		* @param fVolume: usually between [0,1], where 0 is silent and 1 is full. value larger than 1 is also possible. 
		*/
		static void SetVolume(float fVolume);

		/** Get the volume of average if all categories
		* @return usually between [0,1], where 0 is silent and 1 is full. value larger than 1 is also possible. 
		*/
		static float GetVolume();
		
		/** create a given audio source by name. If no audio source with the name is loaded before, we will create one new; otherwise we will overwrite the previous one. 
		* @param sName: the audio source name. Usually same as the audio file path, however it can be any string.  
		* @param sWavePath: if NULL, it will defaults to sName. Please note, in order to play the same music at the same time, they must be created with different names. 
		* @param bStream: whether to stream the music once created. 
		* @return CAudioSource2_ptr object returned. It may be null if failed. 
		*/
		static ParaAudioSource Create(const char* sName, const char* sWavePath, bool bStream);

		/** get audio source by name. The source should be created by Create() function. */
		static ParaAudioSource Get(const char* sName);

		/** get a given audio source by name. If no audio source with the name is loaded before, we will create one. 
		* @param sName: the audio source name. Usually same as the audio file path, however it can be any string.  
		* @param sWavePath: if NULL, it will defaults to sName. Please note, in order to play the same music at the same time, they must be created with different names. 
		* @param bStream: whether to stream the music once created. 
		* @return CAudioSource2_ptr object returned. It may be null if failed. 
		*/
		static ParaAudioSource CreateGet(const char* sName, const char* sWavePath, bool bStream);

		/**
		* set the audio distance model. 
		* see: http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.htm
		* @param eDistModel: int of following. 
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
		*/
		static void SetDistanceModel(int eDistModel);

		//////////////////////////////////////////////////////////////////////////
		//
		// handy functions to play audio files.
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* Prepare and play a wave object from a standard wave file (wav, mp3, ogg/vorbis).
		* If a wave file is already prepared before. It will be reused. 
		* @param szWavePath Path to the wave file. It can be from asset_manifest or relative to current directory path. 
		* @param nLoop 0 means non-looping. 1 means looping. 
		*/
		static bool PlayWaveFile(const char* szWavePath, int nLoop);
		static bool PlayWaveFile1(const char* szWavePath);
		/** more information, please see: midiOutShortMsg
		* @param dwMsg: MIDI message. The message is packed into a DWORD value with the first byte of the message in the low-order byte. The message is packed into this parameter as follows.
		*/
		static int PlayMidiMsg(DWORD dwMsg);
		/**
		* step a midi track.
		* @param channel: midi track number.
		*/
		static int StopMidiMsg(int channel = 0);

		/**
		* stop a wave file
		* @param szWavePath Path to the wave file. 
		* @param bImmediateStop if false, it plays the wave to completion, then stops. For looping waves, 
		* this flag plays the current iteration to completion, then stops (ignoring any subsequent iterations).
		* In either case, any release (or tail) is played. To stop the wave immediately, use true. 
		*/
		static bool StopWaveFile(const char* szWavePath, bool bImmediateStop);
		static bool StopWaveFile1(const char* szWavePath);

		/**
		* release a wave file
		* @param szWavePath Path to the wave file. 
		*/
		static bool ReleaseWaveFile(const char* szWavePath);

		
		/// ---------------- Audio Recorder ----------------
		static bool StartRecording();

		/** Can be called at any time between StartRecording and StopRecording() to retrieve recorded audio.  It is recommended that you call it every so often with long recordings to prevent the internal buffer from growing too large.
		Once successfully retrieved, the captured audio will be deleted from the internal buffer.
		@param nMaxSize: max number of bytes to copy. if -1, it will read all
		@return array of bytesto copy the audio data to.  
		*/
		static const std::string& GetCapturedAudio(int nMaxSize);

		static void StopRecording();

		static bool SaveRecording(const char* szWavePath);
		static bool SaveRecording2(const char* szWavePath, const char* pBuffer, int nSize);



		//////////////////////////////////////////////////////////////////////////
		//
		// the audio listener
		//
		//////////////////////////////////////////////////////////////////////////

		///** Sets the position of the listener.
		//Note that you will still have to set velocity after this call for proper doppler effects.
		//Use move() if you'd like to have cAudio automatically handle velocity for you. 
		//@param pos: New position for the listener. */
		//static void setPosition(float x, float y, float z);

		///** Sets the direction the listener is facing
		//@param dir: New direction vector for the listener. 
		//*/
		//static void setDirection(float x, float y, float z);

		///** Sets the up vector to use for the listener
		//Default up vector is Y+, same as OpenGL.
		//@param up: New up vector for the listener. */
		//static void setUpVector(float x, float y, float z);

		///** Sets the current velocity of the listener for doppler effects
		//@param vel: New velocity for the listener. */
		//static void setVelocity(float x, float y, float z);

		///** Convenience function to automatically set the velocity and position for you in a single call
		//Velocity will be set to new position - last position 
		//@param pos: New position to move the listener to. */
		//static void move(const PARAVECTOR3& pos);

		///** Returns the current position of the listener */
		//static void getPosition(float& x, float& y, float& z) const;

		///// Returns the current direction of the listener
		//static void getDirection(float& x, float& y, float& z) const;

		///// Returns the current up vector of the listener
		//static void getUpVector(float& x, float& y, float& z) const;

		///// Returns the current velocity of the listener
		//static void getVelocity(float& x, float& y, float& z) const;
	};
}