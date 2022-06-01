//
// Created by wuxiangan on 2022/4/27.
//

#ifndef ANDROIDSTUDIO_PARAENGINEMEDIAPLAYER_H
#define ANDROIDSTUDIO_PARAENGINEMEDIAPLAYER_H
#include <jni.h>
#include <string>

#include "IMediaPlayer.h"

namespace ParaEngine {
    class ParaEngineMediaPlayer: public IMediaPlayer {
    public:
        static ParaEngineMediaPlayer* GetSingleton();
        virtual void PlayMidiNote(int note, int velocity);
        virtual void PlayMidiNote(int note, int velocity, int baseNote);
        virtual void PlayMidiNote(int note, int velocity, int baseNote, int channel);
        virtual void PlayMidiFile(std::string filename);
        virtual void SetVolume(float volume);
        virtual void PlayMidiData(std::vector<char> datas);
    private:
        static const std::string classname;
    };
}


#endif //ANDROIDSTUDIO_PARAENGINEMEDIAPLAYER_H
