//
//  MidiPlayer.h
//  Paracraft
//
//  Created by Paracraft on 23/3/2022.
//

#ifndef ParaEngineMediaPlayer_h
#define ParaEngineMediaPlayer_h
#include <string>
#include "IMediaPlayer.h"

namespace ParaEngine {
    class ParaEngineMediaPlayer: public IMediaPlayer {
    public:
        static ParaEngineMediaPlayer* GetSingleton();
        virtual void PlayMidiNote(int note, int velocity);
        virtual void PlayMidiFile(std::string filename);
        virtual void SetVolume(float volume);
    private:
        static const std::string classname;
    };
}

#endif /* MidiPlayer_h */
