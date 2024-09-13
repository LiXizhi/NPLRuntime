//-----------------------------------------------------------------------------
// Class: GUI mouse base macOS class
// Authors:	big
// Company:	Tatfook
// Date: 2024.9.13
// Desc:
//-----------------------------------------------------------------------------
#import <Cocoa/Cocoa.h>

#include "ParaEngine.h"
#include "2dengine/GUIMouseVirtual.h"

using namespace ParaEngine;

static bool bMacOSShowCursor = true;

void CGUIMouseVirtual::ShowCursor(bool bShowCursor)
{
    if (bShowCursor && !bMacOSShowCursor) {
        [NSCursor unhide];
        bMacOSShowCursor = true;
    } else if (!bShowCursor && bMacOSShowCursor) {
        [NSCursor hide];
        bMacOSShowCursor = false;
    }
}
