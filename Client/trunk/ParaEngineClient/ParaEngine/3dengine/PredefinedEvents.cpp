//-----------------------------------------------------------------------------
// Class:	Predefined Events
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.8.18
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Events_def.h"
#include "PredefinedEvents.h"

namespace ParaEngine
{
	/*enum EventType{
		EVENT_MOUSE=0,
		EVENT_KEY,
		EVENT_EDITOR,
		EVENT_SYSTEM,
		EVENT_NETWORK,
		EVENT_LAST
	};*/

	Event PredefinedEvents::SelectionChanged = Event(EVENT_EDITOR, ED_SELECTION_CHANGED, "event_type = 2; event_id = 0;");
	Event PredefinedEvents::ObjectDeleted = Event(EVENT_EDITOR, ED_OBJECT_DELETED, "event_type = 2; event_id = 1;");
	Event PredefinedEvents::ObjectCreated = Event(EVENT_EDITOR, ED_OBJECT_CREATED, "event_type = 2; event_id = 2;");
	Event PredefinedEvents::ObjectModified = Event(EVENT_EDITOR, ED_OBJECT_MODIFIED, "event_type = 2; event_id = 3;");

	Event PredefinedEvents::SceneLoaded = Event(EVENT_EDITOR, ED_SCENE_LOADED, "event_type = 2; event_id = 4;");
	Event PredefinedEvents::SceneUnLoaded = Event(EVENT_EDITOR, ED_SCENE_UNLOADED, "event_type = 2; event_id = 5;");
	Event PredefinedEvents::SceneSaving = Event(EVENT_EDITOR, ED_SCENE_SAVING, "event_type = 2; event_id = 6;");
	Event PredefinedEvents::SceneSaved = Event(EVENT_EDITOR, ED_SCENE_SAVED, "event_type = 2; event_id = 7;");

	Event PredefinedEvents::SettingChange = Event(EVENT_SYSTEM, SystemEvent::SYS_WM_SETTINGCHANGE, "event_type = 4; ");

}


