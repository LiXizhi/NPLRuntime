//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.cpp
// Authors: big
// CreateDate: 2023.8.8
//-----------------------------------------------------------------------------

#include "ParaScriptingScreenRecorder.h"
#include "jni/ScreenRecorder.h"
#include <luabind/object.hpp>
#include <luabind/luabind.hpp>

namespace ParaScripting
{
    void ParaScriptingScreenRecorder::LuabindRegisterScreenRecorderGlobalFunctions(lua_State *L)
    {
        using namespace luabind;

        module(L)
        [
            namespace_("ScreenRecorder")
            [
                class_<ParaScripting::ParaScriptingScreenRecorder>("ParaScriptingScreenRecorder"),
                def("start", ParaScripting::ParaScriptingScreenRecorder::Start),
                def("stop", ParaScripting::ParaScriptingScreenRecorder::Stop),
                def("play", ParaScripting::ParaScriptingScreenRecorder::Play),
                def("save", ParaScripting::ParaScriptingScreenRecorder::Save),
                def("removeVideo", ParaScripting::ParaScriptingScreenRecorder::RemoveVideo)
            ]
        ];
    }

	void ParaScriptingScreenRecorder::Start()
	{
        ParaEngine::ScreenRecorder::start();
    }

    void ParaScriptingScreenRecorder::Stop()
    {
        ParaEngine::ScreenRecorder::stop();
    }

    std::string ParaScriptingScreenRecorder::Save()
    {
        return ParaEngine::ScreenRecorder::save();;
    }

    void ParaScriptingScreenRecorder::Play()
    {
        ParaEngine::ScreenRecorder::play();
    }

    void ParaScriptingScreenRecorder::RemoveVideo()
    {
        ParaEngine::ScreenRecorder::removeVideo();
    }
}
