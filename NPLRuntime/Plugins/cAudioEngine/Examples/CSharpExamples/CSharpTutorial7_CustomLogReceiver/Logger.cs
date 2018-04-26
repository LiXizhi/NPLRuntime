//****************************************************************
//cAudio 2.3.0 Tutorial 7 C#
//Custom log receiver 
//****************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using cAudio;

namespace CSharpTutorial7_CustomLogReceiver
{
    class Logger : ILogReceiver
    {
        public override bool OnLogMessage(string sender, string message, cAudio.LogLevel level, float time)
        {
            Console.WriteLine("Message");
            return true;
        }

    }
}
