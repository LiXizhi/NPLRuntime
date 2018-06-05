#ifndef CMP3DECODERPLUGIN_H
#define CMP3DECODERPLUGIN_H

#include "IAudioPlugin.h"
#include "cMP3DecoderFactory.h"

using namespace cAudio;

class cMP3DecoderPlugin : public IAudioPlugin
{
	bool installPlugin(ILogger* logger)
	{
		//This plugin has no first time initialization to do, so this is an easy function
		return true;
	}

	const char* getPluginName()
	{
		return "MP3Decoder";
	}

	void uninstallPlugin()
	{
		//Nothing to clean up
	}

	void onCreateAudioManager(IAudioManager* manager)
	{
		cMP3DecoderFactory* factory = new cMP3DecoderFactory();

		if(factory)
		{
			manager->registerAudioDecoder(factory, "mp3");
		}
	}

	void onCreateAudioCapture(IAudioCapture* capture)
	{
		//Do nothing with this "event"
	}

	void onDestroyAudioManager(IAudioManager* manager)
	{
		cAudio::IAudioDecoderFactory* factory = manager->getAudioDecoderFactory("mp3");
		manager->unRegisterAudioDecoder("mp3");

		if(factory)
		{
			delete factory;
		}
	}

	void onDestoryAudioCapture(IAudioCapture* capture)
	{
		//Do nothing with this "event"
	}
};

#endif //! CMP3DECODERPLUGIN_H