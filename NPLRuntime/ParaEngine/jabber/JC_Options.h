#pragma once
#include <string>

namespace ParaEngine
{
	using namespace std;

	/// <summary>
	/// Option names.  These must be well-formed XML element names.
	/// </summary>
	class JCOptions
	{
	public:
		JCOptions();

		/// <summary>
		/// Default namespace.
		/// </summary>
		string NAMESPACE;

		/// <summary>
		/// The IP or hostname of the machine to connect to.
		/// </summary>
		string NETWORK_HOST;

		/// <summary>
		/// The name of the server to connect to.
		/// </summary>
		string SERVER;

		/// <summary>
		/// The identity of the thing we're connecting to.  For components, the component ID.
		/// </summary>
		string TO;
		/// <summary>
		/// The identity that we expect on the X.509 certificate on the other side.
		/// </summary>
		string SERVER_ID;
		/// <summary>
		/// How often to do keep-alive spaces (seconds).
		/// </summary>
		float KEEP_ALIVE;
		/// <summary>
		/// Port number to connect to or listen on.
		/// </summary>
		int PORT;
		/// <summary>
		/// Do SSL on connection?
		/// </summary>
		bool SSL;
		/// <summary>
		/// Automatically negotiate TLS.
		/// </summary>
		bool AUTO_TLS;
		/// <summary>
		/// Automatically negotiate XEP-138 compression.
		/// </summary>
		bool AUTO_COMPRESS;
		/// <summary>
		/// Allow plaintext logins.
		/// </summary>
		bool PLAINTEXT;
		/// <summary>
		/// Do SASL connection?
		/// </summary>
		bool SASL;
	

		/// <summary>
		/// The user to log in as.
		/// </summary>
		string USER;
		/// <summary>
		/// The password for the user, or secret for the component.
		/// </summary>
		string PASSWORD;
		/// <summary>
		/// The resource to bind to.
		/// </summary>
		string RESOURCE;
		/// <summary>
		/// Default priority for presence.
		/// </summary>
		int PRIORITY;
		

		/// <summary>
		/// Automatically login.
		/// </summary>
		bool AUTO_LOGIN;
		/// <summary>
		/// Automatically retrieve the roster.
		/// </summary>
		bool AUTO_ROSTER;
		/// <summary>
		/// Automatically send back 501/feature-not-implemented to IQs that have not been handled.
		/// </summary>
		bool AUTO_IQ_ERRORS;
		/// <summary>
		/// Automatically send presence.
		/// </summary>
		bool AUTO_PRESENCE;
	};
}
