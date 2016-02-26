//---------------------------------------
// File: AudioEngine.h
// Author: WangTian
//
// Desc: Implement ParaEngine Audio Engine
// Email: mitnick_wang@hotmail.com
//			wangtian@cad.zju.edu.cn
// Date: 2006.8
//---------------------------------------
#ifdef USE_XACT_AUDIO_ENGINE
#include "dxstdafx.h"
#include "ParaEngine.h"
#include "SceneObject.h"
#include "OceanManager.h"
#include "BipedObject.h"
#include "AutoCamera.h"
#include "AssetManifest.h"
#include "ParaEngineSettings.h"

#include "AudioEngine.h"
#include "util/mutex.h"
#include <process.h>
#include <strsafe.h>

using namespace ParaEngine;

// this may be used, since I use a different thread for recording data. 
// However it is not needed, since I call StopRecording() in the beginning of every recording function, which stops the second thread. 
ParaEngine::mutex	g_SoundCaptureMutex;

/** we will terminate the sound recording if it has been too long, such as 200 seconds. 
* maybe, just in case, the user forget to stop recording. 
*/
const int MAX_RECORDSOUND_LOOP_COUNT = 1000*8;

/** whether it is recording or not. */
BOOL g_bIsRecording = FALSE;

#define NOTIFY_RECORDDATA_EVENT_INDEX	0
#define NOTIFY_STOP_EVENT_INDEX	1
#define NOTIFY_EVENT_NUM	2
HANDLE  g_hNotificationEvents[NOTIFY_EVENT_NUM];

vector<string> g_captureDeviceList;
GUID  g_AudioDriverGUIDs[20];

void WINAPI AudioEngineNotificationCallback(const XACT_NOTIFICATION* pNotification);

CAudioEngine::CAudioEngine(void)
{
	// Banks
	m_pSoundBank = NULL;
	m_pWaveBankInMemory = NULL;
	m_pWaveBankStream = NULL;
	m_bStreamingWaveBankPrepared = false;

	// Engine
	m_pEngine = NULL;

	m_pbWaveBankInMemory = NULL;
	m_pbSoundBank = NULL;

	m_bEnableAudioEngine = true;
	m_bEnableBGMusic = true;
	m_bEnableDialogMusic = true;
	m_bEnableAmbientSound = true;
	m_bEnableUISound = true;
	m_bEnable3DSound = true;
	m_bEnableInteractiveSound = true;

	ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
	float fVolumeDefault = 1.f;
	CDynamicAttributeField* pField = NULL;
	if((pField = settings.GetDynamicField("SoundVolume")))
	{
		fVolumeDefault = (float)(*pField);
	}

	m_fBGMusicVolume = fVolumeDefault;
	m_fDialogMusicVolume = fVolumeDefault;
	m_fAmbientSoundVolume = fVolumeDefault;
	m_fUISoundVolume = fVolumeDefault;
	m_f3DSoundVolume = fVolumeDefault;
	m_fInteractiveSoundVolume = fVolumeDefault;

	m_strCurrentAudioBankName = "";
	m_bInitializeBanks = false;

	m_iCurrentBGMusic = XACTINDEX_INVALID;
	m_pCurrentBGMusic = NULL;

	m_iCurrentDialog = XACTINDEX_INVALID;
	m_pCurrentDialog = NULL;

	m_bHandleStreamingWaveBankPrepared = false;
	m_bHandleDialogCuePrepared = false;
	m_bHandleDialogCueStop = false;

	m_bHandleBGCuePrepared = false;
	m_bHandleBGCueStop = false;

	//////////////////////////////////////////////////////////////////////////
	// for sound capture
	m_pDSCapture = NULL;
	m_pDSBCapture = NULL;
	m_pDSNotify = NULL;
	m_nCaptureDeviceIndex = 0;
	m_sCapureFileName = "temp/capturedsound.wav";
	
	m_guidCaptureDevice = GUID_NULL;
	m_bRecording = false;
	m_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ];  
	m_abInputFormatSupported[20];
	m_dwCaptureBufferSize;
	m_dwNextCaptureOffset;
	m_dwNotifySize;
	m_pWaveFile = NULL;
	m_bCaptureDeviceAvailable = true;
	m_hRecordingThread = NULL;

	m_hStreamingWaveBankFile = NULL;
}

CAudioEngine::~CAudioEngine(void)
{
	ReleaseSoundCapture();
	SAFE_DELETE_ARRAY(m_pbSoundBank);
}


void CAudioEngine::CleanupAudioEngine()
{
	if( m_pEngine )
	{
		// Note: No Release() functions?
		//SAFE_RELEASE(m_pSoundBank);
		//SAFE_RELEASE(m_pWaveBankInMemory);
		//SAFE_RELEASE(m_pWaveBankStream);
		m_pSoundBank = NULL;
		m_pWaveBankInMemory = NULL;
		m_pWaveBankStream = NULL;

		m_pEngine->ShutDown();
		m_pEngine->Release();
		m_pEngine = NULL;
	}
	m_codeDrivenFiles.clear();

	SAFE_DELETE_ARRAY(m_pbSoundBank )

		// After pEngine->ShutDown() returns it is safe to release memory mapped files
		if( m_pbWaveBankInMemory )
			UnmapViewOfFile( m_pbWaveBankInMemory );
	m_pbWaveBankInMemory = NULL;

	// After pEngine->ShutDown() returns, it is safe to release audio file memory
	if( m_hStreamingWaveBankFile != INVALID_HANDLE_VALUE && m_hStreamingWaveBankFile != NULL )
		CloseHandle( m_hStreamingWaveBankFile );

	m_bStreamingWaveBankPrepared = false;
}

CAudioEngine * CAudioEngine::GetInstance()
{
	static CAudioEngine g_singletonAudioEngine;
	return &g_singletonAudioEngine;
}

HRESULT CAudioEngine::InitAudioEngine()
{
	HRESULT hr = E_FAIL;
	ParaEngine::Lock lock_(m_mutex);

	// Switch to auditioning mode based on command line.  Change if desired
	bool bAuditionMode = false; 
	bool bDebugMode = false;

	DWORD dwCreationFlags = 0;
	if( bAuditionMode ) dwCreationFlags |= XACT_FLAG_API_AUDITION_MODE;
	if( bDebugMode ) dwCreationFlags |= XACT_FLAG_API_DEBUG_MODE;

	hr = XACTCreateEngine( dwCreationFlags, &m_pEngine );

	if( FAILED(hr) || m_pEngine == NULL )
	{
		m_pEngine = NULL;
		return E_FAIL;
	}

	// Load the global settings file and pass it into XACTInitialize
	VOID* pGlobalSettingsData = NULL;
	DWORD dwGlobalSettingsFileSize = 0;
	bool bSuccess = false;
	DWORD dwBytesRead = 0;
	{
		CParaFile file("Audio/Global.xgs");
		if(!file.isEof())
		{
			dwGlobalSettingsFileSize = (DWORD)file.getSize();
			// Using CoTaskMemAlloc so that XACT can clean up this data when its done
			pGlobalSettingsData = CoTaskMemAlloc(dwGlobalSettingsFileSize);
			if( pGlobalSettingsData )
			{
				dwBytesRead = (DWORD)file.read(pGlobalSettingsData, dwGlobalSettingsFileSize);
				if( dwBytesRead > 0)
				{
					bSuccess = true;
				}
			}
		}
	}
	
	if( !bSuccess )
	{
		if( pGlobalSettingsData )
			CoTaskMemFree(pGlobalSettingsData);
		pGlobalSettingsData = NULL;
		dwGlobalSettingsFileSize = 0;
	}


	// Initialize & create the XACT runtime 
	XACT_RUNTIME_PARAMETERS xrParams = {0};
	xrParams.pGlobalSettingsBuffer = pGlobalSettingsData;
	xrParams.globalSettingsBufferSize = dwGlobalSettingsFileSize;
	xrParams.lookAheadTime = 250;
	xrParams.fnNotificationCallback = AudioEngineNotificationCallback;
	hr = m_pEngine->Initialize( &xrParams );
	if( FAILED(hr) )
		return hr;

	// The "wave bank prepared" notification will let the app know when it is save to use
	// play cues that reference streaming wave data.
	XACT_NOTIFICATION_DESCRIPTION desc = {0};
	desc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	desc.type = XACTNOTIFICATIONTYPE_WAVEBANKPREPARED;
	m_pEngine->RegisterNotification(&desc);

	// The "cue stop" notification will let the app know when it a song stops so a new one 
	// can be played
	desc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	desc.type = XACTNOTIFICATIONTYPE_CUESTOP;
	desc.cueIndex = XACTINDEX_INVALID;
	m_pEngine->RegisterNotification(&desc);

	// The "cue prepared" notification will let the app know when it a a cue that uses 
	// streaming data has been prepared so it is ready to be used for zero latency streaming
	desc.flags = XACT_FLAG_NOTIFICATION_PERSIST;
	desc.type = XACTNOTIFICATIONTYPE_CUEPREPARED;
	desc.cueIndex = XACTINDEX_INVALID;
	m_pEngine->RegisterNotification(&desc);

	// Setup 3D audio struct
	ZeroMemory( &m_listener, sizeof(X3DAUDIO_LISTENER) );
	m_listener.OrientFront = Vector3(0,0,1);
	m_listener.OrientTop = Vector3(0,1,0);
	m_listener.Position = Vector3(0,0,0);
	m_listener.Velocity = Vector3(0,0,0);

	ZeroMemory( &m_emitter, sizeof(X3DAUDIO_EMITTER) );
	m_emitter.pCone = NULL;
	m_emitter.OrientFront = Vector3(0,0,1);
	m_emitter.OrientTop = Vector3(0,1,0);
	m_emitter.Position = Vector3(0,0,0); 
	m_emitter.Velocity = Vector3(0,0,0);
	m_emitter.ChannelCount = 2;
	m_emitter.ChannelRadius = 1.0f;
	m_emitter.pChannelAzimuths = NULL;
	m_emitter.pVolumeCurve = NULL;
	m_emitter.pLFECurve = NULL;
	m_emitter.pLPFDirectCurve = NULL;
	m_emitter.pLPFReverbCurve = NULL;
	m_emitter.pReverbCurve = NULL;
	m_emitter.CurveDistanceScaler = 1.0f;
	m_emitter.DopplerScaler = NULL;

	m_DelayTimes[0] = 0.0f;
	m_DelayTimes[1] = 0.0f;
	m_MatrixCoefficients[0] = 1.0f;
	m_MatrixCoefficients[1] = 0.0f;
	m_MatrixCoefficients[2] = 1.0f;
	m_MatrixCoefficients[3] = 0.0f;

	ZeroMemory( &m_dspSettings, sizeof(X3DAUDIO_DSP_SETTINGS) );
	m_dspSettings.pMatrixCoefficients = m_MatrixCoefficients;
	m_dspSettings.pDelayTimes = m_DelayTimes;
	m_dspSettings.SrcChannelCount = 2;
	m_dspSettings.DstChannelCount = 2;

	// Initialize 3D settings
	hr = XACT3DInitialize( m_pEngine, m_x3DInstance );
	if ( FAILED(hr) )
		return E_FAIL;

	m_bEnableAudioEngine = true;

	// set initial volume
	SetVolume(GetVolume());
	return S_OK;
}

