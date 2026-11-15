//-----------------------------------------------------------------------------
// Class:	MidiMsg
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2014.4.22
// Desc: simple midi playback
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "MidiMsg.h"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
using namespace ParaEngine;

CMidiMsg::CMidiMsg() : 
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
m_deviceMidiOut(NULL), 
#endif
m_bIsValid(false), m_bIsLoaded(false), m_dwVolume(0xffffffff), m_mediaPlayer(NULL)
{
}

void ParaEngine::CMidiMsg::SetMediaPlayer(ParaEngine::IMediaPlayer* mediaPlayer) {
	m_mediaPlayer = mediaPlayer;
}

bool ParaEngine::CMidiMsg::CheckLoad()
{
	if (m_bIsLoaded)
	{
		return m_bIsValid;
	}
	else
	{
		m_bIsLoaded = true;
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
		unsigned int err = midiOutOpen(&m_deviceMidiOut, 0, 0, 0, CALLBACK_NULL);
		if (err != MMSYSERR_NOERROR)
		{
			OUTPUT_LOG("error opening default MIDI device: %d\n", err);
		}
		else
		{
			m_bIsValid = true;
			midiOutSetVolume(m_deviceMidiOut, m_dwVolume);
			OUTPUT_LOG("successfully opened default MIDI device\n");
		}
#endif
		return m_bIsValid;
	}

}

DWORD ParaEngine::CMidiMsg::GetVolume() const
{
	return m_dwVolume;
}

void ParaEngine::CMidiMsg::SetVolume(DWORD val)
{
	if (m_dwVolume != val)
	{
		m_dwVolume = val;

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
		if (IsLoaded())
		{
			midiOutSetVolume(m_deviceMidiOut, m_dwVolume);
		}
#endif
	}
}

void ParaEngine::CMidiMsg::SetVolumeFloat(float val)
{
	if (val > 1.0f)
		val = 1.0f;

	if (m_mediaPlayer) return m_mediaPlayer->SetVolume(val);

	DWORD volume = (DWORD)(val * 0xffff);
	volume = (volume << 16) + volume;
	SetVolume(volume);
}

bool ParaEngine::CMidiMsg::IsLoaded() const
{
	return m_bIsLoaded;
}

CMidiMsg& CMidiMsg::GetSingleton()
{
	static CMidiMsg midiPlayer;
	return midiPlayer;
}

int CMidiMsg::PlayMidiMsg(DWORD dwMsg)
{
#ifdef EMSCRIPTEN
	int channel = dwMsg & 0xff;
	char note = (dwMsg & 0xff00) >> 8;
	char velocity = (dwMsg & 0xff0000) >> 16;
	int baseNote = (dwMsg & 0xff000000) >> 24;
	EM_ASM({
		let note = $0;
		let velocity = $1;
		velocity = velocity / 127;
		const synth = new Tone.Synth().toDestination();
        synth.triggerAttackRelease(Tone.Frequency(note, "midi").toNote(), "8n", Tone.now(), velocity);
	}, static_cast<int>(note), static_cast<int>(velocity));
	return S_OK;
#endif
	
	if (m_mediaPlayer) {
        int channel = dwMsg & 0xff;
		char note = (dwMsg & 0xff00) >> 8;
		char velocity = (dwMsg & 0xff0000) >> 16;
        int baseNote = (dwMsg & 0xff000000) >> 24;

        if ((channel - 144) > 0) {
            m_mediaPlayer->PlayMidiNote(note, velocity, baseNote, channel - 144);
        } else {
            m_mediaPlayer->PlayMidiNote(note, velocity, baseNote);
        }
		
		return S_OK;
	}
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	if(CheckLoad())
	{
		int baseNote = (dwMsg & 0xff000000) >> 24;
		int channel = (dwMsg & 0xff) - 0x90;
		if (channel >= 0 && channel < 16)
		{
			midiOutShortMsg(m_deviceMidiOut, (baseNote << 8) | (0xC0 + channel));
		}
		midiOutShortMsg(m_deviceMidiOut, dwMsg);
		return S_OK;
	}
#endif
	return E_FAIL;
}

int CMidiMsg::StopMidiMsg(int channel)
{
#ifdef EMSCRIPTEN
	return S_OK;
#endif
	if (m_mediaPlayer)
	{
		m_mediaPlayer->StopMidiMsg(channel);
		return S_OK;
	}
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	if (CheckLoad())
	{
		midiOutShortMsg(m_deviceMidiOut, 0x7BB0 + (channel & 0x0F));
		return S_OK;
	}
#endif
	return E_FAIL;
}

void CMidiMsg::SafeRelease()
{
	if (m_mediaPlayer)
	{
		m_mediaPlayer->StopMidiMsg(0);
	}
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	if(m_bIsLoaded && m_bIsValid)
	{
		midiOutClose(m_deviceMidiOut);
		m_bIsValid = false;
	}
	PlayMidiFile("");
#endif
}

void CMidiMsg::PlayMidiFile(const std::string& filename, int nLoopCount)
{
	if (CMidiMsg::GetSingleton().GetMediaPlayer()) return CMidiMsg::GetSingleton().GetMediaPlayer()->PlayMidiFile(filename);

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	static std::string g_last_filename;
	static std::string g_last_cmd;
	static std::string g_last_disk_filename;

	if (g_last_filename != filename)
	{
		if (!g_last_filename.empty() || !g_last_disk_filename.empty())
		{
			std::string sCmd = "close ";
			sCmd += g_last_disk_filename;
			mciSendString(sCmd.c_str(), NULL, NULL, NULL);
			g_last_filename.clear();
			g_last_disk_filename.clear();
		}

		if (!filename.empty())
		{
			// if file is from a zip or asset manifest file, we will extract to a temp location. 
			std::string sCmd = "play ";
			if (CParaFile::DoesFileExist(filename.c_str(), false))
			{
				g_last_disk_filename = filename;
				sCmd += filename;
			}
			else
			{
				CParaFile file;
				file.OpenAssetFile(filename.c_str());
				if (!file.isEof())
				{
					string sTempDiskFilename = CParaFile::GetCurDirectory(CParaFile::APP_TEMP_DIR);
					sTempDiskFilename += "tempmidi/";
					// sTempDiskFilename += CParaFile::GetFileName(filename);
					sTempDiskFilename += "temp.mid";
					// delete the source file if exists
					if (ParaEngine::CParaFile::DoesFileExist(sTempDiskFilename.c_str(), false))
					{
						if (!::DeleteFile(sTempDiskFilename.c_str()))
						{
							OUTPUT_LOG("can not replace file %s. because we can not delete it\n", sTempDiskFilename.c_str());
							return;
						}
					}
					CParaFile fileTo;
					if (fileTo.CreateNewFile(sTempDiskFilename.c_str()))
					{
						fileTo.write(file.getBuffer(), (int)(file.getSize()));
						g_last_disk_filename = sTempDiskFilename;
						sCmd += sTempDiskFilename;
					}
					else
					{
						return;
					}
				}
				else
				{
					return;
				}
			}

			g_last_cmd = sCmd;
			mciSendString(sCmd.c_str(), NULL, NULL, NULL);
		}
		g_last_filename = filename;
	}
	else
	{
		if (!g_last_cmd.empty())
		{
			// repeat the last cmd
			mciSendString(g_last_cmd.c_str(), NULL, NULL, NULL);
		}
	}
#endif
}

