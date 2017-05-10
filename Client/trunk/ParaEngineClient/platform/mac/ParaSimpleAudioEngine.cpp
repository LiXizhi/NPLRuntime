//-----------------------------------------------------------------------------
// Class: Simple Audio Engine
// Authors:	LiXizhi
// Company:	ParaEngine
// Date: 2014.10.14
// Desc: just a simple wrapper to cocos' simple audio engine.
// Supporting create in memory buffer by writing to a temporary file.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef STATIC_PLUGIN_CAUDIOENGINE
//#include "cocos2d.h"
#include "FileManager.h"
#include "FileUtils.h"
#include "ParaSimpleAudioSource.h"
#include "ParaSimpleAudioEngine.h"
#include "SimpleAudioEngine.h"


//using namespace CocosDenshion;

//USING_NS_CC;
using namespace ParaEngine;

ParaEngine::CParaSimpleAudioEngine::CParaSimpleAudioEngine()
	:m_nLastFileID(0)
{
	m_sTempAudioDirectory = "temp/audio/";
	PrepareAndCleanTempDirectory();

}

ParaEngine::CParaSimpleAudioEngine::~CParaSimpleAudioEngine()
{
}

CParaSimpleAudioEngine* ParaEngine::CParaSimpleAudioEngine::GetInstance()
{
	static CParaSimpleAudioEngine s_singleton;
	return &s_singleton;
}

void ParaEngine::CParaSimpleAudioEngine::PrepareAndCleanTempDirectory()
{
	std::string sFileName = m_sTempAudioDirectory + "1.txt";
	if (!CParaFile::MakeDirectoryFromFilePath(sFileName.c_str()))
	{
		OUTPUT_LOG("warn: failed to create directory for audio temp file:%s \n", m_sTempAudioDirectory.c_str());
	}
	else
	{
		// delete all files in the directory.
		std::string sFilePattern = m_sTempAudioDirectory + "*.*";
		CFileUtils::DeleteFiles(sFilePattern, true);
	}
}

void ParaEngine::CParaSimpleAudioEngine::Release()
{
	shutDown();
}

bool ParaEngine::CParaSimpleAudioEngine::initialize(const char* deviceName /*= 0x0*/, int outputFrequency /*= -1*/, int eaxEffectSlots /*= 4*/)
{
	return true;
}

void ParaEngine::CParaSimpleAudioEngine::shutDown()
{
	releaseAllSources();
}

void ParaEngine::CParaSimpleAudioEngine::update()
{
	AudioMap_Type::iterator it, itEnd = m_audio_source_map.end();
	for (it = m_audio_source_map.begin(); it != itEnd; it++)
	{
		CParaSimpleAudioSource* pSrc = it->second;
		pSrc->update();
	}
}

IParaAudioSource* ParaEngine::CParaSimpleAudioEngine::getSoundByName(const char* name)
{
	// we do not cache sound by name
	return NULL;
}

void ParaEngine::CParaSimpleAudioEngine::releaseAllSources()
{
	m_nLastFileID = 0;
	AudioMap_Type::iterator it, itEnd = m_audio_source_map.end();
	for (it = m_audio_source_map.begin(); it != itEnd; it++)
	{
		CParaSimpleAudioSource* pSrc = it->second;
		SAFE_DELETE(pSrc);
	}
	m_audio_source_map.clear();
	SimpleAudioEngine::getInstance()->stopAllEffects();
}

void ParaEngine::CParaSimpleAudioEngine::release(IParaAudioSource* source)
{
	if (source)
	{
		AudioMap_Type::iterator it, itEnd = m_audio_source_map.end();
		for (it = m_audio_source_map.begin(); it != itEnd; it++)
		{
			if (((IParaAudioSource*)(it->second)) == source)
			{
				SAFE_DELETE(it->second);
				m_audio_source_map.erase(it);
				break;
			}
		}
	}
}

const char* ParaEngine::CParaSimpleAudioEngine::getAvailableDeviceName(unsigned int index)
{
	return CGlobals::GetString(0).c_str();
}

unsigned int ParaEngine::CParaSimpleAudioEngine::getAvailableDeviceCount()
{
	return 0;
}

const char* ParaEngine::CParaSimpleAudioEngine::getDefaultDeviceName()
{
	return CGlobals::GetString(0).c_str();
}

