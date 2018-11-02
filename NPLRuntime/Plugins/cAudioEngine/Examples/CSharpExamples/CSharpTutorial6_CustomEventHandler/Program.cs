//****************************************************************
//cAudio 2.3.0 Tutorial 6 C#
//Custom Event Handler
//****************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using cAudio;

namespace CSharpTutorial6_CustomEventHandler
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
            Console.WriteLine("cAudio 2.2.0 Tutorial 6: Custom Event Handler. C#\n");
            Console.WriteLine("Available Playback Devices:\n");


            //Grab a list of available devices
            IAudioDeviceList pDeviceList = cAudioCSharpWrapper.createAudioDeviceList();
            
            //Create a Event Handler Object
            EventHandler eventHandler = new EventHandler();


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

            IAudioSource mysound = audioMgr.create("bing", "../../../Media/cAudioTheme1.ogg");

            //We register the event handler with the audio source
            mysound.registerEventHandler(eventHandler);

            if (mysound != null)
            {
                mysound.setVolume(0.5f);
                mysound.play2d(false);

                while (mysound.isPlaying())
                {
                    cAudioCSharpWrapper.cAudioSleep(10);
                }
            }
            audioMgr.Dispose();

            Console.WriteLine(@"Press any key to quit ");
            Console.ReadLine();
        }
    }
}
