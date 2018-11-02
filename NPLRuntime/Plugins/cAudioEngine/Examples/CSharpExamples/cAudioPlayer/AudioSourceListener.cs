using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using cAudio;

namespace cAudioPlayer
{
    public class AudioSourceListener : ISourceEventHandler
    {
        private readonly MainWnd mPlayer;
        public AudioSourceListener(MainWnd player)
        {
            mPlayer = player;
        }

        public override void onUpdate()
        {
            
        }

        public override void onRelease()
        {
        }

        public override void onPlay()
        {
            Console.Out.WriteLine("onPlay");
        }

        public override void onStop()
        {
            Console.Out.WriteLine("onStop");
            mPlayer.OnStop();
        }

        public override void onPause()
        {
            Console.Out.WriteLine("onPause");
        }
    }
}
