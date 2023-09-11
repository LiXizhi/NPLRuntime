//---------------------------------------
// File: AudioEngine2.h
// Author: LiXizhi
// Desc: ParaEngine Audio Engine based on the cAudioEngine plugin dll. 
// Date: 2010.6
//---------------------------------------
#include "ParaEngine.h"
#include "PluginAPI.h"
#include "PluginManager.h"
#include "AssetManifest.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "util/StringHelper.h"
#include "util/MidiMsg.h"
#include "AudioEngine2.h"
#include "ParaEngineCore.h"
#include "IParaEngineApp.h"
#include "MCIController.h"

/**@def define to log verbose. */
// #define DEBUG_AUDIO

using namespace ParaEngine;

/** @def the plugin's class_id */
#define AudioEngine_CLASS_ID Class_ID(0x2b903a29, 0x57e409cf)


#if defined(WIN32)
#define DLL_FILE_EXT  "dll"
#elif defined(PLATFORM_MAC)
#define DLL_FILE_EXT "dylib"
#else
#define DLL_FILE_EXT "so"
#endif

#if defined(_DEBUG) && defined(WIN32)
/** @def the plugin's file path under the working directory. It has external dependency on OpenAL*/
const char* AUDIO_ENGINE_DLL_PATH = ("cAudioEngine_d." DLL_FILE_EXT);
#else
const char* AUDIO_ENGINE_DLL_PATH = ("cAudioEngine." DLL_FILE_EXT);
#endif

ParaEngine::CAudioEngine2::CAudioEngine2()
	:m_pAudioEngine(NULL), m_bEnableAudioEngine(true), m_fGlobalVolume(1.f), m_bAutoMoveListener(true), m_fGlobalVolumeBeforeSwitch(0.f), m_fCaptureAudioQuality(0.1f)
{
}

ParaEngine::CAudioEngine2::~CAudioEngine2()
{
	CleanupAudioEngine();
}

CAudioEngine2* ParaEngine::CAudioEngine2::GetInstance()
{
	static CAudioEngine2 g_instance;
	return &g_instance;
}

IParaAudioEngine* ParaEngine::CAudioEngine2::GetInterface()
{
	return m_pAudioEngine;
}


void ParaEngine::CAudioEngine2::Update()
{
	if (IsAudioEngineEnabled() && GetAutoMoveListener())
	{
		CBaseCamera* pCamera = CGlobals::GetScene()->GetCurrentCamera();
		if (pCamera != NULL)
		{
			//CAudioEngine2::GetInstance()->setPosition((const PARAVECTOR3&)(pCamera->GetEyePosition()));
			Vector3 v = pCamera->GetEyePosition();
			move(reinterpret_cast<const PARAVECTOR3&>(v));
			v = pCamera->GetWorldUp();
			setUpVector(reinterpret_cast<const PARAVECTOR3&>(v));
			v = pCamera->GetWorldAhead();
			setDirection(reinterpret_cast<const PARAVECTOR3&>(v));
		}
#ifdef PARAENGINE_MOBILE
		if (m_pAudioEngine)
			m_pAudioEngine->update();
#endif
	}

}

HRESULT ParaEngine::CAudioEngine2::InitAudioEngine(IParaAudioEngine* pInteface)
{
	if (pInteface != 0)
	{
		SAFE_RELEASE(m_pAudioEngine);
		m_pAudioEngine = pInteface;
	}
	else if (m_pAudioEngine == 0)
	{
		DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->LoadDLL("audioengine", AUDIO_ENGINE_DLL_PATH);
		if (pPluginEntity != 0)
		{
			for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
			{
				ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

				if (pClassDesc && pClassDesc->ClassID() == AudioEngine_CLASS_ID)
				{
					m_pAudioEngine = (IParaAudioEngine*)pClassDesc->Create();
				}
			}
		}
	}
	if (m_pAudioEngine == 0)
	{
		OUTPUT_LOG("error: failed loading audio plugin %s\n", AUDIO_ENGINE_DLL_PATH);
	}
	else
	{
		//Allow the user to choose a playback device
		OUTPUT_LOG("Available Audio Playback Devices: \n");
		unsigned int deviceCount = m_pAudioEngine->getAvailableDeviceCount();
		std::string defaultDeviceName = m_pAudioEngine->getDefaultDeviceName();
		for (unsigned int i = 0; i < deviceCount; ++i)
		{
			std::string deviceName = m_pAudioEngine->getAvailableDeviceName(i);
			if (deviceName.compare(defaultDeviceName) == 0)
			{
				OUTPUT_LOG("%d : %s [DEFAULT]\n", i, StringHelper::AnsiToUTF8(deviceName.c_str()));
			}
			else
			{
				OUTPUT_LOG("%d : %s\n", i, StringHelper::AnsiToUTF8(deviceName.c_str()));
			}
		}

		//Initialize the manager with the user settings
		unsigned int deviceSelection = 0;

#ifdef WIN32
		if (!m_pAudioEngine->initialize("DirectSound3D"))
		{
			if (!m_pAudioEngine->initialize(m_pAudioEngine->getAvailableDeviceName(deviceSelection)))
				return E_FAIL;
		}
#else

		if (!m_pAudioEngine->initialize(m_pAudioEngine->getAvailableDeviceName(deviceSelection)))
		{
			return E_FAIL;
		}
#endif


		// use linear distance model by default. 
		m_pAudioEngine->SetDistanceModel(Audio_DistModel_LINEAR_DISTANCE_CLAMPED);

		// tell to use left handed coordinate system, which will invert the z axis. 
		m_pAudioEngine->SetCoordinateSystem(0);

		// register logger
		m_pAudioEngine->registerLogReceiver([](const char* msg) {
			OUTPUT_LOG("cAudioEngine: %s \n", msg);
		});

	}
	return (m_pAudioEngine != 0) ? S_OK : E_FAIL;
}

