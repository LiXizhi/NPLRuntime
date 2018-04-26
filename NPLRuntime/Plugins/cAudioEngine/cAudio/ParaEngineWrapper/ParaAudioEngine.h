#pragma once

#include "IParaAudioEngine.h"
#include "cAudio.h"
#include <string>
#include <map>

namespace ParaEngine
{
	using namespace std;
	using namespace cAudio;
	class CParaAudioEngine;
	
	/** fix coordinate system based on current coordinate system used. */
	extern PARAVECTOR3 FixCoordinate(const PARAVECTOR3& v);
	
	/** an audio source */
	class CParaAudioSource : public IParaAudioSource, ISourceEventHandler
	{
	protected:
		friend class CParaAudioEngine;
		IAudioSource* m_pSource;
		IAudioSourceEventHandler* m_pEventHandler;
	public:
		CParaAudioSource(IAudioSource* pSource) : m_pSource(pSource), m_pEventHandler(NULL) {};
		CParaAudioSource() : m_pSource(NULL), m_pEventHandler(NULL) {};

		//! Plays the source with the last set parameters.
		/**
		\return True if the source is playing, false if not. */
		virtual bool play(){return m_pSource->play();};

		//! Plays the source in 2D mode.
		/** No automatic attenuation or panning will take place in this mode, but using setPosition will allow you to manually pan mono audio streams.
		\param toLoop: Whether to loop (restart) the audio when the end is reached.
		\return True if the source is playing, false if not. */
		virtual bool play2d(const bool& toLoop = false){return m_pSource->play2d(toLoop);};

		//! Plays the source in 3D mode.
		/**
		\param position: Position to start the sound off at.
		\param soundstr: Affects how the source attenuates due to distance.  Higher values cause the source to stand out more over distance.
		\param toLoop: Whether to loop (restart) the audio when the end is reached.
		\return True if the source is playing, false if not. */
		virtual bool play3d(const PARAVECTOR3& position, const float& soundstr = 1.0 , const bool& toLoop = false){
			auto v = FixCoordinate(position);
			return m_pSource->play3d((const cVector3&)v, soundstr, toLoop);
		};
		
		//! Pauses playback of the sound source.
		virtual void pause(){m_pSource->pause();};

		//! Stops playback of the sound source.    
		virtual void stop(){return m_pSource->stop();};

		//! Controls whether the source should loop or not.
		/** \param toLoop: Whether to loop (restart) the audio when the end is reached. */
		virtual void loop(const bool& toLoop){m_pSource->loop(toLoop);};

		//! Seeks through the audio stream to a specific spot.
		/** Note: May not be supported by all codecs.
		\param seconds: Number of seconds to seek.
		\param relative: Whether to seek from the current position or the start of the stream.
		\return True on success, False if the codec does not support seeking. */
		virtual bool seek(const float& seconds, bool relative = false){return m_pSource->seek(seconds, relative);};

		//! Returns the total amount of time in the audio stream.  See IAudioDecoder for details.
		virtual float getTotalAudioTime(){return m_pSource->getTotalAudioTime();};

		//! Returns the total decoded size of the audio stream.  See IAudioDecoder for details.
		virtual int getTotalAudioSize(){return m_pSource->getTotalAudioSize();};

		//! Returns the original size of the audio stream.  See IAudioDecoder for details.
		virtual int getCompressedAudioSize(){return m_pSource->getCompressedAudioSize();};

		//! Returns the current position in the audio stream in seconds.  See IAudioDecoder for details.
		virtual float getCurrentAudioTime(){return m_pSource->getCurrentAudioTime();};

		//! Returns the current position in the decoded audio stream in bytes.  See IAudioDecoder for details.
		virtual int getCurrentAudioPosition(){return m_pSource->getCurrentAudioPosition();};

		//! Returns the current position in the original audio stream in bytes.  See IAudioDecoder for details.
		virtual int getCurrentCompressedAudioPosition() {return m_pSource->getCurrentCompressedAudioPosition();};

		//! Normally called every frame by the audio manager to update the internal buffers.  Note: For internal use only.
		virtual bool update(){return m_pSource->update();};

