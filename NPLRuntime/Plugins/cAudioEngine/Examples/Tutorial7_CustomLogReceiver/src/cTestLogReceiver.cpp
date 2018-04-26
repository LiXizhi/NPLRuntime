//****************************************************************
//cAudio 2.3.0 Tutorial 7
//Custom log receiver 
//****************************************************************

#include "cTestLogReceiver.h"
#include <iostream>

cTestLogReceiver::cTestLogReceiver()
{

}

cTestLogReceiver::~cTestLogReceiver()
{

}


bool cTestLogReceiver::OnLogMessage(const char* sender, const char* message, cAudio::LogLevel level, float time)
{
	//Every time we get a message it will have the following information
	//What/who sent the message
	//The message it contains
	//What log level it is
	//The time the message was logged
	//For now we will just display the word MESSAGE everytime we get a new message
	
	std::cout<<"Message!\n";

	return true;
}