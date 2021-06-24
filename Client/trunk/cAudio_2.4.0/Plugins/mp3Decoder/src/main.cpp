#include "cAudioDefines.h"
#include "cMP3Plugin.h"
#ifdef CAUDIO_PLATFORM_LINUX
#include <cstring>
#endif

//Gives back the plugin class for cAudio to install and use.
extern "C" CAUDIO_API IAudioPlugin* GetPluginModule(const char* version)
{
	//Double-check the version to make sure we can be used
	if(strcmp(CAUDIO_VERSION, version) == 0)
	{
		return new cMP3DecoderPlugin();
	}
	return NULL;
}
