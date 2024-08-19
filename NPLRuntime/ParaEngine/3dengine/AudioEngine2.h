#pragma once
#include "IParaAudioEngine.h"
#include "util/intrusive_ptr.h"
using namespace std;

namespace ParaEngine
{
	class MCIController;

	/** it represent an audio engine source*/
	class CAudioSource2 : public ParaEngine::intrusive_ptr_single_thread_base, public IAudioSourceEventHandler
	{
	public:
		CAudioSource2(const char* sName, IParaAudioSource* pSource = NULL)
			:m_name(sName)
			, m_pSource(pSource)
			, m_nLoopCount(0)
			, m_status(AUDIO_FLAG_Uninitialized)
			, m_bReleaseOnStop(false)
			, m_bIsAsyncLoadingWhileLoopPlaying(false)
			, m_nStartTime(0)
			, m_nStopTime(-1)
			, m_nSeekPos(0)
		{}
		~CAudioSource2() {};
		/**
		* stop a wave file
		* @param szWavePath Path to the wave file.
		* @param dwFlags Flags that affect how the wave is stopped:
		*   - AUDIO_FLAG_STOP_RELEASE Plays the wave to completion, then stops. For looping waves, this flag plays the current iteration to completion, then stops (ignoring any subsequent iterations). In either case, any release (or tail) is played. To stop the wave immediately, use the XACT_FLAG_STOP_IMMEDIATE flag.
		*   - AUDIO_FLAG_STOP_IMMEDIATE Stops the playback immediately.
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT Stop(ParaAudioFlagsEnum dwFlags);

		/** reset the source. */
		void SetSource(IParaAudioSource* pSource)
		{
			m_pSource = pSource;
		};
		IParaAudioSource* GetSource()
		{
			return m_pSource;
		};

		/** set the name */
		void SetName(const char* sName) { m_name = sName; }

		/** get the name */
		const std::string& GetName() const { return m_name; }

		/** this function must be called before the music is played. Otherwise you are at risk of thread-safety. */
		void EnableCallback(bool bEnable = true)
		{
			if (m_pSource != 0)
			{
				if (bEnable)
					m_pSource->registerEventHandler(this);
				else
					m_pSource->unRegisterAllEventHandlers();
			}
		}
	public:
		/** Plays the source with the last set parameters.
		* @return True if the source is playing, false if not. */
		bool play();

		/** Plays the source in 2D mode.
		No automatic attenuation or panning will take place in this mode, but using setPosition will allow you to manually pan mono audio streams.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@param bIgnoreIfPlaying: if true(default), we will only play if the audio is not currently playing.
		@return True if the source is playing, false if not.
		*/
		bool play2d(const bool& toLoop = false, bool bIgnoreIfPlaying = true);

		/** Plays the source in 3D mode.
		@param position: Position to start the sound off at.
		@param soundstr: Affects how the source attenuates due to distance.  Higher values cause the source to stand out more over distance.
		@param toLoop: Whether to loop (restart) the audio when the end is reached.
		@return True if the source is playing, false if not. */
		bool play3d(const PARAVECTOR3& position, const float& soundstr = 1.0, const bool& toLoop = false);

		/// Pauses playback of the sound source.
		void pause();

		/// Stops playback of the sound source.    
		void stop();

		/// Controls whether the source should loop or not.
		/** @param toLoop: Whether to loop (restart) the audio when the end is reached. */
		void loop(const bool& toLoop);

		/** weather it is the audio is still playing while being async loaded. */
		bool IsAsyncLoadingWhileLoopPlaying();

		/** whether the file should be loop playing.*/
		bool IsWaveFileLoopPlaying();

		/** whether the file is looping or not.*/
		bool IsLooping();

		/** whether the file is playing or not.*/
		bool IsPlaying();

		const std::string& GetFilename() const;
		void SetFilename(const std::string& val);

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


	public:

		IParaAudioSource* m_pSource;
		int m_nLoopCount;
		bool m_bReleaseOnStop;
		/** resource key name */
		std::string m_name;
		/** resource filename if any */
		ParaAudioFlagsEnum m_status;
		/** this is true, if an audio resource is being loop played but without being downloaded yet. */
		bool m_bIsAsyncLoadingWhileLoopPlaying;
		int m_nStartTime;
		int m_nStopTime;
		int m_nSeekPos;
	private:
		std::string m_filename;
	};
	typedef ParaIntrusivePtr<CAudioSource2> CAudioSource2_ptr;

