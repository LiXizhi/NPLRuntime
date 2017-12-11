#pragma once

#ifdef PARAENGINE_SERVER
#include "ParaEngineServerApp.h"
#elif defined(WIN32) && !defined(PARAENGINE_MOBILE) && defined(USE_OPENGL_RENDERER)
#include "platform/win32/ParaEngineApp.h"
#else
#include "ParaEngineApp.h"
#endif