void CAudioEngine::SetVolume(float fVolume)
{
	SetBGMusicVolume(fVolume);
	SetDialogVolume(fVolume);
	SetAmbientSoundVolume(fVolume);
	SetUISoundVolume(fVolume);
	Set3DSoundVolume(fVolume);
	SetInteractiveSoundVolume(fVolume);
}

float CAudioEngine::GetVolume()
{
	return (GetBGMusicVolume() + GetUISoundVolume()) / 2.f;
}

void WINAPI AudioEngineNotificationCallback(const XACT_NOTIFICATION* pNotification)
{
	if( pNotification->type == XACTNOTIFICATIONTYPE_WAVEBANKPREPARED &&             
		pNotification->waveBank.pWaveBank == CAudioEngine::GetInstance()->GetWaveBankStream() )
	{
		// Respond to this notification outside of this callback so Prepare() can be called
		CAudioEngine::GetInstance()->SetHandleStreamingWaveBankPrepared(true);
	}

	if( pNotification->type == XACTNOTIFICATIONTYPE_CUEPREPARED ) 
	{
		if ( pNotification->cue.pCue == CAudioEngine::GetInstance()->GetCurrentDialog() )
		{
			CAudioEngine::GetInstance()->SetHandleDialogCuePrepared(true);
		}

		if ( pNotification->cue.pCue == CAudioEngine::GetInstance()->GetCurrentBGMusic() )
		{
			CAudioEngine::GetInstance()->SetHandleBGCuePrepared(true);
		}
	}

	if( pNotification->type == XACTNOTIFICATIONTYPE_CUESTOP )
	{
		if ( pNotification->cue.pCue == CAudioEngine::GetInstance()->GetCurrentDialog() )
		{
			CAudioEngine::GetInstance()->SetHandleDialogCueStop(true);
		}

		if ( pNotification->cue.pCue == CAudioEngine::GetInstance()->GetCurrentBGMusic() )
		{
			CAudioEngine::GetInstance()->SetHandleBGCueStop(true);
		}
	}
}
void CAudioEngine::UpdateAudio()
{
	if(!IsValidAndEnabled())
		return;
	ParaEngine::Lock lock_(m_mutex);
	bool bHandleStreamingWaveBankPrepared = m_bHandleStreamingWaveBankPrepared;
	bool bHandleDialogCuePrepared = m_bHandleDialogCuePrepared;
	bool bHandleDialogCueStop = m_bHandleDialogCueStop;
	bool bHandleBGCuePrepared = m_bHandleBGCuePrepared;
	bool bHandleBGCueStop = m_bHandleBGCueStop;
	lock_.unlock();
	

	if( bHandleStreamingWaveBankPrepared )
	{
		SetHandleStreamingWaveBankPrepared(false);

		// Prepare a new cue for zero-latency playback now that the wave bank is prepared
		if(m_iCurrentDialog != XACTINDEX_INVALID)
		{
			if(m_pSoundBank->Prepare( m_iCurrentDialog, 0, 0, &m_pCurrentDialog ) != S_OK)
			{
				m_iCurrentDialog = XACTINDEX_INVALID;
			}
		}

		if(m_iCurrentBGMusic != XACTINDEX_INVALID)
		{
			if(m_pSoundBank->Prepare( m_iCurrentBGMusic, 0, 0, &m_pCurrentBGMusic ) != S_OK)
			{
				m_iCurrentDialog = XACTINDEX_INVALID;
			}
		}
		m_bStreamingWaveBankPrepared = true;
	}
	if( bHandleDialogCuePrepared )
	{
		SetHandleDialogCuePrepared(false);

		// play the dialog cue
		m_pCurrentDialog->Play();
	}

	if( bHandleDialogCueStop )
	{
		SetHandleDialogCueStop(false);

		// Destroy the cue when it stops
		m_pCurrentDialog->Destroy();
		m_pCurrentDialog = NULL;
		m_iCurrentDialog = XACTINDEX_INVALID;
	}

	if(bHandleBGCuePrepared)
	{
		SetHandleBGCuePrepared(false);
		
		// play the background cue
		m_pCurrentBGMusic->Play();
	}

	if(bHandleBGCueStop)
	{
		SetHandleBGCueStop(m_bHandleBGCueStop);

		// Destroy the cue when it stops
		m_pCurrentBGMusic->Destroy();
		m_pCurrentBGMusic = NULL;
		m_iCurrentBGMusic = XACTINDEX_INVALID;
	}

	if( IsAudioEngineEnabled() && m_bEnable3DSound && m_nFrameToApply3DAudio == 0 )
	{
		m_listener.OrientFront = CGlobals::GetScene()->GetCurrentCamera()->GetWorldAhead();
		m_listener.OrientTop = CGlobals::GetScene()->GetCurrentCamera()->GetWorldUp();
		m_listener.Position = CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();

		// update sound settings when the listener position changes. 
		
		for(POOL_3DCUE_TYPE::iterator itCur = m_pool_3D_Static_Cues.begin(); itCur!=m_pool_3D_Static_Cues.end(); )
		{
			bool bSucceed = false;
			IXACTCue* pCue = itCur->second.pCue;
			if(pCue)
			{
				DWORD dwState = 0;
				if(pCue->GetState(&dwState) == S_OK)
				{
					if(dwState != XACT_CUESTATE_STOPPED)
					{
						m_emitter.Position = itCur->second.vEmitterPosition;
						Vector3 v, v1, v2;
						v1.x = m_listener.Position.x;
						v1.y = m_listener.Position.y;
						v1.z = m_listener.Position.z;
						v2.x = m_emitter.Position.x;
						v2.y = m_emitter.Position.y;
						v2.z = m_emitter.Position.z;
						v = v1 - v2;
						float dis = v.length();
						m_dspSettings.EmitterToListenerDistance = dis;
						XACT3DCalculate( m_x3DInstance, &m_listener, &m_emitter, &m_dspSettings );
						if(XACT3DApply( &m_dspSettings, pCue ) == S_OK)
						{
							bSucceed = true;
						}
					}
				}
			}
			if(bSucceed)
				itCur++;
			else
			{
				if(pCue)
				{
					pCue->Destroy();
				}
				itCur = m_pool_3D_Static_Cues.erase(itCur);
			}
		}
	}
	m_nFrameToApply3DAudio++;
	m_nFrameToApply3DAudio %= 2;
}



//--------------------------------------------------------------------------------------
// Helper function to try to find the location of a media file
//--------------------------------------------------------------------------------------
HRESULT CAudioEngine::FindMediaFileCch( CHAR* strDestPath, int cchDest, LPCSTR strFilename )
{
	OUTPUT_LOG("function deprecated");
	return E_FAIL;
}

bool CAudioEngine::IsValid()
{
	return (m_pEngine!=0);
}

void CAudioEngine::DoWork()
{
	if(IsAudioEngineEnabled() && IsValid())
	{
		UpdateAudio();

		PlayAmbientWater();
		m_pEngine->DoWork();
	}
}

HRESULT CAudioEngine::PlayAmbientWater()
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	// TODO: does this function has to be called each frame?
	static bool bAmbientWaterPlaying = false;

	float waterHeight = 0.0f;
	float characterHeight = 0.0f;
	if (CGlobals::GetOceanManager()->OceanEnabled())
	{
		if(CGlobals::GetScene()->GetCurrentPlayer())
		{
			waterHeight = CGlobals::GetOceanManager()->GetWaterLevel();
			characterHeight = CGlobals::GetScene()->GetCurrentPlayer()->GetPosition().y;
			if(characterHeight - waterHeight < 0.2)
			{
				if(!bAmbientWaterPlaying )
				{
					PlayAmbientSound("Amb_Water");
					bAmbientWaterPlaying  = true;
				}
			}
			else
			{
				if(bAmbientWaterPlaying)
				{
					StopAmbientSound("Amb_Water");
					bAmbientWaterPlaying  = false;
				}
			}
			return S_OK;
		}
		else
			return S_FALSE;
	}
	else
	{
		if(bAmbientWaterPlaying)
		{
			StopAmbientSound("Amb_Water");
			bAmbientWaterPlaying  = false;
		}
	}
	return S_OK;
}

// -------------------------------------------------------
//                        Audio Engine Functions
// -------------------------------------------------------

bool CAudioEngine::IsAudioEngineEnabled()
{
	return m_bEnableAudioEngine;
}
HRESULT CAudioEngine::EnableAudioEngine( )
{
	m_bEnableAudioEngine = true;
	return S_OK;
}
HRESULT CAudioEngine::DisableAudioEngine( )
{
	m_bEnableAudioEngine = false;

	return S_OK;
}
HRESULT CAudioEngine::PauseCategory(const char * strCategoryName)
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	XACTCATEGORY category;
	category = m_pEngine->GetCategory(strCategoryName);
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->Pause(category, true);
	return S_FALSE;
}

HRESULT CAudioEngine::ResumeCategory(const char * strCategoryName)
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	XACTCATEGORY category;
	category = m_pEngine->GetCategory(strCategoryName);
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->Pause(category, false);
	return S_FALSE;
}