IParaAudioSource* ParaEngine::CParaSimpleAudioEngine::create(const char* name, const char* filename_, bool stream /*= false*/)
{
	IParaAudioSource* pSource = getSoundByName(name);
	if (pSource != 0)
		return pSource;
	std::string filename = filename_;
	std::string sFileExtension = CParaFile::GetFileExtension(filename);

	// TODO: Android support ogg, but iOS does not. shall we convert to wav instead?
	//if (sFileExtension == "ogg")
	//{
	//	sFileExtension = "wav";
	//	// replace ogg audio file with wav file.
	//	filename = CParaFile::ChangeFileExtension(filename, sFileExtension);
	//}

	DWORD dwFileFound = CParaFile::DoesFileExist2(filename.c_str(), FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH);
	if (dwFileFound != FILE_NOT_FOUND)
	{
		if ((dwFileFound & FILE_ON_ZIP_ARCHIVE) > 0)
		{
			CParaFile file;
			if (file.OpenFile(filename.c_str(), true, 0, false, dwFileFound))
			{
				return createFromMemory(name, file.getBuffer(), file.getSize(), sFileExtension.c_str());
			}
		}
		else
		{
			CParaSimpleAudioSource* pSrc = new CParaSimpleAudioSource(filename.c_str());
			SimpleAudioEngine::getInstance()->preloadEffect(filename.c_str());
			m_audio_source_map[name] = pSrc;
			return (IParaAudioSource*)pSrc;
		}
	}
	return NULL;
}


std::string ParaEngine::CParaSimpleAudioEngine::GetTempFileName(const char* name, bool* bIsFileExist)
{
	std::string filename = m_sTempAudioDirectory;
	int nFileID = 0;
	auto iter = m_temporary_ids.find(name);
	if (iter != m_temporary_ids.end())
	{
		if (bIsFileExist)
			*bIsFileExist = true;
		nFileID = iter->second;
	}
	else
	{
		m_temporary_ids[name] = m_nLastFileID;
		nFileID = m_nLastFileID;
		m_nLastFileID++;
		if (bIsFileExist)
			*bIsFileExist = false;
	}
	char fileid[32];
	itoa(nFileID, fileid, 10);
	filename += fileid;
	return filename;
}


IParaAudioSource* ParaEngine::CParaSimpleAudioEngine::createFromMemory(const char* name, const char* data, size_t length, const char* extension)
{
	CParaFile file;
	bool bFileExist = false;
	std::string filename = GetTempFileName(name, &bFileExist);
	filename.append(".");
	filename.append(extension);
	filename = CParaFile::GetWritablePath() + filename;
	if (bFileExist)
	{
		CParaSimpleAudioSource* pSrc = new CParaSimpleAudioSource(filename.c_str());
		SimpleAudioEngine::getInstance()->preloadEffect(filename.c_str());
		m_audio_source_map[name] = pSrc;
		return (IParaAudioSource*)pSrc;
	}
	else
	{
		if (file.OpenFile(filename.c_str(), false))
		{
			OUTPUT_LOG("Audio Engine extract audio data %s to %s \n", name, filename.c_str());
			file.write(data, (int)length);
			file.close();

			CParaSimpleAudioSource* pSrc = new CParaSimpleAudioSource(filename.c_str());
			SimpleAudioEngine::getInstance()->preloadEffect(filename.c_str());
			m_audio_source_map[name] = pSrc;
			return (IParaAudioSource*)pSrc;
		}
		else
		{
			OUTPUT_LOG("error: Audio Engine failed to extract audio data %s  to %s \n", name, filename.c_str());
		}
	}


	return NULL;
}

IParaAudioSource* ParaEngine::CParaSimpleAudioEngine::createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, ParaAudioFormats format)
{
	return NULL;
}

void ParaEngine::CParaSimpleAudioEngine::SetDistanceModel(ParaAudioDistanceModelEnum eDistModel)
{

}

void ParaEngine::CParaSimpleAudioEngine::setPosition(const PARAVECTOR3& pos)
{

}

void ParaEngine::CParaSimpleAudioEngine::setDirection(const PARAVECTOR3& dir)
{

}

void ParaEngine::CParaSimpleAudioEngine::setUpVector(const PARAVECTOR3& up)
{

}

void ParaEngine::CParaSimpleAudioEngine::setVelocity(const PARAVECTOR3& vel)
{

}

void ParaEngine::CParaSimpleAudioEngine::setMasterVolume(const float& volume)
{
	SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(volume);
	SimpleAudioEngine::getInstance()->setEffectsVolume(volume);
}

void ParaEngine::CParaSimpleAudioEngine::move(const PARAVECTOR3& pos)
{

}

ParaEngine::PARAVECTOR3 ParaEngine::CParaSimpleAudioEngine::getPosition(void) const
{
	return PARAVECTOR3(0, 0, 0);
}

ParaEngine::PARAVECTOR3 ParaEngine::CParaSimpleAudioEngine::getDirection(void) const
{
	return PARAVECTOR3(0, 1.f, 0);
}

ParaEngine::PARAVECTOR3 ParaEngine::CParaSimpleAudioEngine::getUpVector(void) const
{
	return PARAVECTOR3(0, 1.f, 0);
}

ParaEngine::PARAVECTOR3 ParaEngine::CParaSimpleAudioEngine::getVelocity(void) const
{
	return PARAVECTOR3(0, 0, 0);
}

float ParaEngine::CParaSimpleAudioEngine::getMasterVolume(void) const
{
	SimpleAudioEngine::getInstance()->getBackgroundMusicVolume();
	return 1;
}

void ParaEngine::CParaSimpleAudioEngine::SetCoordinateSystem(int nLeftHand)
{

}
#endif
