//****************************************************************
//cAudio 2.3.0 Tutorial 7
//Custom log receiver 
//****************************************************************

//We include the interface of the log receiver
#include "ILogReceiver.h"
#include <string>


//Now we make our new class and inherit the ILogReceiver interface
class cTestLogReceiver : public cAudio::ILogReceiver
{
public:
	//Must have the constructor and desconstructor
	cTestLogReceiver();
	~cTestLogReceiver();

	//Only function that must be included in order for the receiver to work
	bool OnLogMessage(const char* sender, const char* message, cAudio::LogLevel level, float time);

};