	/**
	* CAudioEngine2 is based on the cAudioEngine plugin dll.
	* And it adds async sound asset downloading support to the original interface.
	* It also implement some handy functions to play 2d and 3d sound.
	*/
	class CAudioEngine2 : public IAttributeFields
	{
	public:
		CAudioEngine2();
		virtual ~CAudioEngine2();

		/** get the singleton interface*/
		static CAudioEngine2* GetInstance();

		/** get the audio interface */
		IParaAudioEngine* GetInterface();

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CAudioEngine2; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CAudioEngine2"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CAudioEngine2, GetDeviceName_s, const char**) { *p1 = cls->GetDeviceName(0); return S_OK; }
		ATTRIBUTE_METHOD1(CAudioEngine2, SetDeviceName_s, const char*) { cls->ResetAudioDevice(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAudioEngine2, GetCaptureAudioQuality_s, float*) { *p1 = cls->GetCaptureAudioQuality(); return S_OK; }
		ATTRIBUTE_METHOD1(CAudioEngine2, SetCaptureAudioQuality_s, float) { cls->SetCaptureAudioQuality(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAudioEngine2, GetCaptureFrequency_s, int*) { *p1 = cls->GetCaptureFrequency(); return S_OK; }
		ATTRIBUTE_METHOD1(CAudioEngine2, SetCaptureFrequency_s, int) { cls->SetCaptureFrequency(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAudioEngine2, GetRecordDeviceName_s, const char**) { *p1 = cls->GetRecordDeviceName(0); return S_OK; }
		ATTRIBUTE_METHOD1(CAudioEngine2, SetRecordDeviceName_s, const char*) { cls->ResetAudioRecordDevice(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAudioEngine2, GetAudioDeviceNames_s, const char**) { *p1 = cls->GetAudioDeviceNames(); return S_OK; }
		ATTRIBUTE_METHOD1(CAudioEngine2, GetAudioRecordDeviceNames_s, const char**) { *p1 = cls->GetAudioRecordDeviceNames(); return S_OK; }

	public:

		/** check load the plugin dll if any */
		HRESULT InitAudioEngine(IParaAudioEngine* pInteface = NULL);

		const char* GetAudioDeviceNames();
		const char* GetAudioRecordDeviceNames();

		void ResetAudioDevice(const char* deviceName);
		void ResetAudioRecordDevice(const char* deviceName);
		const char* GetRecordDeviceName(unsigned int index);

		unsigned int GetDeviceCount();
		const char* GetDeviceName(unsigned int index);

		/** shutdown and clean up audio engine. */
		void CleanupAudioEngine();

		/// if the audio engine valid
		bool IsValid();

		/** return true if the audio is both valid and enabled. */
		inline bool IsValidAndEnabled() { return IsValid() && IsAudioEngineEnabled(); };

		/// get is audio engine enabled
		bool IsAudioEngineEnabled() { return m_bEnableAudioEngine; };

		/// enable Audio Engine
		void EnableAudioEngine(bool bEnable);

		/** Returns the global volume modifier for all sources*/
		float GetGlobalVolume() { return m_fGlobalVolume; };

		/** Sets the global volume modifier (will effect all sources)
		* param volume: Volume to scale all sources by. Range: 0.0 to +inf. */
		void SetGlobalVolume(const float& volume);

		/** switch on/off the audio engine. This is usually due to window lose/get focus. */
		void OnSwitch(bool bOn);

		/** create a given audio source by name. If no audio source with the name is loaded before, we will create one new; otherwise we will overwrite the previous one.
		* @param sName: the audio source name. Usually same as the audio file path, however it can be any string.
		* @param sWavePath: if NULL, it will defaults to sName. Please note, in order to play the same music at the same time, they must be created with different names.
		* @param bStream: whether to stream the music once created.
		* @return CAudioSource2_ptr object returned. It may be null if failed.
		*/
		CAudioSource2_ptr Create(const char* sName, const char* sWavePath = NULL, bool bStream = false);

		/** get audio source by name. The source should be created by Create() function. */
		CAudioSource2_ptr Get(const char* sName);

		/** get a given audio source by name. If no audio source with the name is loaded before, we will create one.
		* @param sName: the audio source name. Usually same as the audio file path, however it can be any string.
		* @param sWavePath: if NULL, it will defaults to sName. Please note, in order to play the same music at the same time, they must be created with different names.
		* @param bStream: whether to stream the music once created.
		* @return CAudioSource2_ptr object returned. It may be null if failed.
		*/
		CAudioSource2_ptr CreateGet(const char* sName, const char* sWavePath = NULL, bool bStream = false);

		/** release audio source. It will stop the audio and unload it from memory. */
		void release(CAudioSource2_ptr& audio_src);

		/** Frame move the audio engine. since we use an internal thread for sound updating.
		this function simply updates the listener's position, orientation, upvector and velocity according to the current camera. */
		void Update();

		/** whether to automatically move the listener according to current camera position. default to true. */
		void SetAutoMoveListener(bool bAuto) { m_bAutoMoveListener = bAuto; };

		/** whether to automatically move the listener according to current camera position. */
		bool GetAutoMoveListener() { return m_bAutoMoveListener; };

		float GetCaptureAudioQuality() const;
		/**
		* @param baseQuality: value in range [0.1, 1].   0.1 is lowest quality, 1 is best quality.  0.4 is usual
		*/
		void SetCaptureAudioQuality(float val);

		int GetCaptureFrequency();
		/**
		* @param nFrequency: value like 16000, 22050, 48000, 44100, 32000, 8000. default to the first available one in above order.
		*/
		void SetCaptureFrequency(int nFrequency);
	public:
		//////////////////////////////////////////////////////////////////////////
		// handy functions for playback
		//////////////////////////////////////////////////////////////////////////

		/**
		* an archive class to record history playing events.
		*/
		class CAudioPlaybackHistory
		{
		public:
			CAudioPlaybackHistory()
				: m_bEnable(false)
			{}

			~CAudioPlaybackHistory() {}

			void AddRecord(CAudioSource2* pWave)
			{
				if (m_bEnable) {
					Record arecord(pWave->GetFilename(), pWave->m_nStartTime,
						pWave->m_nStopTime, pWave->m_nSeekPos, pWave->IsLooping(), pWave->m_pSource ? pWave->m_pSource->getTotalAudioTime() : -1);
					m_HistoryRecords.emplace_back(arecord);
				}
			}

			struct Record
			{
				Record()
					: m_WaveFileName("")
					, m_nStartTime(0)
					, m_nEndTime(-1)
					, m_nSeekPos(0)
					, m_bIsLoop(false)
					, m_mTotalTime(-1)
				{

				}

				Record(const std::string& waveFile, int start, int end, int seek, bool isLoop = false, int totalTime = -1)
					: m_WaveFileName(waveFile)
					, m_nStartTime(start)
					, m_nEndTime(end)
					, m_nSeekPos(seek)
					, m_bIsLoop(isLoop)
					, m_mTotalTime(totalTime)
				{

				}

				std::string m_WaveFileName; // audio file name that has been played
				int m_nStartTime; // the game time when starts playing the audio file with name m_WaveFileName
				int m_nEndTime; // the game time when stops playing the audio file with name m_WaveFileName
				int m_nSeekPos; // the seek positon where it starts when the audio engines play the audio file with name m_WaveFileName
				bool m_bIsLoop;
				int m_mTotalTime;
			};

			typedef std::list<Record> Records;
			const Records& GetRecords() const
			{
				return m_HistoryRecords;
			}
			/** find the last element that matches */
			Record* FindLastRecord(const std::string& waveFile)
			{
				Records::iterator result = m_HistoryRecords.end();
				for (Records::iterator iter = m_HistoryRecords.begin(); iter != m_HistoryRecords.end(); ++iter) {
					if (iter->m_WaveFileName == waveFile) result = iter;
				}
				if (result != m_HistoryRecords.end()) {
					return &(*result);
				}
				else {
					return NULL;
				}
			}

			void RemoveRecord(const std::string& waveFile)
			{
				Records::iterator result = m_HistoryRecords.end();
				for (Records::iterator iter = m_HistoryRecords.begin(); iter != m_HistoryRecords.end(); ++iter) {
					if (iter->m_WaveFileName == waveFile) result = iter;
				}
				if (result != m_HistoryRecords.end()) m_HistoryRecords.erase(result);
			}

			bool Enable() { return m_bEnable; }

			void SetEnable(bool val) { m_bEnable = true; }

			void Clear()
			{
				m_HistoryRecords.clear();
			}

		private:
			Records m_HistoryRecords;
			bool m_bEnable;
		};

		CAudioPlaybackHistory& GetPlaybackHistory();

		/**
		* Prepare and play a wave object from a standard audio file (WAV, OGG, mp3).
		* If a wave file is already prepared before. It will be reused.
		* @param sWavePath Path to the wave file.
		* @param bLoop true for looping sound
		* @param dwPlayOffset Play offset to use as the start of the wave. The offset can be described
		* in milliseconds or in samples. The dwFlags argument determines the offset units.
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT PlayWaveFile(const char* sWavePath, bool bLoop = false, bool bStream = false, int dwPlayOffset = 0);

		/** call this function to prepare a given wave file from disk. if the wave file does not exist on disk, this function does nothing.
		* this function is usually called after a wave file has been just downloaded from the web.
		* @param pWave: the input wave file to prepare.
		* @param sWavePath: the wave path on disk or asset manifest key.
		*/
		HRESULT PrepareWaveFile(CAudioSource2_ptr& pWave, const char* sWavePath, bool bStream = false);

		/**
		* stop a wave file
		* @param szWavePath Path to the wave file.
		* @param dwFlags Flags that affect how the wave is stopped:
		*   - AUDIO_FLAG_STOP_RELEASE Plays the wave to completion, then stops. For looping waves, this flag plays the current iteration to completion, then stops (ignoring any subsequent iterations). In either case, any release (or tail) is played. To stop the wave immediately, use the XACT_FLAG_STOP_IMMEDIATE flag.
		*   - AUDIO_FLAG_STOP_IMMEDIATE Stops the playback immediately.
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT StopWaveFile(const char* sWavePath, ParaAudioFlagsEnum dwFlags);

		/**
		* release a wave file
		* @param szWavePath Path to the wave file.
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT ReleaseWaveFile(const std::string& szWavePath);

		/** whether a given wave file is still in the code driven queue. We will enqueue a wave file, when it is played and deque it when it is stopped.
		* if the wave file is not downloaded from the remote server, it will also be enqueued.
		*/
		bool IsWaveFileInQueue(const std::string& filename);
		/** whether a given wave file is still in the code driven queue, and is being loop played
		*/
		bool IsWaveFileLoopPlaying(const std::string& filename);

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
		void SetDistanceModel(ParaAudioDistanceModelEnum eDistModel);

		//////////////////////////////////////////////////////////////////////////
		//
		// the audio listener
		//
		//////////////////////////////////////////////////////////////////////////

		/** Sets the position of the listener.
		Note that you will still have to set velocity after this call for proper doppler effects.
		Use move() if you'd like to have cAudio automatically handle velocity for you.
		@param pos: New position for the listener. */
		void setPosition(const PARAVECTOR3& pos);

		/** Sets the direction the listener is facing
		@param dir: New direction vector for the listener.
		*/
		void setDirection(const PARAVECTOR3& dir);

		/** Sets the up vector to use for the listener
		Default up vector is Y+, same as OpenGL.
		@param up: New up vector for the listener. */
		void setUpVector(const PARAVECTOR3& up);

		/** Sets the current velocity of the listener for doppler effects
		@param vel: New velocity for the listener. */
		void setVelocity(const PARAVECTOR3& vel);

		/** Convenience function to automatically set the velocity and position for you in a single call
		Velocity will be set to new position - last position
		@param pos: New position to move the listener to. */
		void move(const PARAVECTOR3& pos);

		/** Returns the current position of the listener */
		PARAVECTOR3 getPosition() const;

		/// Returns the current direction of the listener
		PARAVECTOR3 getDirection() const;

		/// Returns the current up vector of the listener
		PARAVECTOR3 getUpVector() const;

		/// Returns the current velocity of the listener
		PARAVECTOR3 getVelocity() const;

		void PauseAll();
		void ResumeAll();

		/** OBSOLETED, see CreateGetAudioCapture */
		MCIController* getMCIController();

		/** for audio capture */
		IParaAudioCapture* CreateGetAudioCapture();

		CAudioPlaybackHistory& SetHistoryWithCaptureBegin();
		CAudioPlaybackHistory& SetHistoryWithCaptureEnd();
	private:
		IParaAudioEngine* m_pAudioEngine;
		bool m_bEnableAudioEngine;

		typedef std::map<std::string, CAudioSource2_ptr> AudioFileMap_type;
		AudioFileMap_type m_audio_file_map;

		CAudioPlaybackHistory m_PlaybackHistory;

		std::vector<CAudioSource2_ptr> m_paused_audios;
		float m_fGlobalVolume;
		float m_fGlobalVolumeBeforeSwitch;
		bool m_bAutoMoveListener;
		float m_fCaptureAudioQuality;
	};
}
