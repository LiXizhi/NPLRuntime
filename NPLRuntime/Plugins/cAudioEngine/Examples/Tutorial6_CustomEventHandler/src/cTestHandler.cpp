//****************************************************************
//cAudio 2.3.0 Tutorial 6
//Custom event handler
//****************************************************************

#include "cTestHandler.h"
#include <iostream>

void cTestHandler::onUpdate()
{
	//We comment this out because this will be constantly 
	//called as the sound is playing
	//std::cout<<"Custom Event Handler: Sound source is updating\n";
}

void cTestHandler::onRelease()
{
	std::cout<<"Custom Event Handler: Sound source was released\n";
}

void cTestHandler::onPlay()
{
	std::cout<<"Custom Event Handler: Sound source started playing\n";
}

void cTestHandler::onStop()
{
	std::cout<<"Custom Event Handler: Sound source stoped playing\n";
}

void cTestHandler::onPause()
{
	std::cout<<"Custom Event Handler: Sound source was paused\n";
}