HRESULT CAudioEngine::StopCategory(const char * strCategoryName)
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	XACTCATEGORY category;
	category = m_pEngine->GetCategory(strCategoryName);
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->Stop(category, 0);
		//return m_pEngine->Stop(category, XACT_FLAG_API_STOP_IMMEDIATE);
	return S_FALSE;
}

/** Async wave file call back. */
class CSoundBankCallBackData
{
public:
	CSoundBankCallBackData(const char* sFileName):m_sFileName(sFileName){}

	std::string m_sFileName;
	
	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if(nResult == 0)
		{
			CAudioEngine::GetInstance()->LoadSoundBank(m_sFileName.c_str());
		}
	}
};

// "*.xsb" sound bank file
HRESULT CAudioEngine::LoadSoundBank(const char* filename)
{
	// currently only support one sound bank, the future API should support many. 
	if(m_pSoundBank != 0)
		return S_OK;
	if(!IsValidAndEnabled())
		return E_FAIL;
	string sLocalFilename = filename;
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(filename);
	if(pEntry)
	{
		if(pEntry->DoesFileExist())
		{
			// we already downloaded the file, so load it. 
			sLocalFilename = pEntry->GetLocalFileName();
		}
		else
		{
			CSoundBankCallBackData callBack(filename);
			// we shall wait for asset completion. 
			pEntry->SyncFile_Async(callBack);
			return E_PENDING;
		}
	}

	HANDLE hFile;
	DWORD dwFileSize;
	DWORD dwBytesRead;
	HRESULT hr;

	// Read and register the sound bank file with XACT.  Do not use memory mapped file IO because the 
	// memory needs to be read/write and the working set of sound banks are small.
	if( !CParaFile::DoesFileExist(sLocalFilename.c_str(), false) )
	{
		OUTPUT_LOG("warning: unable to load audio file %s\n", filename);
		return E_FAIL;
	}

	hr = E_FAIL; // assume failure
	hFile = CreateFile( sLocalFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		dwFileSize = GetFileSize( hFile, NULL );
		if( dwFileSize != -1 )
		{
			// Allocate the data here and free the data when receiving the sound bank destroyed notification
			m_pbSoundBank = new BYTE[dwFileSize];
			if( m_pbSoundBank ) 
			{
				if( 0 != ReadFile(hFile, m_pbSoundBank, dwFileSize, &dwBytesRead, NULL) )
				{
					hr = m_pEngine->CreateSoundBank( m_pbSoundBank, dwFileSize, 0, 0, &m_pSoundBank );
				}
			}                
		}
		CloseHandle( hFile ); 
	}                    
	if( FAILED(hr) ) 
		return E_FAIL; 

	return S_OK;
}

/** Async wave file call back. */
class CInMemoryWaveBankCallBackData
{
public:
	CInMemoryWaveBankCallBackData(const char* sFileName):m_sFileName(sFileName){}

	std::string m_sFileName;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if(nResult == 0)
		{
			CAudioEngine::GetInstance()->LoadInMemoryWaveBank(m_sFileName.c_str());
		}
	}
};

HRESULT CAudioEngine::LoadInMemoryWaveBank(const char* filename)
{
	// currently only support one such bank, the future API should support many. 
	if(m_pWaveBankInMemory != 0)
		return S_OK;
	if(!IsValidAndEnabled())
		return E_FAIL;
	string sLocalFilename = filename;
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(filename);
	if(pEntry)
	{
		if(pEntry->DoesFileExist())
		{
			// we already downloaded the file, so load it. 
			sLocalFilename = pEntry->GetLocalFileName();
		}
		else
		{
			CInMemoryWaveBankCallBackData callBack(filename);
			// we shall wait for asset completion. 
			pEntry->SyncFile_Async(callBack);
			return E_PENDING;
		}
	}

	HANDLE hFile;
	DWORD dwFileSize;
	HANDLE hMapFile;
	HRESULT hr;

	// "*-InMemory.xwb" in memory wave bank file
	if( !CParaFile::DoesFileExist(sLocalFilename.c_str(), false) )
	{
		OUTPUT_LOG("warning: unable to load audio file %s\n", filename);
		return E_FAIL;
	}

	// Create an "in memory" XACT wave bank file using memory mapped file IO
	// Memory mapped files tend to be the fastest for most situations assuming you 
	// have enough virtual address space for a full map of the file
	hr = E_FAIL; // assume failure
	hFile = CreateFile( sLocalFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		dwFileSize = GetFileSize( hFile, NULL );
		if( dwFileSize != -1 )
		{
			hMapFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL );
			if( hMapFile )
			{
				m_pbWaveBankInMemory = MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );
				if( m_pbWaveBankInMemory ) 
				{
					hr = m_pEngine->CreateInMemoryWaveBank( m_pbWaveBankInMemory, dwFileSize, 0, 0, &m_pWaveBankInMemory );   
				}                
				CloseHandle( hMapFile ); // pbWaveBank maintains a handle on the file so close this unneeded handle
			}
		}
		CloseHandle( hFile ); // pbWaveBank maintains a handle on the file so close this unneeded handle
	}                    
	if( FAILED(hr) ) 
		return E_FAIL;
	return S_OK;
}

/** Async wave file call back. */
class CStreamWaveBankCallBackData
{
public:
	CStreamWaveBankCallBackData(const char* sFileName):m_sFileName(sFileName){}

	std::string m_sFileName;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if(nResult == 0)
		{
			CAudioEngine::GetInstance()->LoadStreamWaveBank(m_sFileName.c_str());
		}
	}
};

HRESULT CAudioEngine::LoadStreamWaveBank(const char* filename)
{
	// currently only support one such bank, the future API should support many. 
	if(m_pWaveBankStream != 0)
		return S_OK;
	if(!IsValidAndEnabled())
		return E_FAIL;

	string sLocalFilename = filename;
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(filename);
	if(pEntry)
	{
		if(pEntry->DoesFileExist())
		{
			// we already downloaded the file, so load it. 
			sLocalFilename = pEntry->GetLocalFileName();
		}
		else
		{
			CStreamWaveBankCallBackData callBack(filename);
			// we shall wait for asset completion. 
			pEntry->SyncFile_Async(callBack);
			return E_PENDING;
		}
	}

	HRESULT hr;

	// "*-Stream.xwb" stream bank file
	if( !CParaFile::DoesFileExist(sLocalFilename.c_str(), false) )
	{
		OUTPUT_LOG("warning: unable to load audio file %s\n", filename);
		return E_FAIL;
	}

	hr = E_FAIL; // assume failure
	m_hStreamingWaveBankFile = CreateFile( sLocalFilename.c_str(), 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL );
	if( m_hStreamingWaveBankFile != INVALID_HANDLE_VALUE )
	{
		XACT_WAVEBANK_STREAMING_PARAMETERS wsParams;
		ZeroMemory( &wsParams, sizeof(XACT_WAVEBANK_STREAMING_PARAMETERS) );
		wsParams.file = m_hStreamingWaveBankFile;
		wsParams.offset = 0;

		// 64 means to allocate a 64 * 2k buffer for streaming.  
		// This is a good size for DVD streaming and takes good advantage of the read ahead cache
		wsParams.packetSize = 64; 

		hr = m_pEngine->CreateStreamingWaveBank( &wsParams, &m_pWaveBankStream );
	}
	if( FAILED(hr) ) 
		return E_FAIL; // CleanupXACT() will cleanup state before exiting
	return S_OK;
}


HRESULT CAudioEngine::EnableAudioBank(const char * strBankName)
{
	OUTPUT_LOG("warning: ParaAudio.EnableAudioBank() is deprecated. Please use LoadSoundBank(), LoadInMemoryWaveBank(), LoadStreamWaveBank() instead\n");
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bInitializeBanks)
	{
		if(m_strCurrentAudioBankName.compare(strBankName) == 0)
		{
			// target bank already initialized
			return S_OK;
		}
		else
		{
			// there is an existing audio bank working
			OUTPUT_LOG("EnableAudioBank(%s) fail. Audio Bank: %s already working, call DisableAudioBank before EnableAudioBank\n", strBankName, m_strCurrentAudioBankName.c_str());
			return S_FALSE;
		}
	}
	else
	{
		// no banks initialized
		m_strCurrentAudioBankName.clear();

		string strPathWaveInMemory = string("Audio/") + strBankName + "/" + strBankName + "-InMemory.xwb";
		LoadInMemoryWaveBank(strPathWaveInMemory.c_str());

		string strPathWaveStream = string("Audio/") + strBankName + "/" + strBankName + "-Stream.xwb";
		LoadStreamWaveBank(strPathWaveStream.c_str());

		string strPathSound = string("Audio/") + strBankName + "/" + strBankName + ".xsb";
		LoadSoundBank(strPathSound.c_str());

		m_strCurrentAudioBankName = strBankName;
		m_bInitializeBanks = true;
		return S_OK;
	}
}

HRESULT CAudioEngine::UnLoadSoundBank(const char* filename)
{
	// Note: No Release() functions?
	// SAFE_RELEASE(m_pSoundBank);
	m_pSoundBank=NULL;
	SAFE_DELETE_ARRAY(m_pbSoundBank);
	return S_OK;
}

HRESULT CAudioEngine::UnLoadInMemoryWaveBank(const char* filename)
{
	// Note: No Release() functions?
	// SAFE_RELEASE(m_pWaveBankInMemory);
	m_pWaveBankInMemory = NULL;
	if( m_pbWaveBankInMemory )
		UnmapViewOfFile( m_pbWaveBankInMemory );
	m_pbWaveBankInMemory = NULL;
	return S_OK;
}

HRESULT CAudioEngine::UnLoadStreamWaveBank(const char* filename)
{
	// Note: No Release() functions?
	// SAFE_RELEASE(m_pWaveBankStream);
	m_pWaveBankStream = NULL;
	if( m_hStreamingWaveBankFile != INVALID_HANDLE_VALUE && m_hStreamingWaveBankFile != NULL )
		CloseHandle( m_hStreamingWaveBankFile );
	return S_OK;
}


