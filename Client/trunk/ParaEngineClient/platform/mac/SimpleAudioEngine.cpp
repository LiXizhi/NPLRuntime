

#include "SimpleAudioEngine.h"

namespace ParaEngine
{

SimpleAudioEngine* inst = 0;

SimpleAudioEngine* SimpleAudioEngine::getInstance()
{
    if ( inst == 0 )
    {
        inst = new SimpleAudioEngine();
    }

    return inst;
}

SimpleAudioEngine::SimpleAudioEngine()
{

}

SimpleAudioEngine::~SimpleAudioEngine()
{

}

void SimpleAudioEngine::stopAllEffects()
{

}

void SimpleAudioEngine::pauseEffect(unsigned int id)
{

}

void SimpleAudioEngine::stopBackgroundMusic()
{

}

void SimpleAudioEngine::preloadEffect(const char* filename)
{

}

void SimpleAudioEngine::setBackgroundMusicVolume(float volume)
{

}

void SimpleAudioEngine::setEffectsVolume(float volume)
{

}

float SimpleAudioEngine::getBackgroundMusicVolume()
{
    return 1.0f;
}

unsigned int SimpleAudioEngine::playEffect(const char* filename,bool loop)
{
    return 0;
}

void SimpleAudioEngine::playBackgroundMusic(const char* filename,bool loop)
{

}

void SimpleAudioEngine::rewindBackgroundMusic()
{

}

}