void ParaEngine::CAudioEngine2::ResetAudioDevice(const string& deviceName)
{
	if (m_pAudioEngine == nullptr)
	{
		InitAudioEngine();
		return;
	}

	m_pAudioEngine->shutDown();

	if (deviceName.empty())
	{
		unsigned int deviceSelection = 0;
#ifdef WIN32
		if (!m_pAudioEngine->initialize("DirectSound3D"))
		{
			if (!m_pAudioEngine->initialize(m_pAudioEngine->getAvailableDeviceName(deviceSelection)))
				OUTPUT_LOG("error: failed initialize audio device %s\n", m_pAudioEngine->getAvailableDeviceName(deviceSelection));
		}
#else

		if (!m_pAudioEngine->initialize(m_pAudioEngine->getAvailableDeviceName(deviceSelection)))
		{
			OUTPUT_LOG("error: failed initialize audio device %s\n", m_pAudioEngine->getAvailableDeviceName(deviceSelection));
		}
#endif
	}
	else
	{
		if (!m_pAudioEngine->initialize(deviceName.c_str()))
		{
			OUTPUT_LOG("error: failed initialize audio device %s\n", deviceName.c_str());
		}
	}
}

unsigned int ParaEngine::CAudioEngine2::GetDeviceCount()
{
	if (m_pAudioEngine != nullptr)
		return m_pAudioEngine->getAvailableDeviceCount();
	else
		return 0;
}

const char* ParaEngine::CAudioEngine2::GetDeviceName(unsigned int index)
{
	if (m_pAudioEngine != nullptr && index >= 0 && index < m_pAudioEngine->getAvailableDeviceCount())
		return m_pAudioEngine->getAvailableDeviceName(index);
	else
		return CGlobals::GetString(0).c_str();
}

void ParaEngine::CAudioEngine2::CleanupAudioEngine()
{
	CMidiMsg::GetSingleton().SafeRelease();
	SAFE_RELEASE(m_pAudioEngine);
	m_audio_file_map.clear();
}

bool ParaEngine::CAudioEngine2::IsValid()
{
	return (m_pAudioEngine != 0);
}

void ParaEngine::CAudioEngine2::EnableAudioEngine(bool bEnable)
{
	m_bEnableAudioEngine = bEnable;
}

void ParaEngine::CAudioEngine2::SetGlobalVolume(const float& volume)
{
	if (m_fGlobalVolume != volume)
	{
		m_fGlobalVolume = volume;

		if (m_pAudioEngine)
		{
			m_pAudioEngine->setMasterVolume(volume);
		}
	}
	m_fGlobalVolumeBeforeSwitch = m_fGlobalVolume;
}

/** async wave file call back. */
class CWaveFilePlayCallBackData2
{
public:
	CWaveFilePlayCallBackData2(const char* sFileName, bool bLoop = false, bool bStream = false) :m_sFileName(sFileName), m_bLoop(bLoop), m_bStream(bStream) {}
	std::string m_sFileName;
	bool m_bLoop;
	bool m_bStream;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if (nResult == 0)
		{
			CAudioSource2_ptr pWave = CAudioEngine2::GetInstance()->Get(m_sFileName.c_str());
			if (pWave)
			{
				// this fixed a bug, while a looping sound is stopped by the user while async loading. 
				m_bLoop = m_bLoop && pWave->IsWaveFileLoopPlaying();

				if (CAudioEngine2::GetInstance()->PrepareWaveFile(pWave, m_sFileName.c_str(), m_bStream) == S_OK)
				{
					if (m_bLoop)
					{
						// we will only play a wave file if it is looped and still in the playing list, without the user calling StopWaveFile() explicitly. 
						// in most cases, it is a background music, which is usually played 10000 times. 
						CAudioEngine2::GetInstance()->PlayWaveFile(m_sFileName.c_str(), m_bLoop, m_bStream);
					}
					else
					{
						// if a wave file is already in the queue but with a small loop count, we will not play it. 
					}
				}
			}
		}
	}
};


