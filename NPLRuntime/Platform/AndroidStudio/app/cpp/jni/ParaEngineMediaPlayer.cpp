//
// Created by wuxiangan on 2022/4/27.
//

#include "ParaEngineMediaPlayer.h"
#include "JniHelper.h"

namespace ParaEngine {
    const std::string ParaEngineMediaPlayer::classname = "com/tatfook/paracraft/ParaEngineMediaPlayer";

    ParaEngineMediaPlayer* ParaEngineMediaPlayer::GetSingleton() {
        static ParaEngineMediaPlayer s_media_player;
        return &s_media_player;
    }

    void ParaEngineMediaPlayer::PlayMidiNote(int note, int velocity) {
        JniHelper::callStaticVoidMethod(classname, "PlayMidiNote", note, velocity, 0);
    }

    void ParaEngineMediaPlayer::PlayMidiNote(int note, int velocity, int baseNode) {
        JniHelper::callStaticVoidMethod(classname, "PlayMidiNote", note, velocity, baseNode);
    }

    void ParaEngineMediaPlayer::PlayMidiFile(std::string filename) {
        JniHelper::callStaticVoidMethod(classname, "PlayMidiFile", filename);
    }

    void ParaEngineMediaPlayer::SetVolume(float volume) {
        JniHelper::callStaticVoidMethod(classname, "SetVolume", volume);
    }

    void ParaEngineMediaPlayer::PlayMidiData(std::vector<char> datas) {
        JniHelper::callStaticVoidMethod(classname, "PlayMidiData", datas);
    }
}