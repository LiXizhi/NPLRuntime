#pragma once

#define CROSS_PLATFORM


// config file for ParaEngine
#ifdef WIN32
#include "ParaEngineClientConfig.win32.h"
#else
#include "ParaEngineClientConfig.h"
#endif
#include "coreexport.h"

#include "Platforms.h"

#ifndef DROP_ASSET
/** @def drop an asset object, decrease its reference count, and set the pointer to NULL.*/
#define DROP_ASSET(x)    if(x){x->Release();x=0;}
#endif
#ifndef GRAB_ASSET
/** @def increase the reference count of an asset object. */
#define GRAB_ASSET(x)    if((x)!=0){(x)->addref();}
#endif



#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#ifdef EMSCRIPTEN
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#else
#include <boost/bind.hpp>
#endif
#include "util/intrusive_ptr.h"

using namespace boost::placeholders;

namespace NPL
{
	typedef boost::shared_ptr<CNPLUDPRoute> NPLUDPRoute_ptr;
	typedef boost::shared_ptr<CNPLConnection> NPLConnection_ptr;
	typedef boost::shared_ptr<CNPLRuntimeState> NPLRuntimeState_ptr;
	typedef boost::shared_ptr<NPLServerInfo> NPLServerInfo_ptr;

	// Note: using intrusive_ptr with a mutex is about 10% faster than using shared_ptr
	// typedef boost::shared_ptr<NPLMessage> NPLMessage_ptr;
	typedef ParaIntrusivePtr<NPLMessage> NPLMessage_ptr;

	typedef ParaIntrusivePtr<NPLMsgOut> NPLMsgOut_ptr;

	typedef boost::shared_ptr<NPLRuntimeAddress> NPLRuntimeAddress_ptr;
	typedef boost::shared_ptr<NPLTimer> NPLTimer_ptr;
}

#include "util/Log.h"
#include "debugtools/paradebug.h"
#include "PERef.h"
#include "util/Report.h"
#include "math/ParaGlobals.h"
#include "math/ParaMathUtility.h"
#include "IO/FileManager.h"
// give access to global objects and functions
#include "Core/Globals.h"