HRESULT ParaEngine::CAudioEngine2::PrepareWaveFile(CAudioSource2_ptr& pWave, const char* sWavePath, bool bStream)
{
	// prepare the wave file. 
	bool bSucceed = false;

	if (!pWave)
		return E_FAIL;

	if (pWave->GetSource() != NULL)
	{
		OUTPUT_LOG("warning: repeated all to already prepared wave file %s\n", sWavePath);
		return S_OK;
	}

	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(sWavePath);
	if (pEntry)
	{
		if (pEntry->DoesFileExist())
		{
			// we already downloaded the file, so load it.
			IParaAudioSource* pSource = NULL;
			if (bStream)
			{
				// Since local cache filename does not have file extension, however audio engine needs to have an extension in order to load from file. 
				// so we will create a file with the proper extension in the same directory. 
				std::string filename = pEntry->GetLocalFileName() + "." + CParaFile::GetFileExtension(sWavePath);
				if (!CParaFile::DoesFileExist(filename.c_str(), false))
				{
					if (CParaFile::CopyFile(pEntry->GetLocalFileName().c_str(), filename.c_str(), false))
					{
						pSource = m_pAudioEngine->create(sWavePath, filename.c_str(), bStream);
					}
				}
				else
				{
					pSource = m_pAudioEngine->create(sWavePath, filename.c_str(), bStream);
				}
				pWave->SetFilename(filename);
			}
			else
			{
				// always load from memory if no streaming is specified. 
				ParaEngine::CParaFile file(pEntry->GetLocalFileName().c_str());
				if (!file.isEof())
				{
					std::string file_extension = CParaFile::GetFileExtension(sWavePath);
					pSource = m_pAudioEngine->createFromMemory(sWavePath, file.getBuffer(), file.getSize(), file_extension.c_str());
					pWave->SetFilename(pEntry->GetLocalFileName());
				}
			}
			if (pSource)
			{
				pWave->SetSource(pSource);
				bSucceed = true;
			}
		}
	}
	else
	{
#if !(defined(STATIC_PLUGIN_CAUDIOENGINE)) && !(defined(PARAENGINE_MOBILE) && defined(WIN32))
		if (ParaEngine::CParaFile::DoesFileExist(sWavePath, false))
		{
			IParaAudioSource* pSource = m_pAudioEngine->create(sWavePath, sWavePath, bStream);
			if (pSource)
			{
				pWave->SetFilename(sWavePath);
				pWave->SetSource(pSource);
				bSucceed = true;
			}
		}
		else
#endif
		{
			// currently it will unzip file each time a zipped music is played. We may first check a fixed temp location and play it from there before extracting to it. 
			ParaEngine::CParaFile file(sWavePath);
			if (!file.isEof())
			{
				std::string file_extension = CParaFile::GetFileExtension(sWavePath);
				IParaAudioSource* pSource = m_pAudioEngine->createFromMemory(sWavePath, file.getBuffer(), file.getSize(), file_extension.c_str());
				if (pSource)
				{
					pWave->SetFilename(sWavePath);
					pWave->SetSource(pSource);
					bSucceed = true;
				}
			}
		}
	}
	return (bSucceed) ? S_OK : E_FAIL;
}

