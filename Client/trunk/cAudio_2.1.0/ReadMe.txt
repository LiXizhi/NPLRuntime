Modified file: 
See cAudioSource.cpp: 
// fixed by LiXizhi 2010.12.20:  in case audio engine has finished all buffers, 
// but the worker thread still has data, we will force playing. This do happen on slow computers. 

2015.1.18

	- in cMP3Decoder.cpp fix seeking error:
		{
			// Fixed seeking by LiXizhi, 
			position += DataOffset;
		}

		bool cMP3Decoder::seek(float seconds, bool relative)
		{	
			if(Valid)
			{
				// int amountToSeek = seconds * (float)(Context->bit_rate / 8);
				int amountToSeek2 = (int)(seconds / getTotalTime() * (float)(Stream->getSize() - DataOffset));


	- in cAudioSource.cpp fix stop and replay error:
		// LiXizhi 2014.12.2 fixing a bug where stopped music still get played 1-2 seconds on rewind. 
		// if audio is already stopped, we should not read from the buffer (changing the buffer position)
-->		if (oldState != AL_STOPPED)
		{
		}

-->		if (actualread == 0 && ++nActualreadFailCount>2)
		{
			if (isLooping())
			{
				//If we are to loop, set to the beginning and reload from the start
				Decoder->setPosition(0, false);
				getLogger()->logDebug("Audio Source", "Buffer looping.");
-->				break;
			}

	- cAudioManager.cpp
		//Create a new device
		Device = alcOpenDevice(deviceName);
		// LiXizhi: fixing a bug, where device name can not be resolved. 
		if (Device == NULL && deviceName)
			Device = alcOpenDevice(NULL);

	- Fix CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT under iOS and some compile errors under linux. 

2010.12.19
	- cAudioEngine is updated to rev163. I also fixed a bug in cAudioSource.cpp to release openAL buffers on its destructor. 
	- Bug is fixed in cAudioEngine(cAudioSource.cpp). When all buffered data is assumed before the worker thread can refill it, the audio will be stopped prematurely. Now we will force continue to refill the buffers until all stream is played. 

--------------------------------------

This is the new cAudio 2.1.0 Release. This release is mainly a bug fix release.

Visit our site at https://github.com/R4stl1n/cAudio for news and information.

To get help, report bugs, or talk about cAudio visit our forums at: http://www.deathtouchstudios.com/phpBB3/index.php

New:  
	New Memory System (allows for the user to override all memory allocations by cAudio, providing a custom allocation scheme if necessary)
	New Memory Tracker (for finding memory leaks and tracking memory usage statistics)
	Official Mac OS X and IPhone support

Updated:
    Removed old memory system due to numerous bugs.
	Added a ogg vorbis close callback function to prevent a crash on Mac/IPhone
	Reorganized cAudioDefines to order by platform.
	Added defines to disable portions or all of the plugin system.
	Added defines for finer control of what is compiled into the library.
	Removed the ogg/vorbis encoding lib that was mistakenly linked in, which doubled the binary's size.
	
Note for linux users:
By default, EFX is disabled on linux with an ifdef in cAudioDefines. We are doing this because of licensing issues with EFX on linux.  We cannot provide in our packages key EFX headers, so if you want EFX support on linux, you must provide them yourself.

Known Bugs:
Looping with small sounds (less than 192 KB with the default buffer settings) does not work properly if you try to change the loop setting while the sound is playing.

//////////////////
// INSTALLATION //
//////////////////

To compile:

	Windows: Open up the cAudio Microsoft Visual C++ 2008 solution (or codeblocks project file for MinGW) and select Build Solution after you set Debug or Release.

	Linux/Unix: Cd to the Source directory and run make.

	Mac: Open up the xcode project file and compile away.

Libraries will end up in:

    Windows: lib/win32-visual (MSVC) or lib/win32-gcc (Codeblocks).

    Linux: lib/linux-x86.

    Mac: bin/osx (bin/osx-iphone for IPhone)

To compile examples:

	Windows: Automatically built by the MSVC Solution.  For codeblocks, open up the projects in the Examples directory select perferred compiler and build configuration.
	
	Linux/Unix: Cd to the Examples directory and run ./buildAll.sh

	Mac: Open up the respective XCode project file and compile away.
	
Compiled examples will be in:

    Windows: bin/win32-visual (MSVC) or bin/win32-gcc (Codeblocks).

    Linux: bin/linux-x86.

    Mac: bin/osx (bin/osx-iphone for IPhone)