HRESULT CAudioEngine::DisableAudioBank(const char * strBankName)
{
	OUTPUT_LOG("warning: ParaAudio.DisableAudioBank() is deprecated. Please use UnLoadSoundBank(), UnLoadInMemoryWaveBank(), UnLoadStreamWaveBank() instead\n");
	if(!IsValidAndEnabled())
		return E_FAIL;
	if( m_bInitializeBanks )
	{
		if(m_strCurrentAudioBankName.compare(strBankName) == 0)
		{
			UnLoadSoundBank();
			UnLoadInMemoryWaveBank();
			UnLoadStreamWaveBank();

			m_strCurrentAudioBankName.clear();
			m_bInitializeBanks = false;
			m_bStreamingWaveBankPrepared = false;
			return S_OK;
		}
		else
		{
			// bank name don't match
			OUTPUT_LOG("Call DisableAudioBank(%s) tempt to release audio bank %s's resource\n", strBankName, m_strCurrentAudioBankName.c_str());
			return S_FALSE;
		}
	}
	else
	{
		// no bank initialized, can't release any resources
		OUTPUT_LOG("Call DisableAudioBank(%s) fails! No audio bank initialized yet.", strBankName);
		return S_FALSE;
	}
}
string CAudioEngine::GetCurrentAudioBankName()
{
	return m_strCurrentAudioBankName;
}

HRESULT CAudioEngine::SetAudioStereo()
{
	return S_OK;
}

HRESULT CAudioEngine::SetAudio5Point1()
{
	return S_FALSE;
}
int CAudioEngine::GetAudioFormat()
{
	return 2;
}

// ------------------------------------------------------
//                     Background Functions
// ------------------------------------------------------

HRESULT CAudioEngine::PlayBGMusic( const char * strBGMusicName )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableBGMusic)
	{
		XACTINDEX index;
		if(m_pSoundBank)
		{
			index = m_pSoundBank->GetCueIndex(strBGMusicName);
			if(index == XACTINDEX_INVALID )
				return S_FALSE;
		}

		if( m_iCurrentBGMusic == XACTINDEX_INVALID )
		{
			// No background music set
			if(m_pSoundBank)
			{
				m_iCurrentBGMusic = index;
				if(m_bStreamingWaveBankPrepared)
					m_pSoundBank->Prepare( index, 0, 0, &m_pCurrentBGMusic );
			}
			return S_OK;
		}
		else if(m_iCurrentBGMusic == index )
		{
			// already play background music
			ResumeBGMusic();
			return S_OK;
		}
		else
		{
			// new background music set
			// fade out old cue, fade in new cue
			if(m_pSoundBank)
			{
				m_pCurrentBGMusic->Destroy();
				m_pCurrentBGMusic = NULL;
				if(m_bStreamingWaveBankPrepared)
					m_pSoundBank->Prepare( index, 0, 0, &m_pCurrentBGMusic );
				m_iCurrentBGMusic = index;
			}
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT CAudioEngine::PauseBGMusic( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	DWORD dwState;
	if(m_bEnableBGMusic)
	{
		if( m_iCurrentBGMusic == XACTINDEX_INVALID )
		{
			// No background music set
			return S_FALSE;
		}
		else
		{
			// already play background music
			if(m_pCurrentBGMusic)
			{
				m_pCurrentBGMusic->GetState(&dwState);
				if(dwState & XACT_CUESTATE_PLAYING)
					return m_pCurrentBGMusic->Pause(true);
			}
			return S_FALSE;
		}
	}
	else
	{
		return S_FALSE;
	}
}

HRESULT CAudioEngine::ResumeBGMusic( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	DWORD dwState;
	if(m_bEnableBGMusic)
	{
		if( m_iCurrentBGMusic == XACTINDEX_INVALID )
		{
			// No background music set
			return S_FALSE;
		}
		else
		{
			// already play background music
			if(m_pCurrentBGMusic)
			{
				m_pCurrentBGMusic->GetState(&dwState);
				if(dwState & XACT_CUESTATE_PAUSED)
					return m_pCurrentBGMusic->Pause(false);
			}
			return S_FALSE;
		}
	} // if(IsAudioEngineEnabled() && m_bEnableBGMusic)
	else
	{
		return S_FALSE;
	}
}

HRESULT CAudioEngine::StopBGMusic( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableBGMusic)
	{
		if( m_iCurrentBGMusic == XACTINDEX_INVALID )
		{
			// No background music set
			return S_OK;
		}
		else
		{
			// already play background music
			if(m_pCurrentBGMusic)
			{
				m_iCurrentBGMusic = XACTINDEX_INVALID;
				return m_pCurrentBGMusic->Stop(XACT_FLAG_CUE_STOP_IMMEDIATE);
			}
			return S_FALSE;
		}
	}
	else
	{
		return S_FALSE;
	}
}
bool CAudioEngine::IsBGMusicEnabled()
{
	return m_bEnableBGMusic;
}

HRESULT CAudioEngine::EnableBGMusic( )
{
	if( m_bEnableBGMusic == true )
		return S_OK;
	else
	{
		m_bEnableBGMusic = true;
		ResumeBGMusic();
		return S_OK;
	}
}

HRESULT CAudioEngine::DisableBGMusic( )
{
	if( m_bEnableBGMusic == false )
		return S_OK;
	else
	{
		m_bEnableBGMusic = false;
		PauseBGMusic();
		return S_OK;
	}
}
float CAudioEngine::GetBGMusicVolume( )
{
	return m_fBGMusicVolume;
}
HRESULT CAudioEngine::SetBGMusicVolume( const float fVolume )
{
	m_fBGMusicVolume = fVolume;

	if(!IsValidAndEnabled())
		return E_FAIL;
	
	XACTCATEGORY category;
	category = m_pEngine->GetCategory("Background");
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->SetVolume(category, fVolume);
	return S_FALSE;
}

// ------------------------------------------------------
//                         Dialog Functions
// ------------------------------------------------------
// play dialog music
// @param strDialogName: dialog music name
HRESULT CAudioEngine::PlayDialog( const char * strDialogName , const char * strScript )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	if(m_bEnableDialogMusic)
	{
		// TODO: run the script in strScript before setting the dialog

		XACTINDEX index;
		if(m_pSoundBank)
		{
			index = m_pSoundBank->GetCueIndex(strDialogName);
			if(index == XACTINDEX_INVALID )
				return S_FALSE;
		}

		if( m_iCurrentDialog == XACTINDEX_INVALID )
		{
			// No dialog music set
			if(m_pSoundBank)
			{
				m_iCurrentDialog = index;
				if(m_bStreamingWaveBankPrepared)
					m_pSoundBank->Prepare( index, 0, 0, &m_pCurrentDialog );
			}
			return S_OK;
		}
		else if(m_iCurrentDialog == index )
		{
			// already play dialog music
			return S_OK;
		}
		else
		{
			// new dialog music set
			if(m_pSoundBank)
			{
				m_pCurrentDialog->Destroy();
				m_pCurrentDialog = NULL;
				if(m_bStreamingWaveBankPrepared)
					m_pSoundBank->Prepare( index, 0, 0, &m_pCurrentDialog );
				m_iCurrentDialog = index;
			}
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT CAudioEngine::SkipDialog( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	if(m_bEnableDialogMusic)
	{
		if( m_iCurrentDialog == XACTINDEX_INVALID )
		{
			// No dialog music set
			return S_FALSE;
		}
		else
		{
			// already play dialog music
			if(m_pCurrentDialog)
			{
				// TODO: skip to the next marker
				return S_OK;
			}
			return S_FALSE;
		}
	} // if(IsAudioEngineEnabled() && m_bEnableDialogMusic)
	else
	{
		return S_FALSE;
	}
}

HRESULT CAudioEngine::PauseDialog( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	DWORD dwState;
	if(m_bEnableDialogMusic)
	{
		if( m_iCurrentDialog == XACTINDEX_INVALID )
		{
			// No dialog music set
			return S_FALSE;
		}
		else
		{
			// already play dialog music
			if(m_pCurrentDialog)
			{
				m_pCurrentDialog->GetState(&dwState);
				if(dwState & XACT_CUESTATE_PLAYING)
					return m_pCurrentDialog->Pause(true);
			}
			return S_FALSE;
		}
	} // if(IsAudioEngineEnabled() && m_bEnableDialogMusic)
	else
	{
		return S_FALSE;
	}
}

HRESULT CAudioEngine::ResumeDialog( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	DWORD dwState;
	if(m_bEnableDialogMusic)
	{
		if( m_iCurrentDialog == XACTINDEX_INVALID )
		{
			// No dialog music set
			return S_FALSE;
		}
		else
		{
			// already play dialog music
			if(m_pCurrentDialog)
			{
				m_pCurrentDialog->GetState(&dwState);
				if(dwState & XACT_CUESTATE_PAUSED)
					return m_pCurrentDialog->Pause(false);
			}
			return S_FALSE;
		}
	} // if(IsAudioEngineEnabled() && m_bEnableDialogMusic)
	else
	{
		return S_FALSE;
	}
}
// stop dialog music
// @param strDialogName: dialog music name
HRESULT CAudioEngine::StopDialog( )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	if(m_bEnableDialogMusic)
	{
		if( m_iCurrentDialog == XACTINDEX_INVALID )
		{
			// No dialog music set
			return S_OK;
		}
		else
		{
			// already play dialog music
			if(m_pCurrentDialog)
			{
				m_iCurrentDialog = XACTINDEX_INVALID;
				return m_pCurrentDialog->Stop(XACT_FLAG_CUE_STOP_IMMEDIATE);
			}
			return S_FALSE;
		}
	}
	else
	{
		return S_FALSE;
	}
}
// get is dialog music enabled
bool CAudioEngine::IsDialogEnabled()
{
	return m_bEnableDialogMusic;
}
// enable dialog music
HRESULT CAudioEngine::EnableDialog( )
{
	m_bEnableDialogMusic = true;
	return S_OK;
}
// disable dialog music
HRESULT CAudioEngine::DisableDialog( )
{
	m_bEnableDialogMusic = false;
	return S_OK;
}
float CAudioEngine::GetDialogVolume( )
{
	return m_fDialogMusicVolume;
}
HRESULT CAudioEngine::SetDialogVolume( const float fVolume )
{
	m_fDialogMusicVolume = fVolume;

	if(!IsValidAndEnabled())
		return E_FAIL;
	
	XACTCATEGORY category;
	category = m_pEngine->GetCategory("Dialog");
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->SetVolume(category, fVolume);
	return S_FALSE;
}

// ------------------------------------------------------
//                    Ambient Sound Functions
// ------------------------------------------------------

HRESULT CAudioEngine::PlayAmbientSound( const char * strAmbientSoundName )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableAmbientSound)
	{
		if(m_pSoundBank)
		{
			HRESULT hr;
			XACTINDEX index;
			IXACTCue* pointer;
			map <XACTINDEX, IXACTCue*> :: const_iterator iter;

			index = m_pSoundBank->GetCueIndex(strAmbientSoundName);
			iter = m_mapAmbientCuePointer.find(index);
			if ( iter == m_mapAmbientCuePointer.end( ) )
			{
				// haven't played
				hr = m_pSoundBank->Play(index, 0, 0, &pointer);
				if(hr == S_OK)
				{
					m_mapAmbientCuePointer.insert( CuePointerPair (index, pointer) );
					return S_OK;
				}
			}
			else
			{
				// already played, return false, because the Ambient Sound doesn't allow multi-play instance
				return S_FALSE;
			}
		}
	}
	return S_FALSE;
}

