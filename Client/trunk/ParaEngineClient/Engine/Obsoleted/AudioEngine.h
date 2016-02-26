//---------------------------------------
// File: AudioEngine.h
// Author: WangTian
//
// Desc: ParaEngine Audio Engine
// Email: mitnick_wang@hotmail.com
//			wangtian@cad.zju.edu.cn
// Date: 2006.8
//---------------------------------------

#pragma once
#include <xact.h>
#include <xact3d.h>
#include <map>
#include <vector>
#include <list>

#include "util/mutex.h"

using namespace std;

namespace ParaEngine
{
	/// Struct: ParaAudio_3DCUE
	///
	/// ParaAudio_3DCUE structure holds the information needed in the 3D sound 
	/// rendering, include the cue instance, identity key and the emitter position.
	struct ParaAudio_3DCUE
	{
		/// true if playing, false if stopped for garbage collection
		bool bActive;
		/// pointer to the cue instance
		IXACTCue* pCue;
		/// position of the emitter
		Vector3 vEmitterPosition;
		/// identity of a 3D cue instance
		/// this may be the name of the object in ParaEngine
		/// or the script defined name
		string strKeyName;
	};

	/// Class: CAudioEngine
	/// 
	/// The purpose of the audio in ParaEngine is to serve as a lightweight container 
	/// built upon Microsoft Cross-Platform Audio Creation Tool (XACT) engine. 
	/// The main design goal is to provide a framework that allows easy access 
	/// of sound and music without concerning the management and organization 
	/// of linear and 3D audio resources.
	class CAudioEngine
	{
	public:
		typedef map<string, ParaAudio_3DCUE>	POOL_3DCUE_TYPE;

		CAudioEngine(void);
		~CAudioEngine(void);
		/// CAudioEngine is a singleton class
		static CAudioEngine * GetInstance();

		/// Initialize the audio engine
		/// -------------------------------------------
		/// This function ONLY does the following:
		/// 1. Load global setting stored in Audio\Global.xgs
		/// 2. Set the callback function: AudioEngineNotificationCallback
		/// 3. Initialize XACT by calling m_pEngine->Initialize
		/// 4. Register notification
		/// 5. Setup 3D audio struct to Stereo
		///
		/// This function DOESN'T do the following:
		/// 1. Create the XACT wave bank(s)
		/// 2. Create the XACT sound bank(s)
		/// 3. Store indices to the XACT cue(s)
		/// -------------------------------------------
		HRESULT InitAudioEngine();
		/// Shutdown the audio engine
		/// Note that m_pEngine->ShutDown is synchronous and will take some time to complete 
		/// if there are still playing cues.  Also m_pEngine->ShutDown() is generally only 
		/// called when a game exits and is not the preferred method of changing audio 
		/// resources.
		void CleanupAudioEngine();

		/// Currently registered notifications:
		/// 1. Wave bank prepared
		/// 2. Background cue prepared
		/// 3. Background cue stopped
		/// 4. Dialog cue prepared
		/// 5. Dialog cue stopped
		//
		// Only for documentation
		// void WINAPI AudioEngineNotificationCallback(const XACT_NOTIFICATION* pNotification)

		/// if the audio engine valid
		bool IsValid();

		/** return true if the audio is both valid and enabled. */
		inline bool IsValidAndEnabled(){return IsValid() && IsAudioEngineEnabled();};

		/// It is important to allow XACT to do periodic work by calling XACTDoWork().
		/// DoWork internally call the UpdateAudio() function. And called by the audio engine 
		/// to join the game main loop.
		void DoWork();

	//////////////////////////////////////////////////////////////////////////
	//
	// recording functions
	//
	//////////////////////////////////////////////////////////////////////////
	public:
		/**
		* enumerate the available recording devices in this machine. returned as strings separated by ';'
		* this function is not thread safe
		*/
		const char* GetRecordingDeviceEnum();

