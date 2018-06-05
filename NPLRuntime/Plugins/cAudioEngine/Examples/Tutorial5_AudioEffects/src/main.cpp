//****************************************************************
//cAudio 2.3.0 Tutorial 5
//Audio effects
//****************************************************************

#include <iostream>
#include <string>

///Include cAudio.h so we can work wtih cAudio
#include "cAudio.h"

using namespace std;

int main(int argc, char* argv[])
{
    //Some fancy text
    cout << "cAudio 2.3.0 Tutorial 5: Basic Audio Effects. \n \n";

	//Create an uninitialized Audio Manager
    cAudio::IAudioManager* audioMgr = cAudio::createAudioManager(false);

	if(audioMgr)
	{
		//Allow the user to choose a playback device
		cout << "\nAvailable Playback Devices: \n";
		cAudio::IAudioDeviceList* pDeviceList = cAudio::createAudioDeviceList();
		unsigned int deviceCount = pDeviceList->getDeviceCount();
		cAudio::cAudioString defaultDeviceName = pDeviceList->getDefaultDeviceName();
		for(unsigned int i=0; i<deviceCount; ++i)
		{
			cAudio::cAudioString deviceName = pDeviceList->getDeviceName(i);
			if(deviceName.compare(defaultDeviceName) == 0)
				cout << i << "): " << deviceName.c_str() << " [DEFAULT] \n";
			else
				cout << i << "): " << deviceName.c_str() << " \n";
		}
		cout << std::endl;
		cout << "Choose a device by number: ";
		unsigned int deviceSelection = 0;
		cin >> deviceSelection;
		cout << std::endl;

		//Initialize the manager with the user settings
		audioMgr->initialize(pDeviceList->getDeviceName(deviceSelection).c_str());
		CAUDIO_DELETE pDeviceList;
		pDeviceList = 0;

#if CAUDIO_EFX_ENABLED == 1
		cAudio::IAudioEffects* effects = audioMgr->getEffects();
		if(effects)
		{
			cAudio::IEffect* effect = effects->createEffect();
			cAudio::IFilter* filter = effects->createFilter();

			if(effect && effect->isValid() && filter && filter->isValid())
			{
				filter->setHighFrequencyVolume(0.1f);
				filter->setLowFrequencyVolume(0.1f);

				//Create a IAudio object and load a sound from a file
        		cAudio::IAudioSource* mysound = audioMgr->create("footsteps", "../Media/Footsteps.wav",true);

				if(mysound)
				{
					mysound->setVolume(1.0f);
					mysound->attachFilter(filter);
					mysound->attachEffect(0, effect);

					std::cout << std::endl;
					std::cout << "Simultaneous Effects Supported: " << effects->getMaxEffectsSupported() << ". \n";
					std::cout << std::endl;

					{
						std::cout << "Playing sound with no effect or filter. \n";
						filter->setType(cAudio::EFT_NULL);
						effect->setType(cAudio::EET_NULL);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}

					if(effects->isFilterSupported(cAudio::EFT_LOWPASS))
					{
						std::cout << "Playing sound with lowpass filter. \n";
						filter->setType(cAudio::EFT_LOWPASS);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Lowpass filter not supported by this OpenAL device. \n";
					}
					if(effects->isFilterSupported(cAudio::EFT_HIGHPASS))
					{
						std::cout << "Playing sound with highpass filter. \n";
						filter->setType(cAudio::EFT_HIGHPASS);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Highpass filter not supported by this OpenAL device. \n";
					}
					if(effects->isFilterSupported(cAudio::EFT_BANDPASS))
					{
						std::cout << "Playing sound with bandpass filter. \n";
						filter->setType(cAudio::EFT_BANDPASS);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Bandpass filter not supported by this OpenAL device. \n";
					}

					filter->setType(cAudio::EFT_NULL);

					if(effects->isEffectSupported(cAudio::EET_EAX_REVERB))
					{
						std::cout << "Playing sound with EAX Reverb effect. \n";

						//Set the effect to a setting that distinctly shows off the effect (EAX_BATHROOM)
						effect->setType(cAudio::EET_EAX_REVERB);
						cAudio::sEAXReverbParameters param;
						param.Gain = 0.316f;
						param.GainHF = 0.251f;
						param.GainLF = 1.0f;
						param.Density = 0.17f;
						param.DecayHFRatio = 0.54f;
						param.ReflectionsGain = 0.653f;
						param.ReflectionsDelay = 0.01f;
						param.LateReverbDelay = 0.01f;
						param.LateReverbGain = 3.273f;

						effect->setEAXReverbParameters(param);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "EAX Reverb effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_REVERB))
					{
						std::cout << "Playing sound with Reverb effect. \n";

						//Set the effect to a setting that distinctly shows off the effect (EAX_BATHROOM)
						effect->setType(cAudio::EET_REVERB);
						cAudio::sReverbParameters param;
						param.Gain = 0.316f;
						param.GainHF = 0.251f;
						param.Density = 0.17f;
						param.DecayHFRatio = 0.54f;
						param.ReflectionsGain = 0.653f;
						param.ReflectionsDelay = 0.01f;
						param.LateReverbDelay = 0.01f;
						param.LateReverbGain = 3.273f;

						effect->setReverbParameters(param);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Reverb effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_CHORUS))
					{
						std::cout << "Playing sound with Chorus effect. \n";

						//Default settings
						effect->setType(cAudio::EET_CHORUS);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Chorus effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_DISTORTION))
					{
						std::cout << "Playing sound with Distortion effect. \n";

						//Default settings
						effect->setType(cAudio::EET_DISTORTION);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Distortion effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_ECHO))
					{
						std::cout << "Playing sound with Echo effect. \n";

						//Default settings
						effect->setType(cAudio::EET_ECHO);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Echo effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_FLANGER))
					{
						std::cout << "Playing sound with Flanger effect. \n";

						//Default settings
						effect->setType(cAudio::EET_FLANGER);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Flanger effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_FREQUENCY_SHIFTER))
					{
						std::cout << "Playing sound with Frequency Shifter effect. \n";

						//Default settings
						effect->setType(cAudio::EET_FREQUENCY_SHIFTER);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Frequency Shifter effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_VOCAL_MORPHER))
					{
						std::cout << "Playing sound with Vocal Morpher effect. \n";

						//Default settings
						effect->setType(cAudio::EET_VOCAL_MORPHER);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Vocal Morpher effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_PITCH_SHIFTER))
					{
						std::cout << "Playing sound with Pitch Shifter effect. \n";

						//Default settings
						effect->setType(cAudio::EET_PITCH_SHIFTER);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Pitch Shifter effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_RING_MODULATOR))
					{
						std::cout << "Playing sound with Ring Modulator effect. \n";

						//Default settings
						effect->setType(cAudio::EET_RING_MODULATOR);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Ring Modulator effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_AUTOWAH))
					{
						std::cout << "Playing sound with Autowah effect. \n";

						//Default settings
						effect->setType(cAudio::EET_AUTOWAH);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Autowah effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_COMPRESSOR))
					{
						std::cout << "Playing sound with Compressor effect. \n";

						//Default settings
						effect->setType(cAudio::EET_COMPRESSOR);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Compressor effect not supported by this OpenAL device. \n";
					}
					if(effects->isEffectSupported(cAudio::EET_EQUALIZER))
					{
						std::cout << "Playing sound with Equalizer effect. \n";

						//Default settings
						effect->setType(cAudio::EET_EQUALIZER);
						mysound->play2d(false);

						while(mysound->isPlaying())
							cAudio::cAudioSleep(10);
					}
					else
					{
						std::cout << "Equalizer effect not supported by this OpenAL device. \n";
					}
				}
				audioMgr->release(mysound);
				filter->drop();
				effect->drop();
			}
		}
#endif

		cAudio::destroyAudioManager(audioMgr);
	}
	else
	{
		std::cout << "Failed to create audio playback manager. \n";
	}

	std::cout << "Press any key to quit \n";
	std::cin.get();
	std::cin.get();

    return 0;
}
