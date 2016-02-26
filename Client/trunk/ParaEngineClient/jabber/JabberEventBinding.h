#pragma once
#include <string>
#include <bitset>
#include <set>
#include <map>

namespace ParaEngine
{
	using namespace std;
	
	/** Jabber events that a JC client can bind to. such as jc:AddEventListener("JE_OnMessage", "commonlib.echo(msg)");
	* in the event handler, one can access data via the msg variable. 
	*/
	enum JabberEvents{
		JE_NONE,

		JE_OnPresence = 0,//We received a presence packet. 
		JE_OnError,
		JE_OnRegistered,//After calling Register(), the registration succeeded or failed.
		JE_OnRegisterInfo,//after calling Register, information about the user is required.  Fill in the given IQ with the requested information.
		JE_OnIQ,//We received an IQ packet.
		/** Authentication failed.  The connection is not terminated if there is an auth error and there is at least one event handler for this event.
		* msg = {jckey, reason=number:AuthenticationError} 
		*/
		JE_OnAuthError,
		JE_OnLoginRequired,//AutoLogin is false, and it's time to log in.
		/** The connection is connected, but no stream:stream has been sent, yet.
		* msg = {jckey} 	*/
		JE_OnConnect,
		/** The connection is complete, and the user is authenticated.
		* msg = {jckey} */
		JE_OnAuthenticate, // 
		/** The connection is disconnected or auth failed.
		msg = {jckey, errorcode=number:ConnectionError} */
		JE_OnDisconnect,
		/** msg = {jckey, state=number, issuer=string, peer=string,protocol=string,mac=string,cipher=string,compression=string,from =string,to=string} */
		JE_OnTLSConnect,

		/** roster arrived. array of jid roster tables. each jid may belong to 0 or multiple groups and resources. 
		msg = {jckey, 
			{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={string, string, ...},},
			{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={string, string, ...},},
			{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={string, string, ...},},
			...
		}
		*/
		JE_OnRoster,
		JE_OnRosterBegin, // Fired when a roster result starts, before any OnRosterItem events fire.
		JE_OnRosterItem, //event for new roster items. A roster may belong to multiple groups
		JE_OnRosterEnd, //Fired when a roster result is completed being processed.
		JE_OnRosterError,
		/** a roster item's presence arrived. 
		msg = {jckey, jid=string:jid, resource=string, presence=int:Presence}
		*/
		JE_OnRosterPresence,
		/** the user itself's presence arrived.
		msg = {jckey, jid=string:jid, resource=string, presence=int:Presence}
		*/
		JE_OnSelfPresence,

		/** We received a message packet.
		* msg = {jckey, from=string:jid,  subtype=int:StanzaSubType, subject=string, body=string}
		*/
		JE_OnMessage,
		JE_OnStanzaMessageChat,
		JE_OnStanzaMessageGroupChat,
		JE_OnStanzaMessageHeadline,
		JE_OnStanzaMessageNormal,
		/** msg = {jckey, from=string:jid, state=number:ChatStateType} */
		JE_OnChatState,
		/** msg = {jckey, from=string:jid, event=number:MessageEventType} */
		JE_OnMessageEvent,
		
		/** msg = {jckey, error=int:ResourceBindError} */
		JE_OnResourceBindError,
		/** msg = {jckey, error=int:SessionCreateError} */
		JE_OnSessionCreateError,

		/** msg = {jckey, from=string:jid} */
		JE_OnItemSubscribed,
		/** msg = {jckey, from=string:jid} */
		JE_OnItemAdded,
		/** msg = {jckey, from=string:jid} */
		JE_OnItemUnsubscribed,
		/** msg = {jckey, from=string:jid} */
		JE_OnItemRemoved,
		/** msg = {jckey, from=string:jid} */
		JE_OnItemUpdated,
		
		/** msg={jckey, jid=string:jid}*/
		JE_OnSubscriptionRequest,
		/** msg={jckey, jid=string:jid,msg=string}*/
		JE_OnUnsubscriptionRequest,
		/** msg={jckey, jid=string:jid}*/
		JE_OnNonrosterPresence,

		JE_LAST,
		JE_UNKNOWN=0xffff,
	};

	/**
	* Jabber event that could be bound with NPL script. 
	*/
	class CJabberEventBinding
	{
	public:
		CJabberEventBinding();
		~CJabberEventBinding();

		/** a structure for holding the script callback. */
		class CScriptCallBack
		{
		public:
			CScriptCallBack(){};
			CScriptCallBack(const char* callbackScript):m_sCallbackScript(callbackScript){}
			string m_sCallbackScript;
		};

		static void StaticInit();
		//initialize the event mapping table, the old mappings are erased.
		void InitEventMappingTable(bool bDisable=false);

	public:

		/** enable/disable an event */
		void EnableEvent(int eEvent);
		void DisableEvent(int eEvent);

		
		/**
		* Mapping an event to another event.
		*/
		void MapEvent(int eSrcEvent, int eDestEvent);
		/**
		* Unmap an event to any event 
		*/
		void UnmapEvent(int eSrcEvent, int eDestEvent);
		/**
		* Decide if the first event is mapped to the second event
		*/
		bool IsMapTo(int eSrcEvent, int eDestEvent)const;

		//create an event's script, if it already has a script, the old one will be replace by the new one
		void MapEventToScript(int eEvent,const CScriptCallBack *script);

		//delete an event's script, if any
		void UnmapEventToScript(int eEvent);

		/** clear all script mapping from the MapEventToScript() based calls. */
		void ClearAllScripts();

		//return the script of a given event, null if not exists
		const CScriptCallBack * GetEventScript(int eEvent)const;

		//if an event has script,
		bool HasEventScript(int eEvent) const;

		//two helper functions for translating between string and the event value.
		static int StringToEventValue(const string &str);
		static const string& EventValueToString(int value);

	protected:
		bitset<JE_LAST> m_pEventEnable;
		map<int,CScriptCallBack> m_pEventToScript;
		set<int> m_pEventMappingTable[JE_LAST];
		static map<string,int> StringToEventTable;
		static map<int,string> EventToStringTable;
	private:
		static bool g_bStaticInited;
	};
}