		//! Releases all resources used by the audio source, normally used to clean up before deletion.  Note: For internal use only.
		virtual void release(){m_pSource->drop();};

		//! Returns if the source is ready to be used.
		virtual const bool isValid() const {return m_pSource!=0 && m_pSource->isValid();};

		//! Returns if the source is playing.
		virtual const bool isPlaying() const{return m_pSource->isPlaying();};

		//! Returns if the source is paused.
		virtual const bool isPaused() const{return m_pSource->isPaused();};

		//! Returns if the source is stopped.
		virtual const bool isStopped() const{return m_pSource->isStopped();};

		//! Returns if the source is looping.
		virtual const bool isLooping() const{return m_pSource->isLooping();};

		//! Sets the position of the source in 3D space.
		/**
		\param position: A 3D vector giving the new location to put this source. */
		virtual void setPosition(const PARAVECTOR3& position){
			auto v = FixCoordinate(position);
			return m_pSource->setPosition((const cVector3&)v);
		};

		//! Sets the current velocity of the source for doppler effects.
		/**
		\param velocity: A 3D vector giving the speed and direction that the source is moving. */
		virtual void setVelocity(const PARAVECTOR3& velocity){
			auto v = FixCoordinate(velocity);
			return m_pSource->setVelocity((const cVector3&)v);
		};

		//! Sets the direction the source is facing.
		/**
		\param direction: A 3D vector giving the direction that the source is aiming. */
		virtual void setDirection(const PARAVECTOR3& direction){
			auto v = FixCoordinate(direction);
			return m_pSource->setDirection((const cVector3&)v);
		};

		//! Sets the factor used in attenuating the source over distance.
		/** Larger values make it attenuate faster, smaller values make the source carry better.
		Range: 0.0f to +inf (Default: 1.0f).
		\param rolloff: The rolloff factor to apply to the attenuation calculation. */
		virtual void setRolloffFactor(const float& rolloff){return m_pSource->setRolloffFactor(rolloff);};

		//! Sets how well the source carries over distance.
		/** Same as setRolloffFactor(1.0f/soundstrength).
		Range: 0.0f to +inf (Default: 1.0f).
		\param soundstrength: How well the sound carries over distance. */
		virtual void setStrength(const float& soundstrength){m_pSource->setStrength(soundstrength);};

		//! Sets the distance from the source where attenuation will begin.
		/** Range: 0.0f to +inf
		\param minDistance: Distance from the source where attenuation begins. */
		virtual void setMinDistance(const float& minDistance) { m_pSource->setMinDistance(minDistance);};

		//! Sets the distance from the source where attenuation will stop.
		/** Range: 0.0f to +inf
		\param maxDistance: Distance where attenuation will cease.  Normally the farthest range you can hear the source. */
		virtual void setMaxDistance(const float& maxDistance);;

		//! Sets the pitch of the source.
		/** Range: 0.0f to +inf (Default: 1.0f)
		\param pitch: New pitch level.  Note that higher values will speed up the playback of the sound. */
		virtual void setPitch(const float& pitch) {m_pSource->setPitch(pitch);};

		//! Sets the source volume before attenuation and other effects.
		/** Range: 0.0f to +inf (Default: 1.0f).
		\param volume: New volume of the source. */
		virtual void setVolume(const float& volume) {m_pSource->setVolume(volume);};

		//! Sets the minimum volume that the source can be attenuated to.
		/** Range: 0.0f to +inf (Default: 0.0f).
		\param minVolume: New minimum volume of the source. */
		virtual void setMinVolume(const float& minVolume) {m_pSource->setMinVolume(minVolume);};

		//! Sets the maximum volume that the source can achieve.
		/** Range: 0.0f to +inf (Default: 1.0f).
		\param maxVolume: New maximum volume of the source. */
		virtual void setMaxVolume(const float& maxVolume) {m_pSource->setMaxVolume(maxVolume);};

