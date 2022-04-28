//
// Created by wuxiangan on 2022/4/28.
//

#ifndef ANDROIDSTUDIO_IMEDIAPLAYER_H
#define ANDROIDSTUDIO_IMEDIAPLAYER_H

namespace ParaEngine {

    class IMediaPlayer {
    public:
        virtual void PlayMidiNote(int note, int velocity) = 0;  // 播放midi音符
        virtual void PlayMidiFile(std::string filename) = 0; // 播放midi音频文件
        virtual void SetVolume(float volume) = 0;  // 设置播放器音量
    };
}
#endif //ANDROIDSTUDIO_IMEDIAPLAYER_H
