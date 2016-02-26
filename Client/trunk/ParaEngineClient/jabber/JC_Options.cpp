//-----------------------------------------------------------------------------
// Class:	JC options
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.11.28
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "JC_Options.h"

using namespace ParaEngine;

JCOptions::JCOptions()
{
	NAMESPACE = "namespace";

	NETWORK_HOST = "network_host";
	TO = "to";
	SERVER    = "paraengine.com";
	SERVER_ID    = "paraengine.com";
	KEEP_ALIVE   = 20.f;
	PORT         = -1; // default
	SSL          = true;
	AUTO_TLS     = true;
	AUTO_COMPRESS = true;
	PLAINTEXT    = true;
	AUTO_TLS = true;
	AUTO_COMPRESS = true;
	SASL = true;

	USER     = "lxz@paraengine.com/npl";
	PASSWORD = "password";
	RESOURCE = "resource";
	PRIORITY = 0;

	AUTO_LOGIN    = true;
	AUTO_ROSTER   = true;
	AUTO_IQ_ERRORS   = true;
	AUTO_PRESENCE = true;
}