		//! Sets the angle of the inner sound cone of the source.  The cone opens up in the direction of the source as set by setDirection(). 
		/** Note: This causes the sound to be loudest only if the listener is inside this cone.
		Range: 0.0f to 360.0f (Default: 360.0f).
		\param innerAngle: Inside angle of the cone. */
		virtual void setInnerConeAngle(const float& innerAngle) {m_pSource->setInnerConeAngle(innerAngle);};

		//! Sets the angle of the outer sound cone of the source. The cone opens up in the direction of the source as set by setDirection(). 
		/** Note: If the listener is outside of this cone, the sound cannot be heard.  Between the inner cone angle and this angle, the sound volume will fall off.
		Range: 0.0f to 360.0f (Default: 360.0f).
		\param outerAngle: Outside angle of the cone. */
		virtual void setOuterConeAngle(const float& outerAngle) {m_pSource->setOuterConeAngle(outerAngle);};

		//! Sets how much the volume of the source is scaled in the outer cone.
		/** Range: 0.0f to +inf (Default: 0.0f).
		\param outerVolume: Volume of the source in the outside cone. */
		virtual void setOuterConeVolume(const float& outerVolume) {m_pSource->setOuterConeVolume(outerVolume);};

		//! Sets the doppler strength, which enhances or diminishes the doppler effect.  Can be used to exaggerate the doppler for a special effect.
		/** Range: 0.0f to +inf (Default: 1.0f).
		\param dstrength: New strength for the doppler effect. */
		virtual void setDopplerStrength(const float& dstrength) {m_pSource->setDopplerStrength(dstrength);};

		//! Overrides the doppler velocity vector.  It is usually better to let the engine take care of it automatically.
		/** Note: must be set every time you set the position, velocity, or direction.
		\param dvelocity: New doppler vector for the source. */
		virtual void setDopplerVelocity(const PARAVECTOR3& dvelocity) {
			auto v = FixCoordinate(dvelocity);
			m_pSource->setDopplerVelocity((const cVector3&)v);
		};

		//! Convenience function to automatically set the velocity and position for you in a single call.
		/** Velocity will be set to new position - last position.
		\param position: Position to move the source to. */
		virtual void move(const PARAVECTOR3& position){
			auto v = FixCoordinate(position);
			m_pSource->move((const cVector3&)v);
		};

