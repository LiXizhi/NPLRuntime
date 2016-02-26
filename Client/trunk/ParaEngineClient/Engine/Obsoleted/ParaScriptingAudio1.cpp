//-----------------------------------------------------------------------------
// Class:	ParaAudio
// Authors:	Andy Wang
// Emails:	mitnick_wang@hotmail.com
// Company: ParaEngine Dev Studio
// Date:	2006.8
// Desc: 
//-----------------------------------------------------------------------------
#ifdef USE_XACT_AUDIO
#include "ParaEngine.h"
#include "ParaScriptingAudio.h"
#include "AudioEngine.h"
#include "AudioEngine2.h"

using namespace ParaEngine;

namespace ParaScripting
{
	// -------------------------------------------------------
	// ---------------- Audio Engine Functions ----------------
	// -------------------------------------------------------

	bool ParaAudio::IsAudioEngineEnabled()
	{
		return CAudioEngine::GetInstance()->IsAudioEngineEnabled( );
	}
	void ParaAudio::EnableAudioEngine( )
	{
		CAudioEngine::GetInstance()->EnableAudioEngine( );
	}
	int ParaAudio::LoadStreamWaveBank(const char* filename)
	{
		return CAudioEngine::GetInstance()->LoadStreamWaveBank(filename);
	}
	int ParaAudio::LoadInMemoryWaveBank(const char* filename)
	{
		return CAudioEngine::GetInstance()->LoadInMemoryWaveBank(filename);
	}
	int ParaAudio::LoadSoundBank(const char* filename)
	{
		return CAudioEngine::GetInstance()->LoadSoundBank(filename);
	}

	int ParaAudio::UnLoadStreamWaveBank(const char* filename)
	{
		return CAudioEngine::GetInstance()->UnLoadStreamWaveBank(filename);
	}
	int ParaAudio::UnLoadInMemoryWaveBank(const char* filename)
	{
		return CAudioEngine::GetInstance()->UnLoadInMemoryWaveBank(filename);
	}
	int ParaAudio::UnLoadSoundBank(const char* filename)
	{
		return CAudioEngine::GetInstance()->UnLoadSoundBank(filename);
	}

	void ParaAudio::DisableAudioEngine( )
	{
		CAudioEngine::GetInstance()->DisableAudioEngine();
	}

	float ParaAudio::GetVolume()
	{
		return CAudioEngine::GetInstance()->GetVolume();
	}

	void ParaAudio::SetVolume(float fVolume)
	{
		CAudioEngine::GetInstance()->SetVolume(fVolume);
	}

