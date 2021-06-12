#include <stdio.h>
#include "ExternalBrowser.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace ParaEngine {
    void ExternalBrowser::openExternalBrowser(const char* url)
    {
        printf(url);
        NSString *nsUrl = [[NSString alloc] initWithUTF8String:url];
        [[UIApplication sharedApplication] openURL: [NSURL URLWithString: nsUrl]];
    }
}
