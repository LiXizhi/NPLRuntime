//
//  KeyboardiOS.h
//  Paracraft
//
//  Created by big on 2021/6/26.
//

#ifndef KeyboardiOS_h
#define KeyboardiOS_h

#include <stdio.h>
#include "ParaEngine.h"
#include "Framework/InputSystem/VirtualKey.h"

namespace ParaEngine
{
    class KeyboardiOS
    {
        public:
            static string intToString(int i);
            static EVirtualKey toVirtualKey(const string key);
            static void OnChar(const string key);
            static void OnKeyDown(const string key);
            static void OnKeyUp();
            static bool IsKeyPress(const string key);

            static vector<string> allPressesKey;
    };
}

#endif /* KeyboardiOS_h */
