//
// Created by wuxiangan on 2022/4/28.
//

#ifndef ANDROIDSTUDIO_IMEDIAPLAYER_H
#define ANDROIDSTUDIO_IMEDIAPLAYER_H

#include <vector>

namespace ParaEngine {

    class IMediaPlayer {
    public:
        virtual void PlayMidiNote(int note, int velocity) = 0;
        virtual void PlayMidiNote(int note, int velocity, int baseNode) = 0;
        virtual void PlayMidiFile(std::string filename) = 0;
        virtual void SetVolume(float volume) = 0;
        virtual void PlayMidiData(std::vector<char>) {}
    };
}
#endif //ANDROIDSTUDIO_IMEDIAPLAYER_H
