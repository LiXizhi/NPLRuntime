//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description:	API for Audio Engine. 
//-----------------------------------------------------------------------------
#pragma once
#include <string>

using namespace std;

namespace ParaScripting
{
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
		static void EnableAudioEngine();
		/// disable Audio Engine
		static void DisableAudioEngine();

		/** Set the volume of all categories and all currently playing wave files. 
		* @param fVolume: usually between [0,1], where 0 is silent and 1 is full. value larger than 1 is also possible. 
		*/
		static void SetVolume(float fVolume);

		/** Get the volume of average if all categories
		* @return usually between [0,1], where 0 is silent and 1 is full. value larger than 1 is also possible. 
		*/
		static float GetVolume();

		/// pause certain category in Audio Engine
		/// @param strCategoryName: category name
		static void PauseCategory(const char * strCategoryName);
		/// resume certain category in Audio Engine
		/// @param strCategoryName: category name
		static void ResumeCategory(const char * strCategoryName);
		/// stop certain category in Audio Engine
		/// @param strCategoryName: category name
		static void StopCategory(const char * strCategoryName);
		/** initialize the audio banks needed in Audio Engine
		* This function is only for backward compatible, use LoadSoundBank(), LoadInMemoryWaveBank(), and LoadStreamWaveBank() instead. 
		@param strBankName: sound and wave bank name. If the name is Name, then the following files are loaded automatically. 
		in memory wave bank: "audio/Name/Name-InMemory.xwb"
		stream wave bank: "audio/Name/Name-Stream.xwb"
		sound bank: "audio/Name/Haqi.xsb"
		*/
		static void EnableAudioBank(const char * strBankName);

		/** clean up the audio banks needed in Audio Engine
		@param strBankName: sound and wave bank name 
		*/
		static void DisableAudioBank(const char * strBankName);

		/** Load a sound bank. 
		* note: Currently only one can be loaded at a time, future version may support many. 
		* @param filename: the logical *.xsb sound bank file path. It can be an asset file. such as "audio/name/name.xsb"
		* @return: S_OK if loaded. E_PENDING if we are downloading. E_FAIL if cannot load. 
		*/
		static int LoadSoundBank(const char* filename);

		/** unload a sound bank */
		static int UnLoadSoundBank(const char* filename);

		/** Load an in memory wave bank. 
		* note: Currently only one can be loaded at a time, future version may support many. 
		* @param filename: the logical "*-InMemory.xwb". It can be an asset file. such as "audio/name/name-InMemory.xwb"
		* @return: S_OK if loaded. E_PENDING if we are downloading. E_FAIL if cannot load. 
		*/
		static int LoadInMemoryWaveBank(const char* filename);

		/** unload a wave bank */
		static int UnLoadInMemoryWaveBank(const char* filename);

		/** Load an stream wave bank. 
		* note: Currently only one can be loaded at a time, future version may support many. 
		* @param filename: the logical "*-Stream.xwb". It can be an asset file. such as "audio/name/name-Stream.xwb"
		* @return: S_OK if loaded. E_PENDING if we are downloading. E_FAIL if cannot load. 
		*/
		static int LoadStreamWaveBank(const char* filename);

		/** unload a wave bank */
		static int UnLoadStreamWaveBank(const char* filename);

		/// get the current audio bank name
		/// @return : current audio bank name
		static string GetCurrentAudioBankName();
		/// set listener's sound output format to stereo
		static void SetAudioStereo();
		/// set listener's sound output format to 5.1 sound
		static void SetAudio5Point1();
		/// get listener's sound output format
		/// @return: 2:Stereo 6:5Point1
		static int GetAudioFormat();

		/// ---------------- Background Functions ----------------
		/// play background music
		/// @param strBGMusicName: background music name
		static void PlayBGMusic( const char * strBGMusicName );
		/// pause background music
		/// @param strBGMusicName: background music name
		static void PauseBGMusic( );
		/// resume background music
		/// @param strBGMusicName: background music name
		static void ResumeBGMusic( );
		/// stop background music
		/// @param strBGMusicName: background music name
		static void StopBGMusic( );
		/// get is background music enabled
		static bool IsBGMusicEnabled();
		/// enable background music
		static void EnableBGMusic( );
		/// disable background music
		static void DisableBGMusic( );
		/// Get background music volume
		/// @param fVolume: background music volume
		static float GetBGMusicVolume( );
		/// set background music volume
		/// @param fVolume: background music volume
		static void SetBGMusicVolume( const float fVolume );

		/// ---------------- Dialog Functions ----------------
		/// play dialog music
		/// @param strDialogName: dialog music name
		static void PlayDialog( const char * strDialogName , const char * strScript );
		/// skip dialog music to the next marker
		static void SkipDialog( );
		/// pause dialog music
		static void PauseDialog( );
		/// resume dialog music
		static void ResumeDialog( );
		/// stop dialog music
		static void StopDialog( );
		/// get is dialog music enabled
		static bool IsDialogEnabled();
		/// enable dialog music
		static void EnableDialog( );
		/// disable dialog music
		static void DisableDialog( );
		/// get dialog volume
		/// @param fVolume: dialog volume
		static float GetDialogVolume( );
		/// set dialog volume
		/// @param fVolume: dialog volume
		static void SetDialogVolume( const float fVolume );