HRESULT CAudioEngine::PauseAmbientSound( const char * strAmbientSoundName )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableAmbientSound)
	{
		if(m_pSoundBank)
		{
			XACTINDEX index;
			IXACTCue* pointer;
			index = m_pSoundBank->GetCueIndex(strAmbientSoundName);
			if(index != XACTINDEX_INVALID)
			{
				map <XACTINDEX, IXACTCue*> :: const_iterator iter;
				iter = m_mapAmbientCuePointer.find(index);
				if ( iter != m_mapAmbientCuePointer.end( ) )
				{
					pointer = iter->second;
					DWORD dwState;
					pointer->GetState(&dwState);
					if(dwState & XACT_CUESTATE_PLAYING)
					{
						pointer->Pause(true);
						return S_OK;
					}
				}
			}
		}
	}
	return S_FALSE;
}

HRESULT CAudioEngine::ResumeAmbientSound( const char * strAmbientSoundName )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableAmbientSound)
	{
		if(m_pSoundBank)
		{
			XACTINDEX index;
			IXACTCue* pointer;
			index = m_pSoundBank->GetCueIndex(strAmbientSoundName);
			if(index != XACTINDEX_INVALID)
			{
				map <XACTINDEX, IXACTCue*> :: const_iterator iter;
				iter = m_mapAmbientCuePointer.find(index);
				if ( iter != m_mapAmbientCuePointer.end( ) )
				{
					pointer = iter->second;
					DWORD dwState;
					pointer->GetState(&dwState);
					if(dwState & XACT_CUESTATE_PAUSED)
					{
						pointer->Pause(false);
						return S_OK;
					}
				}
			}
		}
	}
	return S_FALSE;
}

HRESULT CAudioEngine::StopAmbientSound( const char * strAmbientSoundName )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableAmbientSound)
	{
		if(m_pSoundBank)
		{
			XACTINDEX index;
			IXACTCue* pointer;
			index = m_pSoundBank->GetCueIndex(strAmbientSoundName);
			if(index != XACTINDEX_INVALID)
			{
				map <XACTINDEX, IXACTCue*> :: iterator iter;
				iter = m_mapAmbientCuePointer.find(index);
				if ( iter != m_mapAmbientCuePointer.end( ) )
				{
					pointer = iter->second;
					pointer->Stop(XACT_FLAG_CUE_STOP_IMMEDIATE);
					m_mapAmbientCuePointer.erase(iter);
					return S_OK;
				}
			}
		}
	}
	return S_FALSE;
}
bool CAudioEngine::IsAmbientSoundEnabled()
{
	return m_bEnableAmbientSound;
}

HRESULT CAudioEngine::EnableAmbientSound( )
{
	if( m_bEnableAmbientSound == true )
		return S_OK;
	else
	{
		m_bEnableAmbientSound = true;
		return ResumeCategory("Ambient");
	}
}

HRESULT CAudioEngine::DisableAmbientSound( )
{
	if( m_bEnableAmbientSound == false )
		return S_OK;
	else
	{
		m_bEnableAmbientSound = false;
		return PauseCategory("Ambient");
	}
}
float CAudioEngine::GetAmbientSoundVolume( )
{
	return m_fAmbientSoundVolume;
}
HRESULT CAudioEngine::SetAmbientSoundVolume( const float fVolume )
{
	m_fAmbientSoundVolume = fVolume;

	if(!IsValidAndEnabled())
		return E_FAIL;
	
	XACTCATEGORY category;
	category = m_pEngine->GetCategory("Ambient");
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->SetVolume(category, fVolume);
	return S_FALSE;
}


// ------------------------------------------------------
//                UserInterface Sound Functions
// ------------------------------------------------------

HRESULT CAudioEngine::PlayUISound( const char * strUISoundName )
{
	if(!IsValidAndEnabled())
		return E_FAIL;

	if(m_bEnableUISound)
	{
		if(m_pSoundBank)
		{
			XACTINDEX index;
			index = m_pSoundBank->GetCueIndex(strUISoundName);
			return m_pSoundBank->Play(index, 0, 0, NULL);
		}
	}
	return S_FALSE;
}

float CAudioEngine::GetUISoundVolume()
{
	return m_fUISoundVolume;
}

HRESULT CAudioEngine::SetUISoundVolume( const float fVolume )
{
	m_fUISoundVolume = fVolume;
	
	if(!IsValidAndEnabled())
		return E_FAIL;
	
	// change all sound that is currently being played. 
	map<string, IXACTWave*>::iterator itCur, itEnd = m_codeDrivenFiles.end();
	for( itCur = m_codeDrivenFiles.begin(); itCur!=itEnd; itCur ++)
	{
		IXACTWave* pWave = itCur->second;
		if(pWave)
		{
			DWORD dwState=0;
			pWave->GetState(&dwState);
			if(dwState==XACT_STATE_PLAYING)
			{
				pWave->SetVolume(fVolume);
			}
		}
	}

	XACTCATEGORY category;
	category = m_pEngine->GetCategory("UI");
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->SetVolume(category, fVolume);
	
	return S_FALSE;
}

// ------------------------------------------------------
//                        3D Sound Functions
// ------------------------------------------------------

// play 3D sound in static position
// @param str3DSoundName: 3D sound name
// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
// @param x, y, z: 3D sound position
HRESULT CAudioEngine::PlayStatic3DSound( const char * str3DSoundName, const char * assetName, float x, float y, float z)
{
	if(!IsValidAndEnabled() || Get3DSoundVolume()<=0.f || assetName == 0)
		return E_FAIL;

	if(m_bEnable3DSound)
	{
		POOL_3DCUE_TYPE::iterator iter = m_pool_3D_Static_Cues.find(assetName);
		if(iter!=m_pool_3D_Static_Cues.end())
		{
			IXACTCue* pCue = iter->second.pCue;
			if(pCue)
			{
				pCue->Play();
			}
			return S_OK;
		}

		// Play the cue as a 3D audio cue
		XACTINDEX index;
		if(m_pSoundBank)
		{
			index = m_pSoundBank->GetCueIndex(str3DSoundName);
			if(index == XACTINDEX_INVALID )
				return S_FALSE;
		}
		
		IXACTCue* pCue = NULL;
		if(m_pSoundBank->Prepare( index, 0, 0, &pCue ) == S_OK)
		{
			// create a cue and put it to active cue pool 
			ParaAudio_3DCUE cue3D;
			cue3D.bActive = true;
			cue3D.strKeyName = assetName;
			cue3D.pCue = pCue;
			cue3D.vEmitterPosition.x = x;
			cue3D.vEmitterPosition.y = y;
			cue3D.vEmitterPosition.z = z;
			m_emitter.Position = cue3D.vEmitterPosition; 
			
			XACT3DCalculate( m_x3DInstance, &m_listener, &m_emitter, &m_dspSettings );
			XACT3DApply( &m_dspSettings, pCue );

			m_pool_3D_Static_Cues[assetName] = cue3D;
			return pCue->Play();
		}
	}
	return S_FALSE;
}
// pause 3D sound in static position
// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
HRESULT CAudioEngine::PauseStatic3DSound( const char * assetName )
{
	return S_OK;
}
// Resume 3D sound in static position
// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
HRESULT CAudioEngine::ResumeStatic3DSound( const char * assetName )
{
	return S_OK;
}
// stop 3D sound in static position
// @param assetName: 3D sound asset name, this is the KEY to the 3D Sound
HRESULT CAudioEngine::StopStatic3DSound( const char * assetName )
{
	POOL_3DCUE_TYPE::iterator iter = m_pool_3D_Static_Cues.find(assetName);
	if(iter!=m_pool_3D_Static_Cues.end())
	{
		IXACTCue* pCue = iter->second.pCue;
		if(pCue)
		{
			pCue->Stop(0);
			pCue->Destroy();
		}
		m_pool_3D_Static_Cues.erase(iter);
	}
	return S_OK;
}
// play 3D sound to dynamic object
// @param str3DSoundName: 3D sound name
// @param objectName: 3D sound object name, this is the KEY to the 3D Sound
//			this is the name of the base class BaseObject
HRESULT CAudioEngine::PlayDynamic3DSound( const char * str3DSoundName, const char * objectName )
{
	return S_OK;
}

float CAudioEngine::Get3DSoundVolume()
{
	return m_f3DSoundVolume;
}

HRESULT CAudioEngine::Set3DSoundVolume( const float fVolume )
{
	m_f3DSoundVolume = fVolume;

	if(!IsValidAndEnabled())
		return E_FAIL;
	
	XACTCATEGORY category;
	category = m_pEngine->GetCategory("3DSound");
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->SetVolume(category, fVolume);
	return S_FALSE;
}

// ------------------------------------------------------
//                  Interactive Sound Functions
// ------------------------------------------------------

