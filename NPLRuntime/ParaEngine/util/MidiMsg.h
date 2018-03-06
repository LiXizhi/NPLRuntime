#pragma once

namespace ParaEngine
{
	/** simple midi audio */
	class CMidiMsg
	{
	public:
		CMidiMsg();
		~CMidiMsg(){
			SafeRelease();
		}

		static CMidiMsg& GetSingleton();
		int PlayMidiMsg(DWORD dwMsg);

		void SafeRelease();

		bool CheckLoad();
		// @param filename: if empty, it will stop last one. 
		static void PlayMidiFile(const std::string& filename, int nLoopCount = 0);

		/**
		* New volume setting. The low-order word contains the left-channel volume setting,
		* and the high-order word contains the right-channel setting. A value of 0xFFFF represents full volume, and a value of 0x0000 is silence.
		*/
		DWORD GetVolume() const;
		void SetVolume(DWORD val);
		/** set both left/right volume
		* @param val: 0-1. where 1 is biggest one, 0 is mute. 
		*/
		void SetVolumeFloat(float val);

		/** only load when first note is played. */
		bool IsLoaded() const;
	private:
#ifdef PARAENGINE_CLIENT
		HMIDIOUT m_deviceMidiOut;
#endif
		bool m_bIsValid;
		bool m_bIsLoaded;
		
		DWORD m_dwVolume;
	};
}