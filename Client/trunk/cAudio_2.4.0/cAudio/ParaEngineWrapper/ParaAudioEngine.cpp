//-----------------------------------------------------------------------------
// Class:	ParaEngine Audio Engine Wrapper for cAudio engine(OpenAL, MP3, Vorbis OGG, WAV)
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.6.26
//-----------------------------------------------------------------------------
#include "PluginAPI.h"
#include "cAudioSource.h"
#include "ParaAudioEngine.h"
#include "ParaAudioCapture.h"
#include "cMP3Plugin.h"
#include "cPluginManager.h"
#include "ILogReceiver.h"

using namespace ParaEngine;
using namespace cAudio;

/** 0 for left hand(directX), 1 for right hand(openGL, default).*/
static int g_nCoordinateSystem = 1;

class ParaAudioLogReceiver : public ILogReceiver
{
public:
	ParaAudioLogReceiver(std::function<void(const char * msg)> receiver) : m_r(receiver) {};
	virtual bool OnLogMessage(const char* sender, const char* message, LogLevel level, float time)
	{
		//std::cout << "[" << LogLevelStrings[level] << "] " << message << std::endl;
		if (m_r)
		{
			m_r(message);
		}

		return true;
	}

private:
	std::function<void(const char * msg)>  m_r;
};
static ParaAudioLogReceiver* s_logReceiver = NULL;

PARAVECTOR3 ParaEngine::FixCoordinate(const PARAVECTOR3& v)
{
	return (g_nCoordinateSystem == 1) ? v : PARAVECTOR3(v.x, v.y, -v.z);
}

CParaAudioEngine::CParaAudioEngine()
	:m_audio_manager(NULL), m_plugin_mp3(NULL), m_pAudioCapture(NULL)
{
#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
	// load plugins for MP3 and effects
	cPluginManager* pluginManager = cAudio::cPluginManager::Instance();
	m_plugin_mp3 = new cMP3DecoderPlugin();
	if (m_plugin_mp3)
	{
		pluginManager->installPlugin(m_plugin_mp3, "plugin_mp3");
	}
#endif

	//Create an initialized Audio Manager
	m_audio_manager = cAudio::createAudioManager(false);

	m_deviceList = cAudio::createAudioDeviceList();
	
	// Fixed a crash by LiXizhi: we will reset the error state here.
	alGetError();
}

CParaAudioEngine::~CParaAudioEngine()
{
	getLogger()->unRegisterLogReceiver("ParaEngine");
	SAFE_DELETE(s_logReceiver);
	SAFE_DELETE(m_pAudioCapture);
	
	if(m_audio_manager!=NULL)
	{
		//Delete all IAudio sounds
		releaseAllSources();
		//Shutdown cAudio
		shutDown();
		// destroy 
		cAudio::destroyAudioManager(m_audio_manager);
	}
	if (m_plugin_mp3 != NULL)
		m_plugin_mp3->drop();
}

void CParaAudioEngine::Release()
{
	delete this;
}


void ParaEngine::CParaAudioEngine::registerLogReceiver(std::function<void(const char * msg)> receiver)
{
	getLogger()->setLogLevel(LogLevel::ELL_INFO);
	
	if (s_logReceiver == NULL)
	{
		s_logReceiver = new ParaAudioLogReceiver(receiver);
	}
	
	// we will remove other log receivers if ParaEngine log is used.
	getLogger()->unRegisterLogReceiver("File");
	getLogger()->unRegisterLogReceiver("Console");
	getLogger()->registerLogReceiver(s_logReceiver, "ParaEngine");
}

void ParaEngine::CParaAudioEngine::SetDistanceModel( ParaAudioDistanceModelEnum eDistModel )
{
	/*ALenum distModel = AL_NONE;
	switch(eDistModel)
	{
	case Audio_DistModel_EXPONENT_DISTANCE:
		distModel = AL_EXPONENT_DISTANCE;
		break;
	case Audio_DistModel_EXPONENT_DISTANCE_CLAMPED:
		distModel = AL_EXPONENT_DISTANCE_CLAMPED;
		break;
	case Audio_DistModel_INVERSE_DISTANCE:
		distModel = AL_INVERSE_DISTANCE;
		break;
	case Audio_DistModel_INVERSE_DISTANCE_CLAMPED:
		distModel = AL_INVERSE_DISTANCE_CLAMPED;
		break;
	case Audio_DistModel_LINEAR_DISTANCE:
		distModel = AL_LINEAR_DISTANCE;
		break;
	case Audio_DistModel_LINEAR_DISTANCE_CLAMPED:
		distModel = AL_LINEAR_DISTANCE_CLAMPED;
		break;
	default:
		distModel = AL_NONE;
		break;
	}
	alDistanceModel(distModel);*/
}

IParaAudioSource* ParaEngine::CParaAudioEngine::getSoundByName( const char* name )
{
	// m_audio_manager->play();
	return NULL;
}

void ParaEngine::CParaAudioEngine::release( IParaAudioSource* source )
{
	if(source)
	{
		AudioMap_Type::iterator it, itEnd = m_audio_source_map.end();
		for ( it=m_audio_source_map.begin(); it != itEnd; it++ )
		{
			if( ((IParaAudioSource*)(it->second)) == source )
			{
				m_audio_manager->release(it->second->m_pSource);
				SAFE_DELETE(it->second);
				m_audio_source_map.erase(it);
				break;
			}
		}
	}
}