HRESULT ParaEngine::CAudioEngine2::PlayWaveFile(const char* sWavePath, bool bLoop, bool bStream/*=false*/, int dwPlayOffset/*=0*/)
{
	if (!IsValidAndEnabled() || GetGlobalVolume() <= 0.f)
		return E_FAIL;
	// check if the wave file is already prepared before
	CAudioSource2_ptr pWave;
	AudioFileMap_type::iterator iter = m_audio_file_map.find(sWavePath);
	if (iter != m_audio_file_map.end())
	{
		pWave = iter->second;
		if (pWave && pWave->m_pSource)
		{
			if (pWave->m_pSource->isPlaying())
			{
				// already playing, so return. 
#ifdef DEBUG_AUDIO
				OUTPUT_LOG("PlayWaveFile: %s already playing. audio pos: %d, audio time:%f, total size %d, volume %f\n", sWavePath,
					pWave->m_pSource->getCurrentAudioPosition(),
					pWave->m_pSource->getCurrentAudioTime(),
					pWave->m_pSource->getTotalAudioSize(),
					pWave->m_pSource->getVolume());
#endif
				return S_OK;
			}
		}
		else
		{
			// it usually means that the music is pending for downloading. 
#ifdef DEBUG_AUDIO
			OUTPUT_LOG("PlayWaveFile: %s pending for download\n", sWavePath);
#endif
			return S_OK;
		}
	}
	else
	{
		// prepare the wave file. 
		bool bSucceed = false;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(sWavePath);
		if (pEntry)
		{
			if (pEntry->DoesFileExist())
			{
				// we already downloaded the file, so load it.
				IParaAudioSource* pSource = NULL;
				if (bStream)
				{
					// Since local cache filename does not have file extension, however audio engine needs to have an extension in order to load from file. 
					// so we will create a file with the proper extension in the same directory. 
					std::string filename = pEntry->GetFullFilePath() + "." + CParaFile::GetFileExtension(sWavePath);
					if (!CParaFile::DoesFileExist(filename.c_str(), false))
					{
						if (CParaFile::CopyFile(pEntry->GetLocalFileName().c_str(), filename.c_str(), false))
						{
							pSource = m_pAudioEngine->create(sWavePath, filename.c_str(), bStream);
						}
					}
					else
					{
						pSource = m_pAudioEngine->create(sWavePath, filename.c_str(), bStream);
					}
					pWave->SetFilename(filename);
				}
				else
				{
					// always load from memory if no streaming is specified. 
					ParaEngine::CParaFile file(pEntry->GetLocalFileName().c_str());
					if (!file.isEof())
					{
						std::string file_extension = CParaFile::GetFileExtension(sWavePath);
						pSource = m_pAudioEngine->createFromMemory(sWavePath, file.getBuffer(), file.getSize(), file_extension.c_str());
						pWave->SetFilename(pEntry->GetLocalFileName());
					}
				}
				if (pSource)
				{
					pWave = new CAudioSource2(sWavePath, pSource);
					m_audio_file_map[sWavePath] = pWave;
					bSucceed = true;
				}
			}
			else
			{
				// push to the queue. 
				m_audio_file_map[sWavePath] = CAudioSource2_ptr(new CAudioSource2(sWavePath));
				CWaveFilePlayCallBackData2 callBack(sWavePath, bLoop, bStream);
				// we shall wait for asset completion. 
				pEntry->SyncFile_Async(callBack);
			}
		}
		else
		{
#if !(defined(STATIC_PLUGIN_CAUDIOENGINE)) && !(defined(PARAENGINE_MOBILE) && defined(WIN32))
			if (ParaEngine::CParaFile::DoesFileExist(sWavePath, false))
			{
				IParaAudioSource* pSource = m_pAudioEngine->create(sWavePath, sWavePath, bStream);
				if (pSource)
				{
					pWave = new CAudioSource2(sWavePath, pSource);
					m_audio_file_map[sWavePath] = pWave;
					bSucceed = true;
#ifdef DEBUG_AUDIO
					OUTPUT_LOG("PlayWaveFile: new audio %s prepared \n", sWavePath);
#endif
				}
			}
			else
#endif
			{
				// currently it will unzip file each time a zipped music is played. We may first check a fixed temp location and play it from there before extracting to it. 
				ParaEngine::CParaFile file(sWavePath);
				if (!file.isEof())
				{
					std::string file_extension = CParaFile::GetFileExtension(sWavePath);
					IParaAudioSource* pSource = m_pAudioEngine->createFromMemory(sWavePath, file.getBuffer(), file.getSize(), file_extension.c_str());
					if (pSource)
					{
						pWave = new CAudioSource2(sWavePath, pSource);
						m_audio_file_map[sWavePath] = pWave;
						bSucceed = true;
					}
				}
			}
		}

		if (!bSucceed)
		{
			m_audio_file_map[sWavePath] = CAudioSource2_ptr(new CAudioSource2(sWavePath));
			OUTPUT_LOG("unable to prepare wave file %s\r\n", sWavePath);
			return E_FAIL;
		}
	}
	// play the sound
	if (pWave)
	{
		if (!(pWave->play2d(bLoop, false)))
		{
			return E_FAIL;
		}
#ifdef DEBUG_AUDIO
		if (pWave->m_pSource)
		{
			OUTPUT_LOG("PlayWaveFile: play2d is called for %s. audio pos: %d, audio time:%f, total size %d, volume %f\n", sWavePath,
				pWave->m_pSource->getCurrentAudioPosition(),
				pWave->m_pSource->getCurrentAudioTime(),
				pWave->m_pSource->getTotalAudioSize(),
				pWave->m_pSource->getVolume());
		}
#endif
	}
	return S_OK;
}

HRESULT ParaEngine::CAudioEngine2::StopWaveFile(const char* sWavePath, ParaAudioFlagsEnum dwFlags)
{
	if (!IsValidAndEnabled())
		return E_FAIL;

	// check if the wave file is already prepared before
	AudioFileMap_type::iterator iter = m_audio_file_map.find(sWavePath);
	if (iter != m_audio_file_map.end())
	{
		CAudioSource2_ptr pWave = iter->second;
		if (pWave && pWave->m_pSource)
		{
			pWave->m_pSource->stop();
		}
		pWave->m_bIsAsyncLoadingWhileLoopPlaying = false;
		return S_OK;
	}
	return S_OK;
}

