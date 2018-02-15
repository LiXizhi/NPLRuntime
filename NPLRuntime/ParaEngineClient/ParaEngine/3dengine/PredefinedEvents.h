#pragma once
#include "Events_def.h"
#include "EventClasses.h"
namespace ParaEngine
{

	/**
	* some predefined events to pass to the event center in the PoseEvent method. 
	*/
	class PredefinedEvents
	{
	public:
		static Event SelectionChanged;
		static Event ObjectDeleted;
		static Event ObjectCreated;
		static Event ObjectModified;
		static Event SceneLoaded;
		static Event SceneUnLoaded;
		static Event SceneSaving;
		static Event SceneSaved;
		static Event SettingChange;

	};
}