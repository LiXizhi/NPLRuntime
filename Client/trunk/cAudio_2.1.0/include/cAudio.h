/*	cAudio.h -- interface of the 'cAudio Engine'

	Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.

*/

#ifndef CAUDIO_H
#define CAUDIO_H

#include "cAudioDefines.h"
#include "cAudioPlatform.h"
#include "cAudioSleep.h"
#include "EAudioFormats.h"
#include "IAudioSource.h"
#include "IAudioCapture.h"
#include "IAudioDecoder.h"
#include "IAudioDecoderFactory.h"
#include "IAudioEffects.h"
#include "IAudioManager.h"
#include "IDataSource.h"
#include "IEffect.h"
#include "IEffectParameters.h"
#include "IFilter.h"
#include "IListener.h"
#include "ILogger.h"
#include "ILogReceiver.h"
#include "IPluginManager.h"
#include "IRefCounted.h"

/*! \mainpage cAudio 2.1.0 API documentation
 *
 * <img src="../cAudioLogo.jpg"></img>
 *
 * \section intro Introduction
 *
 * Welcome to the Main API Documentation for cAudio 2.1.0.  cAudio is an advanced C++ wrapper around OpenAL, a professional and powerful
 * audio library.  Thus cAudio provides you with a number of classes to allow you to easily manipulate your audio world and
 * intergrate audio effects like reverberation, doppler, attenuation, ect.  cAudio also has a plugin system, allowing developers to extend 
 * the functionality of the library.  Furthermore, cAudio is released under the zlib license, meaning it is free for you to use in your projects, 
 * even if they are commercial.  Of course, we suggest you read the full text of the license by looking at the local license.txt file or cAudio.h.
 *
 * Included with the SDK is a number of tutorials.  We suggest you begin with them as they will guide you through the basics of cAudio usage
 * and work up to more advanced topics.  If you find you need more help, found a bug, or just want to talk about cAudio, please visit our forums
 * at <A HREF="http://www.deathtouchstudios.com/phpBB3/index.php" >http://www.deathtouchstudios.com/phpBB3/index.php</A>.
 *
 * \section links Links
 *
 * <A HREF="namespacec_audio.html">Namespaces</A>: Central namespace for the entire engine.  A good place to begin looking.<BR>
 * <A HREF="annotated.html">Class list</A>: List of all classes in the engine with descriptions.<BR>
 * <A HREF="functions.html">Class members</A>: List of all methods.<BR>
 *
 * \section example Short example
 *
 * Below is a simple "Hello World" example of how to use the engine.  Of course this is only the "tip of the iceburg"
 * when it comes to what cAudio is capable of.
 *
 * \code
 * #include <cAudio.h>
 *
 * int main()
 * {
 *	//Create an Audio Manager
 *	cAudio::IAudioManager* manager = cAudio::createAudioManager(true);
 *
 *	//Create an audio source and load a sound from a file
 *	cAudio::IAudioSource* mysound = manager->create("music","../../media/cAudioTheme1.ogg",true);
 *
 *	if(mysound)
 *	{
 *		//Play our source in 2D once.
 *		mysound->play2d(false);
 *
 * 		//Wait for the sound to finish playing
 * 		while(mysound->isPlaying())
 *			cAudio::cAudioSleep(10);
 *	}
 *
 *	//Shutdown cAudio
 *	manager->shutDown();
 *	cAudio::destroyAudioManager(manager);
 *	
 *	return 0;
 * }
 * \endcode
 */

//! Main namespace for the entire cAudio library
namespace cAudio
{
};

#endif