	void ParaAudio::PauseCategory(const char * strCategoryName)
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PauseCategory(strCategoryName) ) )
		{
			OUTPUT_LOG("PauseCategory(%s) failed!\n", strCategoryName);
		}
	}

	void ParaAudio::ResumeCategory(const char * strCategoryName)
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->ResumeCategory(strCategoryName) ) )
		{
			OUTPUT_LOG("ResumeCategory(%s) failed!\n", strCategoryName);
		}
	}

	void ParaAudio::StopCategory(const char * strCategoryName)
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->StopCategory(strCategoryName) ) )
		{
			OUTPUT_LOG("StopCategory(%s) failed!\n", strCategoryName);
		}
	}

	void ParaAudio::EnableAudioBank(const char * strBankName)
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->EnableAudioBank(strBankName) ) )
		{
			OUTPUT_LOG("EnableAudioBank(%s) failed!\n", strBankName);
		}
	}

	void ParaAudio::DisableAudioBank(const char * strBankName)
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->DisableAudioBank(strBankName) ) )
		{
			OUTPUT_LOG("DisableAudioBank(%s) failed!\n", strBankName);
		}
	}

	string ParaAudio::GetCurrentAudioBankName()
	{
		return CAudioEngine::GetInstance()->GetCurrentAudioBankName( );
	}

	// set listener's sound output format to stereo
	void ParaAudio::SetAudioStereo()
	{
	}
	// set listener's sound output format to 5.1 sound
	void ParaAudio::SetAudio5Point1()
	{
	}
	// get listener's sound output format
	int ParaAudio::GetAudioFormat()
	{
		return 2;
	}

	// -------------------------------------------------------
	// ---------------- Background Functions ----------------
	// -------------------------------------------------------

	void ParaAudio::PlayBGMusic( const char * strBGMusicName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayBGMusic(strBGMusicName) ) )
		{
			OUTPUT_LOG("PlayBGMusic() fail. Music didn't found: %s.\n", strBGMusicName);
		}
	}

	void ParaAudio::PauseBGMusic( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PauseBGMusic() ) )
		{
			OUTPUT_LOG("PauseBGMusic() fail.\n");
		}
	}

	void ParaAudio::ResumeBGMusic( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->ResumeBGMusic( ) ) )
		{
			OUTPUT_LOG("ResumeBGMusic() fail.\n");
		}
	}

	void ParaAudio::StopBGMusic( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->StopBGMusic( ) ) )
		{
			OUTPUT_LOG("StopBGMusic() fail.\n");
		}
	}

	bool ParaAudio::IsBGMusicEnabled()
	{
		return CAudioEngine::GetInstance()->IsBGMusicEnabled( );
	}

	void ParaAudio::EnableBGMusic( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->EnableBGMusic( ) ) )
		{
			OUTPUT_LOG("EnableBGMusic() fail.\n");
		}
	}

	void ParaAudio::DisableBGMusic( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->DisableBGMusic( ) ) )
		{
			OUTPUT_LOG("DisableBGMusic() fail.\n");
		}
	}

	float ParaAudio::GetBGMusicVolume( )
	{
		return CAudioEngine::GetInstance()->GetBGMusicVolume( );
	}

	void ParaAudio::SetBGMusicVolume( const float fVolume )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->SetBGMusicVolume(fVolume) ) )
		{
			OUTPUT_LOG("SetBGMusicVolume() fail.\n");
		}
	}

	// -------------------------------------------------------
	// ---------------- Dialog Functions ----------------
	// -------------------------------------------------------

	void ParaAudio::PlayDialog( const char * strDialogName , const char * strScript )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayDialog(strDialogName, strScript) ) )
		{
			OUTPUT_LOG("PlayDialog() fail. Music didn't found: %s.\n", strDialogName);
		}
	}

	void ParaAudio::SkipDialog( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->SkipDialog( ) ) )
		{
			OUTPUT_LOG("SkipDialog() fail.\n");
		}
	}

	void ParaAudio::PauseDialog( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PauseDialog( ) ) )
		{
			OUTPUT_LOG("PauseDialog() fail.\n");
		}
	}

	void ParaAudio::ResumeDialog( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->ResumeDialog( ) ) )
		{
			OUTPUT_LOG("ResumeDialog() fail.\n");
		}
	}

	void ParaAudio::StopDialog( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->StopDialog( ) ) )
		{
			OUTPUT_LOG("StopDialog() fail.\n");
		}
	}

	bool ParaAudio::IsDialogEnabled()
	{
		return CAudioEngine::GetInstance()->IsDialogEnabled( );
	}

	void ParaAudio::EnableDialog( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->EnableDialog( ) ) )
		{
			OUTPUT_LOG("EnableDialog() fail.\n");
		}
	}

	void ParaAudio::DisableDialog( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->DisableDialog( ) ) )
		{
			OUTPUT_LOG("DisableDialog() fail.\n");
		}
	}

	float ParaAudio::GetDialogVolume( )
	{
		return CAudioEngine::GetInstance()->GetDialogVolume( );
	}

	void ParaAudio::SetDialogVolume( const float fVolume )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->SetDialogVolume( fVolume ) ) )
		{
			OUTPUT_LOG("SetDialogVolume() fail.\n");
		}
	}

	// -------------------------------------------------------
	// ---------------- Ambient Sound Functions ----------------
	// -------------------------------------------------------

	void ParaAudio::PlayAmbientSound( const char * strAmbientSoundName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayAmbientSound(strAmbientSoundName) ) )
		{
			OUTPUT_LOG("PlayAmbientSound(%s) fail.\n", strAmbientSoundName);
		}
	}

	void ParaAudio::PauseAmbientSound( const char * strAmbientSoundName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PauseAmbientSound(strAmbientSoundName) ) )
		{
			OUTPUT_LOG("PauseAmbientSound(%s) fail.\n", strAmbientSoundName);
		}
	}

	void ParaAudio::ResumeAmbientSound( const char * strAmbientSoundName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->ResumeAmbientSound(strAmbientSoundName) ) )
		{
			OUTPUT_LOG("ResumeAmbientSound(%s) fail.\n", strAmbientSoundName);
		}
	}

	void ParaAudio::StopAmbientSound( const char * strAmbientSoundName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->StopAmbientSound(strAmbientSoundName) ) )
		{
			OUTPUT_LOG("StopAmbientSound(%s) fail.\n", strAmbientSoundName);
		}
	}

	bool ParaAudio::IsAmbientSoundEnabled()
	{
		return CAudioEngine::GetInstance()->IsAmbientSoundEnabled( );
	}

	void ParaAudio::EnableAmbientSound( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->EnableAmbientSound( ) ) )
		{
			OUTPUT_LOG("EnableAmbientSound() fail.\n");
		}
	}

	void ParaAudio::DisableAmbientSound( )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->DisableAmbientSound( ) ) )
		{
			OUTPUT_LOG("DisableAmbientSound() fail.\n");
		}
	}

	float ParaAudio::GetAmbientSoundVolume( )
	{
		return CAudioEngine::GetInstance()->GetAmbientSoundVolume( );
	}

	void ParaAudio::SetAmbientSoundVolume( const float fVolume )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->SetAmbientSoundVolume( fVolume ) ) )
		{
			OUTPUT_LOG("SetAmbientSoundVolume(%f) fail.\n", fVolume);
		}
	}

	// ----------------------------------------------------------------
	// ---------------- UserInterface Sound Functions ----------------
	// ----------------------------------------------------------------
	void ParaAudio::PlayUISound( const char * strUISoundName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayUISound(strUISoundName) ) )
		{
			OUTPUT_LOG("PlayUISound(%s) fail!\n", strUISoundName);
		}
	}

	float ParaAudio::GetUISoundVolume( )
	{
		return CAudioEngine::GetInstance()->GetUISoundVolume();
	}

	void ParaAudio::SetUISoundVolume( const float fVolume )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->SetUISoundVolume( fVolume ) ) )
		{
			OUTPUT_LOG("SetUISoundVolume(%f) fail!\n", fVolume);
		}
	}

	// ----------------------------------------------------------------
	// ---------------- 3D Sound Functions ----------------
	// ----------------------------------------------------------------

	void ParaAudio::PlayStatic3DSound( const char * str3DSoundName, const char * assetName, float x, float y, float z)
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayStatic3DSound(str3DSoundName, assetName, x, y, z) ) )
		{
			OUTPUT_LOG("PlayStatic3DSound(%s, %s, %f, %f, %f) fail!\n", str3DSoundName, assetName, x, y, z);
		}
	}

	void ParaAudio::PauseStatic3DSound( const char * assetName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PauseStatic3DSound(assetName) ) )
		{
			OUTPUT_LOG("PauseStatic3DSound(%s) fail!\n", assetName);
		}
	}

	void ParaAudio::ResumeStatic3DSound( const char * assetName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->ResumeStatic3DSound(assetName) ) )
		{
			OUTPUT_LOG("ResumeStatic3DSound(%s) fail!\n", assetName);
		}
	}

	void ParaAudio::StopStatic3DSound( const char * assetName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->StopStatic3DSound(assetName) ) )
		{
			OUTPUT_LOG("StopStatic3DSound(%s) fail!\n", assetName);
		}
	}

	void ParaAudio::PlayDynamic3DSound( const char * str3DSoundName, const char * objectName )
	{
		// TODO: further implementation to integrate to ParaEngine
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayDynamic3DSound(str3DSoundName, objectName) ) )
		{
			OUTPUT_LOG("PlayDynamic3DSound(%s, %s) fail!\n", str3DSoundName, objectName);
		}
	}

	float ParaAudio::Get3DSoundVolume( )
	{
		return CAudioEngine::GetInstance()->Get3DSoundVolume();
	}

	void ParaAudio::Set3DSoundVolume( const float fVolume )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->Set3DSoundVolume( fVolume ) ) )
		{
			OUTPUT_LOG("Set3DSoundVolume(%f) fail!\n", fVolume);
		}
	}

	// ----------------------------------------------------------------
	// ---------------- Interactive Sound Functions ----------------
	// ----------------------------------------------------------------

	void ParaAudio::PlayInteractiveSound( const char * strInteractiveSoundName )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->PlayInteractiveSound(strInteractiveSoundName) ) )
		{
			OUTPUT_LOG("PlayInteractiveSound(%s) fail!\n", strInteractiveSoundName);
		}
	}

	float ParaAudio::GetInteractiveSoundVolume( )
	{
		return CAudioEngine::GetInstance()->GetInteractiveSoundVolume();
	}

	void ParaAudio::SetInteractiveSoundVolume( const float fVolume )
	{
		if( CAudioEngine::GetInstance()->IsValidAndEnabled() && FAILED( CAudioEngine::GetInstance()->SetInteractiveSoundVolume( fVolume ) ) )
		{
			OUTPUT_LOG("SetInteractiveSoundVolume(%f) fail!\n", fVolume);
		}
	}

	bool ParaAudio::PlayWaveFile( const char* szWavePath,int nLoopCount )
	{
		/*if( FAILED(CAudioEngine::GetInstance()->PlayWaveFile(szWavePath, nLoopCount)))
		{
		return false;
		}*/
		if(FAILED(CAudioEngine2::GetInstance()->PlayWaveFile(szWavePath, nLoopCount>0)))
		{
			return false;
		}
		return true;
	}

	bool ParaAudio::PlayWaveFile1( const char* szWavePath )
	{
		return PlayWaveFile(szWavePath, 0);
	}

	bool ParaAudio::StopWaveFile( const char* szWavePath, bool bImmediateStop )
	{
		/*if( FAILED(CAudioEngine::GetInstance()->StopWaveFile(szWavePath, bImmediateStop?XACT_FLAG_STOP_IMMEDIATE:XACT_FLAG_STOP_RELEASE)))
		{
			return false;
		}*/
		if(FAILED(CAudioEngine2::GetInstance()->StopWaveFile(szWavePath, bImmediateStop ? AUDIO_FLAG_STOP_IMMEDIATE : AUDIO_FLAG_STOP_RELEASE)))
		{
			return false;
		}
		return true;
	}

	bool ParaAudio::StopWaveFile1( const char* szWavePath )
	{
		return StopWaveFile(szWavePath, true);
	}

	bool ParaAudio::ReleaseWaveFile( const char* szWavePath )
	{
		/*if( FAILED(CAudioEngine::GetInstance()->ReleaseWaveFile(szWavePath)))
		{
			return false;
		}*/
		if(FAILED(CAudioEngine2::GetInstance()->ReleaseWaveFile(szWavePath)))
		{
			return false;
		}
		return true;
	}

	const char* ParaAudio::GetRecordingDeviceEnum()
	{
		return CAudioEngine::GetInstance()->GetRecordingDeviceEnum();
	}

	const char* ParaAudio::GetRecordingFormatEnum()
	{
		return CAudioEngine::GetInstance()->GetRecordingFormatEnum();
	}

	bool ParaAudio::SetRecordingOutput( const char* sOutputFile, int nDeviceIndex, int nWaveFormatIndex )
	{
		return CAudioEngine::GetInstance()->SetRecordingOutput(sOutputFile, nDeviceIndex, nWaveFormatIndex);
	}

	bool ParaAudio::BeginRecording()
	{
		return CAudioEngine::GetInstance()->BeginRecording();
	}

	bool ParaAudio::PauseOrResumeRecording( int nState )
	{
		return CAudioEngine::GetInstance()->PauseOrResumeRecording(nState);
	}

	bool ParaAudio::StopRecording()
	{
		return CAudioEngine::GetInstance()->StopRecording();
	}

	bool ParaAudio::IsRecording()
	{
		return CAudioEngine::GetInstance()->IsRecording();
	}

	bool ParaAudio::IsRecordingPaused()
	{
		return CAudioEngine::GetInstance()->IsRecordingPaused();
	}

	const char* ParaAudio::GetLastRecordedData( int nTimeLength )
	{
		return CAudioEngine::GetInstance()->GetLastRecordedData(nTimeLength);
	}
}
#endif