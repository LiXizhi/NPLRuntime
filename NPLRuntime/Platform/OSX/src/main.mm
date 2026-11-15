#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"
#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"
#include "WindowDelegate.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        std::string sCmdLine;
        for (int i = 1; i < argc; ++i)
        {
            if (!argv[i] || argv[i][0] == '\0')
                continue;
            // Skip macOS LaunchServices / Xcode internal flags (-psn_*, -NSDocumentRevisionsDebugMode)
            if (strncmp(argv[i], "-psn_", 5) == 0)
                continue;
            if (strcmp(argv[i], "-NSDocumentRevisionsDebugMode") == 0)
            {
                ++i;
                continue;
            }
            std::string token = argv[i];
            if (token.find(' ') != std::string::npos && token.find('"') == std::string::npos)
            {
                auto eqPos = token.find('=');
                if (eqPos != std::string::npos && token.find("://") == std::string::npos)
                    token = token.substr(0, eqPos + 1) + "\"" + token.substr(eqPos + 1) + "\"";
                else
                    token = "\"" + token + "\"";
            }
            if (!sCmdLine.empty())
                sCmdLine += " ";
            sCmdLine += token;
        }

        [NSApplication sharedApplication];
        WindowDelegate* winDelegate = [WindowDelegate sharedDelegate];
        if (!sCmdLine.empty())
            [winDelegate setInitialCommandLine:sCmdLine];
        [NSApp setDelegate:winDelegate];
        [NSApp run];
        [WindowDelegate closeDelegate];
        
        return 0;
    }
}
