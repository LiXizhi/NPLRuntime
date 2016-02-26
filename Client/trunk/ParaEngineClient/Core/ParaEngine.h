#pragma once
#include "Platforms.h"

// config file for ParaEngine
#ifdef WIN32
#include "ParaEngineClientConfig.win32.h"
#else
#include "ParaEngineClientConfig.h"
#endif

#include "coreexport.h"

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
#include <boost/bind.hpp>
#include "util/intrusive_ptr.h"
namespace NPL
{
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
#include "Report.h"
#include "ParaGlobals.h"
#include "math/ParaMathUtility.h"
// give access to global objects and functions
#include "Globals.h"
#include "FileManager.h"


