//-----------------------------------------------------------------------------
// ParaScriptingGlobalOSX.mm
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2019.8.12
// ModifyDate: 2022.3.17
//-----------------------------------------------------------------------------
#include <luabind/object.hpp>
#import <Cocoa/Cocoa.h>

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"


using namespace ParaScripting;

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State* L = GetLuaState();
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    NSOpenPanel *panel = [[NSOpenPanel alloc] init];
    
    const char *filter = object_cast<const char*>(inout["filter"]);
    
    if (filter == NULL) {
        filter = "image/*";
    }

    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:YES];
    [panel setCanChooseFiles:YES];

    if (strcmp(filter, "image/*") == 0) {
        [panel setAllowedFileTypes:[NSArray arrayWithObjects:@".jpg", @".png", @"jpeg", nil]];
    } else {
        [panel setAllowsOtherFileTypes:YES];
    }

    if ([panel runModal] == NSModalResponseOK) {
        NSString *path = [panel.URLs.firstObject path];
        inout["result"] = true;
        inout["filename"] = [path UTF8String];

        return true;
    }

    return false;
}