		//! Returns the audio objects position
		virtual const PARAVECTOR3 getPosition() const {
			auto v = m_pSource->getPosition();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the audio objects velocity
		virtual const PARAVECTOR3 getVelocity() const {
			auto v = m_pSource->getVelocity();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the audio objects direction
		virtual const PARAVECTOR3 getDirection() const {
			auto v = m_pSource->getDirection();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the factor used in attenuating the source over distance
		virtual const float getRolloffFactor() const {return m_pSource->getRolloffFactor();};

		//! Returns the strength of the source
		virtual const float getStrength() const {return m_pSource->getStrength();};

		//! Returns the distance from the source where attenuation will begin
		virtual const float getMinDistance() const {return m_pSource->getMinDistance();};

		//! Returns the distance from the source where attenuation will stop
		virtual const float getMaxDistance() const {return m_pSource->getMaxDistance();};

		//! Returns the pitch of the source
		virtual const float getPitch() const {return m_pSource->getPitch();};

		//! Returns the source volume before attenuation and other effects
		virtual const float getVolume() const {return m_pSource->getVolume();};

		//! Returns the minimum volume that the source can be attenuated to
		virtual const float getMinVolume() const {return m_pSource->getMinVolume();};

		//! Returns the maximum volume that the source can achieve
		virtual const float getMaxVolume() const {return m_pSource->getMaxVolume();};

		//! Returns the angle of the inner sound cone of the source
		virtual const float getInnerConeAngle() const {return m_pSource->getInnerConeAngle();};

		//! Returns the angle of the outer sound cone of the source
		virtual const float getOuterConeAngle() const {return m_pSource->getOuterConeAngle();};

		//! Returns how much the volume of the source is scaled in the outer cone
		virtual const float getOuterConeVolume() const {return m_pSource->getOuterConeVolume();};

		//! Returns the doppler strength, which enhances or diminishes the doppler effect 
		virtual const float getDopplerStrength() const {return m_pSource->getDopplerStrength();};

		//! Returns the override for the doppler velocity vector
		virtual const PARAVECTOR3 getDopplerVelocity() const {
			auto v = m_pSource->getDopplerVelocity();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		/** Registers a new event handler to this source
		* @param handler: Pointer to the event handler to register. */
		virtual void registerEventHandler(IAudioSourceEventHandler* handler);

		/// Removes all event handlers attached to this source
		virtual void unRegisterAllEventHandlers();

	public:
		//////////////////////////////////////////////////////////////////////
		//
		// audio event callback, invoked on the update thread (so need lock to feedback to main thread)
		//
		//////////////////////////////////////////////////////////////////////
		/// This function is called when a source updates its buffers.
		virtual void onUpdate();

		/// This function is called when a source is released and soon to be deleted.
		virtual void onRelease();

		/// This function is called when a source starts playing.
		virtual void onPlay();

		/// This function is called when a source stopped playback.
		virtual void onStop();

		/// This function is called when a source is paused.
		virtual void onPause();
	};

	/**
	* ParaEngine Audio Engine Wrapper for cAudio engine(OpenAL, MP3, ogg/Vorbis, WAV)
	* all functions should be called from the main game thread; the sound engine will manage its own update thread internally. 
	* One needs to call initialize(), but no need to call shutdown, udpate() function. They are called in constructor, destructor and internal thread. 
	*/
	class CParaAudioEngine :public IParaAudioEngine
	{
	public:
		typedef std::map<std::string, CParaAudioSource*> AudioMap_Type;

		CParaAudioEngine();
		virtual ~CParaAudioEngine();

		virtual void Release();

	public:
		//! Initializes the manager.
		/**
		\param deviceName: Name of the device to create this manager for.
		\param outputFrequency: Frequency of the output audio or -1 for the device default.
		\param eaxEffectSlots: Number of EFX effect slots to request.  Has no effect if EFX is not supported or compiled out.
		\return True on success, False if initialization of OpenAL failed. */
		virtual bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4);;

		//! Shuts the manager down, cleaning up audio sources in the process.  Does not clean up decoders, data sources, or event handlers.
		virtual void shutDown() {m_audio_manager->shutDown();};

		//! If threading is disabled, you must call this function every frame to update the playback buffers of audio sources.  Otherwise it should not be called.
		virtual void update() {m_audio_manager->update();};

		//! Returns an Audio Source by its "name" and NULL if the name is not found
		/**
		\param name: Name of the audio source to retrieve.
		\return Pointer to the audio source object or NULL if it could not be found.*/
		virtual IParaAudioSource* getSoundByName(const char* name);

		//! Releases ALL Audio Sources (but does not shutdown the manager)
		virtual void releaseAllSources();

		//! Releases a single Audio Source, removing it from the manager.
		/** \param source: Pointer to the source to release. */
		virtual void release(IParaAudioSource* source);

		//! Returns the name of an available playback device.
		/** \param index: Specify which name to retrieve ( Range: 0 to getAvailableDeviceCount()-1 ) 
		\return Name of the selected device. */
		virtual const char* getAvailableDeviceName(unsigned int index);;
		
		//! Returns the number of playback devices available for use.
		/** \return Number of playback devices available. */
		virtual unsigned int getAvailableDeviceCount();;
		
		//! Returns the name of the default system playback device.
		/** \return Name of the default playback device. */
		virtual const char* getDefaultDeviceName();;

		//! Creates an Audio Source object using the highest priority data source that has the referenced filename
		/**
		\param name: Name of the audio source.
		\param filename: Path to the file to load audio data from.
		\param stream: Whether to stream the audio data or load it all into a memory buffer at the start.  You should consider using streaming for really large sound files.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IParaAudioSource* create(const char* name, const char* filename, bool stream = false);
		
		//! Creates an Audio Source from a memory buffer using a specific audio codec.
		/**
		\param name: Name of the audio source.
		\param data: Pointer to a buffer in memory to load the data from.
		\param length: Length of the data buffer.
		\param extension: Extension for the audio codec of the data in the memory buffer.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IParaAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension);
		
		//! Creates an Audio Source from raw audio data in a memory buffer.
		/**
		\param name: Name of the audio source.
		\param data: Pointer to a buffer in memory to load the data from.
		\param length: Length of the data buffer.
		\param frequency: Frequency (or sample rate) of the audio data.
		\param format: Format of the audio data.
		\return A pointer to an Audio Source or NULL if creation failed.
		*/
		virtual IParaAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, ParaAudioFormats format);

		
		/** 
		* set the default coordinate system used. 
		* @param nLeftHand: 0 if it is left handed coordinate system, which is the one used by DirectX.
		* and 1, if it is the right handed coordinate system which is OpenAL(and OpenGL) uses.
		*/
		virtual void SetCoordinateSystem(int nLeftHand);
	public:
		/**
		* set the audio distance model. 
		* see: http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.htm
		*/
		virtual void SetDistanceModel(ParaAudioDistanceModelEnum eDistModel);

		//////////////////////////////////////////////////////////////////////////
		//! Interface for the listener in cAudio.  This class provides abilities to move and orient where your camera or user is in the audio world.
		//////////////////////////////////////////////////////////////////////////

		//! Sets the position of the listener.
		/** Note that you will still have to set velocity after this call for proper doppler effects.
		Use move() if you'd like to have cAudio automatically handle velocity for you. */
		/**
		\param pos: New position for the listener. */
		virtual void setPosition(const PARAVECTOR3& pos) { 
			auto v = FixCoordinate(pos);
			m_audio_manager->getListener()->setPosition((const cVector3&)v);
		};

		//! Sets the direction the listener is facing
		/**
		\param dir: New direction vector for the listener. */
		virtual void setDirection(const PARAVECTOR3& dir)  { 
			auto v = FixCoordinate(dir);
			m_audio_manager->getListener()->setDirection((const cVector3&)v);
		};

		//! Sets the up vector to use for the listener
		/** Default up vector is Y+, same as OpenGL.
		\param up: New up vector for the listener. */
		virtual void setUpVector(const PARAVECTOR3& up)  { 
			auto v = FixCoordinate(up);
			m_audio_manager->getListener()->setUpVector((const cVector3&)v);
		};

		//! Sets the current velocity of the listener for doppler effects
		/**
		\param vel: New velocity for the listener. */
		virtual void setVelocity(const PARAVECTOR3& vel)  { 
			auto v = FixCoordinate(vel);
			m_audio_manager->getListener()->setVelocity((const cVector3&)v);
		};

		//! Sets the global volume modifier (will effect all sources)
		/**
		\param volume: Volume to scale all sources by. Range: 0.0 to +inf. */
		virtual void setMasterVolume(const float& volume)  { m_audio_manager->getListener()->setMasterVolume(volume);};

		//! Convenience function to automatically set the velocity and position for you in a single call
		/** Velocity will be set to new position - last position 
		\param pos: New position to move the listener to. */
		virtual void move(const PARAVECTOR3& pos)  {
			auto v = FixCoordinate(pos);
			m_audio_manager->getListener()->move((const cVector3&)v);
		};

		//! Returns the current position of the listener
		virtual PARAVECTOR3 getPosition(void) const {
			auto v = m_audio_manager->getListener()->getPosition();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the current direction of the listener
		virtual PARAVECTOR3 getDirection(void) const {
			auto v = m_audio_manager->getListener()->getDirection();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the current up vector of the listener
		virtual PARAVECTOR3 getUpVector(void) const {
			auto v = m_audio_manager->getListener()->getUpVector();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the current velocity of the listener
		virtual PARAVECTOR3 getVelocity(void) const {
			auto v = m_audio_manager->getListener()->getVelocity();
			return FixCoordinate((PARAVECTOR3&)(v));
		};

		//! Returns the global volume modifier for all sources
		virtual float getMasterVolume(void) const {return m_audio_manager->getListener()->getMasterVolume();};
	
	private:
		IAudioManager* m_audio_manager;
		IAudioPlugin* m_plugin_mp3;
		IAudioDeviceList* m_deviceList;
		AudioMap_Type m_audio_source_map;
	};
}