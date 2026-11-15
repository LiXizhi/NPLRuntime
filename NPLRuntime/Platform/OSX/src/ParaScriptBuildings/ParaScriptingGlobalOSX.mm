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
    (void)L;
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    if (luabind::type(inout) != LUA_TTABLE)
        return false;

    const char* filter = "image/*";
    if (luabind::type(inout["filter"]) == LUA_TSTRING)
    {
        const char* f = luabind::object_cast<const char*>(inout["filter"]);
        if (f && f[0] != '\0')
            filter = f;
    }

    bool isSave = false;
    if (luabind::type(inout["save"]) == LUA_TBOOLEAN)
        isSave = luabind::object_cast<bool>(inout["save"]);
    else if (luabind::type(inout["issave"]) == LUA_TBOOLEAN)
        isSave = luabind::object_cast<bool>(inout["issave"]);

    NSString* nsTitle = nil;
    if (luabind::type(inout["title"]) == LUA_TSTRING)
    {
        const char* t = luabind::object_cast<const char*>(inout["title"]);
        if (t && t[0] != '\0')
            nsTitle = [NSString stringWithUTF8String:t];
    }

    NSURL* dirUrl = nil;
    if (luabind::type(inout["initialdir"]) == LUA_TSTRING)
    {
        const char* d = luabind::object_cast<const char*>(inout["initialdir"]);
        if (d && d[0] != '\0')
        {
            NSString* nsDir = [NSString stringWithUTF8String:d];
            if (nsDir)
                dirUrl = [NSURL fileURLWithPath:nsDir];
        }
    }

    if (isSave)
    {
        NSSavePanel* savePanel = [NSSavePanel savePanel];
        if (nsTitle)
            [savePanel setTitle:nsTitle];
        if (dirUrl)
            [savePanel setDirectoryURL:dirUrl];
        [savePanel setAllowsOtherFileTypes:YES];
        if ([savePanel runModal] == NSModalResponseOK && savePanel.URL != nil)
        {
            NSString* path = [savePanel.URL path];
            if (path)
            {
                inout["result"] = true;
                inout["filename"] = [path UTF8String];
                return true;
            }
        }
        return false;
    }

    NSOpenPanel* panel = [NSOpenPanel openPanel];
    if (nsTitle)
        [panel setTitle:nsTitle];
    if (dirUrl)
        [panel setDirectoryURL:dirUrl];

    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:YES];
    [panel setCanChooseFiles:YES];

    if (strcmp(filter, "image/*") == 0)
    {
        [panel setAllowedFileTypes:[NSArray arrayWithObjects:@"jpg", @"png", @"jpeg", @"bmp", @"gif", @"webp", nil]];
    }
    else
    {
        [panel setAllowsOtherFileTypes:YES];
    }

    if ([panel runModal] == NSModalResponseOK)
    {
        NSString* path = [panel.URLs.firstObject path];
        if (path)
        {
            inout["result"] = true;
            inout["filename"] = [path UTF8String];
            return true;
        }
    }

    return false;
}