HRESULT CAudioEngine::PlayInteractiveSound( const char * strInteractiveSoundName )
{
	if(!IsValidAndEnabled() || GetInteractiveSoundVolume()<=0.f)
		return E_FAIL;

	if(m_bEnableUISound)
	{
		if(m_pSoundBank)
		{
			XACTINDEX index;
			index = m_pSoundBank->GetCueIndex(strInteractiveSoundName);
			return m_pSoundBank->Play(index, 0, 0, NULL);
		}
	}
	return S_FALSE;
}

float CAudioEngine::GetInteractiveSoundVolume( )
{
	return m_fInteractiveSoundVolume;
}

HRESULT CAudioEngine::SetInteractiveSoundVolume( const float fVolume )
{
	m_fInteractiveSoundVolume = fVolume;

	if(!IsValidAndEnabled())
		return E_FAIL;
	
	XACTCATEGORY category;
	category = m_pEngine->GetCategory("Interactive");
	if(category != XACTCATEGORY_INVALID)
		return m_pEngine->SetVolume(category, fVolume);
	return S_FALSE;
}


IXACTWaveBank* CAudioEngine::GetWaveBankStream()
{
	return m_pWaveBankStream;
}
IXACTCue* CAudioEngine::GetCurrentDialog()
{
	return m_pCurrentDialog;
}
void CAudioEngine::SetHandleStreamingWaveBankPrepared( bool bHandle )
{
	ParaEngine::Lock lock_(m_mutex);
	m_bHandleStreamingWaveBankPrepared = bHandle;
}
void CAudioEngine::SetHandleDialogCuePrepared( bool bHandle )
{
	ParaEngine::Lock lock_(m_mutex);
	m_bHandleDialogCuePrepared = bHandle;
}
void CAudioEngine::SetHandleDialogCueStop( bool bHandle )
{
	ParaEngine::Lock lock_(m_mutex);
	m_bHandleDialogCueStop = bHandle;
}
IXACTCue* CAudioEngine::GetCurrentBGMusic()
{
	ParaEngine::Lock lock_(m_mutex);
	return m_pCurrentBGMusic;
}
void CAudioEngine::SetHandleBGCuePrepared( bool bHandle )
{
	ParaEngine::Lock lock_(m_mutex);
	m_bHandleBGCuePrepared = bHandle;
}
void CAudioEngine::SetHandleBGCueStop( bool bHandle )
{
	ParaEngine::Lock lock_(m_mutex);
	m_bHandleBGCueStop = bHandle;
}

bool CAudioEngine::IsWaveFileInQueue(const string& filename)
{
	map<string, IXACTWave*>::iterator iter = m_codeDrivenFiles.find(filename);
	return (iter!=m_codeDrivenFiles.end());
}

/** async wave file call back. */
class CWaveFileCallBackData
{
public:
	CWaveFileCallBackData(const char* sFileName, int nLoopCount):m_sFileName(sFileName),m_nLoopCount(nLoopCount) {}

	std::string m_sFileName;
	int m_nLoopCount;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if(nResult == 0)
		{
			if(CAudioEngine::GetInstance()->IsWaveFileInQueue(m_sFileName))
			{
				CAudioEngine::GetInstance()->ReleaseWaveFile(m_sFileName.c_str());
				if(m_nLoopCount > 2)
				{
					// we will only play a wave file if it is looped at least 2 times and still in the playing list, without the user calling StopWaveFile() explicitly. 
					// in most cases, it is a background music, which is usually played 10000 times. 
					CAudioEngine::GetInstance()->PlayWaveFile(m_sFileName.c_str(), m_nLoopCount);
				}
				else
				{
					// if a wave file is already in the queue but with a small loop count, we will not play it. 
				}
			}
		}
	}
};

HRESULT CAudioEngine::PlayWaveFile( PCSTR szWavePath,XACTLOOPCOUNT nLoopCount, WORD wStreamingPacketSize/*=0*/,DWORD dwPlayOffset/*=0*/, DWORD dwAlignment/*=2048*/ )
{
	if(!IsValidAndEnabled() || !m_bEnableUISound || GetUISoundVolume()<=0.f)
		return E_FAIL;
	// check if the wave file is already prepared before
	IXACTWave* pWave = NULL;
	map<string, IXACTWave*>::iterator iter = m_codeDrivenFiles.find(szWavePath);
	if (iter!=m_codeDrivenFiles.end())
	{
		pWave = iter->second;
		if(pWave)
		{
			DWORD dwState=0;
			pWave->GetState(&dwState);
			if(dwState==XACT_STATE_PLAYING)
			{
				return S_OK;
			}
			else if(dwState==XACT_STATE_PREPARING)
			{
			}
			else
			{
				/**
				Note: it is rather odd but to play sounds again they need to be prepared again. This seems to limit the XACT code driven API quite a lot 
				and why Microsoft could not have provided a means to 'rewind' I do not know. I did ask them and they said you had to prepare it again and
				it was not meant to be a replacement for DirectSound. I am hoping future upgrades will improve on this.
				*/

				// TODO: I did not figure out a way to reuse a stopped IXACTWave object. Hence I just delete it and create a new one from file. 
				// destroy the old one and prepare it again using the new settings. 
				// TODO: i do not know, this might be a bug of XACT, that sometimes pWave->Destroy() never returns.pWave->Destroy();
				// and that m_pEngine->PrepareWave() may return the same wave object that has been formerly destroyed. 
				//pWave->Destroy();
				m_codeDrivenFiles.erase(iter);
				pWave = NULL;
			}
		}
		else
		{
			// it usually means that the music is pending for downloading. 
			return S_OK;
		}
	}
	// prepare if not prepared before
	if (pWave==NULL)
	{
		bool bSucceed = false;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(szWavePath);
		if(pEntry)
		{
			if(pEntry->DoesFileExist())
			{
				// we already downloaded the file, so load it. 
				if(m_pEngine->PrepareWave(XACT_FLAG_UNITS_MS, pEntry->GetLocalFileName().c_str(), wStreamingPacketSize, dwAlignment, dwPlayOffset, nLoopCount, &pWave) == S_OK)
				{
					m_codeDrivenFiles[szWavePath] = pWave;
					bSucceed = true;
				}
			}
			else
			{
				// push to the queue. 
				m_codeDrivenFiles[szWavePath] = NULL;
				CWaveFileCallBackData callBack(szWavePath, nLoopCount);
				// we shall wait for asset completion. 
				pEntry->SyncFile_Async(callBack);
			}
		}
		else
		{
			if(ParaEngine::CParaFile::DoesFileExist(szWavePath, false))
			{
				if(m_pEngine->PrepareWave(XACT_FLAG_UNITS_MS, szWavePath, wStreamingPacketSize, dwAlignment, dwPlayOffset, nLoopCount, &pWave) == S_OK)
				{
					m_codeDrivenFiles[szWavePath] = pWave;
					bSucceed = true;
				}
			}
			else 
			{
				// TODO: currently it will upzip file each time a zipped music is played. We may first check a fixed temp location and play it from there before extracting to it. 
				ParaEngine::CParaFile file(szWavePath);
				if(!file.isEof())
				{
					//////////////////////////////////////////////////////////////////////////
					// just extract to temp/tempaudio/*.wav first and then play it from there
					string sTempDiskFilename = ParaEngine::CParaFile::GetCurDirectory(ParaEngine::CParaFile::APP_TEMP_DIR);
					sTempDiskFilename += "tempaudio/";
					//sTempDiskFilename += ParaEngine::CParaFile::GetFileName(szWavePath);
					sTempDiskFilename += "temp.wav";
					if(ParaEngine::CParaFile::CreateDirectory(sTempDiskFilename.c_str()))
					{
						if(file.ExtractFileToDisk(sTempDiskFilename.c_str(), true))
						{
							if(m_pEngine->PrepareWave(XACT_FLAG_UNITS_MS, sTempDiskFilename.c_str(), wStreamingPacketSize, dwAlignment, dwPlayOffset, nLoopCount, &pWave) == S_OK)
							{
								m_codeDrivenFiles[szWavePath] = pWave;
								bSucceed = true;
							}
						}
					}
				}
			}
		}
		
		if(!bSucceed)
		{
			m_codeDrivenFiles[szWavePath] = NULL;
			OUTPUT_LOG("unable to prepare wave file %s\r\n", szWavePath);
			return E_FAIL;
		}
	}
	// play the sound
	if(pWave)
	{
		DWORD dwState=0;
		pWave->GetState(&dwState);
		if(dwState==XACT_STATE_PREPARED)
		{
			pWave->SetVolume(GetUISoundVolume());
			pWave->Play();
		}
	}
	return S_OK;
}

HRESULT CAudioEngine::StopWaveFile( PCSTR szWavePath , DWORD dwFlags)
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	// check if the wave file is already prepared before
	IXACTWave* pWave = NULL;
	map<string, IXACTWave*>::iterator iter = m_codeDrivenFiles.find(szWavePath);
	if (iter!=m_codeDrivenFiles.end())
	{
		pWave = iter->second;
		// erase the wave 
		m_codeDrivenFiles.erase(iter);
		if(pWave)
		{
			pWave->Stop(dwFlags);
		}
		return S_OK;
	}
	
	return E_FAIL;
}