HRESULT ParaEngine::CAudioEngine2::ReleaseWaveFile(const std::string& sWavePath)
{
	if (!IsValidAndEnabled())
		return E_FAIL;

	// check if the wave file is already prepared before
	AudioFileMap_type::iterator iter = m_audio_file_map.find(sWavePath);
	if (iter != m_audio_file_map.end())
	{
		CAudioSource2_ptr pWave = iter->second;
		if (pWave && pWave->m_pSource)
		{
			m_pAudioEngine->release(pWave->m_pSource);
			pWave->m_pSource = NULL;
		}
		m_audio_file_map.erase(iter);
		return S_OK;
	}
	return S_OK;
}

bool ParaEngine::CAudioEngine2::IsWaveFileInQueue(const string& filename)
{
	AudioFileMap_type::iterator iter = m_audio_file_map.find(filename);
	return (iter != m_audio_file_map.end());
}


bool ParaEngine::CAudioEngine2::IsWaveFileLoopPlaying(const std::string& filename)
{
	AudioFileMap_type::iterator iter = m_audio_file_map.find(filename);
	if (iter != m_audio_file_map.end())
	{
		return iter->second->IsWaveFileLoopPlaying();
	}
	return false;
}

ParaEngine::CAudioEngine2::CAudioPlaybackHistory& ParaEngine::CAudioEngine2::SetHistoryWithCaptureBegin() {
	int nowTime;
	auto pParaEngine = CParaEngineCore::GetInstance()->GetAppInterface()->GetAttributeObject();
	auto pGameFRC = pParaEngine->GetChildAttributeObject("gameFRC");
	pGameFRC->GetAttributeClass()->GetField("Time")->Get(pGameFRC, &nowTime);

	m_PlaybackHistory.Clear();
	m_PlaybackHistory.SetEnable(true);

	AudioFileMap_type::iterator iter = m_audio_file_map.begin();
	while (iter != m_audio_file_map.end()) 
	{
		auto & source = iter->second;
		if (source->IsPlaying()) {
			source->m_nStartTime = nowTime;
			source->m_nSeekPos = source->m_pSource->getCurrentAudioTime();
			m_PlaybackHistory.AddRecord(&(*source));
		}
		iter++;
	}
	return m_PlaybackHistory;
}

ParaEngine::CAudioEngine2::CAudioPlaybackHistory& ParaEngine::CAudioEngine2::SetHistoryWithCaptureEnd() {
	int nowTime;
	auto pParaEngine = CParaEngineCore::GetInstance()->GetAppInterface()->GetAttributeObject();
	auto pGameFRC = pParaEngine->GetChildAttributeObject("gameFRC");
	pGameFRC->GetAttributeClass()->GetField("Time")->Get(pGameFRC, &nowTime);

	AudioFileMap_type::iterator iter = m_audio_file_map.begin();
	while (iter != m_audio_file_map.end())
	{
		auto& source = iter->second;
		if (source->IsPlaying()) {
			auto record = m_PlaybackHistory.FindLastRecord(iter->first);
			if (record!=NULL) {
				record->m_nEndTime = nowTime;
			}
		}
		iter++;
	}

	return m_PlaybackHistory;
}

/** async wave file call back. */
class CWaveFileDownloadCallBackData2
{
public:
	CWaveFileDownloadCallBackData2(const char* sName, const char* sFileName, bool bStream = false)
		: m_sName(sName), m_sFileName(sFileName), m_bStream(bStream) {}
	std::string m_sName;
	std::string m_sFileName;
	bool m_bStream;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if (nResult == 0)
		{
			if (pAssetFileEntry->DoesFileExist())
			{
				// create again once finished. 
				CAudioSource2_ptr pWave = CAudioEngine2::GetInstance()->Get(m_sName.c_str());
				if (!pWave)
				{
					CAudioEngine2::GetInstance()->Create(m_sName.c_str(), m_sFileName.c_str(), m_bStream);
				}
				else
				{
					bool bIsLoopPlaying = pWave->IsWaveFileLoopPlaying();
					if (CAudioEngine2::GetInstance()->PrepareWaveFile(pWave, m_sFileName.c_str(), m_bStream) == S_OK)
					{
						if (bIsLoopPlaying)
						{
							pWave->play2d(true);
						}
					}
				}
			}
		}
	}
};

