#pragma once

#include <array>
#include <mutex>
#include <string>
#include "IMediaPlayer.h"
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
#include <mmsystem.h>
#endif

namespace ParaEngine
{
    class CMidiMsg
    {
    public:
        ~CMidiMsg() { SafeRelease(); }
        static CMidiMsg& GetSingleton();
        int PlayMidiMsg(DWORD message);
        int StopMidiMsg(int channel = 0);
        static bool PlayMidiFile(const std::string& filename, int loop = 0);
        bool StopMidiFile(const std::string& filename, bool immediate);
        DWORD GetVolume() const;
        void SetVolume(DWORD volume);
        void SetVolumeFloat(float volume);
        bool CheckLoad();
        bool IsLoaded() const;
        void SafeRelease();

        // Optional replacement for hosts that provide their own media player.
        void SetMediaPlayer(IMediaPlayer* player);
        IMediaPlayer* GetMediaPlayer() const;

    private:
        bool OpenLocked();
        void ReleaseLocked();
        mutable std::mutex m_mutex;
        std::string m_filename;
        IMediaPlayer* m_mediaPlayer = nullptr;
        DWORD m_volume = 0xffffffff;
        bool m_loaded = false;
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
        HMIDIOUT m_device = nullptr;
        std::array<unsigned char, 16> m_programs{};
#endif
    };
}
