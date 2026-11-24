#include "ParaEngine.h"
#include "MidiMsg.h"
#ifdef PARAENGINE_MIDI_OUTPUT
#include "../../Platform/Midi/MidiPlayback.h"
#endif

using namespace ParaEngine;

CMidiMsg& CMidiMsg::GetSingleton()
{
    static CMidiMsg player;
    return player;
}

bool CMidiMsg::OpenLocked()
{
    if (m_loaded || m_mediaPlayer) return true;
#ifdef PARAENGINE_MIDI_OUTPUT
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
    if (midiOutOpen(&m_device, MIDI_MAPPER, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR)
    {
        midiOutSetVolume(m_device, m_volume);
        m_loaded = true;
        return true;
    }
#endif
    m_loaded = Midi::Open();
#endif
    return m_loaded;
}

bool CMidiMsg::CheckLoad()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return OpenLocked();
}

bool CMidiMsg::IsLoaded() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_loaded || m_mediaPlayer;
}

int CMidiMsg::PlayMidiMsg(DWORD message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const unsigned status = message & 0xf0, channel = message & 15;
    const unsigned data1 = (message >> 8) & 127, data2 = (message >> 16) & 127;
    if (m_mediaPlayer)
    {
        if (status == 0x90 && data2)
            m_mediaPlayer->PlayMidiNote(data1, data2, (message >> 24) & 127, channel);
        else if (status == 0x80 || status == 0x90 || (status == 0xb0 && (data1 == 120 || data1 == 123)))
            m_mediaPlayer->StopMidiMsg(channel);
        else
            return E_FAIL;
        return S_OK;
    }
#ifdef PARAENGINE_MIDI_OUTPUT
    if (!OpenLocked()) return E_FAIL;
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
    if (m_device)
    {
        if (status == 0x90 && data2)
        {
            const DWORD program = message >> 24 ? (message >> 24) & 127 : m_programs[channel];
            if (midiOutShortMsg(m_device, (program << 8) | 0xc0 | channel) != MMSYSERR_NOERROR) return E_FAIL;
        }
        if (midiOutShortMsg(m_device, message & 0xffffff) != MMSYSERR_NOERROR) return E_FAIL;
        if (status == 0xc0) m_programs[channel] = static_cast<unsigned char>(data1);
        return S_OK;
    }
#endif
    return Midi::PlayMessage(message) ? S_OK : E_FAIL;
#else
    return E_FAIL;
#endif
}

int CMidiMsg::StopMidiMsg(int channel)
{
    return PlayMidiMsg(0x7bb0 | (channel & 15));
}

bool CMidiMsg::PlayMidiFile(const std::string& filename, int loop)
{
    auto& player = GetSingleton();
    if (filename.empty()) return player.StopMidiFile("", true);
    std::unique_lock<std::mutex> lock(player.m_mutex);
    if (!player.m_mediaPlayer)
    {
#ifdef PARAENGINE_MIDI_OUTPUT
        // A package read or manifest download can take time. Live notes keep playing.
        lock.unlock();
        CParaFile file;
        std::vector<Midi::Event> events;
        if (file.OpenAssetFile(filename.c_str()) != 1 ||
            !Midi::ReadSequence(file.getBuffer(), file.getSize(), Midi::SampleRate, events))
            return false;
        lock.lock();
        if (!player.m_mediaPlayer && !Midi::PlayFile(std::move(events), loop > 0)) return false;
#else
        return false;
#endif
    }
    if (player.m_mediaPlayer) player.m_mediaPlayer->PlayMidiFile(filename);
    player.m_filename = filename;
    return true;
}

bool CMidiMsg::StopMidiFile(const std::string& filename, bool immediate)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!filename.empty() && filename != m_filename) return false;
    if (m_mediaPlayer) m_mediaPlayer->PlayMidiFile("");
#ifdef PARAENGINE_MIDI_OUTPUT
    else Midi::StopFile(immediate);
#endif
    if (immediate) m_filename.clear();
    return true;
}

DWORD CMidiMsg::GetVolume() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_volume;
}

void CMidiMsg::SetVolume(DWORD volume)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_volume = volume;
    const float left = (volume & 65535) / 65535.f, right = (volume >> 16) / 65535.f;
    if (m_mediaPlayer) m_mediaPlayer->SetVolume((left + right) * 0.5f);
#ifdef PARAENGINE_MIDI_OUTPUT
    Midi::SetGain(left, right);
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
    if (m_device) midiOutSetVolume(m_device, m_volume);
#endif
#endif
}

void CMidiMsg::SetVolumeFloat(float volume)
{
    const DWORD word = static_cast<DWORD>((std::max)(0.f, (std::min)(volume, 1.f)) * 65535.f);
    SetVolume((word << 16) | word);
}

void CMidiMsg::SetMediaPlayer(IMediaPlayer* player)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_mediaPlayer == player) return;
    ReleaseLocked();
    m_mediaPlayer = player;
    if (player) player->SetVolume(((m_volume & 65535) + (m_volume >> 16)) / 131070.f);
}

IMediaPlayer* CMidiMsg::GetMediaPlayer() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_mediaPlayer;
}

void CMidiMsg::SafeRelease()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ReleaseLocked();
}

void CMidiMsg::ReleaseLocked()
{
    if (m_mediaPlayer) m_mediaPlayer->StopMidiMsg(0);
#ifdef PARAENGINE_MIDI_OUTPUT
    Midi::Close();
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
    if (m_device)
    {
        midiOutReset(m_device);
        midiOutClose(m_device);
        m_device = nullptr;
    }
    m_programs.fill(0);
#endif
#endif
    m_loaded = false;
    m_filename.clear();
}
