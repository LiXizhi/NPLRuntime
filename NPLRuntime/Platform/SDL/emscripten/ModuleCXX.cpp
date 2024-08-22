

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "Globals.h"

#include "3dengine/MoviePlatform.h"
#include "util/StringHelper.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#endif

static void TakeScreenShot(const std::string& filename, int width, int height)
{
    ParaEngine::CGlobals::GetMoviePlatform()->TakeScreenShot(filename, width, height);
}

static std::string ParaMisc_base64(const std::string source)
{
    return ParaEngine::StringHelper::base64(source);
}

static std::string ParaMisc_unbase64(const string source)
{
    return ParaEngine::StringHelper::unbase64(source);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(CXX_INTERFACE) {
    emscripten::function("TakeScreenShot", &TakeScreenShot);
    emscripten::function("ParaMisc_base64", &ParaMisc_base64);
    emscripten::function("ParaMisc_unbase64", &ParaMisc_unbase64);
}
#endif