CAudioSource2_ptr ParaEngine::CAudioEngine2::Create(const char* sName, const char* sWavePath, bool bStream)
{
	CAudioSource2_ptr pWave;
	AudioFileMap_type::iterator iter = m_audio_file_map.find(sName);
	if (iter != m_audio_file_map.end())
	{
		pWave = iter->second;
		if (!pWave)
		{
			pWave = new CAudioSource2(sName);
			iter->second = pWave;
		}
		if (pWave->m_pSource)
		{
			pWave->m_pSource->stop();
		}
	}
	else
	{
		pWave = new CAudioSource2(sName);
		m_audio_file_map[sName] = pWave;
	}
	if (m_pAudioEngine == 0)
		return pWave;

	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(sWavePath);
	if (pEntry)
	{
		if (pEntry->DoesFileExist())
		{
			// we already downloaded the file, so load it.
			IParaAudioSource* pSource = NULL;
#ifdef PARAENGINE_MOBILE
			// streaming should be disabled since it is too slow on mobile disk. 
			OUTPUT_LOG("streaming audio file %s is disabled since it is too slow on mobile disk\n", pEntry->GetLocalFileName().c_str());
			bStream = false;
#endif
			if (bStream)
			{
				// Since local cache filename does not have file extension, however audio engine needs to have an extension in order to load from file. 
				// so we will create a file with the proper extension in the same directory. 
				std::string filename = pEntry->GetFullFilePath() + "." + CParaFile::GetFileExtension(sWavePath);
				// OUTPUT_LOG("info:streaming audio file from %s\n", filename.c_str());
				if (!CParaFile::DoesFileExist(filename.c_str(), false))
				{
					if (CParaFile::CopyFile(pEntry->GetLocalFileName().c_str(), filename.c_str(), true))
					{
						pSource = m_pAudioEngine->create(sName, filename.c_str(), bStream);
					}
					else
					{
						OUTPUT_LOG("warning: failed to copy audio file from %s to %s\n", pEntry->GetLocalFileName().c_str(), filename.c_str());
					}
				}
				else
				{
					pSource = m_pAudioEngine->create(sName, filename.c_str(), bStream);
				}
				pWave->SetFilename(filename);
			}
			else
			{
				// always load from memory if no streaming is specified. 
				ParaEngine::CParaFile file(pEntry->GetLocalFileName().c_str());
				// OUTPUT_LOG("info:playing in memory audio file from %s\n", pEntry->GetLocalFileName().c_str());

				if (!file.isEof())
				{
					std::string file_extension = CParaFile::GetFileExtension(sWavePath);
					pSource = m_pAudioEngine->createFromMemory(sName, file.getBuffer(), file.getSize(), file_extension.c_str());
					pWave->SetFilename(sWavePath);
				}
				else
				{
					OUTPUT_LOG("warning: failed to open audio file %s\n", pEntry->GetLocalFileName().c_str());
				}
			}
			if (pSource)
			{
				pWave->SetSource(pSource);

				// if there is pending looped sound, we will play it. For non-looping sound that has just finished downloading, we will ignore it. 
				if (pWave->m_bIsAsyncLoadingWhileLoopPlaying)
				{
					pWave->play2d(true);
				}
			}
		}
		else
		{
			CWaveFileDownloadCallBackData2 callBack(sName, sWavePath, bStream);
			// we shall wait for asset completion. 
			pEntry->SyncFile_Async(callBack);
		}
	}
	else
	{
#if !(defined(STATIC_PLUGIN_CAUDIOENGINE)) && !(defined(PARAENGINE_MOBILE) && defined(WIN32))
		if (ParaEngine::CParaFile::DoesFileExist(sWavePath, false))
		{
			IParaAudioSource* pSource = m_pAudioEngine->create(sName, sWavePath, bStream);
			if (pSource)
			{
				pWave->SetFilename(sWavePath);
				pWave->SetSource(pSource);
			}
		}
		else
#endif
		{
			// currently it will unzip file each time a zipped music is played. We may first check a fixed temp location and play it from there before extracting to it. 
			ParaEngine::CParaFile file(sWavePath);
			if (!file.isEof())
			{
#ifdef PARAENGINE_MOBILE
				OUTPUT_LOG("audio file opened: %s \n", sWavePath);
#endif
				std::string file_extension = CParaFile::GetFileExtension(sWavePath);
				IParaAudioSource* pSource = m_pAudioEngine->createFromMemory(sWavePath, file.getBuffer(), file.getSize(), file_extension.c_str());
				if (pSource)
				{
					// Note, this is from zip archive
					pWave->SetFilename(sWavePath);
					pWave->SetSource(pSource);
				}
			}
		}
	}
	return pWave;
}

CAudioSource2_ptr ParaEngine::CAudioEngine2::Get(const char* sName)
{
	CAudioSource2_ptr pWave;
	AudioFileMap_type::iterator iter = m_audio_file_map.find(sName);
	if (iter != m_audio_file_map.end())
	{
		pWave = iter->second;
		return pWave;
	}
	return pWave;
}

CAudioSource2_ptr ParaEngine::CAudioEngine2::CreateGet(const char* sName, const char* sWavePath, bool bStream)
{
	CAudioSource2_ptr pWave = Get(sName);
	if (pWave)
	{
		return pWave;
	}
	else
	{
		return Create(sName, sWavePath, bStream);
	}
}

void ParaEngine::CAudioEngine2::release(CAudioSource2_ptr& audio_src)
{
	if (audio_src && audio_src->GetSource())
	{
		ReleaseWaveFile(audio_src->GetName());
	}
}

void ParaEngine::CAudioEngine2::setPosition(const PARAVECTOR3& pos)
{
	if (m_pAudioEngine)
		m_pAudioEngine->setPosition((pos));
}

void ParaEngine::CAudioEngine2::setDirection(const PARAVECTOR3& dir)
{
	if (m_pAudioEngine)
		m_pAudioEngine->setDirection((dir));
}

