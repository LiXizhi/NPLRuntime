//-----------------------------------------------------------------------------
// ParaScriptingGlobalOSX.mm
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2019.8.12
// ModifyDate: 2022.3.17
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"
#include <luabind/object.hpp>
#import <Cocoa/Cocoa.h>

using namespace ParaScripting;

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State* L = GetLuaState();
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    NSOpenPanel *panel = [[NSOpenPanel alloc] init];

    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:YES];
    [panel setCanChooseFiles:YES];
    [panel setAllowsOtherFileTypes:YES];

    if ([panel runModal] == NSModalResponseOK) {
        NSString *path = [panel.URLs.firstObject path];
        inout["result"] = true;
        inout["filename"] = [path UTF8String];

        return true;
    }

    return false;
}
