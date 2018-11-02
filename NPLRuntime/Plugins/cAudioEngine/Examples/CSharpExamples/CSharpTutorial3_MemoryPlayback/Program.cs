//****************************************************************
//cAudio 2.3.0 Tutorial 3 C#
//Basic Memory Playback *Virtual file systems*
//bling.h created with bin2h http://deadnode.org/sw/bin2h/
//****************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using cAudio;

namespace CSharpTutorial3_MemoryPlayback
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
            Console.WriteLine("cAudio 2.2.0 Tutorial 3: Memory Playback. C#\n");
            Console.WriteLine("Available Playback Devices:\n");


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

            //Create the Bling class which holds our blign sound
            Bling bling = new Bling();

            IAudioSource currentSource = audioMgr.createFromMemory("bling",  System.Text.Encoding.Default.GetString(bling.bling),bling.bling_size,"wav");
            if (currentSource != null)
            {
                currentSource.setVolume(0.5f);
                currentSource.play2d(false);

                while (currentSource.isPlaying())
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