IParaAudioSource* ParaEngine::CParaAudioEngine::create( const char* name, const char* filename, bool stream /*= false*/ )
{
	IParaAudioSource* pSource = getSoundByName(name);
	if(pSource!=0)
		return pSource;
	// Fixed a crash by LiXizhi: we will reset the error state here.
	alGetError();
	IAudioSource* pSrc = m_audio_manager->create(name, filename, stream);
	if(pSrc)
	{
		CParaAudioSource* pSrc_ = new CParaAudioSource(pSrc);
		m_audio_source_map[name] =pSrc_;
		return pSrc_;
	}
	return NULL;
}

IParaAudioSource* ParaEngine::CParaAudioEngine::createFromMemory( const char* name, const char* data, size_t length, const char* extension )
{
	IParaAudioSource* pSource = getSoundByName(name);
	if(pSource!=0)
		return pSource;
	// Fixed a crash by LiXizhi: we will reset the error state here.
	alGetError();
	IAudioSource* pSrc = m_audio_manager->createFromMemory(name, data, length, extension);
	if(pSrc)
	{
		CParaAudioSource* pSrc_ = new CParaAudioSource(pSrc);
		m_audio_source_map[name] =pSrc_;
		return pSrc_;
	}
	return NULL;
}

IParaAudioSource* ParaEngine::CParaAudioEngine::createFromRaw( const char* name, const char* data, size_t length, unsigned int frequency, ParaAudioFormats format )
{
	IParaAudioSource* pSource = getSoundByName(name);
	if(pSource!=0)
		return pSource;
	// Fixed a crash by LiXizhi: we will reset the error state here.
	alGetError();
	IAudioSource* pSrc = m_audio_manager->createFromRaw(name, data, length, frequency, (AudioFormats)format);
	if(pSrc)
	{
		CParaAudioSource* pSrc_ = new CParaAudioSource(pSrc);
		m_audio_source_map[name] =pSrc_;
		return pSrc_;
	}
	return NULL;
}

void ParaEngine::CParaAudioEngine::releaseAllSources()
{
	AudioMap_Type::iterator it, itEnd = m_audio_source_map.end();
	for ( it=m_audio_source_map.begin(); it != itEnd; it++ )
	{
		CParaAudioSource* pSrc = it->second;
		SAFE_DELETE(pSrc);
	}
	m_audio_source_map.clear();

	m_audio_manager->releaseAllSources();
}

void ParaEngine::CParaAudioEngine::SetCoordinateSystem( int nLeftHand )
{
	g_nCoordinateSystem = nLeftHand;
}

const char* ParaEngine::CParaAudioEngine::getAvailableDeviceName(unsigned int index)
{
	auto ret = m_deviceList->getDeviceName(index).c_str();
	return toUTF8(ret);
}

unsigned int ParaEngine::CParaAudioEngine::getAvailableDeviceCount()
{
	return m_deviceList->getDeviceCount();
}

const char* ParaEngine::CParaAudioEngine::getDefaultDeviceName()
{
	return toUTF8(m_deviceList->getDefaultDeviceName().c_str());
}

bool ParaEngine::CParaAudioEngine::initialize(const char* deviceName /*= 0x0*/, int outputFrequency /*= -1*/, int eaxEffectSlots /*= 4*/)
{
	return m_audio_manager->initialize(deviceName, outputFrequency, eaxEffectSlots);
}


void ParaEngine::CParaAudioEngine::shutDown()
{
	m_audio_manager->shutDown();
}

ParaEngine::IParaAudioCapture* ParaEngine::CParaAudioEngine::CreateGetAudioCapture(bool initializeDefault /*= true*/)
{
	if (! m_pAudioCapture)
	{
		m_pAudioCapture = new CParaAudioCapture();
	}
	return m_pAudioCapture;
}

//! If threading is disabled, you must call this function every frame to update the playback buffers of audio sources.  Otherwise it should not be called.
void ParaEngine::CParaAudioEngine::update()
{
#ifndef CAUDIO_USE_INTERNAL_THREAD
	if (m_audio_manager) {
		m_audio_manager->update();
	}
	if (m_pAudioCapture) {
		m_pAudioCapture->updateCaptureBuffer();
	}
#endif
}

///////////////////////////////////////////////////////////////////////
// Audio Source
///////////////////////////////////////////////////////////////////////

void ParaEngine::CParaAudioSource::registerEventHandler( IAudioSourceEventHandler* handler )
{
	if(m_pEventHandler == NULL && m_pSource!=NULL)
	{
		m_pEventHandler = handler;
		m_pSource->registerEventHandler(this);
	}
}

void ParaEngine::CParaAudioSource::unRegisterAllEventHandlers()
{
	m_pEventHandler = NULL;
	m_pSource->unRegisterAllEventHandlers();
}

void ParaEngine::CParaAudioSource::onPlay()
{
	if(m_pEventHandler!=0)
	{
		m_pEventHandler->onPlay();
	}
}

void ParaEngine::CParaAudioSource::onStop()
{
	if(m_pEventHandler!=0)
	{
		m_pEventHandler->onPlay();
	}
}

void ParaEngine::CParaAudioSource::onUpdate()
{
	if(m_pEventHandler!=0)
	{
		m_pEventHandler->onUpdate();
	}
}

void ParaEngine::CParaAudioSource::onRelease()
{
	if(m_pEventHandler!=0)
	{
		m_pEventHandler->onRelease();
	}
}

void ParaEngine::CParaAudioSource::onPause()
{
	if(m_pEventHandler!=0)
	{
		m_pEventHandler->onPause();
	}
}

void ParaEngine::CParaAudioSource::setMaxDistance(const float& maxDistance)
{
	m_pSource->setMaxAttenuationDistance(maxDistance);
}
