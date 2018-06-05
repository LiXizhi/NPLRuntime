//****************************************************************
//cAudio 2.3.0 Tutorial 2 C#
//Basic 3d Audio. Moving Audio source. Must be mono sound source
//****************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using cAudio;

namespace CSharpTutorial2_3DSound
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            //Some fancy text
            Console.WriteLine("cAudio 2.2.0 Tutorial 2: Basic 3D Audio. C#\n");
            Console.WriteLine("Available Playback Devices:\n");

            //Hold audio source x position
            float rot = 0;
            //Grab a list of available devices
            IAudioDeviceList pDeviceList = cAudioCSharpWrapper.createAudioDeviceList();


            //Get the default device name.
            string defaultDeviceName = pDeviceList.getDefaultDeviceName();

            for (uint i = 0; i < pDeviceList.getDeviceCount(); i++)
            {
                string deviceName = pDeviceList.getDeviceName(i);

                if (deviceName.Equals(defaultDeviceName))
                    Console.WriteLine(" " + i + "): " + deviceName + " [DEFAULT]");
                else
                    Console.WriteLine(" " + i + "): " + deviceName);
            }

            Console.WriteLine("\n");
            Console.WriteLine("Choose a device by number: ");
            string deviceSelection = Console.ReadLine();
            uint deviceSelect = Convert.ToUInt32(deviceSelection);
            Console.WriteLine("\n");

            //Create an uninitialized Audio Manager
            IAudioManager audioMgr = cAudioCSharpWrapper.createAudioManager(false);
            audioMgr.initialize(pDeviceList.getDeviceName(deviceSelect));

            //Grab the listener object, which allows us to manipulate where "we" are in the world
            //It's useful to bind this to a camera if you are using a 3d graphics engine
            IListener listener = audioMgr.getListener();

            //Create the audio file
            IAudioSource mysound = audioMgr.create("bing", "../../../Media/bling.ogg");

            if (mysound != null)
            {
                listener.setPosition(new cVector3(0,0,0));
                mysound.play3d(new cVector3(0,0,0), 2.0f, true);
                mysound.setVolume(1.0f);
                mysound.setMinDistance(1.0f);
                mysound.setMaxDistance(100.0f);
                
                //Play for 10 seconds
			    const int ticksToPlay = 10000;
			    int currentTick = 0;
			    int currentSecTick = 0;

                while (mysound.isPlaying() && currentTick < ticksToPlay)
                {
         
                    rot += 0.1f * 0.017453293f;
                    float x = (float)(5.0f * Math.Cos(rot) - 0.0f * Math.Sin(rot));
				    float z = (float)(0.0f * Math.Cos(rot) + 5.0f * Math.Sin(rot));
				    mysound.move(new cVector3(x,0.0f,z));
                    
                    ++currentTick;

				    if(currentTick/1000 > currentSecTick)
				    {
					    ++currentSecTick;
					    Console.Write(".");
				    }
                    cAudioCSharpWrapper.cAudioSleep(10);
                }
            }

            audioMgr.Dispose();

            Console.WriteLine(@"Press any key to quit ");
            Console.ReadLine();
        }
    }
}
