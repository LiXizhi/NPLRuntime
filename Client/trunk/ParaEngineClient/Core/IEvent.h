#pragma once

namespace ParaEngine
{
	using namespace std;

	/** the event interface */
	struct IEvent
	{
	public:
		/** get event type */
		virtual int GetEventType() const { return -1; }

		/** build script code and return. it does not cache the script code. Use GetScriptCode() instead for performance reason. */
		virtual string ToScriptCode() const { return ""; };

		/** return true, if firing event does not immediately invoke the handler. */
		virtual bool IsAsyncMode() const { return true; }

		/** get event id */
		virtual int GetEventID() const { return 0; }
	};

	/** sync file call back function or class. */
	typedef boost::signals2::signal<void(const IEvent*, const std::string&)>  EventHandler_Callback_t;
}