void ParaEngine::CAudioEngine2::setUpVector(const PARAVECTOR3& up)
{
	if (m_pAudioEngine)
		m_pAudioEngine->setUpVector((up));
}

void ParaEngine::CAudioEngine2::setVelocity(const PARAVECTOR3& vel)
{
	if (m_pAudioEngine)
		m_pAudioEngine->setVelocity((vel));
}

void ParaEngine::CAudioEngine2::move(const PARAVECTOR3& pos)
{
	if (m_pAudioEngine)
		m_pAudioEngine->move((pos));
}

ParaEngine::PARAVECTOR3 ParaEngine::CAudioEngine2::getPosition() const
{
	if (m_pAudioEngine)
	{
		return (m_pAudioEngine->getPosition());
	}
	return PARAVECTOR3(0, 0, 0);
}

ParaEngine::PARAVECTOR3 ParaEngine::CAudioEngine2::getDirection() const
{
	if (m_pAudioEngine)
	{
		return (m_pAudioEngine->getDirection());
	}
	return PARAVECTOR3(0, 0, 0);
}

ParaEngine::PARAVECTOR3 ParaEngine::CAudioEngine2::getUpVector() const
{
	if (m_pAudioEngine)
	{
		return (m_pAudioEngine->getUpVector());
	}
	return PARAVECTOR3(0, 0, 0);
}

ParaEngine::PARAVECTOR3 ParaEngine::CAudioEngine2::getVelocity() const
{
	if (m_pAudioEngine)
	{
		return (m_pAudioEngine->getVelocity());
	}
	return PARAVECTOR3(0, 0, 0);
}

void ParaEngine::CAudioEngine2::SetDistanceModel(ParaAudioDistanceModelEnum eDistModel)
{
	if (m_pAudioEngine)
	{
		m_pAudioEngine->SetDistanceModel(eDistModel);
	}
}

void ParaEngine::CAudioEngine2::OnSwitch(bool bOn)
{
	if (bOn)
	{
		if (m_fGlobalVolumeBeforeSwitch > 0.f)
		{
			CAudioEngine2::GetInstance()->SetGlobalVolume(m_fGlobalVolumeBeforeSwitch);
		}
	}
	else
	{
		float fVol = GetGlobalVolume();

		if (fVol > 0.f)
		{
			SetGlobalVolume(0.f);
			m_fGlobalVolumeBeforeSwitch = fVol;
		}
	}
}

float ParaEngine::CAudioEngine2::GetCaptureAudioQuality() const
{
	return m_fCaptureAudioQuality;
}

void ParaEngine::CAudioEngine2::SetCaptureAudioQuality(float val)
{
	m_fCaptureAudioQuality = val;
}

int ParaEngine::CAudioEngine2::GetCaptureFrequency()
{
	auto pAutoCapture = CreateGetAudioCapture();
	if (pAutoCapture)
	{
		return pAutoCapture->getFrequency();
	}
	return 0;
}

void ParaEngine::CAudioEngine2::SetCaptureFrequency(int nFrequency)
{
	auto pAutoCapture = CreateGetAudioCapture();
	if (pAutoCapture)
	{
		pAutoCapture->setFrequency(nFrequency);
	}
}

ParaEngine::CAudioEngine2::CAudioPlaybackHistory& ParaEngine::CAudioEngine2::GetPlaybackHistory()
{
	return m_PlaybackHistory;
}

void ParaEngine::CAudioSource2::onUpdate()
{
#ifdef DEBUG_AUDIO
	OUTPUT_LOG("audio source onUpdate: %s \n", m_name.c_str());
#endif
}

void ParaEngine::CAudioSource2::onRelease()
{
#ifdef DEBUG_AUDIO
	OUTPUT_LOG("audio source onRelease: %s \n", m_name.c_str());
#endif
}

void ParaEngine::CAudioSource2::onPlay()
{
#ifdef DEBUG_AUDIO
	OUTPUT_LOG("audio source onPlay: %s \n", m_name.c_str());
#endif
}

void ParaEngine::CAudioSource2::onStop()
{
#ifdef DEBUG_AUDIO
	OUTPUT_LOG("audio source onStop: %s \n", m_name.c_str());
#endif
	if (m_bReleaseOnStop)
	{

	}
}

void ParaEngine::CAudioSource2::onPause()
{
#ifdef DEBUG_AUDIO
	OUTPUT_LOG("audio source onPause: %s \n", m_name.c_str());
#endif
}

bool ParaEngine::CAudioSource2::play()
{
	if (m_pSource)
	{
		return m_pSource->play();
	}
	return false;
}