		/**
		* enumerate the available recording formats on this machine. returned as strings separated by ';'
		* this function is not thread safe
		*/
		const char* GetRecordingFormatEnum();

		/**
		* set the recording output properties for recording with BeginRecording() and StopRecording() function. 
		* @param sOutputFile: the output file. it can be nil. Default is "temp/soundcapture.wav",
		* @param nDeviceIndex: -1 to use the current one. or it should be a valid index as in GetRecordingDeviceEnum();
		* @param nWaveFormatIndex: -1 to use the current one. or it should be a valid index as in GetRecordingFormatEnum();
		*/
		bool SetRecordingOutput(const char* sOutputFile, int nDeviceIndex, int nWaveFormatIndex);

		/** start recording to the current recording output */
		bool BeginRecording();

		/** whether it is recording sound.*/
		bool IsRecording();

		/** whether it is recording is paused.*/
		bool IsRecordingPaused();

		/** try return a snapshot of the recorded wave data in the last nTimeLength time.
		* just turn the music to strings to be displayed by the GUI.
		*/
		const char* GetLastRecordedData(int nTimeLength);

		/**
		* @param nState: 0 for toggle pause and resume. -1 for pause, 1 for resume. 
		*/
		bool PauseOrResumeRecording(int nState);

		/** stop recording and save output to the current recording output */
		bool StopRecording();

		/** Set the volume of all categories and all currently playing wave files. 
		* @param fVolume: usually between [0,1], where 0 is silent and 1 is full. value larger than 1 is also possible. 
		*/
		void SetVolume(float fVolume);

		/** Get the volume of average if all categories
		* @return usually between [0,1], where 0 is silent and 1 is full. value larger than 1 is also possible. 
		*/
		float GetVolume();
	private:
		/** init sound capture devices */
		HRESULT InitSoundCapture();

		/** release sound capture devices */
		HRESULT ReleaseSoundCapture();

		/**
		* Tests to see if 20 different standard wave formats are supported by the capture device 
		*/
		HRESULT ScanAvailableInputFormats();

		/**
		* Returns 20 different wave formats based on nIndex
		*/
		void GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx );

		/**
		* Converts a wave format to a text string
		*/
		void ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName, int cchFormatName );

		/** init notifications*/
		HRESULT InitNotifications();

		/** force closing all notifications*/
		HRESULT DestoryNotifications();

		const static int NUM_REC_NOTIFICATIONS = 10;
		
		LPDIRECTSOUNDCAPTURE       m_pDSCapture;
		LPDIRECTSOUNDCAPTUREBUFFER m_pDSBCapture;
		LPDIRECTSOUNDNOTIFY        m_pDSNotify;
		GUID                       m_guidCaptureDevice;
		BOOL                       m_bRecording;
		WAVEFORMATEX               m_wfxInput;
		DSBPOSITIONNOTIFY          m_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ];  
		BOOL                       m_abInputFormatSupported[20];
		DWORD                      m_dwCaptureBufferSize;
		DWORD                      m_dwNextCaptureOffset;
		DWORD                      m_dwNotifySize;
		CWaveFile*                 m_pWaveFile;
		bool m_bCaptureDeviceAvailable;
		/// the current sound capture device index
		int m_nCaptureDeviceIndex;
		/// the current sound capture format index
		int m_nWaveFormatIndex;
		/// wave file to which the captured sound is saved. 
		string m_sCapureFileName;

		/** all supported sound capture format separated by ';' */
		string m_strCaptureFormats;
		
		/** record data */
		HRESULT RecordCapturedData();

		/** the thread to record sound.*/
		static unsigned int __stdcall RecordSoundLoop(void* parameter);
		
		/// the thread handle used to record the sound
		HANDLE m_hRecordingThread;
	//////////////////////////////////////////////////////////////////////////
	//
	// playing functions
	//
	//////////////////////////////////////////////////////////////////////////
	public:
		// ---------------- Audio Engine Functions ----------------
		/// get is audio engine enabled
		bool IsAudioEngineEnabled();
		/// enable Audio Engine
		HRESULT EnableAudioEngine( );
		/// disable Audio Engine
		HRESULT DisableAudioEngine( );
		/// pause certain category in Audio Engine
		/// @param strCategoryName: category name
		HRESULT PauseCategory(const char * strCategoryName);
		/// resume certain category in Audio Engine
		/// @param strCategoryName: category name
		HRESULT ResumeCategory(const char * strCategoryName);
		/// stop certain category in Audio Engine
		/// @param strCategoryName: category name
		HRESULT StopCategory(const char * strCategoryName);
		/// initialize the audio banks needed in Audio Engine
		/// @param strBankName: sound and wave bank name
		HRESULT EnableAudioBank(const char * strBankName);
		/// clean up the audio banks needed in Audio Engine
		/// @param strBankName: sound and wave bank name
		HRESULT DisableAudioBank(const char * strBankName);
		/// get the current audio bank name
		/// @return : current audio bank name
		string GetCurrentAudioBankName();
		/// set listener's sound output format to stereo
		HRESULT SetAudioStereo();
		/// set listener's sound output format to 5.1 sound
		HRESULT SetAudio5Point1();
		/// get listener's sound output format
		/// @return: 2:Stereo 6:5Point1
		int GetAudioFormat();

		// ---------------- Background Functions ----------------
		/// play background music
		/// @param strBGMusicName: background music name
		HRESULT PlayBGMusic( const char * strBGMusicName );
		/// pause background music
		/// @param strBGMusicName: background music name
		HRESULT PauseBGMusic( );
		/// resume background music
		/// @param strBGMusicName: background music name
		HRESULT ResumeBGMusic( );
		/// stop background music
		/// @param strBGMusicName: background music name
		HRESULT StopBGMusic( );
		/// get is background music enabled
		bool IsBGMusicEnabled();
		/// enable background music
		HRESULT EnableBGMusic( );
		/// disable background music
		HRESULT DisableBGMusic( );
		/// Get background music volume
		/// @param fVolume: background music volume
		float GetBGMusicVolume( );
		/// set background music volume
		/// @param fVolume: background music volume
		HRESULT SetBGMusicVolume( const float fVolume );

		// ---------------- Dialog Functions ----------------
		/// play dialog music
		/// @param strDialogName: dialog music name
		HRESULT PlayDialog( const char * strDialogName , const char * strScript);
		/// skip dialog music to the next marker
		HRESULT SkipDialog( );
		/// pause dialog music
		/// @param strDialogName: dialog music name
		HRESULT PauseDialog( );
		/// resume dialog music
		/// @param strDialogName: dialog music name
		HRESULT ResumeDialog( );
		/// stop dialog music
		/// @param strDialogName: dialog music name
		HRESULT StopDialog( );
		/// get is dialog music enabled
		bool IsDialogEnabled();
		/// enable dialog music
		HRESULT EnableDialog( );
		/// disable dialog music
		HRESULT DisableDialog( );
		/// get dialog volume
		/// @param fVolume: dialog volume
		float GetDialogVolume( );
		/// set dialog volume
		/// @param fVolume: dialog volume
		HRESULT SetDialogVolume( const float fVolume );

		// ---------------- Ambient Sound Functions ----------------
		/// play ambient sound
		/// @param strAmbientSoundName: ambient sound name
		HRESULT PlayAmbientSound( const char * strAmbientSoundName );
		/// pause ambient sound
		/// @param strAmbientSoundName: ambient sound name
		HRESULT PauseAmbientSound( const char * strAmbientSoundName );
		/// resume ambient sound
		/// @param strAmbientSoundName: ambient sound name
		HRESULT ResumeAmbientSound( const char * strAmbientSoundName );
		/// stop ambient sound
		/// @param strAmbientSoundName: ambient sound name
		HRESULT StopAmbientSound( const char * strAmbientSoundName );
		/// get is ambient sound enabled
		bool IsAmbientSoundEnabled();
		/// enable ambient sound music
		HRESULT EnableAmbientSound( );
		/// disable ambient sound music
		HRESULT DisableAmbientSound( );
		/// get ambient sound volume
		/// @param fVolume: ambient sound volume
		float GetAmbientSoundVolume( );
		/// set ambient sound volume
		/// @param fVolume: ambient sound volume
		HRESULT SetAmbientSoundVolume( const float fVolume );

		// ---------------- UserInterface Sound Functions ----------------
		/// play UI sound
		/// @param strUISoundName: UI sound name
		HRESULT PlayUISound( const char * strUISoundName );
		/// get UI sound volume
		/// @param fVolume: UI sound volume
		float GetUISoundVolume( );
		/// set UI sound volume
		/// @param fVolume: UI sound volume
		HRESULT SetUISoundVolume( const float fVolume );

		// ---------------- 3D Sound Functions ----------------

		/// play 3D sound in static position. 3d sound whose position does not change in the world once created. 
		/// @param str3DSoundName: 3D sound name
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		/// @param x, y, z: 3D sound position
		HRESULT PlayStatic3DSound( const char * str3DSoundName, const char * assetName, float x, float y, float z);
		/// pause 3D sound in static position
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		HRESULT PauseStatic3DSound( const char * assetName );
		/// Resume 3D sound in static position
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		HRESULT ResumeStatic3DSound( const char * assetName );
		/// stop 3D sound in static position
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		HRESULT StopStatic3DSound( const char * assetName );
		/// play 3D sound to dynamic object
		/// @param str3DSoundName: 3D sound name
		/// @param objectName: 3D sound object name, this is the KEY to the 3D Sound
		//			this is the name of the base class BaseObject
		HRESULT PlayDynamic3DSound( const char * str3DSoundName, const char * objectName );
		/// get 3D sound volume
		/// @param fVolume: 3D sound volume
		float Get3DSoundVolume( );
		/// set 3D sound volume
		/// @param fVolume: 3D sound volume
		HRESULT Set3DSoundVolume( const float fVolume );

		// ---------------- Interactive Sound Functions ----------------
		/// play Interactive sound
		/// @param strInteractiveSoundName: interactive sound name
		/// NOTE: Interactive sound is now behave like UI sound, and it don't promise Zero-Latency
		HRESULT PlayInteractiveSound( const char * strInteractiveSoundName );
		/// get interactive sound volume
		/// @param fVolume: interactive sound volume
		float GetInteractiveSoundVolume( );
		/// set interactive sound volume
		/// @param fVolume: interactive sound volume
		HRESULT SetInteractiveSoundVolume( const float fVolume );


		// ---------------- Loading Functions ----------------
		/** Load a sound bank. 
		* note: Currently only one can be loaded at a time, future version may support many. 
		* @param filename: the logical *.xsb sound bank file path. It can be an asset file. such as "audio/name/name.xsb"
		* @return: S_OK if loaded. E_PENDING if we are downloading. E_FAIL if cannot load. 
		*/
		HRESULT LoadSoundBank(const char* filename);
		/** unload a sound bank */
		HRESULT UnLoadSoundBank(const char* filename=NULL);

		/** Load an in memory wave bank. 
		* note: Currently only one can be loaded at a time, future version may support many. 
		* @param filename: the logical "*-InMemory.xwb". It can be an asset file. such as "audio/name/name-InMemory.xwb"
		* @return: S_OK if loaded. E_PENDING if we are downloading. E_FAIL if cannot load. 
		*/
		HRESULT LoadInMemoryWaveBank(const char* filename);
		/** unload a wave bank */
		HRESULT UnLoadInMemoryWaveBank(const char* filename=NULL);

		/** Load an stream wave bank. 
		* note: Currently only one can be loaded at a time, future version may support many. 
		* @param filename: the logical "*-Stream.xwb". It can be an asset file. such as "audio/name/name-Stream.xwb"
		* @return: S_OK if loaded. E_PENDING if we are downloading. E_FAIL if cannot load. 
		*/
		HRESULT LoadStreamWaveBank(const char* filename);
		/** unload a wave bank */
		HRESULT UnLoadStreamWaveBank(const char* filename=NULL);
		
	private:
		HRESULT FindMediaFileCch(CHAR* strDestPath, int cchDest, LPCSTR strFilename);
		HRESULT PlayAmbientWater();

		/// UpdateAudio will: 
		/// 1. handle the notifications caught by the callback function
		/// 2. update the 3d sound position
		void UpdateAudio();
	private:

		/// XACT Audio Engine pointer
		IXACTEngine* m_pEngine;

		/// Bank data or handle
		HANDLE m_hStreamingWaveBankFile;
		VOID* m_pbWaveBankInMemory; // Handle to in memory wave bank data.  Its memory mapped so call UnmapViewOfFile() upon cleanup to release file
		VOID* m_pbSoundBank; // Pointer to sound bank data.  Call delete on it when the sound bank is destroyed

		/// Audio engine and categories activation
		bool m_bEnableAudioEngine;
		bool m_bEnableBGMusic;
		bool m_bEnableDialogMusic;
		bool m_bEnableAmbientSound;
		bool m_bEnableUISound;
		bool m_bEnable3DSound;
		bool m_bEnableInteractiveSound;

		/// Categories volumes
		float m_fBGMusicVolume;
		float m_fDialogMusicVolume;
		float m_fAmbientSoundVolume;
		float m_fUISoundVolume;
		float m_f3DSoundVolume;
		float m_fInteractiveSoundVolume;

		/// current bank name
		/// the active banks, please refer to the documentation
		string m_strCurrentAudioBankName;
		/// if according banks initialized
		bool m_bInitializeBanks;

		/// ambient cue index <--> ambient cue instance
		typedef pair <XACTINDEX, IXACTCue*> CuePointerPair;
		/// map of cue index and the cue instance
		/// since ambient sound doesn't allow multi play instance
		map<XACTINDEX, IXACTCue*> m_mapAmbientCuePointer;

		/// 3D environment
		/// 3D handle instance
		X3DAUDIO_HANDLE m_x3DInstance;
		/// Counter to apply 3D audio occasionally
		int m_nFrameToApply3DAudio;
		/// DSP settings
		X3DAUDIO_DSP_SETTINGS m_dspSettings;
		/// 3D audio engine listener
		X3DAUDIO_LISTENER m_listener;
		/// 3D audio engine emitter
		X3DAUDIO_EMITTER m_emitter;
		/// delay times
		FLOAT32 m_DelayTimes[2];
		/// current map only to stereo, TODO: support 5.1 sound
		FLOAT32 m_MatrixCoefficients[2 * 2];

		/// Static 3D cue, 3d sound whose position does not change in the world once created. 
		POOL_3DCUE_TYPE m_pool_3D_Static_Cues;
		/// Dynamic 3D cue, 3d sound whose position will change in the world after created. 
		POOL_3DCUE_TYPE m_pool_3D_Dynamic_Cues;

		/// Critical section object to make shared data thread safe while avoiding deadlocks.
		/// m_cs will protect the class shared data structures between the callback and the app thread.
		ParaEngine::mutex  m_mutex;

		bool m_bHandleStreamingWaveBankPrepared;
		bool m_bHandleDialogCuePrepared;
		bool m_bHandleDialogCueStop;
		bool m_bHandleBGCuePrepared;
		bool m_bHandleBGCueStop;

		/// Banks pointers
		IXACTSoundBank* m_pSoundBank;
		IXACTWaveBank* m_pWaveBankInMemory;
		IXACTWaveBank* m_pWaveBankStream;

		/// if streaming wave bank prepared
		bool m_bStreamingWaveBankPrepared;

		/// Current background music cue index and the cue instance
		/// since ambient sound doesn't allow multi cue playing, only one pair is available
		XACTINDEX m_iCurrentBGMusic;
		IXACTCue* m_pCurrentBGMusic;

		/// Current dialog music cue index and the cue instance
		/// since ambient sound doesn't allow multi cue playing, only one pair is available
		XACTINDEX m_iCurrentDialog;
		IXACTCue* m_pCurrentDialog;

		/// code driven wave files
		map<string, IXACTWave*> m_codeDrivenFiles;
		
	public:
		IXACTWaveBank* GetWaveBankStream();
		IXACTCue* GetCurrentDialog();
		IXACTCue* GetCurrentBGMusic();
		void SetHandleStreamingWaveBankPrepared( bool bHandle );
		void SetHandleDialogCuePrepared( bool bHandle );
		void SetHandleDialogCueStop( bool bHandle );
		void SetHandleBGCuePrepared( bool bHandle );
		void SetHandleBGCueStop( bool bHandle );

		/**
		* Prepare and play a wave object from a standard PCM, XMA (Xbox 360) file.
		* If a wave file is already prepared before. It will be reused. 
		* @param szWavePath Path to the wave file. 
		* @param nLoopCount Number of times to loop this file. If there is no loop region, but the loop count is nonzero, the entire wave is looped.
		* @param wStreamingPacketSize Default it 0, Packet Size, in 2 KB increments. If wStreamingPacketSize is 0, 
		* the file is an in-memory wave. A fully prepared wave is returned. For streaming waves, 
		* the client needs to call IXACTWave::GetState to ensure that the wave is prepared before playback. 
		* Loop region information is read from the wave file. 
		* @param dwPlayOffset Play offset to use as the start of the wave. The offset can be described 
		* in milliseconds or in samples. The dwFlags argument determines the offset units. 
		* @param dwAlignment Alignment of wave data on the media from which it is streamed. This must be at least 2048. 
		* Otherwise, it must be a multiple of 2048. 
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT PlayWaveFile(PCSTR szWavePath,XACTLOOPCOUNT nLoopCount, WORD wStreamingPacketSize=0,DWORD dwPlayOffset=0, DWORD dwAlignment=2048);
		/**
		* stop a wave file
		* @param szWavePath Path to the wave file. 
		* @param dwFlags Flags that affect how the wave is stopped: 
		*   - XACT_FLAG_STOP_RELEASE Plays the wave to completion, then stops. For looping waves, this flag plays the current iteration to completion, then stops (ignoring any subsequent iterations). In either case, any release (or tail) is played. To stop the wave immediately, use the XACT_FLAG_STOP_IMMEDIATE flag. 
		*   - XACT_FLAG_STOP_IMMEDIATE Stops the playback immediately. 
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT StopWaveFile(PCSTR szWavePath, DWORD dwFlags);

		/**
		* release a wave file
		* @param szWavePath Path to the wave file. 
		* @return Returns S_OK if successful. Otherwise, an error code.
		*/
		HRESULT ReleaseWaveFile(PCSTR szWavePath);


		/** whether a given wave file is still in the code driven queue. We will enqueue a wave file, when it is played and deque it when it is stopped. 
		* if the wave file is not downloaded from the remote server, it will also be enqueued. 
		*/
		bool IsWaveFileInQueue(const string& filename);
	};
}


// Old Audio Engine Interface

//// --------- BackGround Music ---------
//// play background music
//HRESULT PlayBGMusic(const char * name);
//// enable background music
//HRESULT EnableBGMusic( );
//// disable background music
//HRESULT DisableBGMusic( );

//// --------- UserInterface Sound ---------
//HRESULT PlayUISound(const char * name);

// --------- Audio Engine ---------
// void EnableAudioEngine(bool enable);
// bool IsAudioEngineEnabled();

//HRESULT PlayAudioCue( XACTINDEX iCueIndex );
//HRESULT StopAudioCue( XACTINDEX iCueIndex );

//XACTINDEX GetCueIndexByName(const char * strName);