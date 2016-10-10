#ifndef AUDIO_ENGINE_MAC_H
#define AUDIO_ENGINE_MAC_H

#include "IParaAudioEngine.h"

namespace ParaEngine
{

    class MacAudioSourceEventHandler : public IAudioSourceEventHandler
    {

    }

    class MacParaAudioSource : public IParaAudioSource
    {

    }

    class MacParaAudioEngine : public IParaAudioEngine
    {
    public:
        static MacParaAudioEngine* GetInstance();
    }
}

#endif