		/// ---------------- Ambient Sound Functions ----------------
		/// play ambient sound
		/// @param strAmbientSoundName: ambient sound name
		static void PlayAmbientSound( const char * strAmbientSoundName );
		/// pause ambient sound
		/// @param strAmbientSoundName: ambient sound name
		static void PauseAmbientSound( const char * strAmbientSoundName );
		/// resume ambient sound
		/// @param strAmbientSoundName: ambient sound name
		static void ResumeAmbientSound( const char * strAmbientSoundName );
		/// stop ambient sound
		/// @param strAmbientSoundName: ambient sound name
		static void StopAmbientSound( const char * strAmbientSoundName );
		/// get is ambient sound enabled
		static bool IsAmbientSoundEnabled();
		/// enable ambient sound music
		static void EnableAmbientSound( );
		/// disable ambient sound music
		static void DisableAmbientSound( );
		/// get ambient sound volume
		/// @param fVolume: ambient sound volume
		static float GetAmbientSoundVolume( );
		/// set ambient sound volume
		/// @param fVolume: ambient sound volume
		static void SetAmbientSoundVolume( const float fVolume );

		/// ---------------- UserInterface Sound Functions ----------------
		/// play UI sound
		/// @param strUISoundName: UI sound name
		static void PlayUISound( const char * strUISoundName );
		/// get UI sound volume
		/// @param fVolume: UI sound volume
		static float GetUISoundVolume( );
		/// set UI sound volume
		/// @param fVolume: UI sound volume
		static void SetUISoundVolume( const float fVolume );

		/// ---------------- 3D Sound Functions ----------------
		/// play 3D sound in static position
		/// @param str3DSoundName: 3D sound name
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		/// @param x, y, z: 3D sound position
		static void PlayStatic3DSound( const char * str3DSoundName, const char * assetName, float x, float y, float z);
		/// pause 3D sound in static position
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		static void PauseStatic3DSound( const char * assetName );
		/// Resume 3D sound in static position
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		static void ResumeStatic3DSound( const char * assetName );
		/// stop 3D sound in static position
		/// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
		static void StopStatic3DSound( const char * assetName );
		/// play 3D sound to dynamic object
		/// @param str3DSoundName: 3D sound name
		/// @param objectName: 3D sound object name, this is the KEY to the 3D Sound
		///			this is the name of the base class BaseObject
		static void PlayDynamic3DSound( const char * str3DSoundName, const char * objectName );
		/// get 3D sound volume
		/// @param fVolume: 3D sound volume
		static float Get3DSoundVolume( );
		/// set 3D sound volume
		/// @param fVolume: 3D sound volume
		static void Set3DSoundVolume( const float fVolume );

		/// ---------------- Interactive Sound Functions ----------------
		/// play Interactive sound
		/// @param strInteractiveSoundName: interactive sound name
		/// NOTE: Interactive sound is now behave like UI sound, and it don't promise Zero-Latency
		static void PlayInteractiveSound( const char * strInteractiveSoundName );
		/// get interactive sound volume
		/// @param fVolume: interactive sound volume
		static float GetInteractiveSoundVolume( );
		/// set interactive sound volume
		/// @param fVolume: interactive sound volume
		static void SetInteractiveSoundVolume( const float fVolume );

		//// play user interface sound
		//// @param strSoundAssetName: UI sound name
		//static void PlayUISound( const char * strSoundAssetName );

		//// enable audio engine according to param enable
		//static void EnableAudioEngine(bool enable);

		//// get is audio engine enabled
		//static bool IsAudioEngineEnabled();

		//////////////////////////////////////////////////////////////////////////
		//
		// code driven audio functions. for file based wave file only.
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* Prepare and play a wave object from a standard PCM, XMA (Xbox 360) file.
		* If a wave file is already prepared before. It will be reused. 
		* @param szWavePath Path to the wave file. 
		* @param nLoopCount Number of times to loop this file. If there is no loop region, but the loop count is nonzero, the entire wave is looped.
		*/
		static bool PlayWaveFile(const char* szWavePath,int nLoopCount);
		static bool PlayWaveFile1(const char* szWavePath);
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

		//////////////////////////////////////////////////////////////////////////
		//
		// recording related
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* enumerate the available recording devices in this machine. returned as strings separated by ';'
		* this function is not thread safe
		*/
		static const char* GetRecordingDeviceEnum();

		/**
		* enumerate the available recording formats on this machine. returned as strings separated by ';'
		* this function is not thread safe
		*/
		static const char* GetRecordingFormatEnum();

		/**
		* set the recording output properties for recording with BeginRecording() and StopRecording() function. 
		* @param sOutputFile: the output file. it can be nil. Default is "temp/soundcapture.wav",
		* @param nDeviceIndex: -1 to use the current one. or it should be a valid index as in GetRecordingDeviceEnum();
		* @param nWaveFormatIndex: -1 to use the current one. or it should be a valid index as in GetRecordingFormatEnum();
		*/
		static bool SetRecordingOutput(const char* sOutputFile, int nDeviceIndex, int nWaveFormatIndex);

		/** start recording to the current recording output */
		static bool BeginRecording();


		/**
		* @param nState: 0 for toggle pause and resume. -1 for pause, 1 for resume. 
		*/
		static bool PauseOrResumeRecording(int nState);


		/** stop recording and save output to the current recording output */
		static bool StopRecording();

		/** whether it is recording sound.*/
		static bool IsRecording();

		/** whether it is recording is paused.*/
		static bool IsRecordingPaused();

		/** try return a snapshot of the recorded wave data in the last nTimeLength time.
		* just turn the music to strings to be displayed by the GUI.
		*/
		static const char* GetLastRecordedData(int nTimeLength);
	};

}
