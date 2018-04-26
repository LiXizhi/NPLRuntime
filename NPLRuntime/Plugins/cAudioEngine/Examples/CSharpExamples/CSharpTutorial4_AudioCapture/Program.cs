//****************************************************************
//cAudio 2.3.0 Tutorial 4 C#
//Audio Capture and playback
//****************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using cAudio;

namespace CSharpTutorial4_AudioCapture
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
            Console.WriteLine("cAudio 2.2.0 Tutorial 4: Audio Capture. C#\n");
            Console.WriteLine("Available Playback Devices:\n");

            //Setup the default frequency and duration
            UInt32 CAPTURE_FREQUENCY = 22050;
            UInt32 CAPTURE_DURATION = 10;

            AudioFormats CAPTURE_FORMAT = AudioFormats.EAF_16BIT_MONO;

            String formatName;

	        if(CAPTURE_FORMAT == AudioFormats.EAF_8BIT_MONO)
		        formatName = "8 Bit Mono";
	        else if(CAPTURE_FORMAT == AudioFormats.EAF_8BIT_STEREO)
		        formatName = "8 Bit Stereo";
            else if (CAPTURE_FORMAT == AudioFormats.EAF_16BIT_MONO)
		        formatName = "16 Bit Mono";
	        else
		        formatName = "16 Bit Stereo";

            //Create an audio manager with default settings
            IAudioManager audioMgr = cAudioCSharpWrapper.createAudioManager();
            //! The capture interface can be gotten at any time, and can even be used completely seperate from the manager
		    // Also it is possible to have more than one capture interface at the same time, useful for recording from multiple sources
            IAudioCapture capture = cAudioCSharpWrapper.createAudioCapture(false);

            // ! Here we enumerate different devices
		    IAudioDeviceList pAudioDeviceList = cAudioCSharpWrapper.createAudioDeviceList(cAudio.IDeviceType.DT_RECORDING);

            bool captureReady = pAudioDeviceList.isSupported();
            Console.WriteLine("Capturing Supported: " + captureReady + "\n");

            if (captureReady)
            {
                Console.WriteLine("Available Capture Devices: ");
                UInt32 deviceCount = pAudioDeviceList.getDeviceCount();
				string defaultDeviceName = pAudioDeviceList.getDefaultDeviceName();
				for(UInt32 i=0; i < deviceCount; ++i)
				{
					string deviceName = pAudioDeviceList.getDeviceName(i);
					if(deviceName.Equals(defaultDeviceName))
						Console.WriteLine("" + i + "): " + deviceName + " [DEFAULT] ");
					else
						Console.WriteLine("" + i + "): " + deviceName);
				}
				Console.WriteLine("");
				Console.WriteLine("Choose a device by number: ");
                string deviceSelection = Console.ReadLine();
                uint deviceSelect = Convert.ToUInt32(deviceSelection);
                Console.WriteLine("\n");

                captureReady = capture.initialize(pAudioDeviceList.getDeviceName(deviceSelect), CAPTURE_FREQUENCY, CAPTURE_FORMAT);

                Console.WriteLine("Ready to capture audio: " + captureReady + "\n");

                UInt32 targetRecordSize = CAPTURE_FREQUENCY * CAPTURE_DURATION * capture.getSampleSize();
                
                Console.WriteLine("Capture Device: " + capture.getDeviceName() + ".");
				Console.WriteLine("Capture Frequency: " + CAPTURE_FREQUENCY + " hertz.");
				Console.WriteLine("Capture Duration: " + CAPTURE_DURATION + " seconds.");
				Console.WriteLine("Capture Format: " + formatName + ".");
				Console.WriteLine("Sample Size: " + capture.getSampleSize() + " bytes.");
				Console.WriteLine("Target size of audio: " + targetRecordSize + " bytes.");
				Console.WriteLine("");

                UInt32 currentsize = 0;
				Console.WriteLine("Starting capture...");
				if(capture.beginCapture())
				{
					while(currentsize < targetRecordSize)
					{
						currentsize = capture.getCurrentCapturedAudioSize();

						//Sleep for 1 ms to free some CPU
                        cAudioCSharpWrapper.cAudioSleep(1);
					}
				}

                capture.stopCapture();
                Console.WriteLine("Capture stopped... \n");

                //Grab the total size again, ensures we get ALL the audio data
                //Not completely necessary, as starting a capture again will clear the old audio data
                currentsize = capture.getCurrentCapturedAudioSize();
                Console.WriteLine("Captured " + currentsize + " bytes of audio data. \n ");

                //Create a IAudio object and load a sound from a file
                var buffer = capture.getCapturedAudioBuffer();
                IAudioSource mysound = audioMgr.createFromAudioBuffer("sound1", buffer, CAPTURE_FREQUENCY, CAPTURE_FORMAT);
                buffer.Dispose();

                if (mysound != null)
                {
                    mysound.setVolume(0.5f);
                    mysound.play2d(false);

                    while (mysound.isPlaying())
                    {
                        cAudioCSharpWrapper.cAudioSleep(10);
                    }
                }
            }

            audioMgr.Dispose();

            Console.WriteLine(@"Press any key to quit ");
            Console.ReadLine();
        }
    }
}
