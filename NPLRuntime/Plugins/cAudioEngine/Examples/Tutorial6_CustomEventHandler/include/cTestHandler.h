//****************************************************************
//cAudio 2.3.0 Tutorial 6
//Custom event handler
//****************************************************************


///Include the ISourceHandler.h so we can inherit the interface
#include "ISourceEventHandler.h"

class cTestHandler : public cAudio::ISourceEventHandler
{
	//In order for this handler to work it must have each of the following functions
public:

	// This function calls on source update
	void onUpdate();
	// This function calls on source release
	void onRelease();
	// This function calls on source play
	void onPlay();
	// This function calls on source stop
	void onStop();
	// This function calls on source pause
	void onPause();

};