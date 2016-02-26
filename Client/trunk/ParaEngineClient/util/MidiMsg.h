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
	private:
#ifdef PARAENGINE_CLIENT
		HMIDIOUT m_deviceMidiOut;
#endif
		bool m_bIsValid;
		bool m_bIsLoaded;
	};
}