HRESULT CAudioEngine::ReleaseWaveFile( PCSTR szWavePath )
{
	if(!IsValidAndEnabled())
		return E_FAIL;
	
	// check if the wave file is already prepared before
	IXACTWave* pWave = NULL;
	map<string, IXACTWave*>::iterator iter = m_codeDrivenFiles.find(szWavePath);
	if (iter!=m_codeDrivenFiles.end())
	{
		pWave = iter->second;
		if(pWave)
		{
			pWave->Stop(XACT_FLAG_STOP_IMMEDIATE);
			// TODO: i do not know, this might be a bug of XACT, that sometimes pWave->Destroy() never returns.
			//pWave->Destroy();
		}
		m_codeDrivenFiles.erase(iter);
		return S_OK;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ScanAvailableInputFormats()
// Desc: Tests to see if 20 different standard wave formats are supported by
//       the capture device 
//-----------------------------------------------------------------------------
HRESULT CAudioEngine::ScanAvailableInputFormats()
{
	HRESULT       hr;
	WAVEFORMATEX  wfx;
	DSCBUFFERDESC dscbd;
	LPDIRECTSOUNDCAPTUREBUFFER pDSCaptureBuffer = NULL;

	
	ZeroMemory( &wfx, sizeof(wfx));
	wfx.wFormatTag = WAVE_FORMAT_PCM;

	ZeroMemory( &dscbd, sizeof(dscbd) );
	dscbd.dwSize = sizeof(dscbd);

	// Try 20 different standard formats to see if they are supported
	for( INT iIndex = 0; iIndex < 20; iIndex++ )
	{
		GetWaveFormatFromIndex( iIndex, &wfx );

		// To test if a capture format is supported, try to create a 
		// new capture buffer using a specific format.  If it works
		// then the format is supported, otherwise not.
		dscbd.dwBufferBytes = wfx.nAvgBytesPerSec;
		dscbd.lpwfxFormat = &wfx;

		if( FAILED( hr = m_pDSCapture->CreateCaptureBuffer( &dscbd, 
			&pDSCaptureBuffer, 
			NULL ) ) )
			m_abInputFormatSupported[ iIndex ] = FALSE;
		else
			m_abInputFormatSupported[ iIndex ] = TRUE;

		SAFE_RELEASE( pDSCaptureBuffer );
	}

	//////////////////////////////////////////////////////////////////////////
	// get all supported capture file format 
	{
		TCHAR        strFormatName[255];
		WAVEFORMATEX wfx;

		m_strCaptureFormats.clear();
		for( INT iIndex = 0; iIndex < 20; iIndex++ )
		{
			if( m_abInputFormatSupported[ iIndex ] )
			{
				// Turn the index into a WAVEFORMATEX then turn that into a
				// string and put the string in the listbox
				GetWaveFormatFromIndex( iIndex, &wfx );
				ConvertWaveFormatToString( &wfx, strFormatName, 255 );
				m_strCaptureFormats += strFormatName;
			}
			m_strCaptureFormats += ";";
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetWaveFormatFromIndex()
// Desc: Returns 20 different wave formats based on nIndex
//-----------------------------------------------------------------------------
void CAudioEngine::GetWaveFormatFromIndex( INT nIndex, WAVEFORMATEX* pwfx )
{
	INT iSampleRate = nIndex / 4;
	INT iType = nIndex % 4;

	switch( iSampleRate )
	{
	case 0: pwfx->nSamplesPerSec = 48000; break;
	case 1: pwfx->nSamplesPerSec = 44100; break;
	case 2: pwfx->nSamplesPerSec = 22050; break;
	case 3: pwfx->nSamplesPerSec = 11025; break;
	case 4: pwfx->nSamplesPerSec =  8000; break;
	}

	switch( iType )
	{
	case 0: pwfx->wBitsPerSample =  8; pwfx->nChannels = 1; break;
	case 1: pwfx->wBitsPerSample = 16; pwfx->nChannels = 1; break;
	case 2: pwfx->wBitsPerSample =  8; pwfx->nChannels = 2; break;
	case 3: pwfx->wBitsPerSample = 16; pwfx->nChannels = 2; break;
	}

	pwfx->nBlockAlign = pwfx->nChannels * ( pwfx->wBitsPerSample / 8 );
	pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
}

//-----------------------------------------------------------------------------
// Name: ConvertWaveFormatToString()
// Desc: Converts a wave format to a text string
//-----------------------------------------------------------------------------
void CAudioEngine::ConvertWaveFormatToString( WAVEFORMATEX* pwfx, TCHAR* strFormatName, int cchFormatName )
{
	snprintf( strFormatName, cchFormatName, 
		TEXT("%u Hz, %u-bit %s"), 
		pwfx->nSamplesPerSec, 
		pwfx->wBitsPerSample, 
		( pwfx->nChannels == 1 ) ? TEXT("Mono") : TEXT("Stereo") );
}


//-----------------------------------------------------------------------------
// Name: DSoundEnumCallback()
// Desc: Enumeration callback called by DirectSoundEnumerate
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,
									VOID* pContext )
{
	// Set aside static storage space for 20 audio drivers
	static DWORD dwAudioDriverIndex = 0;
	
	GUID* pTemp  = NULL;

	if( pGUID )
	{
		if( dwAudioDriverIndex >= 20 )
			return TRUE;

		pTemp = &g_AudioDriverGUIDs[dwAudioDriverIndex++];
		memcpy( pTemp, pGUID, sizeof(GUID) );
	}

	HWND hSoundDeviceCombo = (HWND)pContext;

	// Add the string to the combo box
	g_captureDeviceList.push_back(strDesc);
	
	return TRUE;
}


const char* ParaEngine::CAudioEngine::GetRecordingDeviceEnum()
{
	static string g_str;
	static BOOL g_firsttime = TRUE;
	if(g_firsttime)
	{
		//////////////////////////////////////////////////////////////////////////
		// enumerate devices
		//////////////////////////////////////////////////////////////////////////
		g_captureDeviceList.clear();
		g_str = "";
		DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,	NULL);
		int nCount = (int) g_captureDeviceList.size();
		for (int i=0;i<nCount;++i)
		{
			g_str += g_captureDeviceList[i];
			g_str += ";";
		}
	}
	return g_str.c_str();
}

const char* ParaEngine::CAudioEngine::GetRecordingFormatEnum()
{
	if( FAILED(InitSoundCapture()))
		return NULL;
	return m_strCaptureFormats.c_str();
}

bool ParaEngine::CAudioEngine::SetRecordingOutput( const char* sOutputFile, int nDeviceIndex, int nWaveFormatIndex )
{
	StopRecording();

	if(nDeviceIndex>0 && m_nCaptureDeviceIndex != nDeviceIndex)
	{
		// if the user is choosing a different device, we will release sound capture first. 
		ReleaseSoundCapture();
		m_nCaptureDeviceIndex = nDeviceIndex;
	}

	if(sOutputFile!=NULL)
	{
		m_sCapureFileName = sOutputFile;
	}

	if( FAILED(InitSoundCapture()))
		return false;

	
	if(nWaveFormatIndex>0 && nWaveFormatIndex<20)
		m_nWaveFormatIndex = nWaveFormatIndex;
		
	return true;
}

bool ParaEngine::CAudioEngine::BeginRecording()
{
	HRESULT hr;
	StopRecording();

	if( FAILED(InitSoundCapture()))
		return false;

	//////////////////////////////////////////////////////////////////////////
	// create the capture buffer. 
	{
		ZeroMemory( &m_wfxInput, sizeof(m_wfxInput));
		m_wfxInput.wFormatTag = WAVE_FORMAT_PCM;

		GetWaveFormatFromIndex( m_nWaveFormatIndex, &m_wfxInput );

		
		DSCBUFFERDESC dscbd;

		SAFE_RELEASE( m_pDSNotify );
		SAFE_RELEASE( m_pDSBCapture );

		// Set the notification m_wfxInput
		WAVEFORMATEX* pwfxInput = &m_wfxInput;
		m_dwNotifySize = max( 1024, (int)(pwfxInput->nAvgBytesPerSec / 8) );
		m_dwNotifySize -= m_dwNotifySize % pwfxInput->nBlockAlign;   

		// Set the buffer sizes 
		m_dwCaptureBufferSize = m_dwNotifySize * NUM_REC_NOTIFICATIONS;

		SAFE_RELEASE( m_pDSNotify );
		SAFE_RELEASE( m_pDSBCapture );

		// Create the capture buffer
		ZeroMemory( &dscbd, sizeof(dscbd) );
		dscbd.dwSize        = sizeof(dscbd);
		dscbd.dwBufferBytes = m_dwCaptureBufferSize;
		dscbd.lpwfxFormat   = pwfxInput; // Set the format during creation

		if( FAILED( hr = m_pDSCapture->CreateCaptureBuffer( &dscbd, &m_pDSBCapture, NULL ) ) )
		{
			OUTPUT_LOG("error: failed creating sound capture buffer at InitNotifications.\r\n");
			return false;
		}

		m_dwNextCaptureOffset = 0;
		
	}

	//////////////////////////////////////////////////////////////////////////
	// create the wave file
	{
		SAFE_DELETE( m_pWaveFile );
		m_pWaveFile = new CWaveFile;
		if( NULL == m_pWaveFile )
		{
			OUTPUT_LOG("error: failed creating the log file \r\n");
			return false;
		}

		// Get the format of the capture buffer in g_wfxCaptureWaveFormat
		WAVEFORMATEX wfxCaptureWaveFormat;
		ZeroMemory( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX) );
		m_pDSBCapture->GetFormat( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX), NULL );

		// Load the wave file
		static char strFileName[MAX_PATH];
		if((int)m_sCapureFileName.size()<MAX_PATH)
			strcpy(strFileName, m_sCapureFileName.c_str());
		else
		{
			strFileName[0]='\0';
			OUTPUT_LOG("error: sound capture file name too long in %s\r\n", m_sCapureFileName.c_str());
			return false;
		}

		if( FAILED( hr = m_pWaveFile->Open( strFileName, &wfxCaptureWaveFormat, WAVEFILE_WRITE ) ) )
		{
			OUTPUT_LOG("error: failed opening wave file for capturing %s \r\n", m_sCapureFileName.c_str());
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// start capturing
	{
		// start notification.
		if( FAILED( hr = InitNotifications() ) )
		{
			OUTPUT_LOG("error: failed creating sound capture buffer at InitNotifications.\r\n");
			return false;
		}

		if( FAILED( hr = m_pDSBCapture->Start( DSCBSTART_LOOPING ) ) )
		{
			OUTPUT_LOG("error: failed starting capturing \r\n");
			return false;
		}
	}
	g_bIsRecording = TRUE;
	return true;
}

bool ParaEngine::CAudioEngine::StopRecording()
{
	g_bIsRecording = FALSE;

	HRESULT hr;

	if(m_pDSCapture == NULL || m_pDSBCapture == NULL)
		return true;
	

	// Stop the buffer, and read any data that was not 
	// caught by a notification
	if( FAILED( hr = m_pDSBCapture->Stop() ) )
	{
		OUTPUT_LOG("error: failed stopping capturing \r\n");
		return false;
	}

	// close notifications.
	DestoryNotifications();

	if( FAILED( hr = RecordCapturedData() ) )
	{
		OUTPUT_LOG("error: failed RecordCapturedData \r\n");
		return false;
	}

	// Close the wav file
	SAFE_DELETE( m_pWaveFile );
	
	return true;
}

HRESULT ParaEngine::CAudioEngine::InitSoundCapture()
{
	HRESULT hr;
	if(!m_bCaptureDeviceAvailable)
		return E_FAIL;
	if(m_pDSCapture != NULL)
		return S_OK;

	//////////////////////////////////////////////////////////////////////////
	// just ensure that device enumeration is called at least once before continue
	if(GetRecordingDeviceEnum() =="") 
	{
		m_bCaptureDeviceAvailable = false;
		OUTPUT_LOG("warning: no audio capturing devices available in this machine.\r\n");
		return E_FAIL;
	}

	int nDeviceCount = (int) g_captureDeviceList.size();
	assert(nDeviceCount>0);

	//////////////////////////////////////////////////////////////////////////
	// use the selected device
	if(nDeviceCount>m_nCaptureDeviceIndex)
		m_guidCaptureDevice = g_AudioDriverGUIDs[m_nCaptureDeviceIndex];
	else
		m_guidCaptureDevice = g_AudioDriverGUIDs[0];

	for(int i=0;i<NOTIFY_EVENT_NUM;++i)
		g_hNotificationEvents[i] = CreateEvent( NULL, FALSE, FALSE, NULL );

	ZeroMemory( &m_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * 
		(NUM_REC_NOTIFICATIONS + 1) );
	m_dwCaptureBufferSize = 0;
	m_dwNotifySize        = 0;
	m_pWaveFile           = NULL;

	// Create IDirectSoundCapture using the preferred capture device
	if( FAILED( hr = DirectSoundCaptureCreate( &m_guidCaptureDevice, &m_pDSCapture, NULL ) ) )
	{
		m_bCaptureDeviceAvailable = false;
		OUTPUT_LOG( "error: failed creating DirectSoundCaptureCreate\r\n");
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////////
	// scan all available file format
	ScanAvailableInputFormats();
	return S_OK;
}

HRESULT ParaEngine::CAudioEngine::ReleaseSoundCapture()
{
	//TODO: stop thread and capturing sound

	StopRecording();

	SAFE_DELETE( m_pWaveFile );

	// Release DirectSound interfaces
	SAFE_RELEASE( m_pDSNotify );
	SAFE_RELEASE( m_pDSBCapture );
	SAFE_RELEASE( m_pDSCapture ); 
	

	for(int i=0;i<NOTIFY_EVENT_NUM;++i)
	{
		if(g_hNotificationEvents[i]!=NULL)
		{
			CloseHandle( g_hNotificationEvents[i] );
			g_hNotificationEvents[i] = NULL;
		}
		
	}
	return S_OK;
}

HRESULT ParaEngine::CAudioEngine::RecordCapturedData()
{
	// TODO: this function should be thread safe 
	HRESULT hr;
	VOID*   pbCaptureData    = NULL;
	DWORD   dwCaptureLength;
	VOID*   pbCaptureData2   = NULL;
	DWORD   dwCaptureLength2;
	UINT    dwDataWrote;
	DWORD   dwReadPos;
	DWORD   dwCapturePos;
	LONG lLockSize;

	if( NULL == m_pDSBCapture )
		return S_FALSE;
	if( NULL == m_pWaveFile )
		return S_FALSE;

	if( FAILED( hr = m_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) )
	{
		OUTPUT_LOG("error: error in m_pDSBCapture->GetCurrentPosition\r\n");
		return E_FAIL;
	}

	lLockSize = dwReadPos - m_dwNextCaptureOffset;
	if( lLockSize < 0 )
		lLockSize += m_dwCaptureBufferSize;

	// Block align lock size so that we are always write on a boundary
	lLockSize -= (lLockSize % m_dwNotifySize);

	if( lLockSize == 0 )
		return S_FALSE;

	// Lock the capture buffer down
	if( FAILED( hr = m_pDSBCapture->Lock( m_dwNextCaptureOffset, lLockSize, 
		&pbCaptureData, &dwCaptureLength, 
		&pbCaptureData2, &dwCaptureLength2, 0L ) ) )
	{
		OUTPUT_LOG("error: error in RecordCapturedData ..lock \r\n");
		return E_FAIL;
	}

	// Write the data into the wav file
	if( FAILED( hr = m_pWaveFile->Write( dwCaptureLength, 
		(BYTE*)pbCaptureData, 
		&dwDataWrote ) ) )
	{
		OUTPUT_LOG("error: error in RecordCapturedData .. write\r\n");
		return E_FAIL;
	}

	// Move the capture offset along
	m_dwNextCaptureOffset += dwCaptureLength; 
	m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

	if( pbCaptureData2 != NULL )
	{
		// Write the data into the wav file
		if( FAILED( hr = m_pWaveFile->Write( dwCaptureLength2, 
			(BYTE*)pbCaptureData2, 
			&dwDataWrote ) ) )
		{
			OUTPUT_LOG("error: error in RecordCapturedData ..write \r\n");
			return E_FAIL;
		}

		// Move the capture offset along
		m_dwNextCaptureOffset += dwCaptureLength2; 
		m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
	}

	// Unlock the capture buffer
	m_pDSBCapture->Unlock( pbCaptureData,  dwCaptureLength, 
		pbCaptureData2, dwCaptureLength2 );

	return S_OK;
}

unsigned int ParaEngine::CAudioEngine::RecordSoundLoop( void* parameter )
{
	HRESULT hr;
	DWORD dwResult;
	BOOL bDone = FALSE;
	int nFrameCount = 0;
	
	while( !bDone ) 
	{ 
		dwResult = MsgWaitForMultipleObjects( 2, g_hNotificationEvents, 
			FALSE, INFINITE, QS_ALLEVENTS );
		switch( dwResult )
		{
		case WAIT_OBJECT_0 + 0:
			// g_hNotificationEvents[0] is signaled

			// This means that DirectSound just finished playing 
			// a piece of the buffer, so we need to fill the circular 
			// buffer with new sound from the wav file
			if( FAILED( hr = CGlobals::GetAudioEngine()->RecordCapturedData() ) )
			{
				OUTPUT_LOG("Error occurred when RecordCapturedData() \r\n");
				bDone = TRUE;
			}
			nFrameCount++;

			if(nFrameCount> MAX_RECORDSOUND_LOOP_COUNT)
			{
				OUTPUT_LOG("warning: RecordCapturedData() has recorded too much data. it is suspended internally. \r\n");
				bDone = TRUE;
			}
			break;

		case WAIT_OBJECT_0 + 1:
			// g_hNotificationEvents[1] is signaled;
			// recording has stopped, we will stop this thread as well.

			bDone = TRUE;

			if( FAILED( hr = CGlobals::GetAudioEngine()->RecordCapturedData() ) )
			{
				OUTPUT_LOG("Error occurred when RecordCapturedData() \r\n");
				bDone = TRUE;
			}
			
			break;
		case WAIT_OBJECT_0 + 2:
			// Windows messages are available
			// TODO: this is an infinite loop, we shall quick when capturing is stopped.
			bDone = TRUE;
			//Sleep(200);
			break;
		}
	}
	_endthreadex( 0 );
	return 0;
}

HRESULT ParaEngine::CAudioEngine::InitNotifications()
{
	HRESULT hr; 

	if( NULL == m_pDSBCapture )
		return E_FAIL;

	// Create a notification event, for when the sound stops playing
	if( FAILED( hr = m_pDSBCapture->QueryInterface( IID_IDirectSoundNotify, 
		(VOID**)&m_pDSNotify ) ) )
	{
		OUTPUT_LOG("error: m_pDSBCapture->QueryInterface IID_IDirectSoundNotify \r\n");
		return E_FAIL;
	}

	// Setup the notification positions
	for( INT i = 0; i < NUM_REC_NOTIFICATIONS; i++ )
	{
		m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
		m_aPosNotify[i].hEventNotify = g_hNotificationEvents[NOTIFY_RECORDDATA_EVENT_INDEX];
	}
	// for the recording stopped notification.
	m_aPosNotify[NUM_REC_NOTIFICATIONS].dwOffset = DSBPN_OFFSETSTOP;
	m_aPosNotify[NUM_REC_NOTIFICATIONS].hEventNotify = g_hNotificationEvents[NOTIFY_STOP_EVENT_INDEX];

	// Tell DirectSound when to notify us. the notification will come in the from 
	// of signaled events that are handled in WinMain()
	if( FAILED( hr = m_pDSNotify->SetNotificationPositions( NUM_REC_NOTIFICATIONS+1, m_aPosNotify ) ) )
	{
		OUTPUT_LOG("error: m_pDSBCapture->SetNotificationPositions \r\n");
		return E_FAIL;
	}

	/* Launch CheckKey thread to check for terminating keystroke. */
	unsigned int threadID=0;
	m_hRecordingThread = (HANDLE)_beginthreadex( NULL, 0, &RecordSoundLoop, NULL, 0, &threadID );
	if(m_hRecordingThread==0)
		return E_FAIL;
	return S_OK;
}

HRESULT ParaEngine::CAudioEngine::DestoryNotifications()
{
	if(m_hRecordingThread!=NULL)
	{
		// Wait until second thread terminates. If you comment out the line
		// below, Counter will not be correct because the thread has not
		// terminated, and Counter most likely has not been incremented to
		// 1000000 yet.
		WaitForSingleObject( m_hRecordingThread, INFINITE );

		// Destroy the thread object.
		CloseHandle( m_hRecordingThread );
		m_hRecordingThread = NULL;
	}

	SAFE_RELEASE( m_pDSNotify );

	return S_OK;
}

bool ParaEngine::CAudioEngine::PauseOrResumeRecording( int nState )
{
	// TODO:
	return true;
}

bool ParaEngine::CAudioEngine::IsRecording()
{
	return g_bIsRecording;
}

bool ParaEngine::CAudioEngine::IsRecordingPaused()
{
	return false;
}

const char* ParaEngine::CAudioEngine::GetLastRecordedData( int nTimeLength )
{
	// TODO: just turn the music to strings to be displayed by the GUI.
	return NULL;
}
#endif