bool ParaEngine::CAudioSource2::play2d(const bool& toLoop /*= false*/, bool bIgnoreIfPlaying /*=true*/)
{
	if (m_pSource)
	{
		if (!(bIgnoreIfPlaying && m_pSource->isPlaying()))
		{
			auto pParaEngine = CParaEngineCore::GetInstance()->GetAppInterface()->GetAttributeObject();
			auto pGameFRC = pParaEngine->GetChildAttributeObject("gameFRC");
			pGameFRC->GetAttributeClass()->GetField("Time")->Get(pGameFRC, &m_nStartTime);

			// record this event to the playback history
			CAudioEngine2::GetInstance()->GetPlaybackHistory().AddRecord(this);

			return m_pSource->play2d(toLoop);
		}
	}
	else
	{
		m_bIsAsyncLoadingWhileLoopPlaying = toLoop;
	}
	return false;
}

bool ParaEngine::CAudioSource2::play3d(const PARAVECTOR3& position, const float& soundstr /*= 1.0 */, const bool& toLoop /*= false*/)
{
	if (m_pSource && !(m_pSource->isPlaying()))
	{
#ifdef PARAENGINE_MOBILE
		/** 3d sound is disabled for mobile version */
		m_pSource->loop(toLoop);
		return true;
#else
		return m_pSource->play3d(position, soundstr, toLoop);
#endif
	}
	return false;
}

void ParaEngine::CAudioSource2::pause()
{
	if (m_pSource)
	{
		m_pSource->pause();
	}
	m_bIsAsyncLoadingWhileLoopPlaying = false;
}

void ParaEngine::CAudioSource2::stop()
{
	if (m_pSource)
	{
		m_pSource->stop();
		auto pParaEngine = CParaEngineCore::GetInstance()->GetAppInterface()->GetAttributeObject();
		auto pGameFRC = pParaEngine->GetChildAttributeObject("gameFRC");
		pGameFRC->GetAttributeClass()->GetField("Time")->Get(pGameFRC, &m_nStopTime);
		// record this event to the playback history
		auto record = CAudioEngine2::GetInstance()->GetPlaybackHistory().FindLastRecord(this->GetFilename());
		if (record!=NULL) { 
			record->m_nEndTime = m_nStopTime;
		}
	}
	m_bIsAsyncLoadingWhileLoopPlaying = false;
}

void ParaEngine::CAudioSource2::loop(const bool& toLoop)
{
	if (m_pSource)
	{
		m_pSource->loop(toLoop);
	}
}

bool ParaEngine::CAudioSource2::IsAsyncLoadingWhileLoopPlaying()
{
	return m_bIsAsyncLoadingWhileLoopPlaying;
}

bool ParaEngine::CAudioSource2::IsWaveFileLoopPlaying()
{
	if (m_pSource)
		return m_pSource->isLooping() && m_pSource->isPlaying();
	else
		return IsAsyncLoadingWhileLoopPlaying();
}

/** whether the file is looping or not.*/
bool ParaEngine::CAudioSource2::IsLooping()
{
	return m_pSource ? m_pSource->isLooping() : false;
}

bool ParaEngine::CAudioSource2::IsPlaying()
{
	if (m_pSource)
		return m_pSource->isPlaying();
	else
		return IsAsyncLoadingWhileLoopPlaying();
}

const std::string& ParaEngine::CAudioSource2::GetFilename() const
{
	return !m_filename.empty() ? m_filename : m_name;
}

void ParaEngine::CAudioSource2::SetFilename(const std::string& val)
{
	if (m_name == val)
		m_filename.clear();
	else
		m_filename = val;
}

void ParaEngine::CAudioEngine2::PauseAll()
{
	m_paused_audios.clear();
	for (auto iter : m_audio_file_map)
	{
		CAudioSource2_ptr pAudioSrc = iter.second;
		if (pAudioSrc && pAudioSrc->IsPlaying())
		{
			pAudioSrc->pause();
			m_paused_audios.push_back(pAudioSrc);
		}
	}
}

void ParaEngine::CAudioEngine2::ResumeAll()
{
	for (auto pAudioSrc : m_paused_audios)
	{
		if (pAudioSrc)
			pAudioSrc->play();
	}
	m_paused_audios.clear();
}

MCIController* ParaEngine::CAudioEngine2::getMCIController()
{
	static MCIController controller;
	return &controller;
}

ParaEngine::IParaAudioCapture* ParaEngine::CAudioEngine2::CreateGetAudioCapture()
{
	if (m_pAudioEngine)
	{
		return m_pAudioEngine->CreateGetAudioCapture();
	}
	return NULL;
}

int ParaEngine::CAudioEngine2::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass != NULL);

	pClass->AddField("DeviceName", FieldType_String, (void*)SetDeviceName_s, (void*)GetDeviceName_s, NULL, NULL, bOverride);
	pClass->AddField("CaptureAudioQuality", FieldType_Float, (void*)SetCaptureAudioQuality_s, (void*)GetCaptureAudioQuality_s, NULL, "[0.1, 1]", bOverride);
	pClass->AddField("CaptureFrequency", FieldType_Int, (void*)SetCaptureFrequency_s, (void*)GetCaptureFrequency_s, NULL, "16000", bOverride);

	return S_OK;
}

