//-----------------------------------------------------------------------------
// Class: For ParaGlobal namespace.
// Authors: Big
// Emails: onedous@gmail.com
// Date: 2019.8.12
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include <luabind/object.hpp>
#import <Cocoa/Cocoa.h>

using namespace ParaScripting;

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

